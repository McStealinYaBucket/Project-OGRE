# phase5-wave17 shell-5k-8: accept_owner groups, WinMain, AboutToShutdown, OnGetMinMaxInfo, boost::function inlining

## Setup

- **Build dirs:** `build-sh/` (PLAYER) and `build-sh-studio/` (STUDIO), both rebuilt from the shared tree. Other agents' uncommitted edits (Game.cpp, Replicator, ConcurrentRakPeer, UserInputService ...) were in the tree throughout.
- **Scripts in `build-sh/w17/`** (copied from w16 and extended):
  - `cxx.sh SRC OUT` compiles one TU with build-sh's own ninja command into `obj-P/` (`EXTRA=` adds flags), leaving build-sh's objects alone;
  - `cxp.sh` compiles a probe copy of a TU from anywhere;
  - `lk.sh` relinks a variant from one frozen object set, so every A/B differs only in my objects;
  - `ab.sh` / `abN.sh` run the wave-11 protocol (2 runs, regenerate, 2 runs, regenerate, run `r`); `abN.sh` uses `rootN/`, a private root whose `ida/exports/PLAYER-names.csv` carries the two name fixes below;
  - `accept.py` lists every `_internal_accept_owner`-shaped body with its weak_ptr `operator=` callee and unwind target;
  - `dcmp.py` gives a difflib ratio of one function against 2013 (for probes without markers).
- **What I did not do:** write the shared `reccmp/`, `ida/` or `Client/_generated`, open IDA, edit `Client/Rendering`, CMakeLists.txt, CLAUDE.md or DECOMP_PLAN.md, or commit.
- **Memory:** every heavy command ran through `tools/capped.sh`, one build at a time, with no cap hits.

## Results

Figures are from link FIN (the final tree, built by ninja in build-sh) under the protocol with the private names (`runs/FIN-r.json`: 15,533 entries, 12,698 at effective 100). The baseline A is the same frozen objects with the HEAD shell files (`runs/A-r.json`).

| Function | PLAYER | Before | Now | Marker |
|---|---|---|---|---|
| `Application::AboutToShutdown` | 0x00404ea0 | 20.0 | **100** | FUNCTION |
| `Application::OnGetMinMaxInfo` | 0x00403fa0 | 94.9 | **100** | FUNCTION |
| `View::AboutToShutdown` | 0x00465d60 | unwritten | **100** | FUNCTION |
| `RegisterWindowClass` | 0x00442870 | 100 | 100 (kept) | FUNCTION |
| `WndProc` | 0x004426a0 | 75.0 | 76.4 | STUB |
| `WinMain` | 0x00442900 | unwritten | 52.7 | STUB |
| `View::HandleWindowsMessage` | 0x00466510 | 95.4 | 95.4 | STUB |
| five `sp_pointer_construct<Job…>` | 0x005b1970 0x005b8e10 0x00633950 0x0072bc60 0x0072bcd0 | 97.67 | **100** | (rows) |
| `_internal_accept_owner` S-S / N-N / StandardOut | 0x005bd080 / 0x005baa50 / 0x006678b0 | 98 / unpaired / 100 | **100 / 100 / 100** | (rows) |
| `sp_pointer_construct<Registry / FWBase / YFSO>` | 0x005bad60 0x0061a8d0 0x008790e0 | 97.6 | **100** | (rows) |

Other gains:
- WinMain's `CComModule` brought in ATL rows, all at 100: CComCriticalSection ctor, AtlAdd/AtlMultiply, CAtlModule::Lock/Unlock, UpdateRegistryFromResourceD, CRegObject::QueryInterface/AddRef, and the `CComModule` 0x00be2478 / `CRegObject` 0x00be24a4 vftables.
- The `initialization_error` vftable 0x00bdad5c is at 100.

A → FIN, restricted to code this change can move (shell addresses, accept_owner, sp_pointer_construct, ATL), counted by `cmp.py`:
- 29 entries reach 100, and none that was at 100 is lost.
- 20 entries appear below 100. They are:
  - my new STUBs;
  - eight ATL helpers that WinMain's CComModule pulls in (75–97);
  - other agents' new UserInputService, ControllerService and RakPeerInterface rows.

The −8 Lua/Http rows in the intermediate links were no-.reloc coincidences: patchtestreg's `and eax,0x7fc000` met the "PageUp" string, and httpGetPostWinHttp's `0x800000` met data (both read with --verbose). They are back at 100 in FIN. The remaining A → FIN differences are in files other agents were editing between the two scoring runs (Game, RunService, UserInputService, Replicator, ConcurrentRakPeer).

