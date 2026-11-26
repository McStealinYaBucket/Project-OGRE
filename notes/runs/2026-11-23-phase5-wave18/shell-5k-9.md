# phase5-wave18 shell-5k-9: UserInput, Application ctor, UploadSessionLogs, STUDIO bind stand-in

## Setup

- **Build dirs:** `build-sh/` (PLAYER) and `build-sh-studio/` (STUDIO), both rebuilt from HEAD f709dfd plus the shared tree.
  - Other agents' uncommitted edits were in the tree the whole time: DataModel, Game, LuaInstanceBridge, LuaMemory, Players, Player, Replicator, ConcurrentRakPeer, SecurityContext and ContentProvider.
- **Scripts:** in `build-sh/w18/`, copied from w17 and pointed at w18. New ones:
  - `d.py` disassembles 2013 with names;
  - `dd.py` dumps dwords (vftables);
  - `xr.py` scans for direct call sites;
  - `losscheck.sh`;
  - `abS.sh` is the STUDIO protocol against `rootS/`, a private root whose `tools/ida/gen_reccmp_csv.py` copy has `MATCH_DROP = set()`.
- **Method:** every A/B relinks one frozen object set and changes only my objects (`lk.sh`), as in w17.
  - The scoring protocol is ab.sh's (two runs, generate, two runs, generate, run `r`).
  - PLAYER final: `runs/I-r.json`, 15,663 entries, 12,778 at effective 100. Link J is the final source and has the same code as I: 22 bytes differ, all timestamps or the PDB GUID.
  - STUDIO: `runs/SA-r.json` (HEAD placeholder) against `runs/SB-r.json` / `runs/SC-r.json`.
- **Baseline caveat:** PLAYER's A could only be scored quickly (`runs/q-A-2.json`), with the HEAD shell sources swapped in for that one run.
  - Markers pair through live source lines, so other agents' edits move their own rows between any two runs.
  - Every Players/Player/Replicator/DataModel::LegacyLock/LuaAllocator/ObjectBridge/ContentId-vftable difference below is theirs. Their sources changed between my scoring runs.
- **What I did not touch:** the shared `reccmp/`, `ida/`, `Client/_generated`, `Client/Rendering`, CMakeLists.txt, CLAUDE.md and DECOMP_PLAN.md. I made no commits.
  - One exception: to measure, I swapped single marker lines for about 20 seconds and restored them right away with byte-exact copies.
    - `Client/_generated/player/stubs_00.cpp`: one marker line, for one quick run;
    - `Client/App/include/Util/Http.h`: one TEMPLATE line, for one quick run.
    - `git diff` is clean for both.
- **Memory:** heavy commands went through `tools/capped.sh`, one build at a time. There were no cap hits.

## Results (PLAYER, I-r against q-A-2; STUDIO SA-r against SB-r/SC-r)

| Function | Address | Before | Now | Marker |
|---|---|---|---|---|
| WndProc | 0x004426a0 | 76.4 | **100** | FUNCTION |
| Application::UploadSessionLogs | 0x00403f40 | 6.45 | **100** | FUNCTION |
| MainLogManager::MainLogManager | 0x0043fa50 | 88.63 | **100** | FUNCTION (PLAYER) |
| MainLogManager::CullLogs | 0x004403a0 | 78.26 | **100** | FUNCTION (PLAYER) |
| UserInput::centerCursor | 0x0045e200 | – | **100** | FUNCTION |
| RobloxCriticalSection::RobloxCriticalSection | 0x0045e190 | – | **100** | FUNCTION |
| safe_queue<Message>::push / deque::push_back | 0x00460ee0 / 0x00461700 | – | **100** | TEMPLATE (UserInput.h) |
| RunningAverageTimeInterval<1>::sample (shell copy) | 0x00460e50 | 54.74 | **100** | (row) |
| UserInput, TeleportCallback, Teleporter vftables | 0x00be4d9c, 0x00bd93ac, 0x00bd93bc | – | **100** | VTABLE |
| Teleporter D0, thread_data<bind<Application>> ×2 + vftable, CComModule ctor, `~signal<void()>` | 0x004022b0 … | – / 78.65 | **100** | (rows) |
| View::HandleWindowsMessage | 0x00466510 | 95.4 | 98.85 (**100** once the generated stub goes) | STUB |
| UserInput::PostUserInputMessage | 0x0045f1b0 | stub | 95.65 (only SleepingJob::wake unresolved) | STUB |
| WinMain | 0x00442900 | 52.7 | 72.4 | STUB |
| Application::Application | 0x00403000 | 1.71 | 50.5 | STUB |
| UserInput::UserInput | 0x0045e300 | – | 42.7 | STUB |
| View::initializeSizes | 0x004678e0 | 74.2 | 74.2 | STUB |
| STUDIO bind / list1<weak_ptr<DataModel>> | 0x0042b260 / 0x0042e370 | unpaired | **100** (engine 0x0098a310 / 0x0098a120 stay 100) | (rows) |

