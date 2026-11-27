# phase5-wave20 shell-5k-11: Document/View destructors, ReleaseWindow, Application::Shutdown, accessor stand-ins gone, prime_list

## Setup

- **Build dirs:** `build-sh/` (PLAYER) and `build-sh-studio/` (STUDIO), both rebuilt from HEAD 3327f67 plus the
  shared tree.
  - Other agents' uncommitted edits were live the whole time: App ScriptContext, Game, Commands and
    UserController; Network Player, Players, Replicator, Server and ServerReplicator, plus a new
    `Client/Network/Util.cpp`.
  - The first PLAYER build stopped on a half-edited ScriptContext.cpp. A `-k 0` rebuild finished with 0 failures.
- **Scripts:** in `build-sh/w20/`, copied from w19 and pointed at w20. New ones:
  - `cxs.sh` / `cxss.sh` compile one tree TU with its own player_client / studio_client ninja command into
    `obj-P/` / `obj-S/`;
  - `sel.py ADDRFILE A.json B.json…` prints each original address's ratio in each run.
  - `build-sh-studio/w20` is a symlink to `build-sh/w20`, so STUDIO `lk.sh` substitutions resolve.
  - `cmd-win-placeholder.txt` was refreshed: the integrator added `OGRE_ATOMIC_CTOR_NOINLINE` to player_client.
- **Private reccmp root:** `build-sh/w20/root/` with w19's `player-dx.csv`. Every figure below comes from the full
  protocol (`ab.sh`: two runs, regenerate, two runs, regenerate, one run). Baseline: sweep26
  (`build/sweep26/P-final.json`, 13,039 PLAYER).
- **PLAYER links:**
  - **X:** the build as made, with the accessor stand-ins removed, ReleaseWindow, ~Document, stepDataModelJob and
    the prime_list stand-in.
  - **XH:** X with HEAD's placeholder.obj.
  - **XS / Y2:** SoundId stand-in trials.
  - **Y:** X plus the View layout and `~View`. **Y is the candidate.** Its run is `runs/Y-r.json`: 15,933 entries,
    13,040 at effective 100.
- **STUDIO links:** **SH** (HEAD placeholder) and **SP** (prime_list stand-in). Only placeholder.cpp touches
  STUDIO.
- **What I did not touch:** the shared `reccmp/`, `ida/`, `Client/_generated`, `Client/Rendering`, App, Network and
  Base sources, CMakeLists.txt, CLAUDE.md and DECOMP_PLAN.md. No commits.
- **Memory:** every heavy command went through `tools/capped.sh`, one build at a time. No cap hits.

## Results (PLAYER, sweep26 → Y-r)

| Function | Address | Before | Now | Marker |
|---|---|---|---|---|
| Document::~Document | 0x0041d020 | 0 | **100** | FUNCTION |
| View::~View | 0x00465b90 | unpaired | **100** | FUNCTION |
| FunctionMarshaller::ReleaseWindow | 0x00437180 | 9.0 | **100** | FUNCTION |
| Application::Shutdown | 0x00404f10 | 73.3 | **99.13, effective 100** | FUNCTION |
| safe_static_init_staticData | 0x00436ce0 | – | **100** | (generator) |
| map<DWORD, FunctionMarshaller*> erase and its helpers | 0x004377e0, 0x00437ba0, 0x004380e0 | – | **100** | (generator) |
| doDeclare / callDoGetClassIndex / doGetClassIndex for RunService and ControllerService; create\<ControllerService\>; shared_ptr\<ControllerService\>(p, Deleter) | 0x00461d90, 0x00462170, 0x00462180, 0x004692e0, 0x00469490, 0x00469a60, 0x00469ad0, 0x00469ae0 | 46–56 or unpaired | **100** | (generator) |
| sp_counted_impl_pd\<RunService\*\> / \<ControllerService\*\> vftables | 0x00b2d4b0, 0x00b2d618 | 20 | **100** | (generator) |
| sp_counted_impl_p\<UserInputSleepingJob\> vftable | 0x00be4dc0 | – | **100** | (generator) |
| UserInput::~UserInput | 0x0045e990 | 93.5 | 98.7 | STUB |
| UserInput::setGame | 0x0045e630 | 22.7 | 74.5 | STUB |
| View::initializeInput | 0x00466840 | 27.5 | 55.8 | STUB |
| UserInputSleepingJob::stepDataModelJob | 0x00462920 | 7.8 | 60.6 | STUB |
| UserInput::ProcessUserInputMessage | 0x0045f2a0 | – | 0 (new stub) | STUB |
| UserInputSleepingJob ctor | 0x004627b0 | 81.3 | 81.3 | STUB |
| UserInput ctor | 0x0045e300 | 46.5 | 46.5 | STUB |

