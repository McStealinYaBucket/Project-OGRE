# phase5-wave14 shell-5k-5: View resolution and window code, RecordToggleVerb::startAction, rechecks

## Setup

- **Build dirs.** `build-sh/` (PLAYER) and `build-sh-studio/` (STUDIO). Both were rebuilt from the current shared tree, which includes app-14's and network-5f's uncommitted work.
- **View.cpp is not in CMakeLists.txt.** I compile it privately with player_client's command (`build-sh/w14/cx.sh`). Every PLAYER link is `link2.sh` (all build-sh objects) plus View.obj inserted after GameVerbs.obj (`build-sh/w14/cl.sh`).
- **Scoring.** Same protocol as wave 13, on the private root `build-sh/w14/root`: shared CSVs, 2 runs, regenerate, 2 runs, regenerate, final run `r` (`ab.sh`). Quick runs used `score.sh` or `quick.sh`.
- **What I did not touch.** I did not write the shared `reccmp/` or `Client/_generated`, open IDA, edit `Client/Rendering`, edit CMakeLists.txt, or commit. Every heavy command ran through `tools/capped.sh`, and none hit the cap.
- **Earlier attempt.** An earlier run of this same task (build-sh/w14 A..J, 01:17–02:40) left View.cpp, the ScreenshotVerb ctor and three placeholder stand-ins in the tree with no report. I measured that state again and continued from it. Its J-r scores for View.cpp were mis-paired, because View.obj was older than View.cpp.

## Figures (final links W / SS, effective 100%)

| | sweep20 | this wave | |
|---|---|---|---|
| PLAYER | 11,997 | **12,154** (`runs/W-r.json`) | +161 / −4 |
| STUDIO | 11,378 | **11,502** (`runs/SS-r.json`) | +127 / −3 |
| Row 5k (`row_coverage --row 5k`) | 997/3,493 (28.5%) | **1,029/3,493 (29.5%)** | +32 |

The totals include app-14's and network-5f's work. **Losses against sweep20, each read with `--verbose`:**
- **genericConvert\<ProtectedString\> / \<ContentId\>** (both targets) and **STUDIO's UploadSetting convertToValue**: app-14's. These are the generated stubs and TU order, as app-14 reported.
- **SoundService vftables 0x00b44908 / 0x00b451d0** (PLAYER, 100 → 50): mine, pairing only.
  - startAction makes the shell instantiate `ServiceProvider::create<SoundService>`. That matches 2013, whose only copy of create\<SoundService\> is the shell's 0x0043c7d0, with 12 engine callers.
  - With it, the shell's out-of-line `FactoryProduct<SoundService>::className` copy (0x00408ae0) takes the pairing of 2013's folded `className`/`getClassName` body 0x006ee210.
  - The engine vftable slot points to our engine `getClassName` (0x0053fb90), which is now left without a pair.
  - The same change gains 19 entries, so the net effect is +17 on link T against link S.

## Classes and functions written (from the 2013 bytes)

| Function | PLAYER | Score | Notes |
|---|---|---|---|
| `View::restoreResolution` | 0x004676a0 | **100, FUNCTION** | written by the earlier attempt; 100 once the CSVs are regenerated. The flag-load order matches without a native RobloxWndInit |
| `View::getCurrentDesktopResolution` | 0x00467f00 | **100, FUNCTION** | earlier attempt; ehhandler row after regeneration |
| `View::getFullscreenSizes` | 0x00468070 | **100, FUNCTION** | earlier attempt; the shell's vector copy ctor 0x00468a30 pairs after regeneration |
| `View::ShowWindow` | 0x004682c0 | **100, FUNCTION** | new; see the ShowWindow note below |
| `View::HandleWindowsMessage` | 0x00466510 | 70.4, STUB | new; see the HandleWindowsMessage note below |
| `View::initializeSizes` | 0x004678e0 | 52.3, STUB | `largest` is now built x then y as two statements (2013 order), and `fullscreenSize` is assigned whole. 2013 keeps defaultSize in edi and fullscreenSize in ebx (one more saved register, 4 more frame bytes) |
| `View::calcDefaultResolution` | 0x00467dd0 | STUB | the body needs `DebugSettings::videoMemory()` 0x00659910 and `gfxcard()` 0x00659170 (App header). The stub now computes the tail (`numlines * aspect`), so LTCG keeps the float argument initializeSizes pushes. Its comment records the whole 2013 table: Intel 1200 (600 with "828"), memory ≤32/64/128/256 → 600/1024/1200/1280, more → 1600 |
| `View::changeResolution` | 0x00467ae0 | 90.9, STUB | LTCG register choice for findBestMonitorMatch's arguments (2013 passes dmBest in ebx and szDevice in edx) |
| `View::findBestMonitorMatch` | 0x00467410 | 73.0, STUB | the same argument registers. 2013 also realigns its frame (and esp,-8); ours does not |
| `View::SetFullscreen` | 0x00467880 | STUB | see the SetFullscreen note below |
| `RecordToggleVerb::startAction` | 0x00439370 | 89.0, STUB | `ServiceProvider::create<SoundService>(game->getDataModel().get())`, `videoControl->startRecording`, `fileName = videoControl->getFileName()`, `GameSettings::singleton().videoRecordingSignal(true)`. Only the startRecording call differs (unwritten; 2013 passes soundService in ecx and `this` in edi). **Side gains:** create\<SoundService\> 0x0043c7d0 → 100; the shell SoundService/SoundChannel Creator chain (0x00434990, 0x00434aa0, 0x00434d80, 0x00435120, 0x00435650, 0x004357c0, 0x0043e040, 0x0043e050) |