## 1. The `_internal_accept_owner` groups: the integrator's guess was wrong; the fix is weak_ptr<Job>::operator=

`accept.py` on 2013 finds **four** bodies. They differ by which weak_ptr `operator=` they call and which `~shared_ptr` their unwind uses:

| 2013 | operator= | unwind | Who |
|---|---|---|---|
| 0x005baa50 | native 0x0060a3a0 | native 0x007e4e40 | Registry, Thread, FWBase, YieldFunctionStateObject, AsyncHttpCache |
| 0x005bd080 | shell 0x00418f90 | shell 0x00405b50 | the Job types (the sp_pointer_construct<Job> callers) |
| 0x006678b0 | native 0x0060a3a0 | shell 0x00405b50 | StandardOut |
| 0x007a2e50 | shell 0x00418f90 | shell 0x00405b50 | DescribedBase types |

Ours (link A) had the Job types on native op= + shell dtor, the same as StandardOut. So they folded with StandardOut's copy.

- **Integrator's suggestion (`~shared_ptr` of Registry/Thread/FWBase/YFSO in the shell), link D:** net 0. It gains the five `sp_pointer_construct<Job>` but loses StandardOut's pair and three YieldFunctionStateObject/LibraryService callers.
  - callMemberYieldFunction 0x0087c710 calls the **native** `~shared_ptr` fold in 2013, so ~shared_ptr<YFSO> is native there.
  - queueResumeThread's `bind<Thread>` fold shows the same for Thread.
  - Not kept.
- **Kept: `weak_ptr<TaskScheduler::Job>::operator=(shared_ptr<Job>)` in `Client/Win/placeholder.cpp`** (PLAYER only, no marker). In 2013 this comes from the shell's own job creation (View::initializeJobs). The Job group becomes shell/shell, as in 2013.
- **It needs two names in `ida/exports/PLAYER-names.csv`** (Unresolved 1). Today 0x005bd080 carries YieldFunctionStateObject's `match:` name, which pins it to our native/native body.
  - With the stand-in but the shared names (link E): no change.
  - With the names alone (G5 under rootN): +5 −2.
  - With both (N1 under rootN): G5 → N1 +3 −0 (0x005bd080, 0x006678b0, 0x006684b0). All seven rows listed above are at 100 in FIN.

## 2. WinMain, Application::AboutToShutdown, OnGetMinMaxInfo

**WinMain 0x00442900** (main.cpp; STUB 52.7) is the 2016 shape with these differences:
- no SSE2 test and no client-settings fetch;
- `CreateWindow(…, CW_USEDEFAULT, 0, 800, 600, …)`;
- `Initialize(hWnd)` only;
- catch `const RBX::initialization_error&` (FuncInfo 0x00ce3e8c; the class is declared in Application.h);
- no VirtualProtect.

What is left in WinMain is callee conventions: the Application ctor takes `this` on the stack in 2013, and 2013 calls the CComModule dtor 0x00442b40 out of line.

**Application members:** the ctor 0x00403000, dtor 0x004032d0, LoadAppSettings 0x004049b0, ParseArguments 0x00405070, Initialize 0x00404150 and Shutdown 0x00404f10 are declared and stubbed (noinline). SIZE 0x140 comes from WinMain's frame. `scoped_ptr<boost::thread>` sits at +0x124 and +0x128.

**The catch matters:**
- With a nothrow Initialize stub, LTCG drops the catch. WinMain's frame then becomes `and esp,-8`, RegisterWindowClass's frame grows from 0x30 to 0x38 (100 → 51.6), and AboutToShutdown keeps loading appPtr itself.
- The Initialize stub now throws `initialization_error` on a null window. This is a stand-in for the real body's throw (View::initializeView).
- With the throw in place, WinMain gets 2013's `lea ebp,[esp-0x1a8]` frame and RegisterWindowClass is back at 100.

**Application::AboutToShutdown 0x00404ea0** is written in full and is at 100. Against 2016:
- it also sets the event at +0x04;
- it joins and resets the thread at +0x124;
- it calls the View's AboutToShutdown under two null tests.

It takes `this` in eax only once View::AboutToShutdown uses `this`: a STUB that ignores `this` makes LTCG drop the argument, and the tail call loses its `mov ecx,[esi]`.

