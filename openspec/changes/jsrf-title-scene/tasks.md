# Tasks

- [x] 1.1 Root cause: the 3D scene is fixed-function (mode 0x4, composite
      matrix written) and the executor drew those batches as screen space;
      CPU backend shows the same
- [x] 1.2 nv2a_vsh_ff.c: fixed-function pipeline as NV2A microcode (xemu
      vsh-ff.c semantics), ctest nv2a-vsh-ff
- [x] 1.3 Executor: decode fixed-function registers, swap the synthesised
      program and light constants in per batch, restore after;
      RECOMP_PB_FF=0 turns it off
- [x] 1.4 Silent Metal run: title city pan renders in 3D
      (after-ff1-f026.png vs before-m1-f026.png); A/B 40.2 vs 36.6 flips/s
- [x] 1.5 Gates: Mac build, posix ctests (0 FAIL), D3D11 cross-compile of
      the toolkit libraries with llvm-mingw 20260922, pytest scripts, ruff
- [ ] 1.6 BLiNX 2 and Burnout 3 goldens (Metal, D3D11) before merge
      (orchestrator). Burnout 3 is affected (283 fixed-function batches
      with a composite), so its goldens are a real gate
- [ ] 1.7 JSRF title scene on D3D11 under Proton
- [ ] 1.8 Owed at merge: game.toml toolkit pin
- [x] 1.9 Review fixes: specular-off oD1/oB1 = (0,0,0,1); specular alpha
      forced to 1 when 0x20000 is clear; SET_POINT_SIZE and point params
      write oPts; over-long programs skip their batches with a warning;
      the cache compares key fields; ctest cases for each
- [x] 1.10 cat docs/env.md row for RECOMP_PB_FF (branch docs/pb-ff)
- [x] 1.11 Static check of other games for composite writes (design, Other
      games)
- [ ] 1.12 SSX 3 runtime pushbuffer scan before it takes this toolkit
