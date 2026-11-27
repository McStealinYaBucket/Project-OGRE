# Wave 19: App and Base follow-ups (app-19)

- **Owner and scope:** Client/App and Client/Base. No commits, no IDA database opened, shared reccmp/ not written, nothing under Client/Rendering/ touched. One Network-facing header edit (NetworkSettings `m_unk0xec` made public, line count unchanged) so the network agent's ServerReplicator.cpp compiles.
- **Header note for the other agents:** app-19-headers.md (written first, updated twice).
- **Build dirs:** build-ap/ (PLAYER), build-ap-studio/ (STUDIO). Scripts, probes and logs are in build-ap/w19/; snapshots are in build-ap/w/snaps/W19*, runs in build-ap/w/runs/W19*.
- **How I measured:** private root build-ap/w/root, with tools/, reccmp/*.csv and reccmp-project.yml refreshed from HEAD 826ad6e. For each snapshot: two reccmp runs, gen_reccmp_csv from the pair, two more runs, a second regeneration, then a final run. Every scoring link deleted the .pdb first.
- **Memory:** every heavy command went through tools/capped.sh, one build at a time, no cap hits.

## Figures (effective 100%, against sweep25)

| | sweep25 | this wave | lost |
|---|---|---|---|
| PLAYER (W19De) | 12,944 | **13,033** (+89) | 0 |
| STUDIO (W19S2e) | 12,155 | **12,215** (+60) | 7, all no-.reloc coincidences (below) |

- **Shared-tree caveat:** the tree also held the network and shell agents' uncommitted work, so the totals include some of their gains (for example the Network job vftables, killPlayer and ServerReplicator).
- **STUDIO losses, each read with --verbose:**
  - isDenormal 0x00898bc0 and isNanInfDenormVector3: `and eax,0x7fffff` read as an address.
  - httpGetPostWinHttp 0x0091cd80 and RakNet IsOlderOrderedPacket 0x0065f060: an immediate equal to a GuiLabel vftable address.
  - luaK_setoneret, luaK_nil, luaK_posfix: the usual Lua masks.
- **Intermediate PLAYER links lost Players/Replicator/View functions.** Those came from the other agents' half-edited files; the final link W19De has none of them.

## Changes

### 1. Headers others needed (all landed)

- **Security::Impersonator / Context::ptr (SecurityContext.h/.cpp):**
  - 2013's `ptr()` has a plain `static thread_specific_ptr<Context> value;`: 0x0041ceb0 calls `heap_new<delete_data>` 0x004216a0. There is no tssCleanup (iOS 10-29 has neither), so I removed it. ptr's pairing went 0.18 → 0.45.
  - ptr, the Impersonator ctor and the dtor are now inline in the header; the ctor and dtor are `__declspec(noinline)`.
    - The noinline is a stand-in. With plain inline, Game.obj inlined the dtor into doClearVerbs, which dropped 1.0 → 0.71. 2013's engine always calls the shell copies.
  - **Shell stand-in experiment (relink with a /GL TU that uses Impersonator):** the shell then owns the copies, but LTCG inlines `release`/`reset` into them. 2013 calls its shell copies of those (0x0041e8a0 / 0x0041e8f0). The run lost 19 entries before regeneration and gained none, so I do not recommend the stand-in.
  - **Status:** the Impersonator markers stay STUB (0.31 / 0.20), and Players::onChildAdded stays at 99.28.
- **New STUB declarations and bodies (all at their own addresses):**

  | Function | PLAYER | STUDIO |
  |---|---|---|
  | FriendService::issueFriendRequestOrMakeFriendship(int,int) | 0x006b1d30 | 0x00810a30 |
  | FriendService::rejectFriendRequestOrBreakFriendship(int,int) | 0x006b2040 | 0x00810d40 |
  | Humanoid::setHealth(float) | 0x006db0e0 | 0x0081dd80 |
  | `Settings::saveState()` | 0x006576d0 | 0x0073fe60 |

  - **Settings::saveState:** 2013 has no GlobalSettings::saveState. The body reads Settings::settingsErased and settingsFile.
- **Game::globalExit:** written (`s_profanityFilter.reset()`, GLOBAL 0x00de68d0 / 0x00f8935c). FUNCTION in both targets (0x00653780 / 0x007cc240).
- **V8DataModel/SleepingJob.h + v8datamodel/SleepingJob.cpp (new):**
  - Layout: `DataModelJob`, then `tbb::atomic<bool> isAwake` +0x218, `desiredFps` +0x220, `Time lastWakeTime` +0x228. SIZE 0x230.
  - wake's `lock xchg byte` is TBB 2.1 fetchstore1; sleep() is a plain store.
  - FFlag::LastWakeTimeSleepingJobError gates lastWakeTime in wake and the faked stats in error. 2013 registers it in SleepingJob.obj, 0x00a72540.
  - The ctor, sleep, sleepTime, error and wake are FUNCTION in both targets. The vftable is 100 in PLAYER; STUDIO is 0.857, from the getDesiredConcurrencyCount twin (pairing only).
- **V8DataModel/TeleportCallback.h (new):** the 3-slot 2013 interface. The VTABLE marker stays with the shell's Teleporter.h.
- **Base rbx/atomic.h:** `OGRE_ATOMIC_CTOR_SPEC` on `atomic()` is `__declspec(noinline)` only under `OGRE_ATOMIC_CTOR_NOINLINE`, for the shell's Application.cpp.
  - It is opt-in because the 2013 engine inlines the ctor once (0x006c3d31).
  - Eight blank lines were reused, so the asserts at 182/224 keep their lines.
- **Base rbx/threadsafe.h:** head_waittime tests `!queue.empty()` first. PLAYER 0x005c5830 and 0x0063f200 are now 100.
- **Renames:**
  - **NetworkSettings `m_unk0x78` → `isQueueErrorComputed`: not applied.** It would break the network agent's ConcurrentRakPeer.cpp:293 mid-wave. The integrator applies NetworkSettings.h:54, NetworkSettings.cpp:91 and ConcurrentRakPeer.cpp:293 together.
  - **Game::m_unk0x30 stays unnamed.** The only user besides Game is iOS `-[ControlView …]`, which calls `signal<void(DataModel*)>::connect` (iOS 0x0004c1d8). No symbol, string or 2016 member names it.

### 2. Item 2

- **FFlag::DebugCrashEnabled** defaults true (`.data` byte 1 at 0x00d3ae00).
- **Bridge registerClassLibrary:** the 16 functions now carry markers at their 2013 addresses (ida-sync-21's names). All 16 are FUNCTION in both targets.
  - 10 of them were mispaired at 0.94 in each target before.
  - The 2013 address order (Color3, Ray, …, InputObject) is the order of first use in openState. The bodies are template COMDATs, so decomplint will report descents in LuaAtomicClasses.cpp. They are expected.
- **copyValue<ContentId>/<SoundId> fold: cause found, fix is the shell's.**
  - Since wave 18's UserInput.cpp, the shell instantiates `~ContentId`. ContentId's copyValue funclet then jumps to the shell copy (0x0041d920) and SoundId's to the native copy (0x006aa350). The handlers differ, so the bodies stop folding.
  - 2013 sends both funclets (and 263 others) to the shell thunk 0x0041c8a0.
  - Suggested fix (in the headers note): a shell stand-in that also instantiates `~SoundId`. Untested, because shell files are not mine.

### 3. Game

- **setupDataModel (PLAYER 0x00655630): FUNCTION.**
  - New V8DataModel/Commands.h (2013 path `Client\App\V8DataModel\Commands.cpp`):
    - CameraVerb (SIZE 0x1c: workspace +0xc, ServiceClient<Selection> +0x10) and its six inline-ctor Camera*Command subclasses. Their vftables are emitted by Game.obj, and their D0s fold into 0x00653a40 (100).
    - CameraCenter/ZoomExtents commands and ToggleViewMode.
  - New opaque V8DataModel/CommonVerbs.h (SIZE 0x724), with a STUB ctor in v8datamodel/CommonVerbs.cpp (0x00864450).
  - The ~CommonVerbs STUB is in Game.cpp (0x006531e0, 2013's Game.obj copy).
  - STUB bodies for the 21 Commands.cpp functions are in address order (they were an empty file before).
  - **Gained at 100 in PLAYER:** all 15 vftables (6 camera, CameraVerb, CameraCenter, ZoomExtents, ToggleViewMode, …), shared_ptr<CommonVerbs> reset and ctor.
  - **STUDIO 0x007cd6d0 stays STUB at 98.1:** its five push_back growth calls pair the folded `vector<int>::_Insert_n` twin instead of the EnumDescriptor::Item one (pairing only).
- **Size annotations:** SecurePlayerGame / UnsecuredStudioGame 0x34 (with per-target DECOMP_SIZE_ASSERT); LegacyLock::Implementation 0x20 (on a reused blank line, so DataModel.cpp's assert lines do not move); Lua::YieldingThreads 0x18 (openState news 0x18 at 0x00688db7).
- **Not done:**
  - shutdownGameDataModel 0x00654400 (EH frame).
  - STUDIO ~Game (99.07: two unfolded shutdownDM copies).
  - STUDIO Game::Game.

### 4. ScriptContext / YieldingThreads

- **WaitingThread has `Time startTime` / `Time resumeTime`** (the 2013 copy ctor 0x008a9080 moves dwords). The ctor zeroes resumeTime, then assigns startTime + timeout.
  - The ctor, copy ctor and nine heap/vector helpers rose from 0.37–0.76 to 100.
- **YieldingThreads::resume rewritten from the 2013 bytes:** FUNCTION in both targets (0x008a9bb0 / 0x009ed7d0).
  - Order: lock, then pop, then `if (thread)`.
  - `threadsResumedRate.sample()` goes through a local `ScriptContext*`, which gives 2013's `lea ecx,[eax+0x1e0]`; a direct member access gives `add ecx`.
- **openState (0.983):** probes of two variants (direct member read, a local reference) did not change the 14 register bytes. They come from register numbering across the whole function, not from the gc lines.

## Patterns proven

- **A function with a static local in a header can be the /GL shell's COMDAT for every engine caller** (Security::Context::ptr 0x0041ceb0). LTCG may still inline its callees where 2013 kept calls (`thread_specific_ptr::release`/`reset`).
- **VC9 forms `lea ecx,[eax+off]` instead of `mov ecx,[ecx]; add ecx,off`** when the base pointer comes from a named local, not a repeated member access (YieldingThreads::resume 0x008a9bb0).
- **A shell instantiation of a wrapper type's destructor splits otherwise identical engine bodies.** Their EH funclets reach different twins, so the handlers stop folding (copyValue<ContentId>/<SoundId>).

## Checks

- reccmp_addr_padding: 1,715 files, 0 bad.
- check_2016_copies: silent.
- check_folded: PLAYER 3,179 annotated, 0 problems; STUDIO 2,894, 0. None of the remaining "failed to find" lines are in my files.
- Every FUNCTION marker I added or flipped is at 100 in its target's final run: W19De (PLAYER) or W19S2e (STUDIO).
- Assert lines kept:
  - atomic.h 182/224 (blank lines reused).
  - DataModel.cpp (blank line reused).
  - No assert in LuaAtomicClasses.cpp, Commands.cpp, SleepingJob.cpp, Game.cpp, FriendService.cpp or Humanoid.cpp.
- decomplint was not run. Expected new descents: LuaAtomicClasses.cpp (registerClassLibrary, template COMDAT order) and Game.cpp (~CommonVerbs 0x006531e0 is first, which is ascending).

## Unresolved

1. **Integrator:**
   - Regenerate the shared CSVs.
   - Apply the NetworkSettings rename (3 lines, above).
   - CMake picks up the new App sources through its glob; reconfigure.
2. **Shell (shell-5k-10):**
   - Switch UserInput.h and Teleporter.h to App's SleepingJob.h and TeleportCallback.h.
   - Define OGRE_ATOMIC_CTOR_NOINLINE in Application.cpp.
   - Try the ~SoundId stand-in for the copyValue fold.
   - Do not add an Impersonator stand-in (measured −19).
3. **App, next:**
   - Why 2013's LTCG kept release/reset out of line in the Impersonator shell copies.
   - CommonVerbs members (62 verbs; ctor 0x00864450, dtor 0x006531e0).
   - Commands.cpp bodies laid out to the 2013 assert lines (116..1135).
   - openState's 14 register bytes.
   - shutdownGameDataModel.
   - STUDIO ~Game / Game::Game.
   - The DataModel/ScriptContext unnamed groups (not started).
4. **IDA owner (names):**

   | Address (PLAYER / STUDIO) | Name |
   |---|---|
   | 0x00653780 / 0x007cc240 | Game::globalExit |
   | 0x00de68d0 | s_profanityFilter |
   | 0x00df4e48 / 0x00f8d5b8 | FFlag::LastWakeTimeSleepingJobError |
   | 0x00653700 | CameraVerb::~CameraVerb |
   | 0x0041c8a0 | the shell `~basic_string` thunk |

   - SleepingJob ctor/sleep: 0x0072b000 / 0x0072b0e0.
   - The 0x00653a40 D0 folds six Camera*Command D0s and CameraVerb's.
   - SecurityContext: tssCleanup no longer exists.

## Files

- **App, modified:** humanoid/Humanoid.cpp, include/V8DataModel/{FriendService,Game,Humanoid,NetworkSettings,Settings}.h, include/script/ScriptEvent.h, include/security/SecurityContext.h, script/{LuaAtomicClasses,ScriptEvent}.cpp, security/SecurityContext.cpp, util/FastFlags.cpp, v8datamodel/{Commands,DataModel,FriendService,Game,Settings}.cpp.
- **App, new:** include/V8DataModel/{Commands,CommonVerbs,SleepingJob,TeleportCallback}.h, v8datamodel/{CommonVerbs,SleepingJob}.cpp.
- **Base:** include/rbx/atomic.h, include/rbx/threadsafe.h.
