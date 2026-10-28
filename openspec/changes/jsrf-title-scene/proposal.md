# jsrf-title-scene

## Why
Past Now Loading, Jet Set Radio Future's title attract scene pans across the
city. On Metal every building facade came out as a flat, axis-aligned
rectangle stacked over the screen (`runs/jsrf-title-scene/before-m1-f026.png`).
The CPU backend showed the same picture, so the fault is in shared executor
state, not in a backend.

JSRF draws its 3D scene in fixed-function mode
(SET_TRANSFORM_EXECUTION_MODE 0, about 26.6M fixed-function matrix dwords
written, with the composite at 0x0680). The executor had no fixed-function
transform: those batches went down the screen-space path, so object-space
positions were drawn as if they were pixels.

## What changes
- Toolkit (`nv2a_vsh_ff.c`, new): the NV2A fixed-function transform and
  lighting pipeline, written out as NV2A vertex microcode with xemu's
  vsh-ff.c semantics:
  - composite transform and the viewport epilogue;
  - skinning;
  - lighting: infinite, local and spot lights, specular, colour material;
  - texgen and texture matrices;
  - fog;
  - point size and point parameters.
- Toolkit (`nv2a_pb_exec.c`):
  - decodes the fixed-function registers;
  - once a composite matrix has been written, a fixed-function batch swaps its
    synthesised program and light constants in, draws as a program batch
    and restores both.
  - Every consumer of vertex programs (the CPU interpreter and the MSL and
    HLSL translators) draws it through the path it already has, so Metal and
    D3D11 cannot disagree.
- `RECOMP_PB_FF=0` (debug tier) turns the synthesis off.
- A ctest, `nv2a-vsh-ff`, covers the builder.
- Game: nothing title-specific.
- Burnout 3 sends a composite matrix and draws 283 fixed-function batches in
  the Mac scan. Those now take the transform too, as on xemu (see design).

## Out of scope
- Two-sided lighting: back colours are passed through, not lit, as in xemu.
- The surface scale factor on the point size (points stay native size at a
  raised render scale).
- Proton, Deck and Windows runs.
- Owed at merge:
  - the `game.toml` toolkit pin.
- The cat `docs/env.md` row for `RECOMP_PB_FF` lands on cat branch
  `docs/pb-ff`.
