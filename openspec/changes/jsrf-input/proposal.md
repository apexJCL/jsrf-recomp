# jsrf-input

## Why
The tracker item: with a gamepad connected on the Mac, nothing responds at the title
("PLEASE PRESS START TO BEGIN"), and the log has no input lines
(runs/jsrf-showcase/20261018-201654). The keyboard does nothing either.

## What changes
- Game: `config/seed_functions.json` gains five XPP function starts the
  disassembler missed (two seen as ICALL failures once the pad enumerated,
  three more found by the same pattern), regenerated with `skates analyze`
  then `skates recomp`.
- Game: `src/main.c` turns `RECOMP_USB` on by default and calls
  `xbox_OhciInit()` after the guest memory is mapped. JSRF's statically
  linked XAPI then finds the toolkit's USB controller model, enumerates the
  emulated Xbox controller on port 1 and reads its reports, which carry the
  host pad (SDL GameController on POSIX, XInput on Windows), the keyboard
  merged onto port 0 and `RECOMP_PAD_SCRIPT` steps.
- Toolkit (`fix/jsrf-input`):
  - the `new-game` template's `main.c` does the same, so the next game made
    with `xbr new` gets a pad without having to find this out again;
  - `xbox_OhciInit` reads `RECOMP_USB` as a switch (`=0` or empty is off),
    so the game's default can be turned off from the environment. Before,
    any value, `0` included, turned the model on;
  - the controller model resolves the physical addresses JSRF's XAPI hands
    it the way that XAPI meant them: descriptors (HCCA, EDs, TDs) and a
    buffer in its TD's own allocation go to the contiguous window first,
    and a heap page the title named through `MmGetPhysicalAddress` is RAM. Without
    this the pad never enumerated (descriptors read from .text) and, once
    it did, its reports never reached the title's buffer;
  - the `RECOMP_USB` description no longer says "(unused path)";
  - `usb_ohci_posix` gets a second case, `RECOMP_USB=0` leaves the
    apertures as plain memory, and a resolution block over a test hook;
  - `RECOMP_USB_STATS` and the OHCI report count resolutions by class.

## Out of scope
- Rumble, a second pad and memory units: each is a follow-up once a player
  asks for it. `RECOMP_USB_PADS` already plugs in more pads.
- Proton, Deck and Windows runs.
- New Game now reaches "Now Loading" and then stops on "There's a problem
  with the disc": `xbox_HeapAlloc` runs out of memory loading
  `Media\Misc\Misc.dat` (676864 bytes, 50.5 of 50.9 MB used). That is a
  memory-layout bug the pad only made reachable; it gets its own item.
- BLiNX 2 overrides XInput in its own code and never calls `xbox_OhciInit`,
  so it is not affected. Burnout 3, SSX 3 and Spartan set `RECOMP_USB=1`
  themselves, so their init is unchanged; the address resolution is shared,
  so Burnout 3's pad and race golden are re-run, and SSX 3 and Spartan get
  smoke runs, before merge.