My changes add 19 entries at 100 (above) and lose one, which is twin pairing:

- **0x0070fb30, boost::shared_ptr\<RunService\>(RunService\*, Deleter): 100 → 64.8.**
  - setGame now creates the RunService in the shell, which 2013 also does: 2013's shell `create<RunService>`
    0x00461f60 calls its own shell copy of the shared_ptr ctor, 0x00462100.
  - Our link has one body for the two originals. The shell's LTCG copy wins the COMDAT, so the engine copy goes,
    and the generator pairs that one body (`/GS`) with the engine original 0x0070fb30 instead of 0x00462100.
  - The engine `create<RunService>` 0x007162e0 (93.9 before) is unpaired for the same reason. The shell's copy
    0x00461f60 now scores 97.4.
  - The case shell-5k-10 warned about did not happen: 0x00469520 and the 22 `sp_counted_impl_pd` vftables kept
    their pairs.

Every other sweep26 100 that Y loses comes from someone else's live source or from a known coincidence. I read each
with `--verbose`:

- **App's Commands.cpp** (in edit): the CameraVerb and ToggleViewMode vftables and `Game::setupDataModel`, whose six
  CameraVerb ctor calls no longer pair.
- **Network's untracked Util.cpp:** initVersion1, where `rot13` no longer pairs.
- **ServerReplicator twin churn:** receiveCluster, OnReceive, sendItemsPacket, serializeSFFlag(s),
  rebroadcastEvent, `Event::getInstance`, `EventInvocation::replicateEvent` and `Player::getUserID`. The same rows
  flip between X, XH, XS and Y, which differ only in shell objects.
- **No-.reloc immediates:** httpGetPostWinHttp (`and edx, 0x800000`) and exp2reg (`or ebp, 0x800002`).

Between X and Y (the View layout change) the only gain is `~View`, and every loss is one of the rows above.

## 1. The App accessors landed; the stand-ins are gone (task 1)

- **app-20-headers.md landed:** `Game::getOverlayDataModel` and `ControllerService::setHardwareDevice`.
- **Removed:** the `#ifdef OGRE_APP_INPUT_ACCESSORS` / `#else` branches in View.cpp (initializeInput) and
  UserInput.cpp (setGame).
- **Assert lines kept:** View.cpp 739 and UserInput.cpp 148–150. The removed lines went back into each file's blank
  gap.
- **Measured** in X and Y against sweep26: the gains are the RunService and ControllerService rows in the table; the
  one loss is the 0x0070fb30 twin above.
- **Still open:** setGame's frame is 8 bytes short (2013 keeps one more 8-byte local). initializeInput's
  registers follow its callers: 2013 has `this` in edi. The UserInput ctor convention also still waits on the
  real callers:
  - View 0x00466cf0: `setDataModel(createDataModel(true, NULL, overlay))`, then 0x00468730, a LegacyLock plus
    `create<ControllerService>`, initializeInput, 0x004665f0, `userInput->setKeyboardDesired(true)` 0x0045ee20,
    0x004684b0 and resetScheduler. It is called only from 0x00466e50, which uses DualLegacyLock.
  - Document::Initialize 0x0041d960: about 20 unknown callees.

## 2. Document, View, FunctionMarshaller, UserInput (task 2)

- **Document layout from ~Document 0x0041d020**, which destroys its members in reverse, together with the verb
  factory 0x0041dd30:

  | Offset | Member |
  |---|---|
  | +0x00 | `scoped_ptr<View>` |
  | +0x04 | `shared_ptr<Game> game` |
  | +0x0c | `scoped_ptr<ToggleFullscreenVerb>` |
  | +0x10 | `scoped_ptr<LeaveGameVerb>` |
  | +0x14 | `scoped_ptr<RecordToggleVerb>` |
  | +0x18 | `scoped_ptr<ScreenshotVerb>` |
  | +0x1c | `FunctionMarshaller* marshaller` |
  | +0x20 | a shared_ptr (type not proven) |

  `~Document` is an empty body: **100**. GameVerbs.cpp and Application.cpp follow the renames (`marshaller`, and
  `.get()` on the View).
