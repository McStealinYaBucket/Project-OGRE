# phase5-wave19 shell-5k-10: UserInput dtor/devices/job, initializeInput, Shutdown, Application ctor, DirectInput libs

## Setup

- **Build dir:** `build-sh/` (PLAYER), rebuilt from HEAD 826ad6e plus the shared tree.
  - Other agents' uncommitted edits were in the tree the whole time: Players, Replicator, FriendService, Game, SecurityContext, Humanoid, Settings, LuaAtomicClasses, and App's new SleepingJob.h and TeleportCallback.h.
  - One of my builds failed at the link with LNK2005 (SecurityContext changes landed between compiles). A rebuild fixed it.
- **Scripts:** in `build-sh/w19/`, copied from w18 and pointed at w19. New ones:
  - `lkB.sh` (relink with probe objects, plus `dx` for the DirectX libraries);
  - `cxo.sh` / `cxpo.sh` (compile with a private `ovr/` include directory first and `-DOGRE_APP_INPUT_ACCESSORS`);
  - `cxa.sh` (compile one shell TU with `-DOGRE_ATOMIC_CTOR_NOINLINE`);
  - `vbp.sh` (`--verbose` against the private CSVs, diff lines only).
  - `lk.sh` takes `EXTRALIBS`.
  - Fix in `cxo.sh`: the ninja commands use `-I`, not `/I`, so w18's ` /I` substitution never applied.
- **Private reccmp root:** `build-sh/w19/root/`. Its `reccmp-project.yml` adds `reccmp/player-dx.csv`, five DirectInput data rows: IID_IDirectInput8A 0x00bc3628, GUID_SysKeyboard 0x00bc35c8, GUID_SysMouse 0x00bc35d8, c_dfDIMouse2 0x00bc3824, c_dfDIKeyboard 0x00bc3a2c.
- **Links:**
  - **D** is the commit candidate: build-sh objects plus `Document.obj`. Scored with the full protocol (`ab.sh D P` → `runs/D-r.json`: 15,867 entries, 12,936 at effective 100).
  - **E** = D plus the DirectX libraries.
  - **C** = App-accessor probe.
  - **H** / **I** = Teleporter and atomic probes. I has `-DOGRE_ATOMIC_CTOR_NOINLINE` on all 16 shell TUs: 12,937.
  - **Fv1..Fv4** = initializeSizes probes.
- **Baseline:** sweep25 (`build/sweep25/P-final.json`).
- **What I did not touch:** the shared `reccmp/`, `ida/`, `Client/_generated`, `Client/Rendering`, App/Base headers, CMakeLists.txt, CLAUDE.md and DECOMP_PLAN.md. No commits.
- **Memory:** every heavy command went through `tools/capped.sh`, one build or link at a time. No cap hits.

## Results (PLAYER, sweep25 → D-r unless noted)

| Function | Address | Before | Now | Marker |
|---|---|---|---|---|
| RobloxCriticalSection::~RobloxCriticalSection | 0x0045e5c0 | unpaired | **100** | FUNCTION |
| UserInput::removeJobs | 0x0045e900 | 0 | **100** | FUNCTION |
| UserInputSleepingJob D0 / vftable | 0x004628d0 / 0x00be4e18 | – | **100** | SYNTHETIC / VTABLE |
| UserInput::createMouse / createKeyboard | 0x0045ec60 / 0x0045ecf0 | 4.2 | 97.8; **100** with the DX libraries (E) | STUB |
| UserInput::~UserInput | 0x0045e990 | – | 93.5 | STUB |
| UserInputSleepingJob ctor | 0x004627b0 | – | 81.3 (98.9 in probe C, EH table aside) | STUB |
| UserInput::createAccelerators | 0x0045ed80 | 6.7 | 54.1 (link H) | STUB |
| Application::Shutdown | 0x00404f10 | 1.7 | 73.3 | STUB |
| View::initializeInput | 0x00466840 | – | 27.5 (stand-in body); 50.2 written (probe C) | STUB |
| UserInput::setGame | 0x0045e630 | 22.9 | 22.7 (stand-in); 89.4 written (probe C) | STUB |
| Application::Application | 0x00403000 | 50.5 | 50.5; **96.7** with the shell-wide atomic define (I) | STUB |
| intrusive_ptr_target<islot>::operator new | 0x0040eb40 | 66.7 | **100** with the shell-wide define (I) | (row) |
| TeleportCallback vftable (App header now) | 0x00bd93ac | 100 | 100 (H, I) | VTABLE |
| UserInput ctor | 0x0045e300 | 42.7 | 46.5 | STUB |

