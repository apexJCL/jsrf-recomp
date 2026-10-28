# Design

## Measurements
All runs are silent Metal on the Mac. They start from the jsrf-showcase hdd
and use scripted Start taps (`runs/jsrf-heap/`).

| run     | reclaim         | result |
|---------|-----------------|--------|
| base    | off (old build) | OOM on Misc.dat, 110 lines |
| reclaim | env =1          | no OOM; the Corn talk scene after New Game |
| fix     | game default    | no OOM; the Corn talk scene ("Hey, where's our rhythm?") |
| off     | env =0          | OOM on Misc.dat, 4,356 lines, new report (below) |

The report from `off`:

    [HEAP] exhausted: 792 live blocks 25104 KB (+22955 KB slack in reused
    blocks), 363 free blocks 45 KB, 387 KB above the cursor; largest free
    run 387 KB; reclaim off
    [HEAP]   1 live blocks of 3099648 bytes (3027 KB) holding 0 KB of requests
    ...

Several blocks of 1 to 3 MB each hold a request under 1 KB.

## Memory budget against retail
A retail Xbox has 64 MB of unified RAM. The toolkit's JSRF map:

- image up to 0x288620;
- the toolkit region at 0x700000;
- an 8 MB main stack at 0x780000;
- a 48.5 MB heap at 0x00F80000–0x04000000.

The contiguous window is a separate 64 MB arena at 0x80000000, not an alias
of the heap. At the failure it held 11.2 MB, with a peak of 34.7 MB. So the
title has more total memory here than on retail. The shortage is
fragmentation from the reuse policy, not the budget.

## Leaks checked
- Frees are honoured: more than 1,024 frees are matched to their blocks.
- With reclaim on, NtFreeVirtualMemory release and decommit go through the
  heap.
- MmFreeContiguousMemory frees through the first-fit arena.
- The `used` figure in the old OOM line is the bump cursor, not live bytes.

## Comparison with BLiNX 2
BLiNX 2 has a 71 MB heap (128 MB map), its cursor sits around 15 MB early,
and it has never hit an OOM.

## Next blocker: page protection (SIGBUS loading the stage)
With reclaim on, New Game loads `Stg00_0*.dat`. It then faulted in
`memset` inside `xbox_HeapAlloc` for an NtAllocateVirtualMemory, writing
guest 0x02219000.

Logging the protect calls showed the cause. The title calls
NtProtectVirtualMemory(0x02218000, 480, PAGE_READONLY), and it toggles a
288,000-byte table between read-only and read/write about 740 times. The Mac
host page is 16 KB, so the read-only call covered 0x02218000–0x0221C000,
which includes the next heap block.

Separately, a protection outlived its memory on every host. A read-only
block that was decommitted or released faulted when it was zeroed.

The toolkit now (after the review, toolkit 43d4915a):
- keeps each 4 KB guest page's protection in a table and gives each host
  page the most permissive protection of the guest pages in it, so a
  protected page never takes its neighbours with it, and an edge page is
  never left more restrictive than asked (no access, then read-only on part
  of it);
- passes only the base protection to the host: no cache flags (so
  `READWRITE|WRITECOMBINE` succeeds on Windows too), no execute, and
  write-copy as read/write, since copy-on-write on the shared RAM view would
  split it from its mirrors;
- makes a block's pages read/write again when the heap or the contiguous
  arena frees it (which covers NtFree release, MmFreeSystemMemory,
  ExFreePool, MmFreeContiguousMemory, MmDbgFreeMemory), and when NtFree
  decommits pages;
- does nothing of this until a title first takes access away.

On a 4 KB host each host page is one guest page, so the protection applied
is what was asked. The behaviour changes on Windows too (cache flags masked,
unprotect on free), so the Proton run is a merge gate. The
kernel_memory_bridge_posix ctest bus-errors without the change, and checks
that a whole read-only host page does fault on write.

## Upstream deviations (candidates for the upstream PR round)
- `RECOMP_HEAP_RECLAIM=0` means off. Upstream treats any set value as on;
  a game that defaults the switch on needs 0 to turn it back off.
- Guest page protection at the guest's 4 KB grain on hosts with larger
  pages, and undone when memory is freed. Upstream protects whole host pages
  and never undoes a protection.

Run `fix-prot` (160 s): there is no OOM and no crash. Stage 0 renders in 3D
in the GG's Garage, with Corn saying "This is the GG's Garage. Hey, where's
our pizza?", and waits for A.

## Why a game default, not game.toml
`game.toml` configures the CLI pipeline. Runtime switches are set in the
game's `main.c` (VBLANK, TITLE_KEVENTS), or in `src/env/recomp_env_game.h`
for games that have one. skates uses the `main.c` pattern. The switch is read
once at the first heap allocation, so it is set before
`xbox_MemoryLayoutInit`.
