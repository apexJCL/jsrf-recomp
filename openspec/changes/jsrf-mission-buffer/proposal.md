# jsrf-mission-buffer

## Why
With Corn, the load into Dogenzaka hangs on the loading screen. On other
replays it crashes in `sub_00051780`. This is the free that
The tracker item left out of scope.

The loader manager loads a mission's `.bin` (`mssn0110.bin`, 143360 bytes)
into its in-flight slot at `+0x268`. It hands the buffer to the type-0 list
at `+0x198`, and the stage scene's record (`this+0x1040`) comes from that
list.

The DJ event's teardown (scene state 7, `sub_000521B0` → `sub_0004D970` →
`sub_0004D900` → `sub_00025740(2, …)`) runs the type-2 release
`sub_000332F0`, which frees whatever `+0x268` holds. The type-1 release
(`sub_00032790`) and the type-2 step (`sub_00032C70`) free the slot the same
way.
- On Corn's route the release lands right after `mssn0110.dat` opens, while
  `+0x268` still holds the `.bin`, so it frees the buffer the type-0 list
  owns.
- On Gum's route (traced) it does not, and the buffer lives through the
  stage. Yoyo's route was not traced that far.

The heap then reuses the block. These states all read it after the free:
- State 9 (the tracker item's fix substitutes the cached word there).
- State 0xA (`sub_00052460`) waits in `sub_000256C0(3, <stale word>)`,
  which never matches,
  so the loader idles and `Stg12_04`..`08` are never requested.
- Once that is patched, `sub_00051780` reads the whole record during stage
  play and crashes.

So the stage word is not the only thing read from the record. The free
itself has to go.

## What changes
- `src/fixes/mission_buffer.c`, routed from `src/recomp_manual.c`, overrides
  the title's VirtualFree (`sub_00145A5D`). The generated body stays as
  `sub_00145A5D_gen`.
- A new key, `fix.mission_buffer` (`RECOMP_FIX_MISSION_BUFFER`), default on
  (the user, 2026-10-25). With it on, a free of the `+0x268` slot from any
  of its three sites (returns `0x32A30`, `0x32F33`, `0x33596`) is skipped
  when a type-0 entry still points into the block. It returns TRUE and logs
  one `[FIX] mission_buffer: … kept` line per block. Every other free runs
  the generated body.
- A release (`0x32A30`, `0x33596`) that frees the slot with no type-0 owner
  logs one diagnostic line per site, with the loader state, so a release
  before the handoff shows in the log. The step's free at `0x32F33` is its
  normal end of a load on every route, so it does not log.
- `tests/mission_buffer_test.c`, ctest `mission_buffer`.
- Docs: a `docs/env.md` row, a `packaging/config.default.toml` comment, the
  `src/env/recomp_env_game.h` row and the `main.c` comment.
  `fix.dj_stage_word` is unchanged.

## Known gap
A release that lands before the handoff, while the `.bin` is only in
`+0x268`, cannot be caught: no entry points into it yet, so the free goes
through and the list later takes a freed pointer. Nothing has shown it yet.
The diagnostic line names it if it happens, and the Corn replay on the Deck
(slower I/O under Proton) is the check before the tracker item closes.

## Out of scope
- Why the release races the `.dat` load on this host and not on the Xbox
  (the I/O timing of the `.dat` open). The skip restores the memory state
  that the other routes reach.
- Defaulting `RECOMP_HEAP_QUARANTINE`. It also loads Dogenzaka (run
  `20261025-095211`), but it only delays the reuse. The record is read for
  the whole stage, so a FIFO would release it mid-stage.
