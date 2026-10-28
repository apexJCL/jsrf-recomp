# Tasks

- [x] 1.1 Measure bins and voices: `[APU-MIXBIN]`, 8-bin `[APU-VSTAT]` (run base)
- [x] 1.2 Local GPL spike: run dsstdfx on a DSP56300, one silent capture (run gp2)
- [x] 1.3 Toolkit: staged mixdown (0-5 unity, 6-9 half, 10 dropped, 11-31 unity), `=2` keeps the unity sum
- [x] 1.4 ctest `apu_mixdown`: exact levels per mode, negative control
- [x] 1.5 JSRF silent Metal capture on the fix: SFX against music
- [x] 1.6 Gates: Mac build, posix ctests, pytest scripts, ruff
- [x] 1.7 BLiNX 2 Metal golden and B3 Metal race_gate, levels unchanged (B3 -14.0 dBFS vs -14.2 unity)
- [x] 1.8 Deck run under Proton (D3D11, 240 s, no crash, 0/13252 flips mismatched, peak -2.7 dBFS, no clipping)
- [x] 1.9 cat `docs/env.md`: `apu_mixdown_all=2`
- [ ] 2.1 Follow-up: why JSRF's GP register writes (GPRST) never reach the trap
- [ ] 2.2 Follow-up: a real 5.1-to-stereo downmix. Bins 2-5 still go even-left/odd-right, so centre (2) lands only left and LFE (3) only right; Burnout 3 fills 3, 4 and 5