**View::AboutToShutdown 0x00465d60** is written and is at 100. It is 2016's rememberWindowSettings, inlined, with these differences:
- FFlag::UseWindowPlacementToRememberNonFullscreenedState chooses between the 2016 path and a GetWindowPlacement-only path;
- there is no taskbar clamp;
- the placement goes to the registry under `windowPlacementKey`, which now sits above it.

View.cpp's gap shrank by 31 lines, so the assert stays on line 739.

**OnGetMinMaxInfo: 100.** 2013 clamps to the work area with `std::min((int)s.x, work)`, not 2016's `std::max`. In `cmp work,s.x; jl`, the jl takes the work-extent slot. Wave 16's "swapped slots" was this min/max inversion.

**WndProc (76.4) and HandleWindowsMessage (95.4) are still apart: the cause is found.**
- In 2013, HandleWindowsMessage calls UserInput::PostUserInputMessage 0x0045f1b0 with `this` on the stack and uMsg in ecx (an LTCG custom convention). So uMsg stays in ecx all the way up through WndProc.
- Ours is a generated native link stub (`ogre_stub_0045f1b0`, plain thiscall), so ecx holds `this` and uMsg moves to eax.
- This needs the UserInput class.
- The 2013 PostUserInputMessage body:
  - on WM_MOUSEMOVE it calls Game::getDataModel 0x0041ce40 (Game at +0x2ec), then a shell call 0x00460e50 on DataModel+0xaf0;
  - it pushes `{uMsg, wParam, lParam}` into the +0x238 queue of the job at +0x38 (0x00460ee0);
  - it then wakes that job (0x0072b220, right after SleepingJob::error; most likely SleepingJob::wake).

## 3. The boost::function constructor inlining (Submit, stopAction, ScreenshotVerb ctor): found, not fixed

- 2013 calls `assign_to_own` 0x00437a90 from 13 shell sites (the copy ctor inlined at each) and from 30 engine sites. The shell's out-of-line copy ctor 0x00437750 is called only from the engine.
- Our LTCG inlines assign_to_own into an out-of-line copy ctor. Then Submit cannot inline the copy ctor.
- assign_to<bind_t<RecordToggleVerb>> 0x0043d110 (stopAction, and `function<void()>::operator=` 0x0043af20) shows the same pattern.
- **The callee bodies are byte-identical** in both builds (assign_to_own and assign_to).
- **Single-call-site theory: falsified.** Adding a second LTCG caller (`function0::operator=` in the stand-in, link H) changes nothing.
- **`/Os` reproduces 2013's choice** for Submit (`[esi]=0; call assign_to_own`). But it also brings `__EH_prolog3` and `pop ecx`, which 2013's Submit does not have, so the shell is not /Os.
- **Conclusion:** this is a whole-program LTCG inlining-budget effect. It should follow once more of the shell is written. Noted in FunctionMarshaller.cpp.

## 4. UserInput, View.cpp, initializeSizes: not done

- **initializeSizes (74.2):** four probes (cxp.sh + dcmp.py), none better:
  - `fullscreenSize = defaultSize` then `if (!auto)`;
  - member-wise assignment;
  - declaring fullscreenSize before defaultSize;
  - 2016's `fullscreenSize, windowSize`.
  
  2013 returns calcDefaultResolution's result into a different stack slot (`lea edi,[esp+0x20]` before the push). After that it keeps defaultSize in edi and fullscreenSize in ebx.
- **Mapped but not written:**
  - **RemoveJobs 0x00466a30:** three blocks. Each one binds a FunctionMarshaller member (0x004375e0) on +0xa4 (built by 0x004691c0), copies the function by value, copies the job shared_ptr, and calls View helper 0x004658d0 with TaskScheduler::singleton(). The job is then reset (0x00468c50).
  - **Stop 0x00467010:** ends with `Submit(bind(&View::Start-like 0x00466e50, this))`.
  - **Application::Shutdown 0x00404f10:**
    - SetEvent(+8);
    - join and reset +0x128;
    - Document call 0x0041d270, then delete the Document (~Document 0x0041d020);
    - GlobalBasic/GlobalAdvanced `singleton()->0x006576d0`;
    - 0x00653780, then 0x00437180 on +0x138.
  - **UploadSessionLogs 0x00403f40:**
    - an RbxOgre call 0x00487d50 on the gfx view;
    - MainLogManager at Application+0xac (SIZE 0x64, so it ends at 0x110 as the ctor shows), then CreateFakeCrashDump;
    - two MessageBoxes.
