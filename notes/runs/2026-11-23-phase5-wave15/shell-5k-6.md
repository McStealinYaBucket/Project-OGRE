# phase5-wave15 shell-5k-6: ScreenshotVerb::doIt, calcDefaultResolution, CRenderSettingsItem ctor, SoundService pairing

## Setup

- **Build dirs.** `build-sh/` (PLAYER) and `build-sh-studio/` (STUDIO), both rebuilt from the current shared tree (HEAD 7fc0c1a plus app-15's and network's uncommitted work). View.cpp is now in CMakeLists.txt, so every link is the plain `cmake --build build-sh --target player` (no private compile any more).
- **Scoring.** `build-sh/w15/`: `snap.sh` copies each link's exe/pdb/map to `w15/<link>/`; `ab.sh` is the wave-11 protocol (2 runs, regenerate, 2 runs, regenerate, final run `r`) on a private root `w15/root`; `quick.sh` / `vb.sh` score or diff against `w15/root2`.
- **Private name exports.** `w15/root/ida/exports/PLAYER-names.csv` is a copy of the shared export with five rows renamed (below). Every other file in `w15/root` is a symlink to the repo. I did not write the shared `reccmp/`, `ida/` or `Client/_generated`, open IDA, edit `Client/Rendering` or CMakeLists.txt, or commit. Every heavy command ran through `tools/capped.sh`; none hit the cap.
- **One measurement slip.** I edited GameVerbs.cpp while link A was being scored, so A's GameVerbs entries after line 54 were scored against the wrong lines (about 10 entries reported absent). Every later comparison uses B..G, or sweep21 for GameVerbs.

## Figures (final runs, effective 100%)

| | sweep21 | this wave | |
|---|---|---|---|
| PLAYER | 12,200 | **12,420** (`w15/runs/G-r.json`, link G) | +231 / −11 |
| STUDIO | 11,547 | **11,734** (`w15/runs/SA-r.json`, link SA) | +195 / −8 |

The totals include app-15's and network's work. **Losses against sweep21, each read with `--verbose`:**
- **PLAYER:**
  - **luaV_execute, symbexec, constructor, patchtestreg, removevalues, luaK_setreturns:** Lua's `0x7fc000` mask. The original's immediate reads as an address (no `.reloc`). Not code, and not shell files.
  - **AsyncHttpQueue::dispatchGenericCallback 0x0086d480:** already lost in link B, before any change of mine. Not shell.
  - **0x0073da60, 0x0073db90, 0x00844de0, 0x00844f60:** the mis-named DebrisService rows (see ScreenshotVerb::doIt below).
- **STUDIO:** the same Lua mask functions and httpGetPostWinHttp. Not shell.

## Functions (from the 2013 bytes)

| Function | PLAYER | Before | Now | Notes |
|---|---|---|---|---|
| `ScreenshotVerb::doIt` | 0x00438460 | 2.4 (STUB body) | **100, FUNCTION** (needs the renames) | see below |
| `View::calcDefaultResolution` | 0x00467dd0 | 4.5 | **100, FUNCTION** | written in full now that App declares `DebugSettings::videoMemory()` / `gfxcard()` |
| `CRenderSettingsItem` ctor | 0x004567d0 (STUDIO 0x004376e0) | 97.5 (STUDIO 76.3) | **100, FUNCTION** (STUDIO 78.8, STUB) | see below |
| SoundService vftables | 0x00b44908, 0x00b451d0 | 50 | **100** (needs the rename) | see "SoundService" below |
| `View::initializeSizes` | 0x004678e0 | 52.3 | 74.2, STUB | gained from calcDefaultResolution (noinline gone, real body) |
| shell `weak_ptr<DataModel>` bind chain | 0x0043a480, 0x0043a570, 0x0043b850, 0x0043c320, 0x0043cb20, 0x0043dbc0 | unpaired | **100** (needs the renames) | |

**ScreenshotVerb::doIt (0x00438460):**
- 2013 shape:
  ```cpp
  boost::shared_ptr<DataModel> dataModel = game->getDataModel();
  if (!dataModel)
      return;
  dataModel->submitTask(boost::bind(&DataModel::TakeScreenshotTask,
      boost::weak_ptr<DataModel>(dataModel)), DataModelJob::Write);
  ```
