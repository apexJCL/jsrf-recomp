# Tasks

- [x] 1.1 Find the wait: loader -> sound task -> ADXT 0 -> CRI file object
- [x] 1.2 vblank lost wakeup: release-on-set for title-built KEVENTs, ctest
- [x] 1.3 Deferred file I/O APCs as the toolkit default
      (RECOMP_FILE_APC_DEFER=0 = inline), ctest
- [x] 1.4 APC stack cleanup: restore g_esp after the routine
- [x] 1.5 Silent Metal run past Now Loading: title screen "PLEASE PRESS START
      TO BEGIN" (run r13-espfix, 56.3 flips/s)
- [x] 1.6 Gates: Mac build, posix ctests, pytest scripts, ruff
- [x] 1.6a review fixes: multi-wait set generation, signalled object
      before APC, guest CPU around the APC, sequence-bounded drain, ctests;
      Windows cross-build of the kernel and kernel_user_apc; title screen
      again on Metal (run r15-review)
- [ ] 1.7 BLiNX 2 and Burnout 3 goldens (Metal, D3D11) before merge
      (orchestrator)
- [ ] 1.8 Owed at merge: game.toml toolkit pin (public fork sha, known
      only after the squash and public round); cat docs/env.md row is on
      cat docs/file-apc-defer
