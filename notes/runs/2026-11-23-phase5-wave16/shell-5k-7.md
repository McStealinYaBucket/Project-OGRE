# phase5-wave16 shell-5k-7: FriendService Creator stand-in, WndProc / Application, FunctionMarshaller, stopAction

## Setup

- **Build dirs:** `build-sh/` (PLAYER) and `build-sh-studio/` (STUDIO), rebuilt from the shared tree. Other agents' edits landed during the wave: FriendService ctor out of line, Network, boost.hpp and Type.h.
- **Scripts in `build-sh/w16/`:**
  - `cx.sh` / `cxn.sh` compile a TU privately, with the shell's (or Network/Players.cpp's) own ninja command;
  - `lk.sh` relinks a variant from build.ninja's link line;
  - `ab.sh` runs the wave-11 protocol (2 runs, regenerate, 2 runs, regenerate, final run `r`) on the private root `w16/root`;
  - `quick.sh` / `vb.sh` score against the shared CSVs (`root2`);
  - `vbr.sh` gives `--verbose` against a saved CSV set (`root3`).
- **What I did not do:** write the shared `reccmp/`, `ida/` or `Client/_generated`, open IDA, edit `Client/Rendering` or CMakeLists.txt, or commit.
- **Memory:** every heavy command ran through `tools/capped.sh`, and none hit the cap.

## 1. FriendService Creator stand-in: landed

The full report is shell-5k-7-friendservice.md. In short:

- **The file.** `Client/Win/placeholder_creators.cpp` (both shells, no marker) instantiates `FriendService::className()`. Its first Object.h include is `util/object.h` for PLAYER and `Util/Object.h` for STUDIO, matching each target's 2013 shell copies.
- **STUDIO, after regeneration** (SA → SB: +4 / −13): create 0x00474490, create<FriendService> 0x00478250, doDeclare 0x0047e9e0 and the vftable 0x00bc9b7c reach 100. The losses are no-.reloc coincidences.
- **PLAYER, after regeneration** (A2 → B2: +4 / −6):
  - create 0x0041eb70, create<FriendService> 0x00421820, doDeclare 0x00427400 and the sp_counted_impl_pd vftable 0x00b11db0 reach 100;
  - getClassName 0x004200e0, the ctor 0x00422ee0, callDoDeclare and 0x006abba0 stay at 100;
  - the 6 losses are Network packet-cache twin churn.
- **With FriendService.h included in a private copy of Players.cpp** (C2), every Creator row stays at 100. Without the stand-in (D2) they drop to 77–93 or go unpaired.
- **Still open:** static_getCreator 0x0052c200 stays at 90.5, as before. 2013's copy is native, and under /OPT:NOREF the shell keeps its own copy.

## 2. WndProc, Application, View

**New files (PLAYER only; not in CMakeLists.txt, linked privately):**
- `Client/WindowsClient/main.cpp`: `WndProc`, `RegisterWindowClass`, the `appPtr` and `szWindowClass` globals;
- `Client/WindowsClient/Application.h` / `.cpp`: what WndProc calls.

**Integrator:** add `Client/WindowsClient/main.cpp`, `Application.cpp` and `FunctionMarshaller.cpp` to `player_client`, after `View.cpp`.