**Losses:** none from my changes. I compared every sweep25 100 against D-r and against H/I. Each one is another agent's live source:

- Players.cpp, LuaAtomicClasses.cpp and FriendService lines moved after my build (reccmp printed "Failed to find function symbol" for their markers).
- weak_ptr::lock twins are Network's.
- NullVerb D0 and vftable are Game.cpp's.
- DataModelJob D0 0x006f06b0 is App's new SleepingJob D0 fold.
- The thread_specific_ptr delete_data vftables are SecurityContext's.
- The Lua `0x7fffff` and no-.reloc coincidences: isDenormal, luaK_*, exp2reg.

I read the doClearVerbs, GcJob step, DataModelJob D0 and thread_specific_ptr ones with `--verbose`. H→I: +1, −0. D→E: +2 (createMouse/createKeyboard).

## 1. UserInput (task 1)

- **~UserInput 0x0045e990.** 2013 asserts `diMousePtr`, `diKeyboardPtr` and `diPtr` at `Client\WindowsClient\UserInput.cpp` lines 148–150. So the file path is now proven 2013 (header comments updated). The asserts sit on 148–150 by layout: a blank gap after removeJobs.
  - Then Unacquire + `= NULL` for each device. VC9's `CComPtr::operator=(T*)` tests `*this != lp`, which gives 2013's reload-and-test.
  - 93.5% left: 2013's frame has one more dword (`push ecx`), so every offset is +4. The same happens in the probe.
  - The DirectInput device interface now declares its slots up to SetDataFormat (vtable+0x2c), in dinput.h order.
- **removeJobs 0x0045e900 (100).** `if (job) { TaskScheduler::singleton().removeBlocking(job); job.reset(); }`. The shell copy of removeBlocking is 0x0045e220.
- **~RobloxCriticalSection 0x0045e5c0 (100).** It is user-declared and out of line (LTCG: `this` pushed).
- **createMouse / createKeyboard.** CreateDevice(GUID_SysMouse / GUID_SysKeyboard), SetDataFormat(c_dfDIMouse2 / c_dfDIKeyboard), then SetProperty(DIPROP_BUFFERSIZE, 0x800). Each failure goes through AtlThrow.
  - The code is byte-identical to 2013. 100% needs only the SDK data symbols resolved and paired (link E plus the five rows).
- **createAccelerators 0x0045ed80 (54.1).** LoadAccelerators(255), CopyAcceleratorTable to count, `resize`, then copy.
  - 2013 keeps `this` on the stack (it reloads `[esp+0x14]` twice); ours keeps it in ebp. Not solved.
- **UserInputSleepingJob now derives App's real SleepingJob** (the stand-in is gone; SIZE 0x260; vftable 0x00be4e18).
  - The ctor is `SleepingJob("UserInputSleeping", Write, false, arbiter, Interval(0.01), 60.0)`, userInput at +0x234 (m_unk0x230 is not written).
  - stepDataModelJob 0x00462920 is a STUB.
  - The ctor's 81.3 is an EH slot for the by-value arbiter that only setGame's real call creates. With the written setGame (probe C) it scores 98.9, where the only diff is the EH table pointer, which the protocol pairs.
- **setGame 0x0045e630** is written. It assigns `game`, sets `runService = shared_from(create<RunService>(dm))`, runs the job on the overlay DataModel when there is one, and calls `TaskScheduler::add`. The real job's ctor takes `shared_ptr<DataModelArbiter>` (DataModel+0xa8).
  - The body needs `Game::getOverlayDataModel()`, which App lacks, so it is compiled only under `OGRE_APP_INPUT_ACCESSORS`. The `#else` branch is the old stand-in. The marker sits on the signature; a marker above an `#ifdef` does not pair, as measured in link A.
