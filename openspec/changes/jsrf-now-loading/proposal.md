# jsrf-now-loading

## Why
After jsrf-first-boot the title reaches "Now Loading" at 60 flips/s on Metal
and stays there. The title BGM (`D:\Media\Z_ADX\BGM\title.adx`) is opened
and read once at offset 0, and the ADX stream never gets past its header
(ADXT state 1, DECINFO). The loader waits for that stream, so the screen
never changes.

## What changes
- Toolkit: three fixes on the file I/O and event paths CRI's middleware
  threads depend on (design.md):
  - file I/O APCs are deferred to the next alertable wait by default
    (`RECOMP_FILE_APC_DEFER=0` restores inline delivery);
  - title-built KEVENTs get a release-on-set mark;
  - the stack cleanup after a file I/O APC is corrected.
- Game: nothing title-specific.
- Iterate silent Metal runs to the title screen or a menu, or name the next
  blocker with evidence.

## Out of scope
Proton, Deck and Windows runs; the cat `docs/env.md` row and the
`game.toml` toolkit pin bump (owed at merge).