- **View layout from ~View 0x00465b90**, which destroys 10 members:

  | Offset | Member |
  |---|---|
  | +0x00 | `rbx::signal<void()>` (~signal 0x00460d60) |
  | +0x18 | `std::string` |
  | +0xa0 | `scoped_ptr<ViewBase>`, deleted through vtable+0x74 |
  | +0xb4 | a shared_ptr (type not proven) |

  - The shell's ViewBase stand-in now declares slots up to 0x70 and a virtual destructor at +0x74.
  - `~View` is an empty body: **100**.
- **Document::Shutdown 0x0041d270 stays a STUB. It needs App's `OverlayDataModel::DualLegacyLock`** (Unresolved 1).
  What it does, from the bytes:
  1. `if (view) view->RemoveJobs()` (0x00466a30).
  2. If there is a game with a DataModel:
     - when `FLog::PlayerShutdownLuaTimeoutSeconds` > 0 and `find<ScriptContext>` (shell copy 0x00420ef0)
       succeeds, call `ScriptContext::setTimeout`;
     - then, if there is an overlay DataModel and `TaskScheduler::threadCount` (+0x1ac) > 1, take a
       `DualLegacyLock(overlay, dm, Write)` (ctor 0x0068c1f0); otherwise take a `LegacyLock(dm, Write)`;
     - under that lock, call view 0x00465e70 (`this` pushed) and Document 0x0041df60 (`this` in esi), which
       releases the four verbs in the order +0xc, +0x10, +0x18, +0x14.
  3. `view.reset()`.
  4. `if (marshaller) FunctionMarshaller::ReleaseWindow(marshaller)`.
- **Application::Shutdown 0x00404f10: FUNCTION.** It is effective 100 in Y. The one register left is the stub
  Document::Shutdown's `this` (2013 eax, ours ecx).
- **FunctionMarshaller::ReleaseWindow 0x00437180: FUNCTION (100).** The 2016 shape:
  - lock `staticData().windowsCriticalSection`;
  - `--refCount` (+0x4c);
  - on zero, `windows.erase(threadID)` (+0x50) and `DestroyWindow()` (CWindowImplBaseT's, which does not clear
    m_hWnd).
  - The header gains `StaticData` (map +0x0, recursive_mutex +0xc) through `SAFE_STATIC`: the accessor is
    0x00436c70 (41.3, because 2013 calls the StaticData ctor 0x00436d70 out of line) and the init is 0x00436ce0
    (100).
  - `~StaticData` is defined; LTCG inlines it into the atexit destructor 0x00437690.
- **stepDataModelJob 0x00462920 (60.6):**

  ```cpp
  scoped_write_request request(userInput->game->getDataModel().get());
  while (messages.pop_if_present(m)) {
      if (m.uMsg == WM_MOUSEMOVE) {
          Message next;
          if (messages.peek_if_present(next) && next.uMsg == m.uMsg)
              continue;
      }
      userInput->ProcessUserInputMessage(m.uMsg, m.wParam, m.lParam);
  }
  sleep();
  return Stepped;
  ```

  - 2013 keeps `peek_if_present` out of line (0x00462b00, the queue in eax, the value in ebx) and ours inlines it.
    The bodies are the same. A second shell caller in 2013 probably keeps it out of line.
  - ProcessUserInputMessage (0x0045f2a0; the string "ProcessUserInputMessage" names it; LTCG passes `this` in edx
    and uMsg in ecx) is a new stub.
- **~UserInput (98.7):** what is left is 2013 calling the shell's out-of-line `~UserInputBase` 0x0045e0c0 with
  `this` in ecx. Ours pushes `this`. The oracle calls 0x0045e0c0 `scoped_ptr<VirtualHardwareDevice>::~scoped_ptr`,
  but its body is ~UserInputBase: `~signal` at +0x34, the cursors, ~ContentId.
- **The remaining UserInput.cpp asserts, now located:**

  | Assert line | Function | Address |
  |---|---|---|
  | 269 | acquireMouseInternalBase | 0x0045ef20 |
  | 300 | unAcquireMouse | 0x0045f050 |
  | 340 | acquireKeyboard | 0x0045f120 |
  | 479 | (in) ProcessUserInputMessage | 0x0045f2a0 |
  | 636, 775, 781, 804, 825, 892 | later functions | – |

  - 0x0045edf0 is setKeyboardDesiredInternal and 0x0045ee20 is setKeyboardDesired: a RobloxCritSecLoc on
    `"setKeyboardDesired"`, with the `true` argument propagated by LTCG. 0x0045eec0 is acquireMouseInternal.
  - None of them is written. Their 2013 conventions come from LTCG constant propagation through callers this tree
    lacks (View 0x00466cf0 for setKeyboardDesired). Without those callers they cannot match.
