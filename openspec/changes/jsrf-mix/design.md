# Design

## Measurement
- The trace (`RECOMP_TRACE=apu`) prints the RMS and peak of each bin once a
  second, and every bin a voice routes to.
- In JSRF every submix headroom is 1 and `hrtf_headroom` is 0.
  - The 2D voices are halved by the headroom.
  - The 3D voices reach the crosstalk bins without that halving. The VP here
    matches xemu's vp.c.
- Music: stereo 2D voices v68-70 on bins 0/1 at volume -4.4 / -6 dB.
- Player SFX: 3D voice v0 at 0 dB to 6/8/7/9 and to 10.

## What the GP does
A local-only spike (`spike/jsrf-mix-gp`, never merged or pushed) linked
xemu's DSP56300 core. It ran the title's GP program on the VP's mixbins, and
took the output where xemu's GP monitor does: X:0x1400 and X:0x1420.

One silent capture (`runs/jsrf-mix/gp2`, 167 one-second windows) gave:

| Bins | In | Out |
|---|---|---|
| 0/1, music | x | x, 0.0 dB in every window |
| 6/7, SFX | -13.3 to -15.7 dBFS | SFX part -19.3 to -21.8 dBFS, i.e. in -6.0 dB (within 0.3) |
| 10, reverb send | always bin 6 - 6 dB | adds nothing measurable |

Whole-run WAV, 80-160 s:

| Run | Left | Right | Peak | Clipped |
|---|---|---|---|---|
| Stub (base) | -13.4 dBFS | -16.1 dBFS | 0.0 dBFS | 0.37% |
| GP (gp2) | -18.9 dBFS | -18.8 dBFS | -2.5 dBFS | 0% |

The stub was 5.5 dB hot on the left and 2.7 dB on the right. The surplus is
all SFX, so the SFX were about 6 dB too loud on the right and about 9.5 dB
too loud on the left.

## The model
- **Bins 0-5 at unity.** The speakers pass through unchanged, as measured.
- **Bins 6-9 at half.** The crosstalk effect's measured gain. It also
  undoes the 6 dB the 3D voices skipped through `hrtf_headroom` 0, which puts
  them level with the 2D voices again. Only 6/7 carried signal in the
  capture; 8/9 at half is assumed by symmetry, as the other crosstalk pair
  of the same effect.
- **Bin 10 dropped.** It is an effect input. The image mixes back only the
  wet output, and JSRF's I3DL2 room has no wet level. A stub without the
  reverb is nearest to hardware when it drops the send.
- **Bins 11-31 at unity, as before.** Nothing measured them. A first cut
  dropped them too, and Burnout 3's race came out 0.9 dB quieter on Metal
  (-15.2 / -15.1 dBFS against -14.2 for the unity sum in the same session):
  B3 sends to 11/12 and 17/18, and 17/18 sit only about 5 dB below its main
  bins (summed `[APU-MIXBIN]` power over the race). Those sends feed effects
  the title loads itself, whose wet output does reach the speakers, so the
  dry sum stays as the stand-in until a capture says otherwise.
- **Even bins left, odd bins right**, as before.

Only two constants are modelled. No GPL code and no table derived from it
goes into the toolkit.

## Results
Bins each title fills (`[APU-MIXBIN]`): JSRF 0, 1, 6, 7, 10; BLiNX 2
(stage1) 0, 1 only; Burnout 3 0-5, 11, 12, 17, 18. So the change touches
only JSRF; BLiNX 2 and B3 mix the same as before.

| Run (Metal, silent) | Left | Right | Peak | Clipped |
|---|---|---|---|---|
| JSRF stub (base), 80-160 s | -13.4 dBFS | -16.1 dBFS | 0.0 dBFS | 0.37% |
| JSRF GP (gp2), 80-160 s | -18.9 dBFS | -18.8 dBFS | -2.5 dBFS | 0% |
| JSRF fix, 80-143 s | -19.5 dBFS | -19.5 dBFS | -2.5 dBFS | 0% |

- B3 race_gate: PASS, 55.4 race flips/s, -14.0 dBFS (unity in the same
  session: -14.2).
- BLiNX 2 Metal golden: attract, stage1 and story pass.
- Deck, Proton D3D11 (toolkit c6cf4f48 on the merge): JSRF New Game
  ran the 240 s limit, no crash, 0/13252 flips mismatched, peak -2.7 dBFS
  and no clipping (bench-logs/20261018-234006). That run reached only one
  second of SFX, so the levels rest on the Mac A/B; the mixdown is the same
  C on every host.

## Follow-up: GPRST never trapped
In 260 s JSRF made no trapped access to the GP register region (0x30000 and
up), not even GPRST. Real hardware bootstraps the GP from scratch on a
GPRST write, so the spike had to start the GP at the first scratch doorbell
command. Either the title writes the GP through an address the APU hook
does not trap, or the stub has been missing those writes all along. Its
`apu_dsp_gprst_write` path (the doorbell-ack pause) never fires for JSRF.
This needs a trace of the title's GP writes before anything relies on
GPRST.