**ShowWindow (0x004682c0):**
- 2013's own code, against 2016:
  - The placement is a WINDOWPLACEMENT blob read with `RegistryUtil::readBinaryData` from `HKEY_CURRENT_USER\...\RobloxPlayerV4WindowSizeAndPosition`, size first.
  - `RBXASSERT(false)` sits on **line 739**, the View.cpp assert.
  - It calls SetFocus.
  - The TOPMOST/NOTOPMOST pair runs only under the new `FFlag::UseTopmostSettingToBringWindowToFront` (GLOBAL 0x00e25118, initializer 0x00a5c590); otherwise it uses HWND_TOP.
  - It ends with `if (GameBasicSettings::singleton().getFullScreen()) SetFullscreen(true)`, inlined as in 2013.
- `UINT32 size = 0` comes after `::ShowWindow`, which is 2013's store order.
- View.cpp keeps a commented blank gap so the assert stays on line 739.

**HandleWindowsMessage (0x00466510):**
- On WM_ACTIVATE, activation restores fullscreen for any non-zero wParam, then calls SW_RESTORE / SetFocus / SetWindowPos.
- Deactivation calls restoreResolution, then SW_SHOWMINNOACTIVE. Its GetParent test is gated by FFlag::OpenNativeBrowserWindowFromLua.
- Other messages go to `userInput->PostUserInputMessage` (View+0xb0, 0x0045f1b0).
- What remains differs only in frame alignment and argument registers (no callers written).
- View.h gains `boost::scoped_ptr<UserInput> userInput` at +0xb0, plus a minimal `UserInput` declaration with that one method.

**SetFullscreen (0x00467880):**
- It is still inlined into ToggleFullscreenVerb::doIt (46.4), even with ShowWindow as a second caller.
- `__declspec(noinline)` would take doIt to 94.7 (pairing only), but it drops ShowWindow to 67.9, and 2013 inlines it into ShowWindow. I did not keep it.
- I could not score our out-of-line copy. Generated stub `ogre_stub_00467880` in `Client/_generated/player/stubs_00.cpp` carries the same address, and reccmp drops View.cpp's marker as a duplicate.

## Stand-ins (placeholder.cpp)

From the earlier attempt, measured again: `unique_lock<recursive_mutex>::lock` 0x004366c0 → **100**, `use_facet<numpunct<char>>` 0x0041aaa0 → **100**, the ConstProperty ctor 0x00441d40 at 90.6.

ConstProperty's remaining difference is App's header:
- 2013's assert reads `reflection/Property.h` line 255.
- Our header records `reflection/property.h` line 266.

No stand-in was removed: no newly written class instantiates one of them.

## Tried and reverted

- **ScreenshotVerb::doIt 0x00438460** reached 90.6, but the link lost a net −3, so I reverted it.
  - The doIt score is pairing only: the bind/function calls go to the shell copies 0x0043a480 / 0x0043a570, whose names do not pair yet.
  - **Lost:** the six engine rows named `weak_ptr<DataModel>` (0x008445f0, 0x008447e0, 0x00844de0, 0x00844f60, 0x0073da60, 0x0073db90), plus two no-.reloc coincidences.
  - **Gained:** 5 (function1\<void, DataModel*\>::swap / operator= / pop_if_present, ThreadPool getNextTask).
  - **2013 call graph:** engine callers of the `weak_ptr<DataModel>` bind (0x00526859, 0x005a42ba, 0x0066e277, 0x006a9290) call the **shell** copy 0x0043a480. The engine copies 0x008447e0 / 0x00844f60 are called only from DebrisService::addItem 0x00845087, so they are `weak_ptr<Instance>`. The six rows are mis-named.
  - Kept in `build-sh/w14/tried/` and documented in GameVerbs.cpp.
- **CRenderSettingsItem ctor 0x004567d0** as a ternary over two `Vector2int16` temporaries: 97.5 → 90.8, reverted. The remaining difference is unchanged: 2013 loads the temporary into edx and schedules `mov eax,esi` before the store.
- **An overlay root** (Client as a directory of symlinks plus a private `_generated`), to score SetFullscreen without the generated stub. reccmp's marker scan does not follow it (−5,000 entries), so it is dropped.

## Item 4 rechecks (final links)

**PLAYER:** GetAppVersion 0x0043eb80, getCrashEventName 0x0043ee20 and ~MainLogManager 0x00440600 are all 100.