- **View::initializeInput 0x00466840** is written. It does `userInput.reset(new UserInput(hWnd, this, game))` with no `if (userInput)` test. Then, under `LegacyLock(game->getDataModel(), Write)`, it calls `ServiceProvider::create<ControllerService>(dm)->setHardwareDevice(userInput)` (+0x64), and does the same for the overlay DataModel if one exists.
  - View.h gains `shared_ptr<Game> game` at +0x4 (proven by the ctor-argument copy) and the declaration.
  - It needs Game::getOverlayDataModel and ControllerService::setHardwareDevice (Unresolved 2). The tree compiles a stand-in (the `new UserInput` line plus `STUB`).
  - View.cpp's blank gap gave up the lines, so the assert is still on 739.
- **The UserInput ctor convention is not fixed.**
  - LTCG drops initializeInput when it has no caller. In 2013 its callers are View 0x00466cf0 and Document 0x0041d960, and neither is written.
  - With two probe stand-in callers (link C) it is emitted at 50.2%. But LTCG still passes the UserInput object on the stack and View in esi, where 2013 has the object in esi and View in edi. The ctor stays at 45–47%.
  - The real callers (0x00466cf0 creates the DataModel through Game::setDataModel) decide it.

## 2. App headers (task 2; app-19-headers.md)

- **SleepingJob.h:** used (see 1).
- **TeleportCallback.h:** Teleporter.h now includes it. The stand-in class is gone.
  - The VTABLE marker stays in Teleporter.h above the `#include`, with a `// class RBX::TeleportCallback` line. 0x00bd93ac stays at 100 (H, I).
- **Application::Shutdown 0x00404f10 (73.3):**
  1. SetEvent(stopWaitForVideoPreroll);
  2. join and reset the +0x128 thread;
  3. `Document::Shutdown()`, then delete the Document;
  4. GlobalBasicSettings and GlobalAdvancedSettings `singleton()->saveState()`;
  5. `Game::globalExit()`;
  6. `FunctionMarshaller::ReleaseWindow(marshaller)` (static, marshaller in esi).

  - New stubs: `Document::~Document` 0x0041d020 and `Document::Shutdown` 0x0041d270 (new `Client/WindowsClient/Document.cpp`), and `ReleaseWindow` 0x00437180.
  - The remaining diff is 2013's `and esp,-8` frame. Document::Shutdown's real body has an `and esp,-64` frame, and LTCG propagates the alignment. So Shutdown waits on Document::Shutdown.
- **Application ctor and `rbx::atomic<int>()`:**
  - Defining `OGRE_ATOMIC_CTOR_NOINLINE` in Application.cpp alone does nothing (H: still inlined). LTCG keeps the inline definition from the other shell TUs.
  - With the define on **all 16 player_client TUs** (link I), the ctor calls `atomic()` out of line as 2013 does: 50.5 → **96.7**. 0x0040eb40 (malloc plus two atomics) → **100**. Nothing is lost.
  - The rest of the ctor is three unpaired callees: CEvent ctor 0x0041b060, variables_map ctor 0x0047b790, and the boost::thread ctor 0x0040ca30.
  - The single-TU define is not left in the tree.
- **WinMain 0x00442900 (72.6):**
  - 2013 calls `~CComModule` 0x00442b40 out of line from four exits: LoadAppSettings and Initialize failure, which share a block; ParseArguments and !hWnd, which share a block; the catch continuation; the normal exit.
  - Ours calls it from three exits and inlines it on the LoadAppSettings failure.
  - The rest is the callees' LTCG conventions (ParseArguments `this` pushed, Initialize hWnd in edx, Shutdown `this` in ecx). Those follow the real bodies of Application's LoadAppSettings, ParseArguments and Initialize, which are not written.

## 3. initializeSizes and boost::function (task 3)

- **initializeSizes stays at 74.2.** 2013 puts defaultSize in currentDisplaySize's slot ([esp+0x1c]) and keeps fullscreenSize in ebx, loading defaultSize into edi before the branch. I tried four variants (`probe/View_v1..v4.cpp`, links Fv1..Fv4) and disassembled them:
  - v1: int locals for the current size;
  - v2: 2016's `fullscreenSize, windowSize` declared together;
  - v3: `fullscreenSize = defaultSize` first, then override when not Auto. This puts defaultSize in edi (`cmp di,cx`) as 2013 does, but the slots stay swapped and the copy moves before the branch;
  - v4: currentDisplaySize scoped in a block. This is worse.

  None moves defaultSize into 0x1c, so View.cpp is unchanged.
