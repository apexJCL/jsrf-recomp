# Design

## The wait
The loader (sub_000348A0) polls the sound task, which polls ADXT handle 0
(pool 0x27D120, stride 0xA8). ADXT_StartFname opened `title.adx`, and CRI's
file server (sub_00140C30 over the file objects at 0x273780, stride 0x150)
streams it. A read goes through three steps, all on the server's thread:

1. sub_001407E0 sets busy (+0x14C) to 1 and issues ReadFileEx with completion
   routine 0x1401B0.
2. sub_00140BA0 sees busy == 1 and calls SleepEx(0, TRUE).
3. The completion APC runs inside that SleepEx and clears busy. Back in
   sub_00140BA0, busy == 0 means "this read finished": it adds the bytes to
   the position (+0x18) and sets the object's state (byte 1) from 2 to 1.

If busy is already 0 at step 2, or esi is wrong after step 3, step 3's
accounting never runs. The object then sits in state 2 with done == 0 and
busy == 0, and the stream starves. Peeks (run r10) showed exactly that for
0x273780.

## Three faults on the way there

### 1. The vblank event lost wakeups (win32_compat, opt-in per event)
The CRI vsync server (sub_0013B1C0) and fs server (sub_0013B230) both wait on
D3D's vblank notification KEVENT (dev+0x2430), through
BlockUntilVerticalBlank (sub_0018CE50). That function writes 0 to
SignalState before it waits; under RECOMP_TITLE_KEVENTS that write becomes a
host ResetEvent. On the POSIX shim a set followed quickly by a reset left a
waiter asleep, because wait_single re-checks `signaled` only. So the fs
server thread never ran.

Fix: `w32_event_release_on_set(HANDLE)` marks one event. A SetEvent or
PulseEvent on it bumps a generation counter, and a waiter that started before
the bump is released even if a reset followed. This matches NT, where a set
satisfies the current waiters at once. ke_guest_event marks the title-built
events it creates. That is POSIX only and only under RECOMP_TITLE_KEVENTS,
which JSRF, BLiNX 2 and Burnout 3 all turn on (see Effect on other games).

### 2. The APC ran too early (kernel, deferred by default)
bridge_complete_file_io ran the completion APC inside NtReadFile, so busy
was already clear when step 2 looked at it. Now the APC goes on a per-thread
queue (kernel_user_apc.c). The issuing thread's next alertable user-mode
wait runs the queue and returns STATUS_USER_APC, as on hardware. The waits
that do this are KeWaitForSingleObject, KeWaitForMultipleObjects,
NtWaitForSingleObjectEx, NtWaitForMultipleObjectsEx and
KeDelayExecutionThread. The object waits test their objects first, with a
zero timeout while APCs are pending, and deliver only when that times out:
an object that is already signalled wins, as in the kernel. A drain runs
only the APCs that were queued when it began (bounded by sequence number,
so a nested drain does not widen it), so a completion that issues the next
read waits for the following drain. The routine runs holding the guest CPU,
as NtUserIoApcDispatcher's does.

This is the toolkit default: queuing to the next alertable wait is kernel
behaviour, and inline was the shortcut (orchestrator decision, approved by
the user, 2026-10-18). `RECOMP_FILE_APC_DEFER=0` restores inline delivery as
an escape hatch. The game needs nothing title-specific.

### 3. The APC stack cleanup (kernel, shared path)
deliver_one_apc pushed 16 bytes (three arguments and a dummy return), called
the routine, then popped 12 more. The routine is NTAPI, and the kernel
dispatch for NtUserIoApcDispatcher (ordinal 232, 12 argument bytes) already
pops all 16, so g_esp came back 12 bytes high. Inline inside ReadFileEx this
was hidden, because ReadFileEx restores esp with `leave`. Deferred, the APC
runs inside XAPI's SleepEx (0x145C28), which pops esi before its `leave`. So
sub_00140BA0 resumed with a wrong esi and did its accounting on the wrong
object.

Fix: save g_esp before the push and restore it after the call. That is
correct for a stdcall routine and a cdecl one alike. The inline path
(RECOMP_FILE_APC_DEFER=0) changes as well.

## Effect on other games
- BLiNX 2 and Burnout 3 pass no APC routine. All their NtReadFile and
  NtWriteFile call sites (14 in BLiNX 2, 13 in Burnout 3, from
  analysis/disasm) push a zeroed register (`xor ebx, ebx` or equivalent) as
  ApcRoutine, including XAPI's overlapped ReadFile and WriteFile. Neither
  imports ReadFileEx's NtUserIoApcDispatcher path. So faults 2 and 3 do not
  reach them by static evidence. Goldens should still confirm this, since
  both changes sit on the shared path.
- Fault 1 does reach them on the Mac. Both turn RECOMP_TITLE_KEVENTS on in
  main.c (the template default), so their title-built events get
  release-on-set on POSIX. That matches what Win32 and NT already do on the
  Proton and Windows builds. On Metal it is a behaviour change, so their
  Metal goldens should run before the merge.

## Follow-ups (not needed for JSRF so far)
- Alertable waits not hooked: NtSignalAndWaitForSingleObjectEx (230),
  KeRemoveQueue (136), and the alert
  exports KeAlertThread (93), KeAlertResumeThread (92) and KeTestAlertThread
  (155). A thread that waits alertably only through these keeps its file
  APCs queued until it reaches a hooked wait. NtWaitForSingleObject (233) is
  not on the list: it is a KernelMode wait, which takes no user APCs.
- Other APC sources do not use the per-thread queue yet: NtQueueApcThread
  (206) and KeInsertQueueApc (118), and timer APCs (NtSetTimerEx 229,
  KeSetTimerEx 150). They should share kernel_user_apc so that ordering
  across sources holds.
- File I/O still completes synchronously inside NtReadFile and NtWriteFile.
  Only the APC is deferred, and the event is set at once. A title that
  expects STATUS_PENDING, or that times its reads, still sees instant
  completion.
- PulseEvent on an auto-reset event still releases nobody on the POSIX
  shim: a waiter wakes after the pulse has cleared the state. This predates
  the change; release_on_set applies to manual-reset events only.
- Release-on-set as the shim's default for every manual-reset event, not
  only the guest's own (ke_guest_event). It is the Win32 and kernel rule;
  the switch needs the BLiNX 2 and Burnout 3 Metal goldens.
- With RECOMP_WORKERS=inline, or when worker stacks run out, a worker runs
  on its creator's host thread, and its file APCs land on the creator's
  queue.

## Tests
- `tests/kernel_user_apc`: FIFO order, the bounded drain (a chained APC and
  one queued after a nested drain both wait for the next drain), a full
  queue, and per-thread queues.
- `tests/win32_event_posix`: set+reset and PulseEvent release a waiter, for
  a single and a multi-object wait; a waiter that resets the event as it
  wakes does not rob a second one; a wait that begins after set+reset times
  out; an unmarked event and a marked auto-reset event keep the old
  behaviour.
