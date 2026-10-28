# Design: jsrf-input

## How an XDK title reads its pad
XAPI (XPP, the XID class driver and the OHCI host controller driver) is
linked into the XBE, so the title drives the MCPX USB registers itself. In
JSRF (XDK 4134), XAPI's USB init maps the first host controller at
0xFED00000 (`sub_001BD108` stores it), then reads HcRevision and the root
hub descriptor. No kernel export is involved, so the runtime can only answer
at the register level.

Two ways to get a pad into such a title:
1. Override the XInput entry points (`XGetDevices`, `XInputOpen`,
   `XInputGetState`, ...) in `recomp_manual.c`, as BLiNX 2 does. This needs
   each title's addresses, and its own device-insertion bookkeeping.
2. Answer the hardware: the toolkit's OHCI model (`src/usb/ohci.c`) traps the
   aperture, walks the title's ED/TD lists and puts an Xbox controller on a
   root hub port (`src/usb/usb_gamepad.c`). Burnout 3 runs this way. The
   model is game-agnostic and also carries `RECOMP_PAD_SCRIPT`,
   `RECOMP_PAD_LIVE` and the keyboard merge.

JSRF takes the second way, with no game constants.

## Root cause
skates' `main.c` comes from the toolkit's `new-game` template. The template
already routes trapped MMIO faults to the OHCI model
(`xbox_PosixMmioFault` / `xbox_VehMmioFault`), but it never calls
`xbox_OhciInit`, and `RECOMP_USB` is off by default. So the aperture stays
zeroed RAM: XAPI sees no controller ports, enumerates nothing and never
reports a device. That explains why the log has no input lines and why the
host pad, which the model alone polls, is never read.

## Fix
- After the memory layout (the model protects the aperture pages), and before
  the kernel starts, set `RECOMP_USB=1` unless the environment names it, then
  call `xbox_OhciInit()`. This is the same placement Burnout 3 uses, and the
  same default-on pattern as `RECOMP_VBLANK` in this file.
- The template gets the same lines, with a note that a title which overrides
  XInput drops them.
- `ohci.c` tests `recomp_env_on(RENV_USB)`, so `RECOMP_USB=0` means off.
  Only the OHCI model reads the key. Burnout 3 sets `1`, BLiNX 2 never calls
  the init, so neither game's behaviour changes.

## Root cause 2: which copy a physical address names
With the model up, XAPI saw the device arrive and then stalled. On the
console a physical address names one page of RAM. Here the contiguous window
(0x80000000 + P) and the RAM below 64 MB are separate storage, and the
contiguous allocator hands out physical numbers that the image and the heap
also use. The model's `bus_resolve` picks one copy: the image first (Burnout
3's class driver reads a descriptor into its own .data at 0x0041A904), then a
page the allocator owns, then plain RAM. JSRF breaks that order twice:
- XAPI's OHCD gets its HCCA at 0x80082000, so HcHCCA = 0x00082000, inside
  an image that runs to 0x00288620. Image first read the interrupt table,
  then the EDs and TDs, out of .text. The descriptors are always the OHCD's
  contiguous memory, so `rd32`/`wr32` now ask the page map first
  (`bus_resolve_desc`). The SETUP packets sit in the same allocation as
  their TDs (0x00083FB0 beside TDs at 0x00083Fxx). A buffer is resolved the
  same way only when it is in its TD's own allocation (`same_contig_block`):
  both pages live in the page map and `xbox_ContiguousBlockSize` gives both
  the same block end. "Same page" was not enough: Burnout 3's .data buffer
  at 0x0041A904 falls inside a contiguous block (0x3E7000..0x5E7000) that is
  not its TDs' (0x025B5xxx), and must stay image-first. Other buffers keep
  the image-first rule, and JSRF's descriptor reads into its .data
  (0x002648DC) still land there.
- The input reports go into a heap buffer at 0x010DE0CA whose page the
  allocator also owns, so they went to the window and the title's buffer
  never changed: the pad enumerated, the script ran, and Start did nothing.
  `MmGetPhysicalAddress` already records heap pages the title names as RAM
  (`xbox_PhysRamNameMap`, kept for the APU's scatter-gather lists); the model
  now checks it before the page map, outside the TD's own allocation.

The cost: a descriptor a title keeps statically in its image, on a page
that is also contiguous, now reads the window copy. No known title does
this. The real fix is one physical space (the window and low RAM as one
store), a follow-up.

### Affected titles
The resolution is shared by every title that runs the model: Burnout 3,
SSX 3 (`recomp/src/main.c`), Spartan (`tca/src/main.c`) and JSRF. BLiNX 2
overrides XInput and never calls `xbox_OhciInit`, so it is not affected.
Burnout 3's descriptors are at 0x025B50C0, outside its image, and its .data
buffer is outside its TDs' allocation, so the rules leave it as it was. Its
report buffers change only if they are RAM-named heap pages the allocator
also owns, which would have kept its pad from working before. Burnout 3's
pad and race golden are re-run on Metal; SSX 3 and Spartan get smoke runs.

### Visibility and tests
Each resolution is counted by class (image, ramname, pagemap, highwater,
plain, desc-window, td-block), printed on the `RECOMP_USB_STATS` line and in
the OHCI report. `xbox_OhciResolveForTest` exposes the resolver to
`usb_ohci_posix`, which checks five cases: a descriptor in both the image
and the arena (window), a buffer in another block inside the image (image),
a SETUP in its TD's block across a page (window), a RAM-named heap page
(RAM for data, window for a descriptor), and a freed block below the high
water mark (window).

## Missed XPP functions
Once the pad configured, two XAPI completion routines were called through
pointers the disassembler had not found (0x001BE689, then 0x001BDE21). Both
start right after a `ret 8` that ends another function. Scanning XPP for
function ends that start nothing found three more of the same shape
(0x001BD853, 0x001BE0B4, 0x001C213A); all five are seeded.

## Verification
Silent Metal runs from the user's showcase hdd, with `RECOMP_PAD_SCRIPT`
tapping Start every 6 s from t=30 s:
- with the fix (`runs/jsrf-input/usb-start3`): the first tap skips the
  intro, the next opens the NEW GAME / LOAD GAME / VS MODE menu (f024), the
  next starts a new game ("Now Loading", f025);
- with `RECOMP_USB=0` and the same script (`nousb-start2`): the intro and
  title loop untouched, as in the user's run;
- before the RAM-name rule (`usb-start2`): the pad enumerated and the
  script ran, and the title still ignored Start.

After the TD-block rule, on toolkit c391da41 (the tracker item's 3D title):
- JSRF (`usb-start4`): the New Game menu (f026), "Now Loading" (f031), then
  the heap OOM. `RECOMP_USB_STATS` classes: desc-window for every
  descriptor, data image=7 (descriptor reads into .data), ramname for each
  report, td-block=8 (the SETUPs).
- Burnout 3 race gate on Metal (`20261018-211310-b3-race-metal`): PASS, 14
  live presses through the front end into the race, 55.7 flips/s in the
  race. Classes: data image=14, pagemap for the reports, td-block for each
  SETUP, no ramname, as before the change.
