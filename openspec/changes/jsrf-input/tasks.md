# Tasks

- [x] 1.1 Find how JSRF reads its pad: statically linked XAPI on the OHCI
      registers (0xFED00000, sub_001BD108); the template never brings the
      model up
- [x] 1.2 Game main.c: RECOMP_USB on by default, xbox_OhciInit after the
      memory layout
- [x] 1.3 Toolkit: the same in templates/new-game; RECOMP_USB=0 is off;
      env description; usb_ohci_posix off case
- [x] 1.4 Toolkit: descriptors and buffers in their TD's own allocation
      resolve window first; RAM-named heap pages resolve as RAM; class
      counters on RECOMP_USB_STATS; resolution ctest
- [x] 1.5 Seed the five missed XPP starts; analyze, recomp
- [x] 1.6 Silent Metal runs: Start advances the title with the model,
      stays without it (frames)
- [x] 1.7 Gates: Mac build, posix ctests, pytest scripts, ruff
- [ ] 1.8 Real pad on the Mac (the user)
- [x] 1.9 Burnout 3 pad check and race gate on Metal (shared address
      resolution): PASS, 14 live presses to the race, 55.7 race flips/s
- [x] 1.10 cat docs/env.md RECOMP_USB row (cat docs/usb-on)
- [ ] 1.11 Owed at merge: game.toml toolkit pin
