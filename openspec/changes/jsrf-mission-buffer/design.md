# Design

## Evidence (runs/jsrf-hang/)
- `20261025-095428` (Corn, heap trace):
  - `alloc 0x020C5000 size 143360` at flip 2575, just after
    `mssn0110.bin` opens.
  - `free` at flip 2638, just after `mssn0110.dat` opens, with stack
    `33596 2576A 4D934 4DA59 52330 1178C1 51FF9`.
  - The stage record is `0x020C5000`/`0x020C6000`.
- `20261025-101006` (Gum, heap trace on its record block `0x020D0000`): the
  same file order, the `.bin` is allocated, and there is no free through
  the stage load or the rest of the 90 s run.
- `20261025-100321`: the stage word substituted in states 9 to 0xC.
  `Stg12_04`..`08` load, then `[HEAP] exhausted` and SIGBUS in
  `sub_00051780+0xD00`, because it sizes an allocation from the garbage at
  `record+0x38`.
- `20261025-101458`: the first version of this change (dirty tree). One
  `[FIX] … kept` line, then Dogenzaka in-game with Corn through 110 s.
- The committed-tree runs are in `tasks.md` (2.2), with the exe sha256.

## Why skip only those frees, and only then
- The manager is one object for every type. `sub_00025740` passes
  `sub_000128C0(7)` in ecx to the per-type handlers, so the type-0 list
  (`+0x198`, entries `{word, data@+4, pprev@+8, next@+0xC}`; `+8` points
  back at whatever links to the entry, written through on unlink) and the
  in-flight slot (`+0x268`) are fields of the same object.
- Three sites free the slot: the type-1 release (`sub_00032790`, return
  `0x32A30`), the type-2 step (`sub_00032C70`, return `0x32F33`, live in
  the Corn trace at flip 1859) and the type-2 release (`sub_000332F0`,
  return `0x33596`). They keep the manager in different registers (ebp is a
  local of the lifted `sub_00032790`), so the hook takes it from its root as
  `sub_00025680` does (`[[0x22FCE0] + 0x98 + 7*4]`) and checks that the
  freed address is its `+0x268`.
- The skip needs all of:
  - one of the three return addresses;
  - the address is the manager's `+0x268`;
  - a type-0 entry whose data lies in the block (the block size comes from
    `xbox_HeapBlockSize`, so only heap blocks qualify).

  That is the double ownership, and the ownership test is what makes the
  skip safe. Every other free is untouched.
- After the skip the caller clears or reuses `+0x268` as stock does, so the
  slot is free for the `.dat` and the `.bin` stays with the list, as on
  Gum's route.
- No double free, no leak: `sub_000307A0`, the type-0 per-word release,
  frees each entry's data and unlinks it. In stock, Corn's route therefore
  frees the block twice (at `0x33596`, then from the list, possibly after
  the heap reused it). With the skip it is freed once, by the list.
  That later free is read from the code, not yet seen in a run: the heap
  traces (`20261025-105242` and the two runs before it) hold the block to
  flip ~7300, and no scripted pad has got Corn out of the stage yet. If an
  exit path never releases the entry, it costs one 143360-byte heap block
  per visit.
- The list walk is capped at 4096 entries, so a corrupt list cannot spin.
- The `kept` line is logged once per block (a new address logs again), the
  no-owner diagnostic once per release site (not the step, whose free of
  its own finished load is routine: `0x32F33` freed an 831488-byte
  unowned slot at loader state 6 on every Corn run).

## The key
`fix.mission_buffer`, its own key, default on (the user, 2026-10-25). It
changes memory ownership, where `fix.dj_stage_word` substitutes a read
value, and the two interact: with this on, the tracker item's substitution no longer
fires on Corn's route. Separate keys let each be bisected and judged on its
own. Off runs the original free.

## Toolkit
None. `xbox_HeapBlockSize` is already exported at skates' toolkit pin.
