# Design

## 1. The sites, checked against the disassembly

### KickOff, 0x00191290 and 0x001912E0
`sub_00191270` and `sub_001912A0` load the GPU register base
(`[[0x19DCE0]+0x2268]`), set bit 16 of `[regs+0x100410]` (PFB's flush
bit), then spin `test [eax+0x100410], 0x10000; jne` until it clears.
`sub_001912A0` then writes the new PUT. The toolkit's ack loop clears the
bit once the walker has caught up (`kick_cleared` in
`src/kernel/xbox_memory_layout.c`), and calls `xbox_SpinWake()` there,
so a sleeping wait wakes on the clear instead of the 1 ms timeout. This
is the shape the matcher already takes (`TEST_REG_DISP`), and the same
sites BLiNX 2 lowers in its own KickOff (cat `config/spin_waits.json`,
`0x002E7F20` and siblings). `eax` is set before the loop and never
written in it.

### ADX helper, 0x0013B190
```
loc_0013B190: mov eax, [0x25EFA8] ; inc eax ; mov [0x25EFA8], eax
              mov eax, [0x25EFC0] ; cmp eax, 0 ; je loc_0013B190
```
- `0x25EFC0` is written only by the ADX setup/teardown `sub_0013B330`
  (zeroed at init with 0x25EFC4..0x25EFDC). While the thread runs, the
  loop is meant to run until teardown.
- `0x25EFA8` appears twice in the whole generated code, both in this
  loop; nothing reads it through a register either (the only pointer
  near it, `ecx + 0x25EFFF` in `sub_0013C960`, starts above 0x25F000).
  It is a pass counter nobody looks at.
- What the thread is for: the setup creates it suspended
  (`sub_00147F53`, start `0x13B180`, flags 4) and registers
  `sub_0013B0A0`/`sub_0013B0E0` as ADX's lock/unlock callbacks. The lock,
  on its first level, raises the locker to time critical (priority
  `[0x25EF8C]` = 15) and resumes the helper; the unlock suspends it and
  restores the priority. On the one console CPU the helper, at a high
  priority, holds the CPU whenever the time-critical locker blocks, so no
  lower thread runs inside the ADX critical section. It does no audio
  work: ADX's server and stream threads feed the audio, not this one.
  "Letting the helper make progress" means only that its flag test keeps
  running, which a sleeping wait does (it retests on every wake or after
  at most 1 ms).
- On the hosts that exclusion never held anyway: macOS's `SuspendThread`
  only counts, so the helper spins from its first resume to the end
  (30036 of 30036 profiler rounds), and Wine's priorities are not
  exclusive (the helper shares core 0 with the main thread). The game
  runs correctly on both, so the lowering changes no behaviour that was
  relied on. On native Windows, with the guest pinned to one core and real
  priorities, a sleeping helper lets lower guest threads run while a
  locker blocks inside the lock; that is the Mac's case already.

The matcher refused this loop (it writes memory), so the toolkit learns
counters (section 2).

## 2. Counters in the matcher
A site entry may name `"counters": ["0x0025EFA8"]`. For that site the
matcher also accepts:
- a load (`mov`/`movzx`/`movsx`) from a counter, which marks the register
  as holding a count;
- `inc`/`dec`/`add`/`sub` with an immediate on such a register, or on the
  counter word in memory;
- `mov [counter], reg` from such a register.

It refuses a compare that reads a counter or a register holding a count,
and a branch whose flags come from a bump: either makes the loop's exit
depend on the count, a delay loop that a 1 ms sleep per pass would
stretch. Counters are per site and only from the file: `auto` never infers
one. The file's site entries now accept only `va`, `note` and `counters`.

## 3. How it is checked
- `analysis/recomp/spin_waits.json`: the three sites `lowered`, no errors.
- The generated code: `RECOMP_SPIN_WAIT(0x00191290u)`,
  `(0x001912E0u)`, `(0x0013B190u)` on the back edges; the binary links
  `xbox_SpinWait`.
