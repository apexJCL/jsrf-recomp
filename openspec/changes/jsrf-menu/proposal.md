# jsrf-menu

## Why
The tracker item. The user, 2026-10-27: "JSRF works on mac - can we add the overlay
menu + strip?" The toolkit pin (ca891783) has the overlay core and the
stats strip, off and unbound, but not the in-game menu (toolkit 794d6b56),
its live switches (31dd7c4c), the one-input-one-action rules (57ef0d79), the
pad setups (ab46ffdd) or the macOS borderless fullscreen (a665eb23).

## What changes
- `game.toml [toolkit]`: commit 8fb88c79, the `posix-host/portability` tip
  (mod settings API v3 phase A, on top of a665eb23).
  Nothing in the game's code changes: JSRF reads its pad through the
  toolkit's OHCI model, which calls `xbox_InputGetState`, and that applies
  the host actions' pad filter itself (toolkit `src/input/README.md`, "The
  pad filter"). BLiNX 2 needs its own call because it has its own pad glue.
- `packaging/config.default.toml`: `[menu] enabled = true`, `[overlay]
  stats = false`, and `[input.actions]` with `menu.open = ["F10",
  "guide+a", "paddle1"]` and `overlay.stats = "F3"`.
- `config/golden/golden.json`: `RECOMP_MENU=0` and `RECOMP_OVERLAY_STATS=0`
  beside `RECOMP_CONFIG=none`.
- `docs/env.md`, README, CHANGELOG: the menu, the strip and their keys.

## Out of scope
- Mod settings beyond what the pin brings; the overlay
  UI scale.
- A fallback pad chord for hosts that never pass Guide (BLiNX 2's
  `back+left_thumb@400`): it needs a pair of JSRF buttons the game ignores
  on a lone press in play, and a Deck hands-on. Added later by the tracker item (see
  design D2).
- The Game page (`enhance_menu_add` for `fix.*`): the fixes are start-time
  switches, so a live toggle would not apply them.
