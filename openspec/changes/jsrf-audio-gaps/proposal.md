# jsrf-audio-gaps

## Why
The tracker item: JSRF's audio keeps cutting out on the Mac. A silent 90 s Metal
capture (run base1) has two kinds of gap:

- In the guest mix itself: 116 dropouts of exactly 23.19 ms, one every
  0.3715 s, from the first sound on. 0.3715 s is one lap of voice 68, a
  64 KB stereo 44.1 kHz stream ring; 23.19 ms is 1023 frames.
- On the host side: the APU frame thread fell behind real time (rate ratio
  0.935, 327 queue starves, 5.6 s starved) while the Mac's load average was
  12-22 from other agents.

## What changes
- Toolkit (generic MCPX fix): `NV1BA0_PIO_SET_CURRENT_OUTBUF_SGE_OFFSET`
  writes into the GP FIFO SGE table (`NV_PAPU_GPFADDR`), not the voices'
  table (`NV_PAPU_VPSGEADDR`); dropped while GPFADDR is unset. ctest in
  `tests/apu_vp`.
- No host-side change: the remaining queue starvation follows the Mac's
  load from other agents; a QoS class for the APU thread was tried and
  did not help (design.md, section 2).
- Game: nothing title-specific.

## Out of scope
Proton, Deck and Windows runs; the BLiNX 2 and Burnout 3 goldens
(orchestrator, owed before merge because the APU changed); the
`game.toml` toolkit pin bump (at merge).
