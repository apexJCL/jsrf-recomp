# Jet Set Radio Future static recompilation

A port of the original Xbox game *Jet Set Radio Future* to native code with the
[xboxrecomp](https://github.com/apexJCL/xboxrecomp) toolkit, driven by
[xboxrecomp-cli](https://github.com/apexJCL/xboxrecomp-cli). Started with
`xbr new`.

**No game data here, and none ever will be.** Put your own dump of the disc
(`default.xbe` and the game's files) in `game_files/`; git ignores it.

## Progress

An early port. On all three platforms the game boots past the logos to the
title screen, and with a controller it goes into the garage and out onto
Dogenzaka Hill.

| Platform | Renderer | State |
|---|---|---|
| macOS (Apple silicon) | Metal | tested: title screen, the garage and Dogenzaka Hill |
| Linux, Steam Deck (Proton) | Direct3D 11 through DXVK | tested: title screen, the garage and Dogenzaka Hill |
| Windows | Direct3D 11 | played by hand, at a stable frame rate |

Known gaps: a full play-through has not been tried.

## Building

```sh
./skates setup      # once: .venv, llvm-mingw, the toolkit (each download sha256-pinned)
./skates all        # analyze (parse, disasm, funcid, abi), recomp, build
./skates doctor     # what this machine has, and what is next
```

Windows: `skates` (the `skates.cmd` wrapper) or `py -3 skates.py`, from a
short checkout path such as `C:\g\skates`, with
`git config --global core.longpaths true`.

The first build is `build-win/skates_recomp.exe`, cross-compiled with llvm-mingw on
any host, or `build/skates_recomp` with Apple clang on a Mac (Metal). It will
crash the first time: that is where the port starts.
The toolkit is built with the DSP device (`-DXBOXRECOMP_APU_DSP=ON` in
`game.toml`), so configure downloads the prebuilt dsp56300 library; offline,
point `XBOXRECOMP_DSP56300_ARCHIVE` at a local copy of that archive
(`NOTICE` has its licences).
`src/recomp_manual.c` takes hand-written overrides, `config/seed_functions.json`
the function starts the disassembler missed, and `./skates analyze` then
`./skates recomp` regenerate `src/recomp/gen/` (never edit it by hand).
The CLI's `docs/porting.md` has the loop; `game.toml` has every setting,
documented in the CLI's `docs/manifest.md`.

## Options

The game reads its options from `config.toml`: in a bundle, `config/config.toml`
in the game's data folder; in a developer checkout, `config/config.toml` in
the project root. `config.default.toml` (beside the executable,
`packaging/config.default.toml` here) lists every key at its default;
copy a line into your `config.toml` to change it, for example
`scale = 2` under `[render]` to draw at 1280x960. With no `config.toml` the
game draws as stock. Two defaults are not the layer-free build's:
`present.pacing = "sleep"` (no effect yet, since Jet Set Radio Future is
not generated with spin waits; `"spin"` is the console-original timing) and
`present.surface = "auto"`, which in SteamOS Game Mode hands gamescope the
frame-sized surface to scale instead of a window (`"window"` restores that).

F10, Guide+A or Back+L3 held 0.4 s (and, on macOS, an Elite controller's
P1) opens an in-game menu over the game to change these options while it
runs, rebind the host actions and turn mods on or off; Save keeps only
what you changed in `config/config.toml`. The game keeps running behind it
and sees no input. F3 toggles a stats strip (fps, frame ms, pacing, scale,
backend) in the window's top right corner; it starts off.
`[menu] enabled = false` turns the menu off, and `[input.actions]` rebinds
both keys (`docs/env.md`, the toolkit's `docs/runtime/config.md`).
The Windows build reads pads through XInput, which has no paddles, so P1
does nothing there. On a Steam Deck, SteamOS keeps Guide+A for itself and
Steam Input hides the back grips from the game: use Back+L3, or bind R4 to
F10 once in Steam Input (the game's controller settings > Back Grip
Buttons > R4 > Keyboard > F10), and R4 then opens the menu.

## Performance

The title-screen benchmark (`config/bench/title.toml`), run with
`./skates bench-matrix` and written here by
`./skates bench-matrix report --public .`: the frame rate each host holds
at the best settings it keeps up with.

<!-- xbr:bench jsrf-title begin -->
<!-- xbr:bench jsrf-title end -->