- The `[PACING]` block in the game log lists per-site waits, wakes and
  timeouts; the KickOff sites should exit mostly on wakes, the ADX site on
  timeouts (nothing wakes it; it never exits until teardown).
- Audio: the title's ADX music on the Deck (sound on) against a baseline run from
  before the change, by ear and with the CLI's audio check where a capture is
  possible; ADX is the stream the helper's lock guards.
- Measurement: 6 Deck arms of the title matrix with the thread
  log plus one Mac arm, against `runs/bench-matrix/jsrf-title`: process
  cores, fps, 1% low and the Deck's good/slow split.

## Deck finding: the ADX site deadlocks under Proton (2026-10-23)

The three-site build hangs at boot on the Deck. The windowed run stalls
after 8 flips and the borderless run after 125, both with the main thread
blocked at 0 CPU. The same exe with `RECOMP_PRESENT_PACING=spin` (the
lowered sites do not sleep) runs at 59.5 fps. A KickOff-only build boots.

Cause: `xbox_SpinWait` sleeps on one process-wide `CRITICAL_SECTION` plus
a condition variable (kernel_pacing.c). The ADX unlock suspends the helper
with a host `SuspendThread` (xbox_NtSuspendThread). Without guest_sched,
Windows and Wine suspend the thread for real, at any instruction. A
helper suspended inside the wait's lock, or while re-taking it on wake,
holds that lock while it is suspended. From then on, every other
`xbox_SpinWait` and `xbox_SpinWake` blocks, the KickOff sites and the
vblank wake among them. On macOS a suspend is only counted, so the Mac
never shows it.

So the ADX site comes out of config/spin_waits.json for now, and only
the KickOff sites ship. Lowering the ADX site again needs a wait that a
host suspend cannot wedge. It must not take a shared host lock. It must
still hand the guest CPU over, since the helper loops for as long as ADX
is up. One alternative is a guest-side suspend at a safe point
(guest_sched already counts another thread's suspend). That is a toolkit
follow-up, not part of this change.

## The KickOff sleep, gated on the kick event wait (2026-10-23)

The KickOff-only build saved 0.56 core on the Deck but cost about 3.3
steady fps and 10 warm fps in all six arms. The walker still polls for the
kick (about every 110 us under Wine), so the flush bit clears tens of
microseconds after the set. A sleep then pays for a condition-variable wake
and the guest CPU hand-off, and that pause sits on the main thread's path in
every kick. The tracker item ran the same exe with `RECOMP_DEBUG=kick_event=1`: the
pass pokes the sleeping walker, its ack wakes the sleep, and all 6 arms were
good (59.1-59.3 steady fps) while keeping about 0.52 core. Both controls
without it were slow.

So the sites sleep only while the event wait is on. The toolkit
(`feat/spin-wait-kick-gate`) adds a per-site `"sleep": "kick_event"` to the
spin-wait file:

- the generator emits `RECOMP_SPIN_WAIT_KICK`, the same macro test, calling
  `xbox_SpinWaitKick`;
- `xbox_SpinWaitKick` is `xbox_SpinWait` with the sleep allowed only when
  `xbox_Nv2aKickEventOn()`: `kick_event=1`, the ack loop started, and the
  wait not given up (three unpoked kicks in a row turn it back to polling).
  Otherwise the pass is the spin-pacing one: the walker poke, the IRQ safe
  point and the guest CPU yield;
- `xbox_Nv2aKickEventOn` is a read-only getter beside `xbox_Nv2aKickPoke`.
  the tracker item keeps its meaning if it touches the give-up flag.

With `kick_event` off (the default) the two sites run as at `spin`, so
stock behaviour is unchanged at either pacing value. When the tracker item and the
pb-event-kick soak turn `kick_event` on by default, the sleep follows with
no game change.
