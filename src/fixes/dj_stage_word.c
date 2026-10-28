/*
 * dj_stage_word.c - fix.dj_stage_word: the stage after the DJ event.
 *
 * sub_0004AC80 (a method, this in ecx) hands the stage scene its stage word
 * at the switch from an event scene: it reads the word live through the
 * record pointer at this+0x1040, although scene setup (sub_000517AC) cached
 * the same word at this+0x104C when it set the pointer. By the switch after
 * the DJ event the record has been freed. On the Xbox the block still holds
 * the word; here the allocator can hand the range to the SE loader first,
 * and in the hang layout the word read is se_pple.dat sample data
 * (0x1889B927 for 8), so the game loads stage 19 and stalls.
 *
 * With the key on (the default) this runs the function's instructions in
 * stock order, and only where the live word is not a stage word and the
 * cached one is does it pass the cached word on. A legitimate stage word is
 * never replaced, so a record that is intact, or that some path re-pointed
 * on purpose, behaves exactly as stock. Off, the generated body runs.
 * Nothing is written to the guest beyond what the function itself writes.
 * recomp_manual.c routes sub_0004AC80 here (the recompiler's exclude_manual
 * list is that file); main.c sets the key from fix.dj_stage_word
 * (RECOMP_FIX_DJ_STAGE_WORD); tests/dj_stage_word_test.c checks the replay
 * against stub callees.
 */

#include <stdio.h>
#include <stdint.h>
#include "gen/recomp_types.h"

int jsrf_fix_dj_stage_word = 1;

extern void sub_0004AC80_gen(void);
extern void sub_00025680(void);
extern void sub_000256A0(void);

/* sub_000256A0's scene type for a stage (sub_00025390 hands type 3 to
 * sub_00036050, which stores the word as the next stage). */
#define JSRF_SCENE_STAGE 3u

/* A stage word is two halves of one decimal digit each: the path builder
 * (sub_00036210) formats Custom\Stg%d%d from (word >> 16) % 10 and
 * (word & 0xFFFF) % 10, and the disc's stages run Stg08 to Stg49. The hang
 * word 0x1889B927 has halves 6281 and 47399, so it fails this. */
static int jsrf_stage_word_valid(uint32_t w)
{
    return (w >> 16) <= 9 && (w & 0xFFFFu) <= 9;
}

void jsrf_dj_stage_switch(void)
{
    uint32_t word, cached;

    if (!jsrf_fix_dj_stage_word) {
        RECOMP_ABI_CALL(0x0004AC80u, sub_0004AC80_gen);
        return;
    }

    /* 0x4AC80-0x4ACAF. The record and its word are read after the call, as
     * stock reads them: the SE loader that reuses the block may run on
     * another thread, so an earlier sample could miss a late overwrite. */
    PUSH32(g_esp, g_esi);
    g_esi = g_ecx;
    PUSH32(g_esp, 0x0004AC88u);
    RECOMP_ABI_CALL(0x00025680u, sub_00025680);
    if (g_eax == 0) {
        g_eax = MEM32(g_esi + 0x1040);
        word = MEM32(g_eax);
        if (!jsrf_stage_word_valid(word)) {
            cached = MEM32(g_esi + 0x104C);
            if (jsrf_stage_word_valid(cached)) {
                /* Once per record and word: the switch runs once per scene
                 * change, so a repeat is the same stale record seen again. */
                static uint32_t said_rec, said_word;
                if (g_eax != said_rec || word != said_word) {
                    said_rec = g_eax;
                    said_word = word;
                    fprintf(stderr, "[FIX] dj_stage_word: record 0x%08X holds "
                                    "%08X, setup cached %08X: using the "
                                    "cached word\n", g_eax, word, cached);
                    fflush(stderr);
                }
                word = cached;
            }
        }
        g_ecx = word;
        PUSH32(g_esp, g_ecx);
        PUSH32(g_esp, JSRF_SCENE_STAGE);
        PUSH32(g_esp, 0x0004AC9Cu);
        RECOMP_ABI_CALL(0x000256A0u, sub_000256A0);
        g_esp += 8;
        if (g_eax != 0)
            MEM32(g_esi + 0x5C) = 0xA;
    }
    POP32(g_esp, g_esi);
    g_esp += 4;   /* ret */
}