- **boost::function inlining, re-measured on D-r: unchanged.** Submit 75.34, stopAction 88.89, ScreenshotVerb ctor 75.89.

## 4. DirectInput8Create / IID_IDirectInput8A (task 4)

- **What is missing:**
  - `3rdparty/` has no DirectX SDK.
  - The wine prefix has only the runtime DLLs (`dinput8.dll`), no import or static libraries.
  - The Windows SDK v6.0A in the toolchain has no dinput8.lib or dxguid.lib.
- **Declaring the GUID in source is not enough.** createMouse/createKeyboard also need `c_dfDIMouse2` and `c_dfDIKeyboard`: dinput8.lib's static data objects (dilib5.obj, dilib2.obj), full DIDATAFORMAT tables that point at GUID_XAxis and the others. DirectInput8Create (thunk 0x009f09da → IAT 0x00aa1070) needs dinput8.lib's import part. 2013's .rdata shows the dxguid layout too: GUID_XAxis sits at 0x00bc3618, just before IID_IDirectInput8A.
- **A copy of the DirectX SDK (November 2007) libraries** is on this machine, outside the repository: `~/finobe2007/Decompiled 2007 ROBLOX source/3rdparty/Microsoft DirectX SDK (November 2007)/Lib/x86/{dinput8.lib,dxguid.lib}` and `Include/dinput.h`. It also has an October 2004 SDK, which has dxguid.lib but no dinput8.lib.
  - I used it read-only in private links only. Linked, everything resolves: `_DirectInput8Create@20` from DINPUT8.dll, `c_dfDIKeyboard` from dilib2.obj, `c_dfDIMouse2` from dilib5.obj. createMouse and createKeyboard then score 100.
  - I copied nothing into the repository. Whether to vendor these two libraries (and dinput.h, which would replace UserInput.h's hand declarations) is the user's call.

## Checks

- `python3 -I tools/check_2016_copies.py`: silent.
- `reccmp_addr_padding.py`: 1,715 files, 0 bad.
- `reccmp-decomplint Client/WindowsClient Client/Win Client/ClientShared Client/ClientBase --module PLAYER --warnfail`: clean.
- Every changed TU compiles with its player_client command after the last edit: UserInput, View, Application, FunctionMarshaller, Document, Teleporter, main and GameVerbs.
- Assert lines: View.cpp 739 and UserInput.cpp 148–150.
- STUDIO: no STUDIO-side file was changed.

## Files changed

- **New:** `Client/WindowsClient/Document.cpp` (stubs for ~Document and Shutdown).
- `Client/WindowsClient/UserInput.h` / `.cpp`:
  - DirectInput declarations;
  - UserInputSleepingJob on App's SleepingJob;
  - ~RobloxCriticalSection, ~UserInput, removeJobs, createMouse, createKeyboard, createAccelerators;
  - setGame (written body under the accessor macro);
  - the job ctor and stepDataModelJob stub.
- `Client/WindowsClient/View.h` / `.cpp`: `game` at +0x4, initializeInput.
- `Client/WindowsClient/Application.cpp`: Shutdown.
- `Client/WindowsClient/Document.h`: Shutdown and ~Document declarations.
- `Client/WindowsClient/FunctionMarshaller.h` / `.cpp`: the ReleaseWindow stub.
- `Client/WindowsClient/Teleporter.h`: App's TeleportCallback.h.

## Unresolved

1. **Integrator, CMakeLists.txt:**
   - Add `Client/WindowsClient/Document.cpp` to `player_client`, anywhere in the shell set (I linked it after Application.cpp). Without it, Application::Shutdown's calls to Document::Shutdown and ~Document are unresolved.
   - Add the compile definition `OGRE_ATOMIC_CTOR_NOINLINE` to `player_client` (all its TUs). Measured in link I: Application ctor 50.5 → 96.7 and 0x0040eb40 → 100, no losses.
   - Then rerun `tools/gen_link_stubs.py` (delete the exe first).
2. **App owner (two inline accessors, the only blockers for initializeInput and setGame):**
   - `Game.h`: `boost::shared_ptr<OverlayDataModel> getOverlayDataModel() const { return overlayDataModel; }`. 2013's /GL shell keeps a copy at 0x0041ce60 (hidden return slot pushed, `ret 4`).
   - `UserController.h`: ControllerService +0x64 `UserInputBase* hardwareDevice` (now `m_unk0x64`) with `void setHardwareDevice(UserInputBase* value) { hardwareDevice = value; }` (2016 names).
   - Once both exist, delete the `#ifdef OGRE_APP_INPUT_ACCESSORS` / `#else` stand-ins in View.cpp and UserInput.cpp. Keep each assert line: View.cpp 739, UserInput.cpp 148.
   - Caution from probe C: the written setGame and initializeInput make the shell instantiate `shared_ptr<RunService>::operator=` and a `sp_counted_impl_pd<RunService*>` vftable. Before regeneration, these took the pairs of 0x00469520 (used by about 50 engine `ServiceProvider::find<X>`) and of 22 `sp_counted_impl_pd` vftables' `get_untyped_deleter` slot. Measure A/B with the full protocol when the accessors land.
3. **User / integrator, DirectX SDK:**
   - Vendor `dinput8.lib` and `dxguid.lib` (DirectX SDK, e.g. November 2007; the copy on this machine is listed in section 4) and link them into `player`.
   - Generator rows for the five DirectInput data symbols above. Then createMouse and createKeyboard go FUNCTION (measured 100).
4. **IDA owner, PLAYER names:**
   - 0x0045e5c0 `??1RobloxCriticalSection@RBX@@QAE@XZ`; 0x0045e900 `?removeJobs@UserInput@RBX@@UAEXXZ`; 0x0045e990 `??1UserInput@RBX@@QAE@XZ`; 0x004627b0 `??0UserInputSleepingJob@RBX@@QAE@PAVUserInput@1@V?$shared_ptr@VDataModelArbiter@RBX@@@boost@@@Z`; 0x00462920 `stepDataModelJob`.
   - 0x0041d020 / 0x0041d270 Document dtor / Shutdown; 0x00437180 `?ReleaseWindow@FunctionMarshaller@RBX@@SAXPAV12@@Z`; 0x00466840 `?initializeInput@View@RBX@@AAEXXZ`; 0x0045e220 the shell's `TaskScheduler::removeBlocking` copy; 0x0041ce60 `Game::getOverlayDataModel`; 0x00468f30 the shell's `ServiceProvider::create<ControllerService>`.
   - For the Application ctor's three callees: 0x0041b060 `??0CEvent@ATL@@QAE@HH@Z`, 0x0047b790 `??0variables_map@program_options@boost@@QAE@XZ`, and 0x0040ca30 boost::thread's `bind_t<…Application…>` ctor (`??$?0V?$bind_t@XV?$mf0@XVApplication@RBX@@@_mfi@boost@@V?$list1@V?$value@PAVApplication@RBX@@@_bi@boost@@@_bi@3@@_bi@boost@@@thread@boost@@QAE@V?$bind_t@…@PAUdummy@01@@Z`).
   - DirectInput data: 0x00bc3628, 0x00bc35c8, 0x00bc35d8, 0x00bc3824, 0x00bc3a2c (names above).
5. **Shell, next:**
   - Document 0x0041d960 and View 0x00466cf0, the real callers of initializeInput, which set the UserInput ctor's esi convention;
   - Document::Shutdown 0x0041d270 (64-byte aligned frame; it unblocks Shutdown's alignment) and ~Document 0x0041d020;
   - FunctionMarshaller::ReleaseWindow 0x00437180 (static mutex via call_once 0x00436ce0, refcount at +0x4c, window map erase 0x004377e0, DestroyWindow);
   - UserInputSleepingJob::stepDataModelJob 0x00462920;
   - ~UserInput's extra frame dword;
   - createAccelerators' register choice;
   - the remaining UserInput.cpp asserts: 269, 300, 340, 479, 636, 775, 781 and 804 (acquire/unacquire functions).
6. **App owner (from app-19's note, not done):** the SoundId destructor stand-in for the ContentId/SoundId copyValue fold (vftables 0x00b3388c / 0x00b33904). Not tried this wave for lack of time.
