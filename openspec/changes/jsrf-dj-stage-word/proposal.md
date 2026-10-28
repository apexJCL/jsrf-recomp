# jsrf-dj-stage-word

## Why
After the DJ event some runs load stage 19 instead of the next stage and
stall on its loading screen. `sub_0004AC80` hands the stage scene
its stage word at the switch from an event scene. It reads the word live
through the record pointer at `this+0x1040`, but by that switch the record
has been freed. On the Xbox the freed block still holds the word. Here the
allocator can give the range to the SE loader first: in the hang layout the
record (`0x0228E000`) sits inside the 0xB7000 block that holds
`se_pple.dat`, and the word read is sample data (`0x1889B927` where `8` is
expected).

Scene setup (`sub_000517AC`) cached the same word at `this+0x104C` when it
set the pointer, so the right value is still at hand.

## What changes
- `src/recomp_manual.c` overrides `sub_0004AC80` through
  `src/fixes/dj_stage_word.c`. With the fix on it replays the function's
  instructions in stock order. Where the live word is not a stage word and
  the word cached at setup is, it passes the cached word and logs one
  `[FIX] dj_stage_word` line per record and word. Off, the generated body
  (`sub_0004AC80_gen`) runs as is.
- `tests/dj_stage_word_test.c`, ctest `dj_stage_word`: the replay against
  stub callees.
- Config key `fix.dj_stage_word` (default on, user-approved; design.md), env
  `RECOMP_FIX_DJ_STAGE_WORD` through the game env table
  (`src/env/recomp_env_game.h`, `docs/env.md`). `main.c` logs
  `[ENHANCE] fix.dj_stage_word=on|off`.
- `packaging/config.default.toml` gets a `[fix]` table.

## Out of scope
- Finding who frees the record early: the heap trace and quarantine runs
  did not reproduce the hang layout.
- A toolkit mod: skates' toolkit pin has no mod API (design.md).
