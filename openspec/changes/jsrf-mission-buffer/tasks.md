# Tasks

- [x] 1.1 `src/fixes/mission_buffer.c` and its route in `recomp_manual.c`
- [x] 1.2 `tests/mission_buffer_test.c`, ctest `mission_buffer`
- [x] 1.3 Docs for `fix.mission_buffer`, its own key (review MEDIUM 3)
- [x] 1.4 All three `+0x268` free sites, the no-owner diagnostic, more test
      cases (review MEDIUM 1 and 2, NITs)
- [x] 2.1 Corn replay (`runs/jsrf-hang/corn-user.pad`) loads Dogenzaka
- [x] 2.2 Gates on the committed tip (bc5b439, clean tree, exe sha256
      0356dced1920576671feb722d385c7594e5576a2e7291ff4590e063e1a3e2ed6,
      reproduced by a second build): Mac build, game ctest 2/2, POSIX ctests
      (only the known guest_cpu "spin" flake), pytest + ruff, Metal goldens
      title, dogenzaka and dogenzaka-corn pass (one kept line). With the key
      off, dogenzaka-corn FAILs (100% bad: the stage never loads).
      garage-load is bimodal on wall-time pad timing: the load-menu anchor
      lands at flip ~5830 (pass) or ~5900 (FAIL). This exe passed once
      (20261025-111817) and failed four times. One of those failures was
      with the key off, so the bimodality is not this change's.
- [ ] 2.4 Heap trace through the stage exit: the list frees the kept block.
      Not reached: the pause menu offers only Return to Game and the
      GG-notebook, and three scripted tries never got Corn through the
      Garage exit. The trace runs to flip ~7300 with the block still held.
- [ ] 2.3 Deck D3D11 golden (needs a Deck slot). Run instead on the Linux/Proton host
      (GE-Proton11-7, borderless, exe f0c57046 at bc5b439), Deck refs:
      title pass (the first, cold-prefix run was INCOMPLETE); dogenzaka-city
      CLOSE, dogenzaka-intro and -control FAIL in both runs: the same shot a
      few flips off the Deck's (the subtitle a few letters behind), so host
      timing, not rendering; garage INCOMPLETE in both runs: after the
      load menu the guest runs at about 27-35 fps (kickwait about 34 ms a
      flip), so the later anchors are never reached. The Deck run is still
      owed
- [ ] 3.1 Move to a mod with `fix.dj_stage_word` and `fix.mission_buffer` when skates takes a toolkit
      with `mod.h`
- [x] 2.5 Golden scenario `dogenzaka-corn` (Metal references; d3d11
      references from the Linux/Proton host runs 20261025-114234 and -114448, the same
      bytes, with the kept line on D3D11 too; a third run passes CLOSE)
