# Design

All runs are silent Metal runs on the Mac (`SDL_AUDIODRIVER=dummy`), from
the jsrf-showcase save, with `RECOMP_DEBUG=audio_wav=...` and
`RECOMP_TRACE=apu_ring,audio_host`; the WAVs are checked with
`xboxrecomp-cli/src/xboxrecomp_cli/audio_check.py` (with `--log` the host
starves are spliced in). Runs live in `runs/jsrf-audio/`.

## 1. The periodic 23 ms gap (guest mix)

### Evidence
- `audio_check` on base1's WAV alone: 116 dropouts, each 23.17-23.19 ms,
  spaced 0.37154 s = 16384 / 44100 s, the lap of voice 68.
- Voice 68 (`apu_voice_dump=all`, run vdump1): a looping PCM buffer voice,
  format 5A01E000, 65536 bytes = 16384 stereo s16 frames, BA 0x00001004
  (inbuf SGE handle 1, offset 4). Its raw output has a run of exactly 1023
  zero frames at every ring position 0.
- The ring's pages (diag1/diag2): handle 1 -> 0x0037A000, a contiguous
  window page holding zeros; handles 2-17 -> 0x014B0000-0x014BF000, heap
  pages holding the stream. 0x1004-0x1FFF of handle 1 is 4092 bytes = 1023
  frames, the silent run.
- DirectSound did map handle 1 correctly: MmGetPhysicalAddress was called
  for 0x014B0000-0x014BF000 and handle 1 became 0x014B0000. 40 VP frames
  (about 27 ms) later it was 0x0037A000 again (diag3, an SGE watch in the
  VP frame).
- `RECOMP_DEBUG=watch=0x80384000` on the SGE table (run diag5) names the
  writer: `fe_method` <- `apu_hook_direct_wr` <- sub_001A5742 <-
  sub_001A1BC6 <- sub_001A1F5D <- sub_001A23CD <- sub_001A2497, a DPC.
  sub_001A5742 writes methods 0x1800/0x1808,
  `SET_CURRENT_OUTBUF_SGE` / `SET_CURRENT_OUTBUF_SGE_OFFSET`, for handles
  0 and 1 (its two-page output buffer at 0x00379000), every few ms.
- JSRF programs GPFADDR = 0x003B0000 with GPFMAXSGE = 1 (two entries) and
  EPFADDR = 0x003B4000 with EPFMAXSGE = 5; VPSGEADDR is 0x00384000.

### Cause
The front end wrote outbuf SGE entries at `VPSGEADDR + handle * 8`, the
voices' table, so they overwrote the voice entries with the same handle.
JSRF's output buffer handles 0-1
fit the GP FIFO table exactly (GPFMAXSGE = 1), and on hardware DirectSound
gives inbuf handle 1 to the stream ring at the same time, which can only
work if the two are separate tables.

### Fix
`NV1BA0_PIO_SET_CURRENT_OUTBUF_SGE_OFFSET` stores at
`GPFADDR + handle * 8` for handles 0 to GPFMAXSGE inclusive (the bound
xemu uses for its tables, `page_entry <= max_sge`). It drops the entry
while GPFADDR is 0 (it would land at physical 0) or when the handle is
past GPFMAXSGE (it would land past the table), and reports the first
dropped write once on stderr (`[APU] outbuf SGE write dropped`), so a
title that wants the EP table instead shows up in its log. Nothing in the
runtime reads the GP FIFO table while the DSP is a stub, so this only
stops the clobbering. Which table the hardware uses is inferred (xemu
leaves the method a FIXME in vp.c); GP was chosen over EP on JSRF's
evidence: it remaps handles 0-1 with GPFMAXSGE=1, while EPFMAXSGE=5.

`tests/apu_vp` maps a voice buffer at inbuf handles 1-2, then an output
buffer at outbuf handles 0-1, and checks both tables; on the old code it
fails five checks (voice handles 0 and 1 overwritten, GP table empty).
Two more cases drop the write: no GP table set, and handle 2 with
GPFMAXSGE=1 (GP entry 2 and voice entry 2 stay untouched).

### Upstream
Upstream xboxrecomp `origin/main` eb0aa1d5 (v0.14.0) still writes the
entry at `VPSGEADDR + handle * 8`, so the fix (toolkit 07a3b374 plus
cca545ac) is a candidate for the next upstream PR round.

### Effect on other games
Any title whose DirectSound maps outbuf SGEs had its voice handles of the
same numbers overwritten the same way. BLiNX 2 (recomp_0044.c) and
Burnout 3 (recomp_0022.c) both contain a store to 0xFE821808, so both can
lose voice handles 0-1 the same way; whether one of their voices sits there
while it happens is not checked here. Both are owed goldens and a Proton
run because the APU changed.

## 2. Host starvation under load (macOS)

### Evidence
- Before the fix (base1, load 12-22): rate ratio 0.935, 327 host starves
  (5.6 s), 1093 SDL underruns. The APU frame thread was not CPU-bound
  (se_frame 14-16 us, lock uncontended, apuISR at most 0.1 ms; runs lock1,
  dpc1), but ran only 81-89 % of its 1500 se_frames/s, with single
  se_frame gaps up to 133 ms: a normal thread taken off the core by a
  loaded Mac. The jsrf-showcase run 20261018-201530 at a lower load had one
  underrun, at exit.
- After the fix the starvation tracks the load average, not the build:

  | run  | build          | load (1 min) | rate  | host starves | underruns |
  |------|----------------|--------------|-------|--------------|-----------|
  | fix1 | fix            | ~46-54       | 0.829 | 475 (13.3 s) | 1180      |
  | qos1 | fix + QoS      | ~44          | 0.890 | 322 (8.5 s)  | 1030      |
  | fixb | fix            | ~29-39       | 0.970 | 170 (2.7 s)  | 566       |
  | qos2 | fix + QoS      | ~29-39       | 0.938 | 246 (5.1 s)  | 610       |
  | fix2 | fix on c391da41| ~73          | 0.810 | 599 (14.6 s) | -         |

  Every run's guest mix has 0 dropouts.

### Decision
A user-interactive QoS class for the APU frame thread was tried
(`pthread_set_qos_class_self_np`) and dropped: interleaved with the plain
fix it did no better (qos2 0.938 against fixb 0.970 at the same load).
The remaining host gaps come from other agents' load on the shared Mac, not
from the runtime; at the user's normal load the queue stays fed. If they
recur on an idle Mac, the next step is a time-constraint policy like the
kernel timer thread's (`xbox_HostTimerThreadInit`), measured on an idle
host.
