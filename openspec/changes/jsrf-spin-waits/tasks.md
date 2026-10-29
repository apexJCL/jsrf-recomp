# Tasks

- [x] 1.1 Check each site's wait condition in the disassembly (design.md
      section 1)
- [x] 1.2 Toolkit: per-site counters in the spin-wait matcher, tests and
      docs (feat/spin-wait-counter)
- [x] 1.3 Game: config/spin_waits.json, [pipeline] spin_waits, config and
      README wording
- [x] 1.4 analyze + recomp: three sites lowered, then two (KickOff only,
      after 1.6), RECOMP_SPIN_WAIT in the
      generated code, xbox_SpinWait in the binary
- [x] 1.5 Gates: Mac and Windows builds, toolkit pytest, skates pytest and
      ruff
- [x] ~~1.6 Deck: the title boots, the ADX music plays without gaps against
      the baseline~~ superseded by 2.4 (the ADX site was dropped; the music
      checks ran there, with no starves or underruns)
- [x] ~~1.7 Measure: 6 Deck arms with the thread log and one Mac arm against
      runs/bench-matrix/jsrf-title~~ superseded by 2.4
- [x] 1.8 Toolkit pin: none owed, since the KickOff sites use no counters.
      The counters branch (feat/spin-wait-counter 154a2b83) is parked for
      the ADX follow-up

## 2. The KickOff sleep gated on the kick event wait

- [x] 2.1 Toolkit: `"sleep": "kick_event"` per site, `RECOMP_SPIN_WAIT_KICK`,
      `xbox_SpinWaitKick`, `xbox_Nv2aKickEventOn`; pytest and the spin_wait
      ctest (feat/spin-wait-kick-gate)
- [x] 2.2 Game: both KickOff sites gated; README and game.toml wording
- [x] 2.3 Regen, Mac and Windows builds; skates pytest and ruff (both sites
      lowered as kick_event; exe 9a35cc05)
- [x] 2.4 A quiet Mac pair and a Deck pair, with and without kick_event=1.
      Deck: spin control good (59.5 fps, 1.60 cores, main 97%); kick_event
      good in 3 of 6 runs (one on the exe) at 1.08-1.58 cores (main
      20-26%). One s2 run lost the event wait at load (three unpoked kicks)
      and fell back to the spin with the walker polling: 51.7 fps, 474
      hitches (a kick_event issue).
      The first s2 run (117 hitches, no give-up) is unexplained. That exe
      predates the tracker item's missed-watch carry; three s2 reruns on the rebased
      tip are the tracker item. Mac: 2.49 -> 1.66 cores, 1% low 40 -> 47. Audio
      clean.
- [x] 2.5 The toolkit pin: main already pins blinx2-v0.7.0 (public
      178657a, push round 14), which carries feat/spin-wait-kick-gate
      (private a7233436, public a26c06d), so the merge keeps it. The old
      06c97d13 ignored "sleep" and would have lowered both sites ungated.
- [ ] 2.6 Owed: a native Windows run, at the PC's next Windows boot.
