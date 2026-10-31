# Environment keys

The toolkit's keys are listed by `RECOMP_TRACE=help` and `RECOMP_DEBUG=help`.
This page lists the keys JSRF adds (`src/env/recomp_env_game.h`).

| key | tier | default | meaning |
|---|---|---|---|
| `RECOMP_FIX_DJ_STAGE_WORD` | config (`fix.dj_stage_word`) | on | The switch from an event scene to a stage (`sub_0004AC80`) reads the stage word through a record that the DJ event's teardown has freed. When the record no longer holds a stage word and the word cached at scene setup (`+0x104C`) is one, the cached word is used and one `[FIX] dj_stage_word` line is logged. Without it the game can load stage 19 after the DJ scene and stall. The Xbox keeps the freed block's contents, so on hardware the bug never shows. `0`, `false`, `no` or `off` runs the original read. |
| `RECOMP_FIX_MISSION_BUFFER` | config (`fix.mission_buffer`) | on | The loader frees its in-flight slot (`+0x268`) from three sites: the type-1 release, the type-2 step and the type-2 release (returns `0x32A30`, `0x32F33`, `0x33596`). When the slot holds a mission `.bin` that the type-0 list already owns, the free is skipped and one `[FIX] mission_buffer: ... kept` line is logged per block; the list frees the block itself later, so stock frees it twice. Without it, Corn's route frees `mssn0110.bin` in the DJ teardown and then hangs on the Dogenzaka loading screen or crashes in `sub_00051780`. A release that frees the slot before the list takes it cannot be caught; it logs one `[FIX] mission_buffer: ... before any type-0 entry uses it` line per release site (the step frees its own finished load that way every time, so it does not log). On the Xbox the release does not land in that window, because of the I/O timing of the `.dat` open, not because freed memory keeps its contents. `0`, `false`, `no` or `off` runs the original free. |

## Toolkit keys this game ships set

These are the toolkit's keys (its `docs/runtime/config.md`), listed here
for the value JSRF's `packaging/config.default.toml` gives them. Each
variable overrides its key in `config/config.toml`, which overrides the
shipped file. Golden runs pin both off (`config/golden/golden.json`,
beside `RECOMP_CONFIG=none`), so they never run with the menu or the strip.

| config key | variable | shipped | meaning |
|---|---|---|---|
| `menu.enabled` | `RECOMP_MENU` | `true` (code default `false`) | The in-game menu, opened and closed by the `menu.open` host action (`F10`, `guide+a`, `paddle1`, `back+left_thumb@400`). While it is open the game keeps running but reads a released pad. Save writes only the changed keys to `config/config.toml`. The start log says `[ENHANCE] menu.enabled=on (F10, guide+a, paddle1, back+left_thumb@400)` or `=off`. Closed, it changes no frame; the armed hook still takes the chords that open it from the game (Back and L3 read released while both are down, the A of Guide+A is held back). `true`/`false`, 1/0 |
| `overlay.stats` | `RECOMP_OVERLAY_STATS` | `false` | The stats strip (fps, frame ms, pacing, scale, backend) in the window's top right corner at 2 Hz, never in a frame dump. The `overlay.stats` action (`F3`) and the menu's Stats page toggle it while the game runs; the action needs the menu or the strip on, which arm the host actions. 1/0 |
| `storage.ram_cache` | `RECOMP_STORAGE_RAM_CACHE` | `true` (the toolkit's default too) | The cache partition (`Z:`, `Partition3`-`5`) lives in memory, capped at the stock 750 MB, seeded read-only from `<hdd>/Cache` and never written back. `false` (or the variable `0`) puts the files in `<hdd>/Cache` as stock. The start log says `[FILE] ram cache: on, ...` at the first file call |
| `[input.actions]` | (none) | `menu.open = ["F10", "guide+a", "paddle1", "back+left_thumb@400"]`, `overlay.stats = "F3"` | The host actions' bindings. A player's line for an action replaces the shipped one whole. The toolkit refuses a key the game's `[input.bindings]` uses, with an `[INPUT] action ...` line |