- **createAccelerators:** not attempted.

## 3. ~SoundId stand-in for the copyValue fold (task 3): measured, not kept

- **Variant 1** (XS: a default-constructed `SoundId` local) folded the two ContentId descriptor vftables, but it
  also instantiated a shell `SoundId()` ctor, which took the engine's 0x0090e3d0 (100 → 60.5).
- **Variant 2** (Y2: the local comes from `SoundChannel::getSoundId()`, so no ctor is instantiated): Y → Y2 is
  **+2 / −1**.
  - Gained: vftables 0x00b3388c and 0x00b33904, 92.86 → 100.
  - Lost: `TypedPropertyDescriptor<SoundId>::equalValues` 0x006bc870 (100 → unpaired). Its body now folds into
    `equalValues<ContentId>`.
- **Why:** 2013's equalValues\<ContentId\> funclets jump to the shell thunk 0x0041c8a0, but equalValues\<SoundId\>'s
  jump to a *second* thunk, 0x004a2390 (also `jmp [__imp_~basic_string]`, in the native range). copyValue\<ContentId\>
  0x00669fd0 goes to 0x0041c8a0.
- **So the fold is not one ~SoundId twin.** copyValue\<SoundId\> probably reaches 0x0041c8a0 through a ContentId
  temporary, which would make it a question for App's copyValue\<SoundId\> source (Unresolved 2).
- The trial is recorded in placeholder.cpp's header. The tree does not have the stand-in.

## 4. prime_list (task 4): done; see shell-5k-11-primelist.md

- placeholder.cpp instantiates `boost::unordered::detail::next_prime`, so the shell supplies
  `prime_list_template<unsigned>::value`. In X it sits at 0x00843f90 with `length` right after it.
- The 52 users have the same scores in sweep26, XH and X (15 at 100).
- **STUDIO, SH → SP** (identical objects except placeholder.obj; full protocol each): 12,240 → 12,246, **+6 / −0**.
  - SH places `value` in NetworkPacketCache.cpp.obj (0x007cc5f8); SP places it in placeholder.obj (0x00838a88).
  - The six gains are Lua no-.reloc immediates that now read differently: symbexec, luaV_execute, constructor,
    patchtestreg, removevalues, luaK_setreturns. They are layout luck, not code.

## Checks

- `python3 -I tools/check_2016_copies.py`: silent.
- `reccmp_addr_padding.py`: 1,717 files, 0 bad.
- `check_folded.py --target PLAYER` (run on link Y): 3,269 annotated addresses, 0 folding problems.
- `reccmp-decomplint Client/WindowsClient Client/Win Client/ClientShared Client/ClientBase --module PLAYER
  --warnfail`: clean.
- **Every changed shell TU compiles** with its own player_client command after its last edit: UserInput, View,
  Document, Application, GameVerbs, main, FunctionMarshaller and placeholder. STUDIO compiles placeholder.
- **Assert lines:** View.cpp 739; UserInput.cpp 148–150.
- **FUNCTION markers:** all five (0x0041d020, 0x00465b90, 0x00437180, 0x00404f10, and 0x0045e5c0 unchanged) are at
  effective 100 in Y-r.

## Files changed

- `Client/WindowsClient/Document.h` / `.cpp`: layout, ~Document, the Shutdown stub.
- `Client/WindowsClient/View.h` / `.cpp`: layout from ~View, ~View, the ViewBase slots up to the destructor,
  initializeInput without its stand-in.
- `Client/WindowsClient/UserInput.h` / `.cpp`: setGame without its stand-in, stepDataModelJob,
  ProcessUserInputMessage stub.
- `Client/WindowsClient/FunctionMarshaller.h` / `.cpp`: StaticData, refCount and threadID, ReleaseWindow,
  ~StaticData.
