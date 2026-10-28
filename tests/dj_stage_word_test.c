/*
 * dj_stage_word_test.c - fixes/dj_stage_word.c against stub callees over a
 * small guest buffer. The [FIX] path only runs in a rare hang layout, so
 * this is the run that shows the replay keeps stock's stack and registers:
 * the callee sees (3, word) under the right return address, esp and esi come
 * back as on entry (+4 for the ret), +0x5C is set only on a nonzero return,
 * and a legitimate stage word is never replaced.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "gen/recomp_types.h"

RECOMP_TLS uint32_t g_eax, g_ecx, g_edx, g_esp;
RECOMP_TLS uint32_t g_ebx, g_esi, g_edi;
ptrdiff_t g_xbox_mem_offset;

extern int jsrf_fix_dj_stage_word;
void jsrf_dj_stage_switch(void);

#define GUEST_SIZE 0x10000u
#define THIS       0x1000u
#define REC        0x3000u
#define STACK_TOP  0xF000u
#define CALLER_RET 0x00DEAD00u

static uint32_t getter_ret, switch_ret;
static int gen_calls, getter_calls, switch_calls;
static uint32_t switch_type, switch_word, bad_ret;

/* Each stub checks its return address and pops it, as the callee's ret. */
void sub_00025680(void)
{
    getter_calls++;
    if (MEM32(g_esp) != 0x0004AC88u) bad_ret++;
    g_eax = getter_ret;
    g_esp += 4;
}

void sub_000256A0(void)
{
    switch_calls++;
    if (MEM32(g_esp) != 0x0004AC9Cu) bad_ret++;
    switch_type = MEM32(g_esp + 4);
    switch_word = MEM32(g_esp + 8);
    g_eax = switch_ret;
    g_esp += 4;
}

void sub_0004AC80_gen(void)
{
    gen_calls++;
    g_esp += 4;
}

static int fails;
#define CHECK(c, what) do { if (!(c)) { printf("FAIL %s: %s\n", name, what); fails++; } } while (0)

static void run(const char *name, int key, uint32_t getter, uint32_t sw,
                uint32_t live, uint32_t cached, uint32_t want_word, int want_5c)
{
    jsrf_fix_dj_stage_word = key;
    getter_ret = getter; switch_ret = sw;
    gen_calls = getter_calls = switch_calls = 0;
    switch_type = switch_word = bad_ret = 0;
    MEM32(THIS + 0x1040) = REC;
    MEM32(THIS + 0x104C) = cached;
    MEM32(THIS + 0x5C) = 0;
    MEM32(REC) = live;
    g_esp = STACK_TOP;
    PUSH32(g_esp, CALLER_RET);
    g_ecx = THIS; g_esi = 0x5E5E5E5Eu;

    jsrf_dj_stage_switch();

    CHECK(g_esp == STACK_TOP, "esp not back to entry + 4");
    CHECK(g_esi == 0x5E5E5E5Eu, "esi not restored");
    CHECK(bad_ret == 0, "wrong return address under a callee");
    CHECK(MEM32(STACK_TOP - 4) == CALLER_RET, "caller's return slot overwritten");
    CHECK((MEM32(THIS + 0x5C) == 0xA) == want_5c, "+0x5C");
    if (!key) {
        CHECK(gen_calls == 1 && getter_calls == 0, "off must run the generated body");
        return;
    }
    CHECK(gen_calls == 0 && getter_calls == 1, "on must replay");
    if (getter) {
        CHECK(switch_calls == 0, "getter nonzero must skip the switch");
        return;
    }
    CHECK(switch_calls == 1, "switch not called once");
    CHECK(switch_type == 3, "scene type");
    CHECK(switch_word == want_word, "stage word");
    CHECK(g_ecx == want_word, "ecx is the word, as stock leaves it");
}

int main(void)
{
    void *guest = calloc(1, GUEST_SIZE);
    if (!guest) return 2;
    g_xbox_mem_offset = (ptrdiff_t)(uintptr_t)guest;

    /*   name                    key getter sw  live         cached  word  5c */
    run("hang word, cached 8",   1,  0,     1,  0x1889B927u, 8,      8,          1);
    run("hang word, switch 0",   1,  0,     0,  0x1889B927u, 8,      8,          0);
    run("intact record",         1,  0,     1,  8,           8,      8,          1);
    run("re-pointed record",     1,  0,     1,  0x00040002u, 8,      0x00040002u, 1);
    run("both words bad",        1,  0,     1,  0x1889B927u, 0xFFFFFFFFu, 0x1889B927u, 1);
    run("getter nonzero",        1,  1,     1,  0x1889B927u, 8,      0,          0);
    run("key off",               0,  0,     1,  0x1889B927u, 8,      0,          0);

    free(guest);
    printf("%s\n", fails ? "dj_stage_word: FAIL" : "dj_stage_word: ok");
    return fails ? 1 : 0;
}
