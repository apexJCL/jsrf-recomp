# jsrf-first-boot

## Why
The first Metal run boots the kernel bridge, the APU and four workers, then
guest-main (tid 1002) stops with 0 flips in 12 s. Two causes were suspected:
the D3D fence line in `src/main.c` is commented out (`xbr new` only knows the
XDK 5849 fence wait, and this title is XDK 4134), and six function starts are
missing from the ICALL dispatch.

## What changes
- Identify how XDK 4134 D3D waits for the GPU and wire the runtime to it, or
  record why no `xbox_Nv2aMirrorFence` line is needed.
- Seed the six ICALL misses in `config/seed_functions.json` (`observed`).
- Iterate the headless Metal boot (silent, bounded) on further misses and
  spin waits until the first flip, or name the blocker with evidence.

## Out of scope
Proton, Deck and Windows runs; the `xbr new` 4134 signature (a CLI
follow-up).
