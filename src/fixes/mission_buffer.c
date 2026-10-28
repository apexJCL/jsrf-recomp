/*
 * mission_buffer.c - fix.mission_buffer: keep the mission buffer that the
 * stage scene still reads.
 *
 * The loader manager (sub_000128C0(7)) loads a mission's .bin into the
 * in-flight slot at +0x268, hands it to the type-0 list at +0x198 (the
 * stage scene's record at this+0x1040 comes from that list, through
 * sub_00025700(0, ...) in sub_00051780), and only later reuses the slot for
 * the .dat. The per-type releases free whatever +0x268 holds. When the
 * type-2 one (sub_000332F0, run by the scene's state 7) runs inside that
 * window, it frees the .bin that the type-0 list owns, and the stage scene
 * goes on reading it: the stage word in states 9 to 0xC, and the whole
 * record during stage play (sub_00051780 sizes allocations from
 * record+0x38). With Corn's route the release lands right after the .dat
 * opens; with Gum's it does not, and the mission buffer lives through the
 * stage. Once the block is reused the scene hangs on the loading screen
 * (state 0xA, sub_00052460, waits in sub_000256C0(3, ...) for a word nobody
 * loaded) or crashes in sub_00051780 (the fix.dj_stage_word crash was the
 * same free, seen through the stage word).
 *
 * sub_00145A5D is the title's VirtualFree; recomp_manual.c routes it here.
 * With the key on, a free of the +0x268 slot from one of the three sites
 * that free it (the type-1 and type-2 releases and the type-2 step) is
 * skipped when a type-0 entry still points into the block, so the buffer
 * stays with the list as on the routes without the race; the caller then
 * clears or reuses +0x268 as stock does. The list frees the block itself
 * later (sub_000307A0 frees each entry's data), so stock frees it twice on
 * Corn's route and this frees it once. Every other free runs the generated
 * body.
 *
 * The skip cannot catch a release that lands before the handoff, while the
 * .bin is still only in +0x268: no entry points into it yet, so the free
 * goes through and the list later takes a freed pointer. A release (not the
 * step, which frees its own finished load there every time) that frees the
 * slot with no owner logs one diagnostic line per site, with the loader
 * state, so a recurrence (slower I/O under Proton, say) shows in the log.
 */

#include <stdio.h>
#include <stdint.h>
#include "gen/recomp_types.h"

/* On by default: a fix for a bug the Xbox's I/O timing hides; main.c sets
 * it from fix.mission_buffer (RECOMP_FIX_MISSION_BUFFER). */
int jsrf_fix_mission_buffer = 1;

extern void sub_00145A5D_gen(void);
extern uint32_t xbox_HeapBlockSize(uint32_t xbox_va);

/* The loader manager, as sub_00025680 fetches it: sub_000128C0(7) on the
 * object at [0x22FCE0] reads slot 7 of its table at +0x98. */
#define JSRF_MGR_ROOT   0x0022FCE0u
#define JSRF_MGR_SLOT   (0x98u + 7u * 4u)
/* Manager offsets: the in-flight slot, the loader state, and the type-0
 * list head. A list entry is {word, data@+4, pprev@+8, next@+0xC}: +8
 * points back at whatever links to the entry, which sub_000307A0 writes
 * through when it unlinks one. */
#define JSRF_SLOT       0x268u
#define JSRF_STATE      0x25Cu
#define JSRF_TYPE0_LIST 0x198u
/* A list longer than this is not the type-0 list: leave the free alone. */
#define JSRF_TYPE0_MAX  4096

/* The return addresses of the three VirtualFree calls on +0x268: in the
 * type-1 release (sub_00032790), the type-2 step (sub_00032C70) and the
 * type-2 release (sub_000332F0). */
static const uint32_t slot_free_ret[] = {
    0x00032A30u, 0x00032F33u, 0x00033596u
};
#define JSRF_SITES (sizeof slot_free_ret / sizeof slot_free_ret[0])

/* The first type-0 entry's data pointer inside [va, va + size), or 0. */
static uint32_t jsrf_type0_ref_in(uint32_t mgr, uint32_t va, uint32_t size)
{
    uint32_t e = MEM32(mgr + JSRF_TYPE0_LIST);
    for (int n = 0; e != 0 && n < JSRF_TYPE0_MAX; n++) {
        uint32_t data = MEM32(e + 4);
        if (data >= va && data - va < size)
            return data;
        e = MEM32(e + 0xC);
    }
    return 0;
}

void jsrf_mission_buffer_free(void)
{
    /* stdcall VirtualFree(addr, size, type): [esp] is the return address. */
    uint32_t ret = MEM32(g_esp);
    size_t site = 0;
    while (site < JSRF_SITES && slot_free_ret[site] != ret)
        site++;
    if (jsrf_fix_mission_buffer && site < JSRF_SITES) {
        uint32_t va = MEM32(g_esp + 4);
        uint32_t root = MEM32(JSRF_MGR_ROOT);
        uint32_t mgr = root ? MEM32(root + JSRF_MGR_SLOT) : 0;
        /* The callers keep the manager in different registers (ebp is a
         * local of the lifted sub_00032790), so take it from its root and
         * make sure the address is its slot. */
        uint32_t size = (va && mgr && MEM32(mgr + JSRF_SLOT) == va)
                        ? xbox_HeapBlockSize(va) : 0;
        uint32_t ref = size ? jsrf_type0_ref_in(mgr, va, size) : 0;
        if (ref) {
            /* Per block, not once: a later visit at a new address logs
             * again. Only logging reads it, so the race is harmless. */
            static uint32_t said_va;
            if (va != said_va) {
                said_va = va;
                fprintf(stderr, "[FIX] mission_buffer: release at 0x%05X "
                                "would free 0x%08X (%u bytes), which mission "
                                "data 0x%08X still uses: kept\n",
                        ret, va, size, ref);
                fflush(stderr);
            }
            g_eax = 1;      /* TRUE, as a successful free returns */
            g_esp += 16;    /* ret 12 */
            return;
        }
        /* The step frees its own finished load at 0x32F33 every time
         * (the Corn and Gum routes both), so only the releases log. */
        if (size && ret != 0x00032F33u) {
            static int said_site[JSRF_SITES];
            if (!said_site[site]) {
                said_site[site] = 1;
                fprintf(stderr, "[FIX] mission_buffer: release at 0x%05X "
                                "frees slot 0x%08X (%u bytes, loader state "
                                "%u) before any type-0 entry uses it: freed "
                                "(once per site)\n",
                        ret, va, size, MEM32(mgr + JSRF_STATE));
                fflush(stderr);
            }
        }
    }
    RECOMP_ABI_CALL(0x00145A5Du, sub_00145A5D_gen);
}
