# phase5-wave21 shell-5k-12: UserInput acquire code, Document::Shutdown, View DataModel restart, STUDIO shared_from stand-in, Document/View sizes

## Setup

- **Build dirs:** `build-sh/` (PLAYER) and `build-sh-studio/` (STUDIO), rebuilt from HEAD 1ba2c85 plus the live
  shared tree. Other agents' uncommitted edits were live the whole time (App OverlayDataModel, ScriptContext,
  PartInstance, Mechanism, NetworkSettings; Network Replicator, ServerReplicator, ConcurrentRakPeer, Streaming,
  Util). Final PLAYER build `build-sh/b-w21-3.log`: 0 failures.
- **Scripts:** `build-sh/w21/` (copied from w20 and pointed at w21). New: `cmpab.py A B [runs]` (per-address best
  effective ratio over a link's runs, gains and losses). `build-sh/w21/rootS/` is a private root whose
  `tools/ida/gen_reccmp_csv.py` has the STUDIO `MATCH_DROP` entry removed (everything else symlinks to `tools/`).
- **Protocol:** every figure is `ab.sh`/`abS.sh` (two runs, regenerate, two runs, regenerate, one run) in a private
  root. `reccmp/` was not written.
- **Caveat on A/B:** reccmp resolves markers through the *current* source, so other agents' edits during a run
  move markers under it. Every Network and App loss between my links was time-ordered (it appears in a later run
  and is absent from an earlier one of the same link) and comes from a file edited in that window
  (Replicator.cpp 02:16, ServerReplicator.cpp 02:32, OverlayDataModel.cpp 02:42). A link built from HEAD
  versions of my shell TUs (`H`) also cannot score my own functions, for the same reason. So the PLAYER
  before-figures below come from sweep27 (`build/sweep27/P-final.json`), and only shell-range rows were compared.
- **What I did not touch:** the shared `reccmp/`, `ida/`, `Client/_generated`, `Client/Rendering`, App, Network and
  Base sources, CMakeLists.txt, CLAUDE.md and DECOMP_PLAN.md. No commits. No new .cpp.
- **Memory:** every heavy command went through `tools/capped.sh`. No cap hits.

## Results

**PLAYER, final link F (the whole live tree): 16,242 entries, 13,259 at effective 100** (sweep27: 13,117).
Against sweep27, F loses one row, `Game::Game` 0x00655e70 (99.65). It is App's in-progress OverlayDataModel.cpp:
the call to App's new STUB `createOverlayDataModel` pairs with the `DualLegacyLock` ctor's name. The same 99.65
shows on link X, so my shell code does not cause it.

New at 100, all from this wave's shell code:

| Function | Address | Before | Now | Marker |
|---|---|---|---|---|
| UserInput::setKeyboardDesiredInternal | 0x0045edf0 | – | **100** | FUNCTION |
| UserInput::setKeyboardDesired | 0x0045ee20 | – | **100** | FUNCTION |
| UserInput::acquireMouseInternalBase | 0x0045ef20 | – | **100** | FUNCTION |
| UserInput::unAcquireMouse | 0x0045f050 | – | **100** | FUNCTION |
| UserInput::acquireKeyboard | 0x0045f120 | – | **100** | FUNCTION |
| UserInput::getWindowRect | 0x00460320 | – | **100** | FUNCTION |
| UserInput::getGameCursorPositionInternal | 0x00460510 | – | **100** | FUNCTION |
| UserInput::getGameCursorPositionExpandedInternal | 0x004605f0 | – | **100** | FUNCTION |
| UserInput::onMouseInside | 0x004606e0 | – | **100** | FUNCTION |
| UserInput::getWindowsCursorPositionInternal | 0x00460780 | – | **100** | FUNCTION |
| Document::FUN_0041df60 (deletes the four verbs) | 0x0041df60 | – | **100** | FUNCTION |
| shell `G3D::Rect2D::xyxy`, `Math::expandVector2` | 0x0045df70, 0x0045dff0 | – | **100** | (generator) |
| `safe_queue<Message>::pop_if_present` / `peek_if_present` | 0x00462a50, 0x00462b00 | – | **100** | (generator) |
| FunctionMarshaller::StaticData ctor / dtor | 0x00436d70, 0x00437690 | – | **100** | (generator) |
| shell `create<ScriptContext>`, `callDoGetClassIndex` / `doGetClassIndex<ScriptContext>`, doDeclare | 0x00421700, 0x00428100, 0x00428110, 0x004272e0 | – / 46 | **100** | (generator) |
| ScriptContext::setTimeout | 0x0067eee0 | 99.4 | **100** | App's marker |
| sp_counted_impl_pd\<ScriptContext\*\> vftable | 0x00b13860 | 20 | **100** | (generator) |
| Game::getOverlayDataModel (shell copy) | 0x0041ce60 | 85.7 | **100** | (generator) |

Written but still STUB:

| Function | Address | Now | What is left |
|---|---|---|---|
| View::FUN_00466cf0 (new DataModel, input, jobs) | 0x00466cf0 | 98.2 | Only the stub callees' conventions: FUN_00468730 takes `this` in esi and FUN_004684b0 in ecx; ours push `this` |
| View::FUN_00466e50 (marshalled DataModel restart) | 0x00466e50 | 74.0 | `OverlayDataModel::dataModelShutdownFinished` is not declared in App (left as a comment); the stub callees |
| UserInput::createAccelerators | 0x0045ed80 | 54.1 → 73.4 | 2013 calls `vector<ACCEL>::resize(n, ACCEL())` (written that way now). Left: LTCG register choice (2013 passes the vector in ebx and reloads `this` from the stack) |
| UserInput::acquireMouseInternal | 0x0045eec0 | 62.1 | 2013 realigns its frame (`and esp,-8`) |
| Document::Shutdown | 0x0041d270 | 6.0 → 41.5 | 2013's frame is 64-byte aligned (`and esp,-0x40`, 0x68 bytes), so some local has `__declspec(align(64))`. FLog::PlayerShutdownLuaTimeoutSeconds is unresolved (Unresolved 1) |
| UserInput::ProcessUserInputMessage | 0x0045f2a0 | 0 → 21.5 | The events need App's 2013 UIEvent (Unresolved 2) |
| ~UserInput | 0x0045e990 | 98.7 → 99.1 | 2013 calls the shell's ~UserInputBase 0x0045e0c0 with `this` in ecx |
| stepDataModelJob | 0x00462920 | 60.6 → 63.0 | pop_if_present is now out of line, as in 2013. peek_if_present is inlined into ours, although its own copy 0x00462b00 now pairs at 100 |
| New stubs: View::RemoveJobs 0x00466a30, FUN_00465e70, FUN_004665f0, FUN_004684b0, FUN_00468730; UserInput::sendMouseEvent 0x004601c0 | | | Placeholder bodies with an engine call, so their callers keep their EH states |

**STUDIO, `shared_from` stand-in (task 3).** Each link was scored on identical objects except
`Client/ClientShared/placeholder.cpp`:

| Link | Stand-in | Generator | At 100 (run r) |
|---|---|---|---|
| SH | HEAD | shared (STUDIO MATCH_DROP kept) | 12,304 |
| SPd | shared_from\<DataModel\> | shared (MATCH_DROP kept) | 12,298 |
| SP | shared_from\<DataModel\> | MATCH_DROP removed | 12,301 |
| **SP2 (in the tree)** | **shared_from\<DataModel\> + \<Instance\>** | **MATCH_DROP removed** | **12,307** |

- **SH → SP2: +5 / −2.**
  - Gained: 0x00445440 shared_from (shell, 80 → 100), RunService::start 0x0079af80 (99.37 → 100),
    shared_from_polymorphic_downcast\<Stats::Item\> 0x005c4f60, shared_polymorphic_downcast\<Stats::Item\>
    0x0062a390, and shared_static_cast\<Instance, DescribedBase\> 0x006833c0.
  - Lost: ServerReplicator::readItem 0x00659ce0 and the DualLegacyLock ctor 0x0084b050. Both files were edited by
    other agents between the SH and SP2 runs.
- **Why Instance as well:** SP (DataModel only) lost Instance::onDescendantAdded 0x006f11e0,
  ServiceProvider::onChildAdded 0x0076f7b0 and RefPropDescriptor\<BillboardGui\>::getVariant 0x0075eaf0. Their
  2013 `shared_from<Instance>` calls go to the shell copy 0x00445440 too.
- In SP2's map, `shared_from<RunService>`, `<DataModel>` and `<Instance>` fold into one shell body (0x00427230),
  which is the 2013 shape. The engine fold for the other types (PartInstance, JointInstance, …) stays native.
- **Integrator:** delete `("STUDIO", 0x00445440)` from `MATCH_DROP` in tools/ida/gen_reccmp_csv.py. SP2 vs SPd is
  the A/B for that: 12,307 vs 12,298 on the same objects. Keep the PLAYER 0x0070fb30 entry.

## 1. Document::Shutdown and the View DataModel functions (task 1)

- `OverlayDataModel::DualLegacyLock` landed in App's header (app-21's work; its report had not landed yet).
- **Document::Shutdown 0x0041d270**, written from the bytes:
  - `if (view) view->RemoveJobs()`.
  - If there is a game with a DataModel:
    - the FLog-gated `ScriptContext::setTimeout`;
    - then, under `DualLegacyLock(overlay, dm, Write)` when there is an overlay DataModel and
      `TaskScheduler::getThreadCount() > 1`, else under `LegacyLock(dm.get(), Write)` (0x006422c0):
      `view->FUN_00465e70()` (2016's Stop has its shape), then `FUN_0041df60()`, which deletes the verbs at
      +0xc, +0x10, +0x18, +0x14 in that order (100).
  - `view.reset()`.
  - `if (marshaller) FunctionMarshaller::ReleaseWindow(marshaller)`.
  - **Against 2016:** the script deadline moved here from PrepareShutdown, and the Game is not shut down.
- **View::FUN_00466cf0:**
  - `game->setDataModel(DataModel::createDataModel(true, NULL, overlay.get()))`;
  - FUN_00468730;
  - under a LegacyLock, `ServiceProvider::create<ControllerService>(dm)`;
  - initializeInput, FUN_004665f0;
  - `userInput->setKeyboardDesired(true)` (no null test);
  - FUN_004684b0, resetScheduler.
- **View::FUN_00466e50:**
  - Callers: no direct one. 0x00467010 (PrepareShutdown's View call) binds it and submits it to the marshaller.
  - Under the DualLegacyLock: `view->VTable0x0c(shared_ptr<DataModel>())` (2016's bindWorkspace), then
    `Game::shutdownGameDataModel`.
  - Then FUN_00466cf0.
  - Under a LegacyLock on the overlay: `find<RunService>(overlay)->start()`, then
    `overlay->dataModelShutdownFinished()`.
  - Then `m_unk0x44 = false` and the signal at +0 fires.
- **Shell `Game::getOverlayDataModel` 0x0041ce60:** an address-take stand-in, added to Client/Win/placeholder.cpp
  (PLAYER only).
  - 2013's copy keeps the plain `__thiscall`, with the return slot pushed, because an engine function calls it
    (DataModel::renderPass2d 0x00645847, not written).
  - Without that caller LTCG passed the return slot in eax.
  - X → Y: 0x0041ce60 goes to 100 and FUN_00466e50 from 62.6 to 74.0. The 10 Network rows Y loses against X are
    the live-edit artefacts described in Setup.
- **Not done:**
  - Document::Initialize 0x0041d960 (973 bytes; about 20 unknown callees, two signal connections, a
    SecurePlayerGame-like ctor 0x00656250).
  - Document::executeScript 0x0041dfd0.
  - 0x0041d140, the Document start function. It calls executeScript, the View setup sequence and
    setKeyboardDesired (0x0041d17c).

## 2. UserInput (task 2)

- **The acquire and cursor code:** setKeyboardDesired(Internal), acquireMouseInternal(Base), unAcquireMouse,
  acquireKeyboard, getWindowRect, getGameCursorPosition(Expanded)Internal, onMouseInside and
  getWindowsCursorPositionInternal.
  - updateKeyboard, updateMouse, onMouseLeave, unAcquireKeyboard and getCursorPositionInternal are inline helpers;
    2013 has no copy of them.
  - 2016's `RobloxCritSecLoc` is inline in UserInput.h: `{RobloxCriticalSection&; ATL::CCritSecLock}`.
  - **Against 2016:**
    - acquireMouseInternalBase has no `if (diMousePtr)`;
    - getWindowRect has no DFFlag::UserInputViewportSizeFixWindows branch;
    - onMouseInside has no UserInputService wrap mode;
    - the DirectInput cooperative levels are the same (mouse FOREGROUND|NONEXCLUSIVE, keyboard
      FOREGROUND|EXCLUSIVE).
  - IDirectInputDevice8A gained SetEventNotification (+0x30) and SetCooperativeLevel (+0x34).
  - **Every UserInput.cpp assert sits on its 2013 line:** 148–150, 269, 300, 340, 775, 781, 804, 825. Still unplaced:
    479, 636, 892 (unwritten functions). The file keeps blank gaps where the unwritten functions go.
- **ProcessUserInputMessage 0x0045f2a0:** the 2013 body handles only WM_MOUSEMOVE, WM_MOUSELEAVE, WM_MOUSEWHEEL,
  WM_SETFOCUS and WM_KILLFOCUS.
  - The events are App's UIEvent, 20 bytes: type, cursor Vector2int16, window size Vector2int16, ?, 0 at +0x10.
  - The wheel direction is the event type: 8 when `(int)wParam > 0.0`, else 9.
  - The focus events are types 0xd and 0xc, sent through 0x00460210. That function tries the overlay DataModel's
    `processUiEvent` first, then the game's.
  - Written up to the events; it stays a STUB.
- **createAccelerators:** the resize call is now `resize(count, ACCEL())`, 54.1 → 73.4.
- **~UserInput and stepDataModelJob:** see the table. Neither was fixable from the shell side this wave.

## 3. STUDIO shared_from (task 3)

- See the STUDIO table. Client/ClientShared/placeholder.cpp now constructs `shared_from<DataModel>` and
  `shared_from<Instance>` by placement new, so no shell `operator=` or destructor is instantiated.

## 4. Sizes (task 4)

- **Document: `// SIZE 0x28`.** Evidence: PLAYER 0x00406600, `operator new(0x28)`, ten zeroed dwords, stored in the
  Application at +0x24.
- **View: `// SIZE 0xcc`.** Evidence: Document::Initialize 0x0041dc45, `operator new(0xcc)`, then the View ctor
  0x00465a80.
- Both `DECOMP_SIZE_ASSERT`s are in Document.cpp, because View.cpp keeps its assert line (739) by layout. Both
  compile.
- **View layout:** +0x44 is now a bool, `m_unk0x44` (FUN_00466e50 clears it).

## Checks

- `python3 -I tools/check_2016_copies.py`: silent.
- `reccmp_addr_padding.py`: 1,726 files, 0 bad.
- `check_folded.py --target PLAYER` (link F): 3,310 annotated addresses, 0 folding problems.
- `reccmp-decomplint Client/WindowsClient Client/Win Client/ClientShared Client/ClientBase --module PLAYER
  --warnfail`: clean, after the "Callers:" comment in UserInput.h was reworded.
- **Every FUNCTION marker in the shell's WindowsClient files** (46) is at effective 100 in F-q and F-r.
- **Assert lines:** UserInput.cpp (10 of 13 placed, all on their 2013 lines) and View.cpp 739.
- **Compiles:** every changed shell TU with its own player_client command. STUDIO compiles
  ClientShared/placeholder.cpp. Client/Win/placeholder.cpp warns C4190 (an extern "C" function holds the
  member-pointer static). The warning is harmless and is the measured form.

## Files changed

- `Client/WindowsClient/UserInput.h` / `.cpp`: RobloxCritSecLoc, the acquire and cursor code, a partial
  ProcessUserInputMessage, sendMouseEvent stub, the DirectInput slots, createAccelerators' resize.
- `Client/WindowsClient/Document.h` / `.cpp`: SIZE, Shutdown, FUN_0041df60, the size asserts.
- `Client/WindowsClient/View.h` / `.cpp`: SIZE, m_unk0x44, VTable0x0c's argument, FUN_00466cf0, FUN_00466e50, stubs
  RemoveJobs / FUN_00465e70 / FUN_004665f0 / FUN_004684b0 / FUN_00468730, RunService.h include (gap kept at 739).
- `Client/Win/placeholder.cpp`: the getOverlayDataModel address-take (PLAYER).
- `Client/ClientShared/placeholder.cpp`: the shared_from\<DataModel\> / \<Instance\> stand-in (STUDIO).
- No new .cpp, no CMakeLists.txt change.

## Unresolved

1. **App, FastLogSettings.cpp:** define `FLog::PlayerShutdownLuaTimeoutSeconds`.
   - It is a LOGVARIABLE, default 0, `.bss` PLAYER 0x00de3f92, registered by initializer 0x00a663b0 (name string
     0x00b2cb58).
   - Document::Shutdown and Document::Initialize (0x0041dbec) read it. Until it is defined, the loads go to
     /FORCE:UNRESOLVED.
2. **App:** declare 2013's `UIEvent` (Util; inline ctor, 20 bytes as in section 2) and
   `OverlayDataModel::dataModelShutdownFinished()` (PLAYER 0x00691310, iOS name).
   - With them, the shell can write ProcessUserInputMessage, sendMouseEvent 0x004601c0, sendEvent 0x00460210 and
     the end of View::FUN_00466e50.
3. **App, OverlayDataModel.cpp (in progress):** Game::Game 0x00655e70 drops to 99.65 because its
   `createOverlayDataModel` call pairs with the `DualLegacyLock` ctor's name (seen on every link this wave).
4. **Integrator:** delete `("STUDIO", 0x00445440)` from `MATCH_DROP`, measured above (SP2 vs SPd: +9 on the same
   objects).
5. **IDA owner, PLAYER names:**
   - 0x00460320 `?getWindowRect@UserInput@RBX@@ABE?AVRect2D@G3D@@XZ`, 0x00460510 getGameCursorPositionInternal,
     0x004605f0 getGameCursorPositionExpandedInternal, 0x004606e0 onMouseInside, 0x00460780
     getWindowsCursorPositionInternal;
   - 0x004601c0 sendMouseEvent, 0x00460210 sendEvent (2016 names, takes a UIEvent), 0x00460370 getWindowSize
     (2016 name, shape matches);
   - 0x004609d0 processInput (string 0x00be4d20);
   - 0x0045df70 / 0x0045dff0, the shell copies of `G3D::Rect2D::xyxy(float,float,float,float)` and
     `RBX::Math::expandVector2`;
   - 0x0041d1c0 `Document::PrepareShutdown`, 0x0041df60 the verb release;
   - 0x00466a30 `View::RemoveJobs` (2016 name), 0x00465e70 (2016's View::Stop has its shape), 0x00466cf0,
     0x00466e50, 0x00467010 (submits 0x00466e50 to the marshaller).
6. **Shell, next:**
   - Document::Initialize 0x0041d960, executeScript, 0x0041d140, the View callees 0x00468730 / 0x004684b0 /
     0x004665f0 / 0x00465e70 / 0x00466a30. They fix FUN_00466cf0's last registers and initializeInput's `this`
     in edi.
   - Shutdown's 64-byte frame alignment.
   - ~UserInputBase's plain thiscall (an engine caller in 2013?).
