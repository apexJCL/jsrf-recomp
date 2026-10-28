# Tasks

- [x] 1.1 Re-pin `game.toml [toolkit]` to 8fb88c79 (a665eb23 plus the tracker item)
- [x] 1.2 `config.default.toml`: `[menu]`, `[overlay]`, `[input.actions]`
- [x] 1.3 `golden.json`: `RECOMP_MENU=0`, `RECOMP_OVERLAY_STATS=0`
- [x] 1.4 `docs/env.md`, README, CHANGELOG
- [x] 1.5 Gates: Mac build; `uv run pytest scripts`, ruff
- [x] 1.6 Mac by hand (scripted with RECOMP_KBM_SCRIPT; seen in the log, not on screen): F10 opens the menu, F3 toggles the strip; the start
      log shows no `[INPUT] action ... refused` line for the shipped bindings
- [x] 1.7 Deck (D3D11, Proton): a scripted F10 opens the menu
      (RECOMP_DEBUG=kbm_script: `[ACTION] menu.open via F10` twice,
      `overlay.stats via F3`)
- [x] 1.8 Goldens on Metal and on the Deck: title, garage, dogenzaka-corn.
      Metal: all pass on a quiet Mac. Under host load (other agents' game
      runs on the Mac) garage-load FAILed once (mae 2.3, bad 1.49%), and the
      quiet rerun passed: garage-load is load-sensitive, as its scenario
      note says. Armed and closed (`RECOMP_MENU=1`, uncommitted): title and
      dogenzaka-corn pass on Metal with `actions hook=on`, so the shipped
      default changes no frame. Deck: title and dogenzaka-corn pass;
      garage's load, charsel and intro EXACT, garage-idle INCOMPLETE: the
      160 s run ends near flip 7100-7500, and garage-idle needs about 8400.
      The baseline (3877790, toolkit ca891783) did worse on the Deck the
      same afternoon, about flip 7000 with 4 INCOMPLETE, so the cause is the
      host and the scenario length, not the pin. No other Deck run
      overlapped (the run lock serialises them; the tracker item's started after).
      garage's `seconds` is now 200
- [ ] 1.9 Owed: the native Windows golden at the PC's next Windows boot
