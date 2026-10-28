# jsrf-heap

## Why
After New Game the title shows "Now Loading", then "There's a problem with
the disc". `xbox_HeapAlloc` cannot find the 676,864 bytes for the
`Media\Misc\Misc.dat` buffer. The failure line says 50.5 of 50.9 MB used,
and the title retries about 4,000 times.

The heap is not full of live data. The toolkit's default reuse hands a whole
freed block to the next request that fits, however small. At the failure,
792 live blocks hold 25.1 MB of requests plus 22.4 MB of slack inside reused
blocks. That leaves 45 KB in free blocks and 387 KB above the cursor.

## What changes
- Toolkit:
  - on first exhaustion, print the heap's real state: live bytes, slack in
    reused blocks, free blocks, the room above the cursor, the largest free
    run, and the biggest live block sizes with what was asked of them;
  - `RECOMP_HEAP_RECLAIM` treats `0` or empty as off, so a game default can
    be A/B'd;
  - a memory_regressions ctest mode (`reclaim-zero`) and a check for the
    waste pattern;
  - guest page protection handled at guest-page grain on 16 KB host pages,
    and undone before NtFree zeroes or releases pages. This was the next
    blocker once the heap had room: a SIGBUS loading Stg00 (design.md).
  The toolkit's default stays off: turning reclaim on moves every later heap
  address in BLiNX 2 and Burnout 3.
- Game: `main.c` defaults `RECOMP_HEAP_RECLAIM` to 1 before the memory
  layout comes up, the same way it defaults `RECOMP_VBLANK` and
  `RECOMP_TITLE_KEVENTS`.

## Out of scope
- Making reclaim the toolkit default (a follow-up, with BLiNX 2 and B3
  goldens).
- Proton, Deck and Windows runs.
- The cat `docs/env.md` row and the `game.toml` toolkit pin bump, which are
  owed at merge.
