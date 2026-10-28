# Tasks

- [x] 1.1 Quantify the gaps on a silent Metal capture (base1): 116 guest-mix
      dropouts of 23.19 ms every 0.3715 s; 327 host starves, rate 0.935
- [x] 1.2 Trace the 23 ms gap to voice 68's first ring page (handle 1 ->
      0x0037A000, zeros) and its writer (outbuf SGE methods from a DPC)
- [x] 1.3 Toolkit: outbuf SGE entries go to GPFADDR; apu_vp ctest that fails
      on the old code
- [x] 1.4 Re-capture: guest-mix dropouts 116 -> 0 (fix1)
- [x] 1.5 macOS: user-interactive QoS on the APU frame thread, interleaved
      A/B under load: no gain, dropped (design.md section 2)
- [x] 1.6 Gates: Mac build, posix ctests, pytest scripts, ruff
- [ ] 1.7 BLiNX 2 and Burnout 3 goldens (Metal, D3D11) and a Proton run
      before merge (orchestrator)
- [ ] 1.8 Owed at merge: game.toml toolkit pin
- [ ] 1.9 The user listens on the Mac (no capture was played here)
