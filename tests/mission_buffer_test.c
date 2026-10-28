/*
 * mission_buffer_test.c - fixes/mission_buffer.c against a stub VirtualFree
 * over a small guest buffer. The skip only runs when the type-2 release
 * races the mission load (Corn's route), so this is where it is
 * exercised: a free of the manager's +0x268 slot from one of its three
 * sites is skipped, with TRUE and a stdcall return, only while a type-0
 * entry points into the block; every other free reaches the generated body.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "gen/recomp_types.h"

RECOMP_TLS uint32_t g_eax, g_ecx, g_edx, g_esp;
RECOMP_TLS uint32_t g_ebx, g_esi, g_edi;
ptrdiff_t g_xbox_mem_offset;

extern int jsrf_fix_mission_buffer;
void jsrf_mission_buffer_free(void);

#define GUEST_SIZE 0x240000u
#define ROOT       0x800u
#define MGR        0x1000u
#define ENTRY_A    0x2000u
#define ENTRY_B    0x2100u
#define BLOCK      0x10000u
#define BLOCK_SIZE 0x23000u
#define STACK_TOP  0xF000u

static int gen_calls;

uint32_t xbox_HeapBlockSize(uint32_t va)
{
    return va == BLOCK ? BLOCK_SIZE : 0;
}

/* The generated VirtualFree: TRUE, and its ret 12. */
void sub_00145A5D_gen(void)
{
    gen_calls++;
    g_eax = 1;
    g_esp += 16;
}

static int fails;
#define CHECK(c, what) do { if (!(c)) { printf("FAIL %s: %s\n", name, what); fails++; } } while (0)

/* LIST_SLOT2: the AB list, with +0x268 holding another block. */
enum list { LIST_AB, LIST_NULL, LIST_LOOP, LIST_SLOT2 };

static void run(const char *name, int key, uint32_t ret, uint32_t va,
                uint32_t data_b, enum list list, int want_gen)
{
    jsrf_fix_mission_buffer = key;
    MEM32(0x0022FCE0u) = ROOT;
    MEM32(ROOT + 0x98 + 7 * 4) = MGR;
    MEM32(MGR + 0x268) = list == LIST_SLOT2 ? 0x30000u : va;
    MEM32(MGR + 0x198) = list == LIST_NULL ? 0 : ENTRY_A;
    MEM32(ENTRY_A + 4) = 0x38000u;   /* data outside the block */
    /* A loop never reaches B: the walk stops at its cap. */
    MEM32(ENTRY_A + 0xC) = list == LIST_LOOP ? ENTRY_A : ENTRY_B;
    MEM32(ENTRY_B + 4) = data_b;
    MEM32(ENTRY_B + 0xC) = 0;
    g_ebx = MGR;
    g_eax = 0;
    gen_calls = 0;
    g_esp = STACK_TOP;
    PUSH32(g_esp, 0x8000u);          /* MEM_RELEASE */
    PUSH32(g_esp, 0);
    PUSH32(g_esp, va);
    PUSH32(g_esp, ret);

    jsrf_mission_buffer_free();

    CHECK(gen_calls == want_gen, want_gen ? "free must run" : "free must be skipped");
    CHECK(g_esp == STACK_TOP, "esp not back past the three arguments");
    CHECK(g_eax == 1, "eax is not TRUE");
    CHECK(g_ebx == MGR, "ebx changed");
}

int main(void)
{
    /* The manager root sits at 0x22FCE0, so the guest covers it. */
    void *guest = calloc(1, GUEST_SIZE);
    if (!guest) return 2;
    g_xbox_mem_offset = (ptrdiff_t)(uintptr_t)guest;

    /*   name                        key ret          va        entry B data        list       gen */
    run("record at the block start", 1,  0x00033596u, BLOCK,    BLOCK,              LIST_AB,   0);
    run("record inside the block",   1,  0x00033596u, BLOCK,    BLOCK + 0x1000u,    LIST_AB,   0);
    run("type-1 release",            1,  0x00032A30u, BLOCK,    BLOCK + 0x1000u,    LIST_AB,   0);
    run("type-2 step",               1,  0x00032F33u, BLOCK,    BLOCK + 0x1000u,    LIST_AB,   0);
    run("no entry in the block",     1,  0x00033596u, BLOCK,    BLOCK + BLOCK_SIZE, LIST_AB,   1);
    run("data below the block",      1,  0x00033596u, BLOCK,    BLOCK - 4u,         LIST_AB,   1);
    run("null list head",            1,  0x00033596u, BLOCK,    BLOCK,              LIST_NULL, 1);
    run("list past the cap",         1,  0x00033596u, BLOCK,    BLOCK,              LIST_LOOP, 1);
    run("another call site",         1,  0x00031000u, BLOCK,    BLOCK + 0x1000u,    LIST_AB,   1);
    run("not the slot",              1,  0x00033596u, BLOCK,    BLOCK + 0x1000u,    LIST_SLOT2, 1);
    run("unknown block",             1,  0x00033596u, 0x20000u, 0x20000u,           LIST_AB,   1);
    run("key off",                   0,  0x00033596u, BLOCK,    BLOCK + 0x1000u,    LIST_AB,   1);
    free(guest);
    printf("%s\n", fails ? "mission_buffer: FAIL" : "mission_buffer: ok");
    return fails ? 1 : 0;
}
