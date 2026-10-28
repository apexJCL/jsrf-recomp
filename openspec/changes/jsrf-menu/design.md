# Design: jsrf-menu

## D1. The menu ships on, the goldens pin it off
BLiNX 2 still ships `menu.enabled` off (its enhance-menu-preview task 5.3
waits for the CLI's golden stock check to learn `menu.enabled`, so that a
golden run with the menu on cannot pass). JSRF ships it on so that players find
it: a menu that must first be switched on in `config/config.toml` is one
most players never see. Shipping it on is the user's decision
("menu on for both games", 2026-10-27): an exception to the rule that
opt-in features default off, because the closed menu changes no frame and
no game input (checked by an armed Metal golden, tasks 1.8). BLiNX 2's
switch to on is a separate item. The goldens are guarded
twice instead of by the CLI check:
- `RECOMP_CONFIG=none` (already in `golden.json`): no file is read, so
  the code default (`false`) holds and the log says `actions hook=off`;
- `RECOMP_MENU=0` and `RECOMP_OVERLAY_STATS=0` in `golden.json`'s env, so
  a later change to the `RECOMP_CONFIG` line cannot bring them in.
An armed menu changes no frame until it is opened: the hook only reads
the host pad and takes presses out of the state the game gets (the A of
Guide+A while Guide is held, which the game never has: the identity map
gives it no Guide). On macOS it also makes `present.fullscreen` a
borderless window rather than a Space (toolkit a665eb23), which is the
behaviour we want there anyway.

## D2. Bindings
`menu.open` is the toolkit's three defaults, written out so a player sees
them, plus `back+left_thumb@400` since the tracker item for hosts that keep Guide
(Steam under Proton, the Game Bar). The pair is safe: in a replay of the
dogenzaka golden play, a lone Back and a lone L3 (a tap and a 0.5 s hold
each, in play) left every later frame equal to the stock replay.
`overlay.stats` gets F3: rule 4 of the host actions wants F-keys for
defaults, F9 to F11 are taken (cursor release, the menu, fullscreen), the
compiled-in key map binds no F-key to the game, and JSRF adds no
`[input.bindings]`. The action needs a feature key to be armed (rule 6);
`menu.enabled = true` is that key, so F3 works with the strip starting
off.