- `Client/WindowsClient/Application.cpp`: Shutdown marked FUNCTION; `.get()` for the View.
- `Client/WindowsClient/GameVerbs.cpp`: `doc.marshaller`.
- `Client/Win/placeholder.cpp`: the prime_list stand-in, plus the SoundId note.
- **New:** `notes/runs/2026-11-23-phase5-wave20/shell-5k-11-primelist.md`. No new .cpp, no CMakeLists.txt change.

## Unresolved

1. **App, `V8DataModel/OverlayDataModel.h`:** declare `OverlayDataModel::DualLegacyLock` (iOS 10-29 symbol
   `RBX::OverlayDataModel::DualLegacyLock::DualLegacyLock(OverlayDataModel*, DataModel*, DataModelJob::TaskType)`).
   - The Windows 2013 ctor (PLAYER 0x0068c1f0, in OverlayDataModel.cpp's range) takes
     `(shared_ptr<OverlayDataModel>, shared_ptr<DataModel>, DataModelJob::TaskType)` by value.
   - Members: `LegacyLock` at +0 (built with `LegacyLock(shared_ptr<DataModel>, type)` 0x006421c0), and at +4 what
     looks like `DataModel::scoped_write_request` (ctor 0x00643100), given
     `taskType == Write ? overlay.get() : NULL`.
   - The dtor is inline; the shell copy is 0x0041d550.
   - Document::Shutdown 0x0041d270 and View 0x00466e50 (the caller of 0x00466cf0) both need it.
2. **App, Sound.cpp:** in 2013 `TypedPropertyDescriptor<SoundId>::equalValues` 0x006bc870 unwinds to 0x004a2390
   (a native `~basic_string` thunk), while copyValue\<ContentId\>/\<SoundId\> unwind to the shell's 0x0041c8a0.
   - Check whether copyValue\<SoundId\> builds a ContentId temporary (it would then share ~ContentId's shell copy).
     That would restore vftables 0x00b3388c / 0x00b33904 without a shell stand-in.
   - A shell `~SoundId` gets +2 / −1 (section 3).
3. **Generator:** our one shell body for `shared_ptr<RunService>(RunService*, Deleter)` takes the engine original
   0x0070fb30. The shell original is 0x00462100: 2013's shell `create<RunService>` 0x00461f60 calls it. Pairing it
   with 0x00462100 would keep 0x0070fb30 unpaired, rather than a lost 100.
4. **IDA owner, PLAYER names:**
   - `0x00465b90 ??1View@RBX@@QAE@XZ`; `0x0041d020 ??1Document@RBX@@QAE@XZ`.
   - `0x00436c70 ?safe_static_do_get_staticData@FunctionMarshaller@RBX@@CAPAUStaticData@12@XZ`;
     `0x00436ce0 safe_static_init_staticData`; 0x00436d70 the StaticData ctor; 0x00437690 its atexit destructor.
   - `0x0045f2a0 ?ProcessUserInputMessage@UserInput@RBX@@QAEXIIJ@Z`.
   - **Wrong name:** 0x0045e0c0 is the shell's `~UserInputBase`, not `scoped_ptr<VirtualHardwareDevice>::~scoped_ptr`.
   - 0x00462a50 / 0x00462b00 `rbx::safe_queue<UserInputSleepingJob::Message>::pop_if_present` /
     `peek_if_present`.
   - UserInput: 0x0045edf0 setKeyboardDesiredInternal, 0x0045ee20 setKeyboardDesired, 0x0045eec0
     acquireMouseInternal, 0x0045ef20 acquireMouseInternalBase, 0x0045f050 unAcquireMouse, 0x0045f120
     acquireKeyboard.
   - 0x00643100 `DataModel::scoped_write_request::scoped_write_request` (0x0062daf0 is probably its dtor).
   - 0x0068c1f0 `OverlayDataModel::DualLegacyLock` ctor; 0x0041d550 its shell dtor copy.
   - 0x00462100 the shell's `shared_ptr<RunService>(RunService*, Creatable<Instance>::Deleter)`.
5. **Shell, next:**
   - Document::Shutdown, once (1) lands.
   - View 0x00466cf0 / 0x00466e50, then Document::Initialize 0x0041d960, the real initializeInput callers. They
     set the UserInput ctor convention and give setKeyboardDesired its caller.
   - The UserInput acquire functions (asserts 269 / 300 / 340), once those callers exist.
   - setGame's 8-byte frame; the UserInputSleepingJob ctor's 8-byte frame (81.3).
   - The StaticData ctor out of line (0x00436d70).
   - createAccelerators.