Figures are from link P1 (all of this wave's shell work) quick-scored on the shared CSVs. P0 → P1 is +1 / −0: the new markers gain only RegisterWindowClass at 100.

| Function | PLAYER | Before | Now | Marker |
|---|---|---|---|---|
| `RegisterWindowClass` | 0x00442870 | unpaired | **100** | FUNCTION |
| `View::resetScheduler` | 0x00466460 | STUB body | **100** | FUNCTION |
| `View::HandleWindowsMessage` | 0x00466510 | 70.4 | 95.4 | STUB |
| `WndProc` | 0x004426a0 | – | 75.0 | STUB |
| `Application::OnGetMinMaxInfo` | 0x00403fa0 | – | 94.9 | STUB |
| `Application::AboutToShutdown` | 0x00404ea0 | – | 20.0 (partial body) | STUB |
| `Application::UploadSessionLogs` | 0x00403f40 | – | 6.5 (stub) | STUB |

**What the bytes showed:**

- **WndProc** forwards only 8 messages to the view: KEYDOWN, MOUSEMOVE, MOUSELEAVE, MOUSEWHEEL, SET/KILLFOCUS, ACTIVATE, ACTIVATEAPP. There is no WM_CHAR or WM_INPUT (jump table 0x00442834, byte index 0x00442848).
  - It has a third command, 40003, which shows a MessageBox ("Video recording doesn't support OpenGL mode yet...", "Information"). This is the WM_COMMAND that RecordToggleVerb::doIt posts.
  - There are no null tests (`appPtr` → `+0x24` Document → View).
  - WM_TIMER uses the 2016 NotifyFGThreadAlive shape: `LogManager::getMainLogManager()` and `MainLogManager::NotifyFGThreadAlive()`. Both are now inline in `Win/LogManager.h`, and the log channel is `FLog::HangDetection`.
- **OnHelp / OnResize.** As Application members, either header-inline or in Application.cpp, LTCG kept them out of line (WndProc 63%). Written out in WndProc they score 75%, so main.cpp has them inline.
- **What still differs in WndProc:**
  - The message is held in eax where 2013 has ecx.
  - 2013 passes `appPtr` to AboutToShutdown in eax. With one caller, our LTCG moves the appPtr load into the callee instead, the same as 2013 does for UploadSessionLogs.
  - Both are expected to settle once WinMain and the other AboutToShutdown callers exist.
- **HandleWindowsMessage:**
  - The deactivate test is `FFlag::OpenNativeBrowserWindowFromLua ? ::GetParent((HWND)lParam) != GetHWnd() : true`. The ternary gives the 2013 `setne cl; mov eax,ecx; test eax,eax`; `!flag || ...` does not (probe compile, 4 variants).
  - The only difference left is uMsg in eax against 2013's ecx, which comes from the caller's convention.
- **resetScheduler** adds three jobs: shared_ptr<TaskScheduler::Job> at +0xc4, +0xbc and +0xa8 (2016 adds only the render job).
  - View.h now declares them as `m_unk0xa8` / `m_unk0xbc` / `m_unk0xc4`, adds `ViewBase* view` at +0xa0 and `GetGfxView()`.
  - It also declares a minimal shell-side `RBX::ViewBase` with only slot 0x30, `onResize`.
  - View.cpp's gap was shortened by 10 lines, so the assert stays on line 739.
- **OnGetMinMaxInfo, against 2016:** 2013 also sets ptMaxTrackSize, from CRenderSettings+0x2e, read through an inline getter (`add eax,0x64`).
  - `std::max` takes the work-area extent first.
  - What is left: the two max temporaries' stack slots are swapped. `std::max(width, s.x)` through the locals scores worse (63.6).
  - `CRenderSettings::minGameWindowSize` (GfxBase, 0x00e1bbf0) is declared `static const` in the ClientBase mirror and stays unresolved until Rendering defines it.
- **Not written** (map in shell-5k-6.md): initializeView, initializeJobs, initializeInput, RemoveJobs, Start, Stop and initializeSizes (74.2).
  - Start and Stop call about ten unwritten Game, Document and RunService helpers (0x0041ce60, 0x0041d550, 0x00461b80, ...).
  - RemoveJobs needs the job classes and marshaller callbacks.
  - None of these could reach 100 without those.

## 3. FunctionMarshaller, stopAction

**`FunctionMarshaller::Submit` 0x00437500: 74.0, STUB** (new `FunctionMarshaller.h` / `.cpp`).
- The layout is proven by its stores: CWindowImpl puts m_hWnd at +4, asyncCalls (`rbx::safe_queue`) at +0x24, and postedAsyncMessage (`rbx::atomic<LONG>`, swap → RbxInterlockedExchange) at +0x48. `this` is on the stack (ret 0x24).
- **The diff:** 2013 inlines the `boost::function` copy ctor (`[esi]=0; call assign_to_own`). Ours calls `function0<void>::function0` out of line, which adds a 4-byte frame slot.

**`RecordToggleVerb::stopAction` 0x00439450: 86.9 → 88.9, STUB.**
- **The 2013 shape:**
  - VideoControl::stopRecording is inlined. It is now in VideoControl.h, and 2013 does not clear the frame callback.
  - `FFlag::FixStopButtonStayingOnScreen` is a new flag in GameVerbs.cpp: .data 0x00dc6c65, default on, initializer 0x00a5b510. It moves `videoRecordingSignal(false)` ahead of the upload test. With the flag off, the signal fires only after a submitted upload.
  - The upload is `Submit(bind(&uploadVideo, this))` on the Document's FunctionMarshaller at **Document+0x1c**. Wave 15's "+0xc → +0x1c" read RecordToggleVerb+0xc (the doc reference).
- **The diff:** the same as Submit. 2013 inlines `function0`'s bind_t ctor and calls `assign_to` 0x0043d110; ours calls the ctor out of line.
- **FrameRateManager.** `ResumeAutoAdjustment` (0x009c7ac0) is declared in a shell-side `RBX::FrameRateManager` in VideoControl.h, because `GfxBase/FrameRateManager.h` is empty. It is unresolved in our link, so its call does not pair.
- **`uploadVideo` 0x004394f0** is declared and stubbed. Its body needs WebBrowserAxDialog.

**Not done:**
- **The ScreenshotVerb ctor** (75.9): no new lead. Its extra 8 bytes of frame look like the same function0 ctor inlining difference seen in Submit and stopAction.
- **UserInput** (ctor 0x0045e300): not started. It is a whole class.

## Checks

- `python3 -I tools/check_2016_copies.py`: silent.
- `reccmp_addr_padding.py`: 1,702 files, 0 bad.
- `reccmp-decomplint --warnfail`:
  - PLAYER: Client/WindowsClient, Client/Win, Client/ClientBase clean;
  - STUDIO: Client/Win, Client/ClientBase clean.
- Both shells build. STUDIO's `build-sh-studio` includes the shared header edits (LogManager.h, VideoControl.h, RenderSettingsItem.h are inline-only additions).
- View.cpp's assert is on line 739.
- **STUDIO links**, regenerated: SA (the current tree) 11,867, SB (+ stand-in) 11,858.
  - +4: all FriendService shell rows reach 100.
  - −13: no-.reloc coincidences (Lua mask, isDenormal's 0x7fffff, IsOlderOrderedPacket, httpGetPostWinHttp), read with `--verbose`. See the friendservice report.

## Files changed

- **New:**
  - `Client/Win/placeholder_creators.cpp`
  - `Client/WindowsClient/main.cpp`
  - `Client/WindowsClient/Application.h`
  - `Client/WindowsClient/Application.cpp`
  - `Client/WindowsClient/FunctionMarshaller.h`
  - `Client/WindowsClient/FunctionMarshaller.cpp`
- **Edited:**
  - `Client/WindowsClient/View.cpp`: resetScheduler, the ternary, the gap.
  - `View.h`: ViewBase stand-in, job members, GetGfxView.
  - `Document.h`: marshaller at +0x1c.
  - `GameVerbs.cpp` / `.h`: stopAction, uploadVideo stub, FixStopButtonStayingOnScreen.
  - `Client/Win/LogManager.h`: getMainLogManager, NotifyFGThreadAlive.
  - `Client/Win/VideoControl.h`: FrameRateManager decl, stopRecording.
  - `Client/ClientBase/RenderSettingsItem.h`: minGameWindowSize, getWindowSize, getUnk0x2e.

## Unresolved

1. **Integrator (CMakeLists.txt):**
   - add `Client/Win/placeholder_creators.cpp` to `OGRE_CLIENT_COMMON_SOURCES`, after `Client/Win/placeholder.cpp`;
   - add `Client/WindowsClient/main.cpp`, `Application.cpp` and `FunctionMarshaller.cpp` to `player_client`.
   - Until FunctionMarshaller.cpp is linked, GameVerbs' Submit call is unresolved (/FORCE:UNRESOLVED).
   - Then regenerate the CSVs, so that 0x006abba0 and STUDIO 0x0080ac20 take the getClassName name.
2. **network-5f-7:** Players.cpp may include `V8DataModel/FriendService.h` once item 1 is linked.
3. **Rendering contributor:**
   - `GfxBase/ViewBase.h`: View.h's minimal ViewBase (slot 0x30 onResize) should then give way to it.
   - `GfxBase/FrameRateManager.h` with `ResumeAutoAdjustment` 0x009c7ac0: VideoControl.h's shell-side declaration should go then.
   - `CRenderSettings::minGameWindowSize` 0x00e1bbf0.
   - Each of these shell stand-ins conflicts with the real header once both are included, so delete them when Rendering lands its headers.
4. **Shell, next:**
   - WinMain plus Application::Initialize; then AboutToShutdown's other callers fix WndProc's appPtr/eax.
   - The boost::function ctor inlining shared by Submit, stopAction and the ScreenshotVerb ctor: 2013 inlines the `function0` ctor and calls `assign_to` / `assign_to_own` out of line, and our LTCG keeps the ctor out of line.
   - UserInput (ctor 0x0045e300); the rest of View.cpp.
5. **Generator / link:** static_getCreator<FriendService> 0x0052c200 (90.5): the /OPT:NOREF shell copy wins over 2013's native one. Same family as wave 15's SoundService className.