**STUDIO:**
- GetAppVersion 0x0041c6d0, the RobloxCrashReporter ctor 0x0041d0c0, WriteCrashDump 0x0041d280 and ~MainLogManager 0x0041dfa0 are all 100.
- getCrashEventName 0x0041c970 stays at 63.8 (STUB). LTCG propagated a constant `this`: 2013 pushes the global MainLogManager 0xfa5d40 and reads `crashEventExtention` from 0xfa5d84. Only the STUDIO caller (RobloxStudio code) can give it that.

ReflectionMetadata writeProperty / writeEvent: not looked at this wave.

## Checks

- `python3 -I tools/check_2016_copies.py`: silent.
- `reccmp_addr_padding`: 1,695 files, 0 bad.
- `reccmp-decomplint` on Client/WindowsClient, Client/Win, Client/ClientBase and Client/ClientShared: clean for PLAYER and for STUDIO.
- The View.cpp assert is on line 739 (ShowWindow at 100 confirms the string).
- Both shells build.

## Files changed

- Client/WindowsClient/View.cpp: HandleWindowsMessage, ShowWindow, FFlag::UseTopmostSettingToBringWindowToFront, calcDefaultResolution's tail, initializeSizes, four markers flipped to FUNCTION, header.
- Client/WindowsClient/View.h: userInput +0xb0, `UserInput` declaration, HandleWindowsMessage / ShowWindow.
- Client/WindowsClient/GameVerbs.cpp/.h: startAction, comments.
- Client/Win/VideoControl.h: `IVideoCapture` slot 0x1c is `std::string& getFileName()`, `VideoControl::getFileName`, the startRecording declaration.
- From the earlier attempt, kept: RenderSettingsItem.h/.cpp (fullscreenSize / windowSize, public members, `getResolutionPreset`), Win/placeholder.cpp (three stand-ins), GameVerbs ScreenshotVerb ctor and the RecordToggleVerb::doIt FUNCTION marker.
- Saved under build-sh/w14:
  - runs: `runs/{M,S,T,V,W,SS}-*.json`
  - links: `{K..W,SS}/`
  - tried: `tried/`
  - scripts: `cl.sh`, `cl2.sh`, `v.py`

## Unresolved

1. **Integrator:**
   - Add `Client/WindowsClient/View.cpp` to player_client in CMakeLists.txt.
   - Rerun `tools/gen_link_stubs.py`. It must drop `ogre_stub_00467880` (SetFullscreen) and the other View stubs, or View.cpp's markers are dropped as duplicates.
   - Regenerate the shared CSVs.
2. **App owner:**
   - Define `FLog::RobloxWndInit` in FastLogSettings.cpp: .data 0x00d30edd, default 1, initializer 0x00a65100, between Verbs and CrashReporterInit.
   - Declare `int DebugSettings::videoMemory() const` (0x00659910) and `std::string DebugSettings::gfxcard() const` (0x00659170, the 2016 names) for calcDefaultResolution.
   - `reflection/Property.h`: the ConstProperty assert is on line 255 in 2013 (ours 266) and spelled `Property.h`.
3. **IDA owner / generator:** the six engine rows 0x008445f0, 0x008447e0, 0x00844de0, 0x00844f60, 0x0073da60 and 0x0073db90 are DebrisService's `weak_ptr<Instance>` bind chain. They should not be named `weak_ptr<DataModel>`: the 2013 `weak_ptr<DataModel>` copies are the shell's 0x0043a480 / 0x0043a570. With that fixed, ScreenshotVerb::doIt can land.
4. **Generator / pairing:** with the shell's create\<SoundService\>, our shell `FactoryProduct<SoundService>::className` copy takes 0x006ee210's pair, which costs the two SoundService vftables. In 2013 the shell inlines className, through the shell copies 0x00439ef0 and 0x00434d10. This needs a TEMPLATE marker or a generator rule.
5. **Rendering contributor:** FrameRateManager::ResumeAutoAdjustment (0x009c7ac0, stores `[this+0xc] = 1`) and ViewBase's frame-data callback. Both are needed for stopAction 0x00439450 and VideoControl::startRecording.
6. **Shell, next:**
   - **stopAction 0x00439450.** In 2013: `recorded = true; capture->stop(); frameRateManager->ResumeAutoAdjustment()`, then the GameSettings signal(false) before or after the upload depending on FFlag::FixStopButtonStayingOnScreen (0x00dc6c65), then `doc.GetMarshaller()->Submit(bind(&uploadVideo))`. This needs RBX::FunctionMarshaller (Document+0x1c, Submit 0x00437500).
   - **uploadVideo 0x004394f0** and **action 0x00438fa0**.
   - **UserInput** (asserts 148–892; `PostUserInputMessage` 0x0045f1b0).
   - **The rest of View.cpp** (0x004660c0..0x004672f0). HandleWindowsMessage's caller (the window procedure) should fix the frame alignment and argument registers of HandleWindowsMessage, changeResolution and findBestMonitorMatch.
   - **The ScreenshotVerb ctor**: 75.9; the frame is 8 bytes smaller than 2013's, and the connect convention differs.
   - **ToggleFullscreenVerb::doIt / SetFullscreen inlining** (see above).
