# Design: fixed-function transform as a synthesised vertex program

## Evidence
- Run m1 (before the change) shows flat facade rectangles at frame f026.
- The CPU backend run cpu1 shows the same, so the fault is shared executor
  state.
- On JSRF the executor counts `mode 0x4`: mode&3 = 0, fixed function.
  - About 26.6M fixed-function matrix dwords are written, so the title
    supplies the transform on every draw.
  - All 3D batches were counted as fixed-function and drawn with
    `program = 0`, which is the screen-space path.
- The executor's earlier comment recorded that no title run so far had
  sent a fixed-function matrix (no SET_MODEL_VIEW/COMPOSITE_MATRIX in the
  stream). That covered BLiNX 2, Burnout 3 and Spartan, which is why
  this path never mattered before.

## Why a synthesised program
The alternative was a fixed-function shader per backend: MSL, HLSL, and the
CPU path. That means three implementations that must agree.

The executor already runs title vertex programs through one decoder that
feeds all three. Writing the fixed-function pipeline as microcode has
three advantages:
- one implementation, unit-testable without a GPU;
- backend parity by construction;
- the existing program-cache, clip and viewport handling apply unchanged.

xemu's vsh-ff.c is the semantic reference. Each step in nv2a_vsh_ff.c says
where it follows it.

## Program layout
- Matrices, texgen planes, the viewport and the fog values live in the
  transform context. That is the same memory as program constants, at rows
  0x00..0x5F, and vsh_method already stores them there.
- The program reads them in place.
- Light colours and vectors are kept apart, as in xemu. The builder returns
  them as 79 extra rows, placed at c[113..191] for the draw:
  - k (0, 1, 0.5, material alpha);
  - scene ambient;
  - emission;
  - specular L, M, N;
  - point size: (max(1, SET_POINT_SIZE/8), clamp low, clamp high, 0);
  - point params p0..p3 and p4..p7;
  - 9 rows per light: ambient, diffuse, specular, direction, half vector,
    position (w = range), attenuation, spot, falloff.
- Colour outputs follow xemu:
  - specular off: oD1 and oB1 = (0,0,0,1), lit or unlit;
  - specular on: oD1 = the lit specular (or v4 unlit), oB1 = v8; when
    LIGHT_CONTROL bit 17 (ALPHA_FROM_MATERIAL_SPECULAR, 0x20000) is clear,
    xemu ignores the specular alpha, so oD1.a and oB1.a are forced to 1;
  - oB0 = v7. Back-face lighting is not computed (xemu passes the back
    colours through too).
- Point size:
  - SET_POINT_SIZE (0x043C) is decoded; a value over 0x1FF is ignored, as in
    xemu. oPts.x = max(1, size/8).
  - With SET_POINT_PARAMS_ENABLE, d = |model-view 0 * position| (vec4, as in
    xemu), oPts.x = (1/sqrt(p0 + p1 d + p2 d^2) + p6) * p3 + p7, clamped to
    [min(p7, 63.875), min(p3 + that, 63.875)].
  - Gap: xemu multiplies by the surface scale factor; that is not applied,
    so points stay native size at a raised render scale. A program-mode oPts write is not
    touched.
- Position epilogue:
  - dp4 by the composite;
  - rcc of w;
  - `oPos.xy = pos * 1/w + viewport offset`;
  - `oPos.z = z/w`;
  - w kept.
  - The composite already holds the viewport scale. Only xy takes the
    offset, as in xemu.

## Executor integration
- `vsh_method` passes every method to `nv2a_ff_method` first. The light and
  material methods it decodes previously fell through as unhandled.
  A script checked the method set against every other NV097 define this file
  handles: none collide.
- `draw_primitive` calls `ff_begin`, which proceeds only when all hold:
  - mode&3 is 0;
  - a composite matrix has been written;
  - the vsh path is enabled;
  - `RECOMP_PB_FF` is not 0.
- `ff_begin` then:
  1. Builds the program, or finds it in a 32-entry cache. An entry stores
     the key fields (`nv2a_ff_key_fields`: the register bits that change
     the microcode, not constants) and matches on the 64-bit FNV hash and a
     memcmp of those fields, so a hash collision cannot pick the wrong
     program.
  2. Saves the program slots and constant rows it displaces.
  3. Writes the program at slot 0 and the light rows.
  4. Sets `start = 0` and `prog_dwords = 0x80000000 | serial`. The backends
     memoise translation on (prog_dwords, start), so a synthesised program
     never aliases a title program. Their shader caches are keyed by
     content hash.
- `ff_end` restores the saved slots and rows after `on_draw`. Title programs
  and constants are untouched.
- Program slots and constant rows are separate memories: the 136 slots hold
  microcode, the 192 rows hold constants. A synthesised program longer than
  the 136 slots cannot be loaded. Its batches are skipped, not drawn as
  screen space (which only produces the flat rectangles this change
  removes). The executor warns once and the report counts them
  (`skipped (program too long)`). JSRF's longest program is 44 slots; eight
  local lights with everything on would be the case that overflows.

## Other games
The path runs only for fixed-function batches after a composite-matrix write
(0x0680). `ff_composite` is sticky: once a title writes it, every later
fixed-function batch takes the synthesis. Checked from existing runs and
the gens, without running the games:
- **BLiNX 2:** the stage 1 and story pushbuffer scans
  (`runs/kh-gpu/blx-cpu-scan`) show 0 composite writes and 0
  fixed-function batches. Unaffected beyond the light and material methods
  now being decoded instead of logged as unhandled.
- **Burnout 3: affected.** The Mac pushbuffer scan
  (`runs/kh-gpu/b3-mac/20261016-104425-cpu-scan`) shows the composite
  written repeatedly at runtime (0x0680 counts growing to x166) and
  `100811 program, 283 fixed-function` batches. It also sends 0x0294,
  0x03BC, 0x0314, 0x0318 and 0x043C. Those 283 batches now take the
  synthesised transform, as they would on xemu, instead of screen space.
  The executor's earlier comment ("Burnout 3 never sends one") was wrong
  and is corrected. The Burnout 3 goldens are a real gate for this change.
- **KUF:** the vanilla bench runs report mode 0x4 with 0 fixed-function
  matrix dwords and no fixed-function batches drawn. No composite writes
  seen.
- **SSX 3:** no pushbuffer scan or VSH report exists in its runs or build
  logs. The gen holds the `0x400680` method constant (6 sites), but the
  BLiNX 2, JSRF and KUF gens hold it too (2 each): it is XDK D3D library
  code, so its presence says nothing. A runtime scan is owed before SSX 3
  takes this toolkit.
- Upstream: Spartan's tree carries `tests/nv2a_ffp_backend/`, which
  suggests an upstream fixed-function backend. Under the upstream-first rule
  that is to be compared before this goes upstream.

## Performance
Metal, 120 s from the same HDD state:

| Run | Synthesis | flips/s |
| --- | --- | --- |
| ff2 | on | 40.2 |
| ffoff | off (`pb_ff=0`) | 36.6 |
| ff3 | on, after the review fixes, `RECOMP_TRACE=vsh`, load average about 29 from other agents | 36.7 |

The synthesis costs nothing measurable. Seven distinct programs are built in
a run, at 15 to 44 slots each (16 to 47 after the review fixes add the
specular and point-size writes). ff3 renders the city as ff2 does
(`after-ff3-f024.png`); its lower rate was measured under heavy host load
and is within the 25% flag threshold.
