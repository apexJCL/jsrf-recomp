# Changelog

What changed for players in each release, newest first, in the style of
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/). A release tag's
section is its GitHub release notes: before tagging v1.0.0, add
`## [1.0.0] - YYYY-MM-DD` with the changes (Added, Changed, Fixed,
Removed). .github/workflows/release-wizard.yml stops when the tag has none.

## [Unreleased]

### Changed
- Fullscreen on macOS is a borderless window over the display, not a
  separate Space.
- **Breaking: new names for controls and keys** in `[input.bindings]` and
  `[input.actions]` of `config/config.toml`. The toolkit now uses upstream
  xboxrecomp's names and no longer reads the old ones, so rewrite any line
  of yours that uses them: `ltrigger` `rtrigger` `lthumb` `rthumb` become
  `left_trigger` `right_trigger` `left_thumb` `right_thumb`; `lstick_*`
  and `rstick_*` become `left_stick_*` and `right_stick_*`; `KP0`..`KP9`
  become `Numpad0`..`Numpad9`; `KPEnter` `KPPlus` `KPMinus` `KPMultiply`
  `KPDivide` `KPPeriod` become `NumpadEnter` `NumpadAdd` `NumpadSubtract`
  `NumpadMultiply` `NumpadDivide` `NumpadDecimal`; `LBracket` `RBracket`
  become `LeftBracket` `RightBracket`; `MouseLeft` `MouseRight`
  `MouseMiddle` become `Mouse1` `Mouse2` `Mouse3`. An old name logs an
  `unknown control`, `unknown input` or `unknown name` line: an old control
  name is ignored and that control keeps its default; an old key name is
  skipped but the rest of the line still applies, so a line left with no
  valid entries unbinds that control (`start = "KPEnter"` leaves Start with
  no key); an action line keeps only its valid entries.
  The game's defaults bind nothing by name, so a file without such lines
  needs no change.

### Added
- The game runs on macOS (Metal), the Steam Deck (Proton) and Windows
  (Direct3D 11): it boots to the title screen, and with a controller it
  goes into the garage and out onto Dogenzaka Hill. It is an early port: a
  full play-through has not been tried.
- **An in-game menu**: F10, Guide+A or Back+L3 held for a moment (and an
  Elite's P1 on macOS) opens it over the game to change the options while
  you play, rebind the host actions and turn mods on or off. Save keeps
  only what you changed. The game keeps running behind it and gets no
  input. On a Steam Deck, SteamOS keeps Guide+A for itself: use Back+L3,
  or bind R4 to F10 once in Steam Input (Back Grip Buttons > R4 >
  Keyboard > F10).
- **A stats strip**: F3 shows fps, frame time, pacing, scale and backend in
  the window's corner; off until you press it.
- Options in `config.toml`: render scale (up to 4x), anisotropic filtering,
  the present filter and fullscreen. The game draws as stock until you set
  one.
