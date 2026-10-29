# jsrf-spin-waits

## Why
The tracker item, from the title benchmark: JSRF's two hottest guest
functions are busy-wait loops that nothing lowers, so the title burns
about 2 host cores waiting (Mac and PC) and saturates the one pinned guest
core on the Deck:

- `sub_001912A0`, D3D's KickOff: the main thread spins on the PFB flush
  bit until the toolkit's ack loop has caught up (sites `0x001912E0`, and
  `0x00191290` in its sibling `sub_00191270`).
- `sub_0013B180`, CRI ADX's lock helper thread: `while (flag == 0)
  counter++` at `0x0013B190`. On the Deck it takes 16-25% of core 0 in
  the slow runs (hypothesis H1 for the Deck's slow mode).

skates has no `config/spin_waits.json`, so `present.pacing = "sleep"`,
the default, has nothing to act on.

## What changes
- Toolkit (generic): the spin-wait matcher accepts a per-site
  `"counters"` list, words the loop may bump (load, inc/dec/add/sub an
  immediate, store back) that nothing else reads. A compare that reads a
  counter, or a branch on a bump's flags, is still refused (a counted
  delay). Without `counters` nothing changes. Tests in
  `tools/recomp/test_spin_waits.py`; docs in `docs/pipeline/04-lifting.md`.
- Game: `config/spin_waits.json` with the two KickOff sites, `[pipeline]
  spin_waits` in `game.toml`, and the "no effect yet" wording dropped from
  `packaging/config.default.toml` and the README.

The ADX site came out after the Deck run (design.md, "Deck finding"):
lowered, it deadlocks the boot under Proton, because the ADX unlock
suspends the helper inside `xbox_SpinWait`'s host lock. With it out, the
counters change is not used by any site, and the KickOff sites need no
toolkit change. The counters branch stays parked for the ADX follow-up.

Phase 2 superseded the KickOff part (design.md, tasks 2.1-2.5): with the
walker polling, the KickOff sleep cost the Deck 3 fps, so the toolkit
gained a per-site `"sleep": "kick_event"` key and `RECOMP_SPIN_WAIT_KICK`
(private a7233436, in blinx2-v0.7.0). Both KickOff sites carry that key:
they sleep under `present.pacing = "sleep"` only while the walker's kick
event wait is on (`RECOMP_DEBUG=kick_event=1`), and spin otherwise. The
"no effect yet" wording became the README and config.default.toml note
that the sleep needs `kick_event=1`.

## Out of scope
- `guest_sched=1` (the faithful model, where the guest CPU skips a
  suspended helper); an event-driven kick in place of the ack loop's poll;
  the Mac's 5 s double hitch; the D3D11 shader cache. All are the tracker item
  findings with their own items.
- The ADX site, until the toolkit has a suspend-safe spin wait.
