# Tasks

- [x] 1.1 Override `sub_0004AC80` in `recomp_manual.c` with the
      generated body kept as `sub_0004AC80_gen`; regenerate with
      `skates analyze` and `skates recomp`
- [x] 1.2 Config key `fix.dj_stage_word`, env `RECOMP_FIX_DJ_STAGE_WORD`
      in the game env table, `docs/env.md` row, `[fix]` in the default
      config
- [x] 1.3 Gates: Mac build, `uv run pytest scripts`, ruff
- [x] 1.4 Metal title scene, silent, key on and off: the `[ENHANCE]` line,
      no `[FIX]` line, flips/s equal
- [x] 1.5 Windows build; Deck smoke to title under flock with the
      `[ENHANCE] fix.dj_stage_word=on` line. The gate ran on toolkit
      9ba0ed3f (`XBOXRECOMP_DIR`), not on the branch's `game.toml` pin
      (06c97d1 = 695540ba), whose exe hits a Proton fatal at about 61 s
      with the key on or off. On 9ba0ed3f: rc=0, no fatal, no
      `[FIX]` line. The flips (6419 vs stock 5836) differ in exe, toolkit
      and fullscreen setting, so they only show there is no drop. The merge
      waits for the skates re-pin in the v0.16 sync: rebase onto it, then
      rerun this smoke. Rerun 2026-10-24 after the rebase onto skates main
      cb1cf5b, regenerated and built on toolkit a6417bff (analyze, then
      recomp with `XBOXRECOMP_CLI_DIR`): ctest `dj_stage_word` passes, and
      the Deck smoke gives rc=0, no fatal, the `[ENHANCE]` line, no `[FIX]`
      line and 6438 flips (6429 before the rebase)
- [x] 1.6 Review fixes (notes/win-checks/ITEM-review.md): the
      replay moved to `src/fixes/dj_stage_word.c` with ctest
      `dj_stage_word` (M1); substitute only for a live word that is not a
      stage word when the cached one is (L1); default-on deviation in
      design.md (L2); bool words without the layer (L3); record and word
      read after `sub_00025680` (L4); named scene type, no guest bound
      left (N1); `scripts/test_env_doc.py` (N2)
- [ ] 2.1 Hang replay on the PC (user play): 0 hangs in repeated DSP ON
      runs past the DJ event
- [ ] 3.1 Follow-up: move to a mod once skates takes a toolkit with `mod.h`
- [ ] 3.2 Follow-up: other readers of the same freed record stay unfixed:
      `sub_0004AC00` and `sub_0004AC50` read `[rec+0x1C]`, `sub_0004ACB0`
      and `sub_0004ACE0` read `[rec]` with scene type 4. A hang after a DJ
      scene that is not Stg19 should be checked against these first
