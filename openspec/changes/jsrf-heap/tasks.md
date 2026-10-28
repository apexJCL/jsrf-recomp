# Tasks

- [x] 1.1 Measure the heap at the failure: a live and slack report in the
      toolkit (run base, off)
- [x] 1.2 Rule out leaks: frees honoured, contiguous arena separate,
      the cursor is not live bytes
- [x] 1.3 `RECOMP_HEAP_RECLAIM=0` means off; ctest mode reclaim-zero plus
      a waste-pattern check
- [x] 1.4 skates defaults reclaim on; a silent Metal run past Now Loading
      into the Corn talk scene (run fix)
- [x] 1.4a Stage load SIGBUS: guest-grain page protection on 16 KB host
      pages, unprotect before NtFree zeroes or releases pages, ctest; the
      GG's Garage stage on Metal (run fix-prot)
- [x] 1.5 Gates: Mac build, posix ctests, pytest scripts, ruff
- [x] 1.4b Review fixes: per-guest-page protection table (most permissive
      guest page wins), base protection only, unprotect in the heap and the
      contiguous arena, exhaustion report's largest run, a fault check in
      the ctest, docs (toolkit 43d4915a)
- [x] 1.5a Proton run (the toolkit change touches the kernel bridge) and the
      Windows build. The first Deck run crashed loading pv_yoyo.dat: a page
      allocation's last page was shared with the next heap block and went
      read-only. Page requests now own whole pages (toolkit 57f8947a), and
      the rerun reaches GG's Garage under Proton
- [x] 1.5b Review fixes: page rounding in the reserve clamp fallback, a
      heap-level page check in memory_regressions (toolkit 795f90e6)
- [ ] 1.6 Follow-up: decide on reclaim as the toolkit default, with BLiNX 2
      and B3 goldens
- [ ] 1.7 Upstream PR round: offer the two deviations in design.md
      ("0 = off", guest-grain protection)
- [ ] 1.8 Follow-ups from the page-rounding review (gaps that predate it):
      - Older NT gaps: the MEM_COMMIT-only path returns success without
        writing back a rounded base and size, and a base hint inside RAM is
        ignored, so NT's base round-down is never applied.
      - Older overflow in heap_alloc_locked: with a size above 0xFFFFF000,
        `result + size` can wrap past XBOX_HEAP_TOP, and with reclaim on
        `(size+15)&~15` wraps to 0. Test `size > XBOX_HEAP_TOP - result`
        instead.
      - src/kernel/README.md still says "Heap (bump allocator, no free)";
        refresh it when the work goes upstream.