- The early return is what gives the null path its own `state = -1` and the duplicated `mov esi,[esp+0x18]`. The `if (shared_ptr x = ...)` form merges the two paths (90.6).
- **Renames, in the private names only.** The four engine rows 0x0073da60, 0x0073db90, 0x00844de0 and 0x00844f60 have their name's `weak_ptr@VDataModel@RBX` changed to `weak_ptr@VInstance@RBX` (DebrisService::addItem's chain, like 0x008445f0 / 0x008447e0, which the IDA sync already renamed).
- **Measured.** With the renames, doIt and the shell chain pair at 100, and nothing else is lost (link C against link B: +3 / 0). With the shared names, the four rows take our shell bodies and doIt scores 97.5. Even then the link nets +1 (link B: +5 / −4), so the body stays.

**View::calcDefaultResolution (0x00467dd0):**
- **The 2013 table:**
  - an Intel card gives 1200 lines, or 600 when the card name also contains "828";
  - otherwise, video memory up to 32, 64, 128 and 256 MB gives 600, 1024, 1200 and 1280 lines, and more gives 1600.
- **Two codegen tells:**
  - The last branch must be written `videoMemory <= 256 ? 1280 : 1600`. The `> 256 ? 1600 : 1280` form emits `setle` and `and 0x140` (95.1); 2013 has `setg` and `and -0x140`.
  - LTCG drops the unused `this` and passes only the result pointer, in edi.
- View.cpp includes `V8DataModel/DebugSettings.h`, and the blank gap above ShowWindow is 13 lines shorter, so the assert stays on line 739 (`tools/assert_lines.py`: 0 delta).

**CRenderSettingsItem ctor (0x004567d0):**
- 2013 assigns in each branch:
  ```cpp
  if (getVideoMemory() >= 16000000) fullscreenSize = Vector2int16(1024, 768);
  else fullscreenSize = Vector2int16(800, 600);
  ```
- VC9 merges the two temporaries, and that is what schedules `mov eax,esi` between the temporary's load into edx and the store.
- **STUDIO** goes from 76.3 to 78.8. Two differences are left:
  - a frame 4 bytes smaller than 2013's (`sub esp,0x1c` against `0x20`; same source, so STUDIO shell context);
  - a no-`.reloc` coincidence: the original's `cmp eax, 0xf42400` reads as a string address inside STUDIO's image.

## SoundService vftables (task 1): a naming fix, not source

- **The 2013 picture:**
  - 2013 links with `/OPT:REF` (CMakeLists.txt's own comment), so the shell's LTCG never emits an out-of-line copy of the inline `FactoryProduct<SoundService>::className`. 2013's create\<SoundService\> 0x0043c7d0 inlines it: it calls 0x00439ef0 and then 0x00434d10.
  - The engine's className is then the only COMDAT, and ICF folds it with the virtual `getClassName` at 0x006ee210. Both vftables point there.
- **Our link uses `/OPT:NOREF`:**
  - LTCG keeps every inline body the shell instantiates, even unreferenced ones. Ours has four: className\<SoundService\> 0x004089f0, plus the NonFactoryProduct classNames for SettingsItem, ReflectionMetadataItem and Players, and Client's className. None of them has a caller or a pointer to it.
  - The shell copy wins the `?className@...` COMDAT. The engine body that the vftables point to (our 0x005446c0, Sound.cpp.obj) is left with only its `?getClassName@...UBEABVName@2@XZ` name.
  - 0x006ee210's `match:` name is `className`, so it pairs with the unreferenced shell copy.
- **No source shape fixes this** while the link is NOREF. The shell has to see className to compile `ServiceProvider::create`/`find`, and Util/Object.h is App's.
- **Fix (measured):** rename 0x006ee210 to the virtual name our engine body carries:
  - name `?getClassName@?$FactoryProduct@VSoundService@Soundscape@RBX@@VInstance@3@$E?sSoundService@23@3QBDBV43@@RBX@@UBEABVName@2@XZ`
  - demangled `public: virtual class RBX::Name const & __thiscall RBX::FactoryProduct<...>::getClassName(void)const`
  - It is the same folded body, so the name is just as true.
- **Result:** in link E (private names), 0x006ee210 pairs with 0x005446c0 at 100, and both vftables go 50 → 100. Nothing is lost (E against C: no shell or SoundService losses).
- **General rule for the generator,** if a rename is not wanted: when an original's `match:` / PUBLICS name resolves to an LTCG shell COMDAT (map: a `*.cpp.obj` of player_client / studio_client, below the engine's first address) that nothing references (no rel32 call or jmp, no pointer), and the same original is a slot of a paired vftable, pair it with the body that the slot in our vftable points to.

