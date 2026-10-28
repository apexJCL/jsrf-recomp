# Design

## The XDK 4134 GPU wait
XDK 5849 D3D keeps a semaphore fence: the device holds the submitted fence
at `+0x2C` and a pointer to the GPU-written word at `+0x30`, and
`xbox_Nv2aMirrorFence` ties the two. XDK 4134 D3D (section `D3D`,
0x0018CB40-0x0019E338) keeps the same fence one slot later. Its device
global is `0x0019DCE0`:

- BlockOnTime `sub_00191440` reads `[dev+0x34]`, then the word it points
  to, then `[dev+0x30]` (0x0019144C, bytes `8b4734 8b08 8b4730`), and spins
  at 0x001914F0 on `[[dev+0x34]]`.
- Device init pushes SET_SEMAPHORE_OFFSET (header `0x00041D6C`) at
  0x00191853.
- The fence reset at 0x0019135A writes `[[dev+0x34]] = [dev+0x30] - 2`.

So the submitted fence is `+0x30` and the GPU-written word's pointer is
`+0x34`. No mirror is registered: the executor writes the semaphore word
itself (`fence_completed` in `xbox_memory_layout.c`), and every run logs
"NV2A PGRAPH fence: no fence mirror registered; 0x400B10 follows the
executor's semaphore releases" (`runs/jsrf-first-boot/`). `main.c` keeps
the line as a commented template, `xbox_Nv2aMirrorFence(0x0019DCE0, 0x30,
0x34)`.

D3D also waits on the channel's DMA pointers, which the runtime already
satisfies:

- KickOff `sub_001912A0` stores PUT to `[[dev+0x2264]+0x40]` (0x001912FE) and
  keeps the submitted PUT at `[dev+0x2C]` (0x00191305).
- The busy check `sub_0018E120` compares `[dev+0x2C]` with DMA_GET
  `[[dev+0x2264]+0x44]` (masked 0x0FFFFFFF), then reads PGRAPH_STATUS
  `[regs+0x400700]`, `regs = [dev+0x438]`.
- Ring waits at 0x00192460 and 0x001925E0 loop until
  `[[dev+0x2264]+0x44] == [dev]` (or `[dev+0x24]`).

`[dev+0x2264]` is the USER channel area at aperture offset 0x800000, so GET
is 0xFD800044, the register the runtime's NV2A ack loop already advances to
PUT after each pushbuffer walk. PGRAPH_STATUS reads 0 (idle) in the
emulated aperture.

### CLI follow-up: the fence signature per XDK
`xbr new` finds the fence by 5849's BlockOnTime prologue,
`8b4630 8b08 8b462c` (`mov eax,[esi+0x30]; mov ecx,[eax]; mov eax,[esi+0x2C]`),
which has no match here. 4134's is `8b4734 8b08 8b4730`
(`mov eax,[edi+0x34]; mov ecx,[eax]; mov eax,[edi+0x30]`): a different base
register and offsets one slot later. The CLI could match both and fill in
`(global, 0x30, 0x34)` for 4134, or say that no mirror is needed.

## Seeds
All six misses were checked in the listing as real starts: the three thread
routines carry `data_imm` XREFs from the thread setup, 0x001780D0 is a
vtable thunk after padding, and the two XPP targets follow a `ret` (XPP is
not 16-byte aligned, so 0x001BCAA3 is a real start).

Two more turned up at runtime: 0x001773E0, a vtable thunk the first run
crashed through, and 0x0007BE30, entry 0 of the state table at 0x0020D2B8,
which the listing had read as jump-table data. Both are `observed`. The
first crash showed the analyzer had not followed the vtables, so a scan of
`.rdata` and `.data` for runs of three or more code pointers, each 16-byte
aligned after nop or int3 padding, added 111 vtable entries (105 from
`.rdata`, 6 from `.data`) without `observed`. This XBE flags both sections
executable, so the scan picks them by name.

## Clear colour (toolkit)
The Smilebit logo's background came out cyan and Dolby's blue. JSRF draws
them on an R5G6B5 back buffer and D3D packs the clear value in that format
(`0x0000FFFF` white, `0x000020E4` near-black), but every path read
`SET_COLOR_CLEAR_VALUE` as A8R8G8B8. The toolkit branch adds
`nv2a_clear_packed16` and `nv2a_clear_argb` to `nv2a_backend_common.h`, as
xemu unpacks the clear per surface format: the CPU path writes the packed
value, and Metal and D3D11 widen it. 32-bit surfaces are unchanged. A
ctest (`nv2a_clear_color`) covers both helpers.

## Now Loading (open)
The boot reaches the "Now Loading" screen at 60 flips/s and stays there.
The last file activity is one 51200-byte `ReadFileEx` of
`D:\Media\Z_ADX\BGM\title.adx` (XAPI APC 0xFE000068, delivered inline);
no read follows and no APU voice starts beyond DirectSound's four idle ones.
The CRI ADXM threads are alive (their loop counters at 0x0025EFA8..B0
advance), the main thread draws, and no ICALL miss or unimplemented kernel
call shows. This is the open blocker, named in the tracker.

## Runs
Headless Metal, `SDL_AUDIODRIVER=dummy`, bounded by a timeout, under the Mac
locks; logs and frames in the workspace `runs/jsrf-first-boot/`. The build
uses the agent's toolkit worktree through `XBOXRECOMP_DIR`.