### Losses

- **No-.reloc coincidences:**
  - `luaK_nil` / `setreturns` / `setoneret` / `posfix`, `constructor` 0x0098f9a0, `Math::isDenormal`, `isNanInfDenormVector3`: Lua's `and r,0x7fffff` against a newly paired original near 0x00800000.
  - `httpGetPostWinHttp` and `IsOlderOrderedPacket`: an immediate read as the NotificationBox vftable.
  - I read each one with `--verbose` (`losscheck.sh`) on link B. All of them are back at 100 in I.
- **Losses from my change:** none found.

## 1. UserInput (new `Client/WindowsClient/UserInput.{h,cpp}`)

- **The class:**
  - `UserInput : UserInputBase` (App's `Util/UserInputBase.h`).
  - SIZE 0x2f8. The full layout comes from the ctor 0x0045e300, against 2016:
    - 2013 has no steppedConnection, inputObjectMap, sdlGameController or previousCursorPosFraction;
    - +0x38 is a `shared_ptr<UserInputSleepingJob>` (RTTI);
    - `game` is at +0x2ec.
  - The vftable has 8 slots and no destructor slot. All slots are declared with their addresses.
- **PostUserInputMessage 0x0045f1b0:**
  - On WM_MOUSEMOVE it samples `mouseStats.osMouseMove`.
  - It then queues `{uMsg, wParam, lParam}` on the job's `safe_queue` at +0x238 and wakes the job (`SleepingJob::wake` 0x0072b220).
  - 2016 handles the message on the spot instead.
  - As a shell (LTCG) body it gets 2013's convention: `this` on the stack, uMsg in ecx. That alone took WndProc to 100 and HandleWindowsMessage to 98.85.
  - The one remaining HandleWindowsMessage diff is its call pairing with the generated `ogre_stub_0045f1b0`, whose marker duplicates mine. With that marker line masked (one run, then restored), HandleWindowsMessage scores **100** and PostUserInputMessage **95.65**. Its only diffs are then the unresolved `wake` and the EH table.
- **The ctor 0x0045e300** is written in full: members in declaration order, LoadCursor ×2, `setGame(game)`, the diKeys memset, `DirectInput8Create` + `AtlThrow`, the SwapMouseButtons CRegKey read, the create* calls, and GetKeyboardLayout.
  - It scores 42.7 only because `this` is in esi in 2013 (an LTCG convention its caller, View, chooses) and on the stack in ours. View's caller is not written.
  - Also: 2013 calls `CRegKey::Open` 0x0041aee0 out of line where we inline it, and DirectInput8Create is unresolved.
  - It emits the vftable, which puts `centerCursor` (slot 3) back to `__thiscall`: 80 → 100.
- **Stubs:** setGame 0x0045e630, createMouse/createKeyboard/createAccelerators, removeJobs, and four slots.
  - Their bodies touch a member, so that LTCG keeps `this`.
- **Not in the tree:**
  - DirectX SDK: `IDirectInput8A`, `IDirectInputDevice8A`, `DirectInput8Create` and `IID_IDirectInput8A` are declared minimally in UserInput.h.
  - App's `SleepingJob`: a one-member declaration in UserInput.h.

## 2. Application (ctor, UploadSessionLogs) and Teleporter

- **Layout:** SIZE 0x140, proven member by member from the ctor 0x00403000.
  - 2016 names where the slot is initialised the same way; there is no mainWindow.
  - `ATL::CEvent` at 0x18, `logsCleanUpThread` at 0x1c, the 2016 launchMode slot at 0x20, `scoped_ptr<Document>` at 0x24 (Shutdown 0x00404f10 calls Document 0x0041d270, then deletes it), `po::variables_map` at 0x28.
  - Three strings, two bools, MainLogManager at 0xac, dumpErrorUploader / processPerfCounter / profanityFilter, the two threads, Teleporter at 0x12c, marshaller at 0x138, spoofMD5 at 0x13c.
  - The EH state numbers match 2013's.
- **The ctor (50.5):** it is otherwise identical to 2013.
  - LTCG inlines `rbx::atomic<int>()`, where 2013 calls the shell copy 0x004022d0 (26 callers in 2013, 3 of them in the shell; the other two are in the unnamed 0x0040eb40, malloc + two atomics). That shifts ebx/edi everywhere.
  - Fixing this needs a Base change or more shell code.
  - Constructing MainLogManager with constant arguments made LTCG fold them into its ctor as 2013 did: **0x0043fa50 and CullLogs 0x004403a0 → 100**. Both are flipped to FUNCTION (PLAYER only; STUDIO is unchanged at 61.0 / 78.3).
- **UploadSessionLogs (100):**
  - first a non-virtual RbxOgre call 0x00487d50 on the gfx view, declared as `ViewBase::FUN_00487d50`, unresolved;
  - then both MessageBoxes on the View's hWnd. 2016 uses mainWindow, which 2013 lacks.
  - It loads appPtr itself, as in 2013.
- **LoadAppSettings / ParseArguments stubs** now use their argument, so WinMain keeps 2013's pushes: 60.4 → 71.9.
  - What remains in WinMain:
    - 2013 calls the CComModule dtor 0x00442b40 out of line;
    - the frame order (comModule at ebp-0x64, MSG at -0x34) differs. Moving `MSG msg` changes nothing;
    - Initialize takes hWnd in edx in 2013;
    - an `0xcf0000` coincidence.
- **Teleporter (new `Teleporter.{h,cpp}`):**
  - `TeleportCallback` (vftable 0x00bd93ac: virtual dtor plus two pure slots) and `Teleporter` (0x00bd93bc, SIZE 0xc).
  - doTeleport 0x0045d450 is a STUB.
  - Both vftables and the shared deleting dtor 0x004022b0 are at 100.

## 3. STUDIO

- **The `bind<weak_ptr<DataModel>>` stand-in, in `Client/ClientShared/placeholder.cpp`:**
  - The shell's 0x0042b260 / 0x0042e370 pair at **100**.
  - The engine 0x0098a310 / 0x0098a120 stay at 100 with MATCH_DROP emptied (SB-r: +2, no loss).
  - **The generator's `MATCH_DROP` can be retired** once this file lands.
- **`list<rbx::signals::connection>::_Buynode`: measured and left out.**
  - xr.py on STUDIO: only InstancePacketCache's ctor (0x0066922d) and two shell sites call the shell copy 0x0049de90. Eleven engine sites call 0x006d13d0, which is the engine fold of every 4-byte-element list's `_Buynode()`.
  - Instantiating list<connection> in the shell (link SB):
    - 0x0049de90 pairs (+1);
    - Http.h's STUDIO TEMPLATE for 0x006d13d0 then names our shell copy, so 0x006d13d0 and three engine callers lose their pair (list<DialogWrapper*> ctor 0x00859be0, the GuiService map 0x0085c890, iostreams chain_base 0x0068db20): −4;
    - net −3.
  - InstancePacketCache 95.59 → 97.06. Its remaining diff is the prime_list `+end` / "circular_buffer" coincidence, so it cannot reach 100 either way.
  - Doing it right needs an App header edit (Unresolved 4).
- **Link SC (bind only, the file as committed) against SA, protocol `r` with MATCH_DROP emptied:**
  - +2 in this scope (0x0042b260 and 0x0042e370 at 100), no losses.
  - Against SB, it gets back 0x006d13d0, 0x0068db20, 0x00859be0 and 0x0085c890, and gives up 0x0049de90.
  - SC-r: 15,113 entries, 12,028 at 100. SA-r: 15,141 / 12,052. The difference is other agents' Players / Replicator / LegacyLock source edits between the runs; filtered out of the comparison, it is +2 / −0.

## 4. The boost::function inlining (task 4)

- Re-measured on link I, after this wave's shell code (UserInput, Application ctor, Teleporter, the new ATL and boost::thread use): **unchanged**.
  - Submit 0x00437500: 75.34.
  - RecordToggleVerb::stopAction 0x00439450: 88.89.
  - ScreenshotVerb ctor 0x004382f0: 75.89.
- The added LTCG code did not move the inlining budget for `assign_to_own` / `assign_to`.

## 5. initializeSizes (not fixed)

- One probe: keeping `currentDisplaySize` as two ints so its slot dies early. The ratio fell from 0.754 to 0.642.
- 2013 reuses currentDisplaySize's slot for defaultSize ([esp+0x1c]) and keeps fullscreenSize in ebx (spill slot 0x18). Ours swaps the two slots.
- Variable declaration order and int locals do not reproduce it.

## Checks

- `python3 -I tools/check_2016_copies.py`: silent.
- `reccmp_addr_padding.py`: 1,708 files, 0 bad.
- `reccmp-decomplint --module PLAYER --warnfail`: Client/WindowsClient, Client/Win and Client/ClientShared are clean.
- PLAYER: build-sh builds with ninja, and the new TUs compile with the player_client command (`cx.sh`). STUDIO: build-sh-studio builds (`--target studio`).
- View.cpp is untouched, so its assert stays on line 739.
- Init lists are written in declaration order (the 2013 store order), and no 2016 comment text is kept.

## Files changed

- **New:**
  - `Client/WindowsClient/UserInput.h` and `UserInput.cpp`;
  - `Client/WindowsClient/Teleporter.h` and `Teleporter.cpp`.
- **`Client/WindowsClient/Application.h` / `.cpp`:**
  - the full layout;
  - the ctor (STUB) and the logsCleanUpHelper stub;
  - UploadSessionLogs (FUNCTION);
  - the stubs now use their arguments.
- **`Client/WindowsClient/View.h`:** includes UserInput.h (the stand-in class is gone) and declares `ViewBase::FUN_00487d50`.
- **`Client/WindowsClient/main.cpp`:** WndProc is FUNCTION.
- **`Client/Win/LogManager.cpp`:** PLAYER markers for the MainLogManager ctor and CullLogs are FUNCTION.
- **`Client/ClientShared/placeholder.cpp`:** the STUDIO bind<weak_ptr<DataModel>> stand-in.

## Unresolved

1. **Integrator, CMakeLists.txt:**
   - Add `Client/WindowsClient/UserInput.cpp` and `Client/WindowsClient/Teleporter.cpp` to `player_client`. I linked them after FunctionMarshaller.cpp and RenderSettingsItem.cpp; order inside the LTCG set did not matter.
   - Without them, WndProc, centerCursor and the critical-section ctor lose their 100, and Teleporter's vftable references an unresolved doTeleport.
   - Then rerun `tools/gen_link_stubs.py`. That drops `ogre_stub_0045f1b0` and its duplicate marker, and View::HandleWindowsMessage 0x00466510 can go FUNCTION (measured 100 with the stub's marker masked).
   - Unresolved after this wave:
     - `RBX::SleepingJob::wake`;
     - `ViewBase::FUN_00487d50`;
     - `DirectInput8Create` and `IID_IDirectInput8A` (no dinput8.lib or dxguid.lib in the toolchain).
2. **Generator owner:** empty `MATCH_DROP` once `Client/ClientShared/placeholder.cpp` lands. SB-r: 0x0042b260 / 0x0042e370 at 100; the engine twins stay at 100.
3. **IDA owner, PLAYER names:**
   - 0x0072b220 `?wake@SleepingJob@RBX@@QAEXXZ`. gen_link_stubs can then bind it, and PostUserInputMessage reaches 100.
   - 0x00460ee0 / 0x00461700: the safe_queue / deque names in UserInput.h's TEMPLATE markers.
   - 0x0045e300, 0x0045e630, 0x004627b0 (`UserInputSleepingJob` ctor).
4. **App owner (STUDIO `_Buynode`):** to give InstancePacketCache its shell callee:
   - instantiate `std::list<rbx::signals::connection>` in a STUDIO shell TU (one line in ClientShared/placeholder.cpp);
   - move Http.h's `TEMPLATE: STUDIO 0x006d13d0` to 0x0049de90;
   - name 0x006d13d0 by an engine 4-byte list, e.g. `?_Buynode@?$list@PAUDialogWrapper@GuiService@RBX@@…@XZ` in GuiService.h. That TEMPLATE must sit in a header: my one-run test placing it in a .cpp did not pair.
5. **App owner:**
   - `GlobalSettings::saveState` 0x006576d0 and `Game::globalExit` 0x00653780 must be declared before Application::Shutdown 0x00404f10 can be written. It also needs Document::Shutdown 0x0041d270 and ~Document 0x0041d020 (shell) and FunctionMarshaller::ReleaseWindow 0x00437180.
   - App's `v8datamodel/SleepingJob.h` and `TeleportCallback.h`: UserInput.h and Teleporter.h carry minimal stand-ins until they exist.
6. **Base owner:** `rbx::atomic<int>()` needs to stay out of line in the LTCG shell (2013 calls 0x004022d0 from the Application ctor). Without that, the Application ctor stays at 50.5.
7. **Rendering contributor:** the RbxOgre function 0x00487d50 (UploadSessionLogs' call on the gfx view) and the real `ViewBase`.
8. **Shell, next:**
   - **View::initializeInput 0x00466840**, the only caller of the UserInput ctor (callers 0x0041dcbb, 0x00466e18). It:
     - does `new UserInput(hWnd [View+0xc], this, game [View+0x4])` with the object in esi;
     - resets View+0xb0, calling the non-virtual ~UserInput 0x0045e990 (`this` pushed; to declare and STUB);
     - takes `LegacyLock(game->getDataModel(), Write)` (0x006421c0);
     - finds ControllerService (0x00468f30) and stores the UserInput in its +0x64;
     - then handles the overlay data model (Game 0x0041ce60).
     
     Writing it gives the UserInput ctor its esi convention. It must take its lines from View.cpp's blank gap so the assert stays on 739. After that: setGame / createMouse / createKeyboard / createAccelerators;
   - Shutdown (item 5);
   - the 0x0040eb40 allocator (two atomics);
   - the CComModule out-of-line dtor in WinMain.