## Not done, with what the bytes show

**ScreenshotVerb ctor 0x004382f0 (75.9):**
- 2013's frame is 8 bytes larger: one 4-byte slot below the bind object, and one between the string temporary and the EH record.
- The temporary form, `game->getDataModel()->screenshotReadySignal.connect(...)`, also scores 75.9 and moves the register further away. Reverted.

**View.cpp 0x004660c0..0x004672f0 (task 2): mapped, not written.**
- The window procedure is main.cpp's `WndProc` 0x004426a0. Its messages:
  - WM_GETMINMAXINFO: 0x00403fa0 with lParam in esi.
  - WM_DESTROY: AboutToShutdown 0x00404ea0 (appPtr in eax), then PostQuitMessage.
  - WM_SIZE: appPtr (0x00e250a0) → +0x24 mainView → +0xa0 ViewBase vslot 0x30(cx, cy).
  - WM_TIMER: FLog byte 0x00e1aa88 "WindowsPlayer: timer event fired", then CrashReporter::NotifyAlive on 0x00e2510c+0x2c.
  - WM_COMMAND: 0x8112 → 0x00403f40; 0x9c42 → ShellExecute of the wiki URL with mainView's hWnd; 0x9c43 → MessageBox "Information".
  - Input and activation messages: View::HandleWindowsMessage, called with View* pushed, uMsg in ecx, wParam in edx and lParam in ebx.
  - It needs RBX::Application (global 0x00e250a0) first.
- **The rest of View.cpp:**

  | PLAYER | Function | What the bytes show |
  |---|---|---|
  | 0x004660c0 | initializeView | the gfx-mode fallback loop: "Trying mode %d...", "Mode %d failed: \"%s\"", lastGFXMode, `initialization_error`, low-video-memory MessageBox, then initializeSizes; caller 0x0041dcb6 |
  | 0x00466460 | resetScheduler | `this` in eax; `TaskScheduler::add` of three jobs, shared_ptrs at +0xc4, +0xbc, +0xa8 |
  | 0x004665f0 | initializeJobs | RenderJob 0x0044e360 and two more jobs |
  | 0x00466840 | initializeInput | `new UserInput` 0x0045e300, LegacyLock, ControllerService |
  | 0x00466a30 | RemoveJobs | three `removeBlocking` calls with marshaller callbacks |
  | 0x00466cf0 | Start's body | createDataModel, initializeInput, initializeJobs, 0x0045ee20, 0x004684b0, resetScheduler |
  | 0x00466e50 | Start | RunService::start |
  | 0x00467010 | Stop | |
  | 0x004671d0, 0x004672f0 | getDataModel-style helpers | called from RobloxView |

  Most callers are unwritten Document/RobloxView code, so LTCG conventions keep these short of 100.

**UserInput (task 2): needs a whole class.**
- RBX::UserInput : UserInputBase, vftable 0x00be4d9c (8 slots).
- The ctor is 0x0045e300: `this` in esi, ret 0x10, a 0x118-byte frame, members out to +0x2f4. It does DirectInput8Create, reads a registry value (CRegKey 0x0041aee0 / 0x00462350, compared against '1' into +0xa3), and calls LoadCursor twice.
- This is a several-hour class, so I did not start it.

