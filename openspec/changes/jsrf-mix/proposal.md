# jsrf-mix

## Why
Character sound effects (skate clash, jumps, voices) play far louder than the
music. The user heard it on the Mac.

The toolkit's GP is a stub. Its mixdown added all 32 mixbins at unity, even
bins to the left and odd bins to the right. JSRF's 3D voices go to the
crosstalk bins 6/8/7/9 at 0 dB, and also at 0 dB to bin 10, the I3DL2 reverb
send. A 260 s silent Metal capture (`runs/jsrf-mix/base`) measured:
- bins 6/7 at -11 dBFS RMS with peaks at 0.0 dBFS;
- music bins 0/1 at -24 to -27 dBFS;
- 0.37% of samples clipped.

## What changes
- Toolkit `apu_dsp.c`: the default mixdown stages the bins the way the
  title's own GP effects image does:
  - speaker bins 0-5 at unity;
  - crosstalk bins 6-9 at half (-6.0 dB);
  - the I3DL2 reverb send, bin 10, dropped;
  - effect sends 11-31 at unity, as before (unmeasured; Burnout 3 uses them).
  The gains come from running JSRF's `dsstdfx` image on a DSP56300 in a
  local-only spike (design.md).
- `apu_mixdown_all` keeps its escape hatches:
  - `=0` is the two-bin read;
  - `=2` is the old unity sum.
- The `apu_mixdown` ctest checks exact levels per mode.
- Trace only: `[APU-MIXBIN]` per-bin RMS and peak, and all 8 voice bins in
  `[APU-VSTAT]` (`RECOMP_TRACE=apu`).

## Impact
- Every title's output changes wherever it uses bins 6 and up. BLiNX 2 and
  Burnout 3 levels are checked at the gates.
- There is no new env key. The `apu_mixdown_all` row in cat `docs/env.md`
  gains `=2`.
