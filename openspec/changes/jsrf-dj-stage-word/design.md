# Design

## The function
`sub_0004AC80` (`this` in ecx):

```
push esi; esi = ecx
call sub_00025680          ; uses the manager at [0x22FCE0], not this
jne out
eax = [esi+0x1040]         ; the stage record
ecx = [eax]                ; its first word: the stage
push ecx; push 3
call sub_000256A0
add esp, 8
je out
[esi+0x5C] = 0xA
out: pop esi; ret
```

Scene setup at `0x517AC`/`0x517B8` stores the record pointer at
`this+0x1040` and the record's first word at `this+0x104C`. With the key
on, the override (`src/fixes/dj_stage_word.c`, routed from
`recomp_manual.c`, which also declares `sub_0004AC80_gen`, because
`exclude_manual` names that file) replays these
instructions in stock order. It reads the record and its word after
`sub_00025680`, as stock does, and keeps every register and stack effect,
so the callees see the same state as stock. Off, `sub_0004AC80_gen` runs.

## Why the cached word, and only for a word that is not a stage
- A stage word is two halves of one decimal digit each: the path builder
  `sub_00036210` formats `Custom\Stg%d%d` from `(word >> 16) % 10` and
  `(word & 0xFFFF) % 10`, and the disc's stages run `Stg08` to `Stg49`. The
  hang word `0x1889B927` (halves 6281 and 47399) is not one.
- The cached word replaces the live one only when the live word is not a
  stage word and the cached one is. A legitimate stage word is never
  replaced, so a record some path re-points on purpose without refreshing
  `+0x104C` keeps stock behaviour.
- In good runs the live word is `8` (quarantine runs), so nothing
  changes.
- Nothing is written to the guest beyond what the function itself writes.
- `tests/dj_stage_word_test.c` (ctest `dj_stage_word`) runs the replay
  against stub callees: `(3, word)` under the stock return addresses, `esp`
  and `esi` back as on entry, `+0x5C` only on a nonzero return, and no
  substitution for a valid live word.

## Default on (deviation, user-approved 2026-10-23)
The project's rule is that opt-in features default off. This is a fix
for a bug the Xbox hides (its freed block keeps the word), not a feature, and it changes
nothing while the record holds a stage word. The user decided to "leave it
on as default" on 2026-10-23. The ENHANCE build reads
`enhance_cfg_bool("fix.dj_stage_word", 1)`, the build without the layer
starts from `jsrf_fix_dj_stage_word = 1`, and both take `0`/`false`/`no`/
`off` to turn it off.

## Config key instead of a mod (deviation)
Game-specific fixes belong in mods over the toolkit's mod layer. skates
pins toolkit `06c97d1`, which has no mod API: `mod.h` landed 517 commits
later. Bumping the pin is a separate, larger change with its own goldens.

So this fix ships as a game-side manual override with a config key,
following the existing `enhance_cfg_bool`/`enhance_cfg_bind_env` pattern.
The coordinator approved the deviation on 2026-10-23. It moves to a mod once
skates takes a toolkit with `mod.h` (tasks.md 3.1).

## The env key
`RECOMP_FIX_DJ_STAGE_WORD` is a game key: `CMakeLists.txt` defines
`RECOMP_ENV_HAVE_GAME_KEYS` on the `recomp_env` target and adds `src/env`
to its include path, so `recomp_env_game.h` joins the table. In the
ENHANCE build it is bound to the config key; without ENHANCE, `main.c`
reads it directly with the same bool words.