**RecordToggleVerb (task 4):**
- **stopAction 0x00439450:**
  - VideoControl's stop is inlined: `[vc+4] = 1`, `capture(+0x10c)->stop()`, `frameRateManager(+0x118)->ResumeAutoAdjustment()` (0x009c7ac0, Rendering).
  - Then `GameBasicSettings::singleton()`, whose dword at +0x78 gates the upload.
  - FFlag 0x00dc6c65 picks whether `GameSettings::singleton().videoRecordingSignal(false)` (+0x84, 0x00439b20) fires before or after the upload.
  - The upload is `Submit(bind(&uploadVideo, this))` on Document+0xc → +0x1c: function0 ctor 0x0043d110, Submit 0x00437500 with the marshaller pushed.
  - It needs a FunctionMarshaller declaration first.
- **uploadVideo 0x004394f0:** "%s/UploadMedia/UploadVideo.aspx?from=client&videodir=%s&rand=", rand / `_itoa`, getUserDirectory, a CRITICAL_SECTION.
- **ReflectionMetadata writeProperty / writeEvent:** not raised. Three waves in a row found no cause for 2013 keeping `get(Property)` / `get(Event)` out of line.

## Checks

- `python3 -I tools/check_2016_copies.py`: silent.
- `reccmp_addr_padding`: 1,696 files, 0 bad.
- `reccmp-decomplint --warnfail` on Client/WindowsClient, Client/ClientBase, Client/Win and Client/ClientShared: clean for PLAYER and for STUDIO.
- View.cpp's assert is on line 739.
- Both shells build: `build-sh` at the final state, `build-sh-studio` at link SA.

## Files changed

- `Client/WindowsClient/GameVerbs.cpp`: ScreenshotVerb::doIt body, marked FUNCTION; comment.
- `Client/WindowsClient/View.cpp`: calcDefaultResolution written (FUNCTION) and its noinline stand-in removed; DebugSettings.h include; 13 gap lines removed; header comment (same line count).
- `Client/ClientBase/RenderSettingsItem.cpp`: the ctor's per-branch assignment; PLAYER FUNCTION, STUDIO stays STUB.
- Saved under `build-sh/w15/`:
  - links: `A..H/`, `SA/`
  - runs: `runs/*-r.json`
  - CSVs: `csv-*`
  - scripts: `snap.sh`, `ab.sh`, `score.sh`, `quick.sh`, `vb.sh`, `pedis.py`, `xref.py`, `summ.py`, `cmp.py`
  - private names: `root/ida/exports/PLAYER-names.csv`

## Unresolved

1. **IDA owner / integrator (names; these land the figures above):**
   - **0x0073da60, 0x0073db90, 0x00844de0, 0x00844f60:** change `weak_ptr<DataModel>` to `weak_ptr<Instance>` in each name (DebrisService::addItem 0x00845000 is the only caller).
     - Without this, ScreenshotVerb::doIt scores 97.5 under the shared names. Its marker says FUNCTION on the measured 100. Flip it to STUB if the rename is refused.
   - **0x006ee210:** rename to `?getClassName@?$FactoryProduct@VSoundService@Soundscape@RBX@@VInstance@3@$E?sSoundService@23@3QBDBV43@@RBX@@UBEABVName@2@XZ` (the className/getClassName fold). This brings the SoundService vftables 0x00b44908 / 0x00b451d0 to 100.
     - The other choice is the generator rule in "SoundService vftables" above.
   - Regenerate the shared CSVs afterwards.
2. **Integrator:** `Client/_generated/player` may still carry generated stubs for View addresses (SetFullscreen 0x00467880 in wave 14). Rerun `tools/gen_link_stubs.py` so View.cpp's markers are not dropped as duplicates.
3. **Rendering contributor:** FrameRateManager::ResumeAutoAdjustment (0x009c7ac0, stores `[this+0xc] = 1`) for stopAction, and ViewBase's frame-data callback for VideoControl::startRecording.
4. **Shell, next:**
   - **main.cpp WndProc 0x004426a0 and RBX::Application.** This also fixes HandleWindowsMessage's convention (70.4).
   - **The rest of View.cpp,** per the map above.
   - **UserInput,** from the ctor 0x0045e300.
   - **FunctionMarshaller,** then stopAction / uploadVideo.
   - **ScreenshotVerb ctor:** the 8-byte frame.
   - **initializeSizes (74.2):** 2013 keeps defaultSize in edi and fullscreenSize in ebx. Declaring `fullscreenSize, windowSize` together, 2016-style, changes nothing.