- **No names** exist for the callees of Shutdown, Stop or RemoveJobs. None of these can reach 100 before the Document/Game/UserInput classes exist.
- **FrameRateManager::ResumeAutoAdjustment** stays declared in VideoControl.h. It is Rendering's.

## Checks

- `python3 -I tools/check_2016_copies.py`: silent.
- `reccmp_addr_padding.py`: 1,705 files, 0 bad.
- `reccmp-decomplint --warnfail`:
  - PLAYER: Client/WindowsClient, Client/Win, Client/ClientBase clean;
  - STUDIO: Client/Win, Client/ClientBase clean.
- PLAYER build-sh builds with ninja (FIN). STUDIO build-sh-studio builds (`--target studio`, 0 failures).
  - The only STUDIO-visible edit is `#include "rbx/TaskScheduler.h"` in Win/placeholder.cpp. Coordinator.h already includes it with the same spelling, so it changes nothing.
- View.cpp's assert is on line 739.

## Files changed

- **`Client/Win/placeholder.cpp`:** the weak_ptr<Job>::operator= stand-in (PLAYER only), with the four-group table in its comment.
- **`Client/WindowsClient/main.cpp`:** WinMain (STUB) and the header comment.
- **`Client/WindowsClient/placeholder.cpp`:** its placeholder WinMain is removed; the file is now comment-only.
- **`Client/WindowsClient/Application.h`:**
  - `initialization_error`;
  - SIZE 0x140, the two thread `scoped_ptr`s;
  - ctor/dtor, LoadAppSettings, ParseArguments, Initialize, Shutdown;
  - comments.
- **`Client/WindowsClient/Application.cpp`:**
  - the stubs (Initialize can throw);
  - AboutToShutdown and OnGetMinMaxInfo written, both FUNCTION.
- **`Client/WindowsClient/View.h` / `View.cpp`:** View::AboutToShutdown (FUNCTION); windowPlacementKey moved above it; the gap shortened.
- **`Client/WindowsClient/FunctionMarshaller.cpp`:** a comment with the task-3 findings.

## Unresolved

1. **IDA owner or integrator, PLAYER names (needed for item 1's gains):**
   - 0x005bd080 → `??$_internal_accept_owner@VNetworkOwnerJob@Network@RBX@@V123@@?$enable_shared_from_this@VJob@TaskScheduler@RBX@@@boost@@QBEXPBV?$shared_ptr@VNetworkOwnerJob@Network@RBX@@@1@PAVNetworkOwnerJob@Network@RBX@@@Z`. It is the Job types' shell/shell body.
   - 0x005baa50 → the YieldFunctionStateObject name 0x005bd080 carries now: `??$_internal_accept_owner@VYieldFunctionStateObject@@V1@@?$enable_shared_from_this@VYieldFunctionStateObject@@@boost@@QBEXPBV?$shared_ptr@VYieldFunctionStateObject@@@1@PAVYieldFunctionStateObject@@@Z`.
   - A generator MATCH_FIX pair would do the same.
   - The private copy is `build-sh/w17/rootN/ida/exports/PLAYER-names.csv`; its diff against the shared file is these two rows.
   - Without the names, the stand-in scores even. With them, the rows in item 1 are at 100.
2. **Integrator, CMakeLists.txt:** `Client/WindowsClient/placeholder.cpp` is now comment-only. It can leave `player_client` once WinMain lives in main.cpp; it was kept only so the listed source still builds.
3. **Rendering contributor:**
   - the RbxOgre function 0x00487d50 that UploadSessionLogs calls on the gfx view;
   - FrameRateManager::ResumeAutoAdjustment 0x009c7ac0;
   - `CRenderSettings::minGameWindowSize`;
   - the real `ViewBase`.
   
   The last two are unchanged from wave 16.
4. **Shell, next:**
   - **UserInput, starting with PostUserInputMessage 0x0045f1b0.** Its LTCG convention (`this` on the stack, uMsg in ecx) is what WndProc and HandleWindowsMessage still lack.
   - **The Application ctor 0x00403000.** It takes `this` on the stack. It starts a thread on 0x004034c0 into +0x1c and constructs MainLogManager at +0xac. It would also let UploadSessionLogs be written.
   - **The boost::function inlining** (item 3) should be re-measured after each new batch of shell code.
