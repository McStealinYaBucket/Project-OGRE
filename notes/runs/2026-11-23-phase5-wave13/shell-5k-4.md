# phase5-wave13 shell-5k-4: CRenderSettingsItem singleton chain, RecordToggleVerb::doIt, VideoControl ctor, shell stand-ins

Builds: `build-sh/` (PLAYER shell objects) and `build-sh-studio/` (STUDIO shell objects). Only the shell
targets were compiled (`--target player_client` / `studio_client`, about 30 s each). Every scored exe is
a private relink: the integrated `build/` (`build-studio/`) objects, which are HEAD e07a4c4 (the sweep19
exe is byte-identical to `build/RobloxPlayerBeta.exe`), plus my shell objects. Script:
`build-sh/w13/link.sh` (the CMake link line, no `/FORCE:MULTIPLE`).

Scoring used wave 11's protocol on a private root (`build-sh/w13/root`, symlinks plus its own
`reccmp/`): shared CSVs, 2 runs, regenerate, 2 runs, regenerate, final run `r` (`build-sh/w13/ab.sh`).
The baseline A (the sweep19 exe/pdb/map, `build-sh/w13/A`) went through the same protocol at the same
time, so the A/B comparison has the same source-line state. Quick single runs used a second root
(`root2`, shared CSVs, `quick.sh`).

I did not write the shared `reccmp/`, open IDA, edit `Client/Rendering`, edit CMakeLists.txt (one
experiment edited it and was reverted, see below) or commit anything. Every heavy command ran through
`tools/capped.sh`; none hit the cap.

## Results

| | A (HEAD shell, same protocol) | H (final tree) | |
|---|---|---|---|
| PLAYER entries at 100% (final run `r`) | 11,725 | **11,762** | **+37 / −0** |
| STUDIO entries at 100% | 11,140 | **11,165** | **+25 / −0** |

The A/B delta overstates the gain. A's HEAD objects cannot pair their markers against today's source
lines in files that moved since HEAD (VideoControl.cpp, GameVerbs.cpp, Network's Player/CrashReporter),
so some of the +37 are A artifacts. **The real gains, entry by entry (H-r vs sweep19 P-final):**

| Address | Entry | sweep19 → H | Why |
|---|---|---|---|
| 0x004069d0 | `~intrusive_ptr<thread_data_base>` | 5.7 → 100 | item 1: placeholder stand-in |
| 0x00456ee0 | `GlobalAdvancedSettingsItem<CRenderSettingsItem>` ctor | 84.3 → 100 | item 2: frame 0x2c via the real `singleton()` caller |
| 0x00407230 | `GlobalAdvancedSettingsItem<CRenderSettingsItem>::singleton` | unpaired → 100 | new TEMPLATE marker |
| 0x00409020 | `FactoryProduct<CRenderSettingsItem>::createInstance` | unpaired → 100 | new TEMPLATE marker (eax result, `sub esp,8` frame, as 2013) |
| 0x00440d20 | `CStringT<char>(const wchar_t*)` | 59.2 → 100 | stand-in (the shell's /GS copy) |
| 0x00be1bec | `RecordToggleVerb` vftable | 85.7 → 100 | doIt now emitted |
| 0x00464c30 | `VideoControl::VideoControl` | unpaired → 100 | written (FUNCTION) |
| 0x00407700 | `GlobalAdvancedSettingsItem<DebugSettings>::singleton` | 74.1 → 100 | stand-in |
| 0x004078b0, 0x00409190, 0x0040d690, 0x00413130, 0x00413860 | DebugSettings Creator::create, createInstance, create<>, shared_ptr ctor, doDeclare | 81.3 / – / 56.3 / – / 46.2 → 100 | the same stand-in (the whole chain becomes the shell's) |
| 0x00b03d30 | `sp_counted_impl_pd<DebugSettings*>` vftable | 20.0 → 100 | the same |
| 0x00403cd0 | `RBX::Http::~Http` | 79.5 → 100 | stand-in (`Http` temporary) |

That is 15 PLAYER entries. **STUDIO, 18 real entries of the +25 (SH-r vs SA-r):**
- `~intrusive_ptr` 0x004256d0, `CStringT(const wchar_t*)` 0x0041e640 and `~Http` 0x00401e20;
- the VideoControl ctor 0x0048c7a0 (FUNCTION);
- the DebugSettings chain: `singleton` 0x004209c0, Creator::create 0x00420b70, createInstance 0x00421040,
  `create<>` 0x004212a0, the shared_ptr ctor 0x00421b20, doDeclare 0x00421c80 and the
  `sp_counted_impl_pd` vftable 0x00bba564;
- the GameSettings chain: `singleton` 0x00428ce0, shared_count 0x00432800, the shared_ptr ctor
  0x00432890, `create<>` 0x00432900, Creator::create 0x004749e0, doDeclare 0x0047ecb0 and the
  `sp_counted_impl_pd` vftable 0x00bc93f4.

The GameSettings chain comes from VideoControl's ctor, which calls `GameSettings::singleton()` in
STUDIO's shell, where nothing else did. The other 7 (VideoControl's dtor, callbacks, vftable and
deleting dtor) are A artifacts: A's objects predate the new VideoControl.cpp lines.

Every loss against sweep19 in H-r is also lost in A-r (other agents' sources moved under HEAD objects:
Network Player/Players/Client, Http.cpp, ProtectedString, EnumProperty, and FLog::CrashReporterInit for
GetAppVersion / getCrashEventName, whose FastLogSettings.cpp app-13 is editing). H vs A: 0 lost.

**Row 5k** (`tools/row_coverage.py --row 5k`, PLAYER): sweep19 969/3,493 (27.7%) → H-r
**981/3,493 (28.1%)**, +12 (the DebugSettings/Http/CRenderSettingsItem entries outside the 5k range
are not counted there).

## 1. intrusive_ptr<thread_data_base> (task item 1)

`Client/Win/placeholder.cpp` now holds a `boost::intrusive_ptr<boost::detail::thread_data_base>` copy
whose dtor runs from an unwind funclet. The original's 18 callers of 0x004069d0 are all engine unwind
funclets (`jmp` from 0x00a117ee…); 0x004069d0 → 100 (PLAYER), 0x004256d0 → 100 (STUDIO), 0 lost.

## 2. The CRenderSettingsItem singleton chain (task item 2)

`RecordToggleVerb::doIt` (PLAYER 0x004396c0, STUB 85.1) is the first real shell caller of
`CRenderSettingsItem::singleton()`. With it:
- our `singleton()` is emitted in the shell and keeps `createInstance` out of line, with its result in
  eax and the 2013 `sub esp,8` padding frame (0x00409020);
- the 8-aligned chain gives the template ctor 0x00456ee0 its 0x2c frame: 100 in PLAYER;
- the CRenderSettingsItem ctor 0x004567d0 goes 77.5 → 96.2. What remains is one register: 2013 loads
  the Vector2int16 temporary into edx and does `mov eax,esi` before the `windowSize` store, ours loads
  it into eax. Not explained.

I added TEMPLATE markers (with their decorated namerefs) for `singleton()` 0x00407230 and
`createInstance` 0x00409020 in `ClientBase/RenderSettingsItem.h`; both are 100. STUDIO's template ctor
0x00437e30 stays 82.4: STUDIO's `singleton()` callers are RobloxStudio code, outside my files.

**CStringT(const wchar_t*) 0x00440d20:** its five callers are all engine (getUserDirectory,
redirectCallback, httpGetPostWinHttp), so no written shell function owns it; a stand-in
`ATL::CStringA narrowed(L"")` in placeholder.cpp gives the shell's /GS copy: 100 in both targets,
0 lost.

## 3. Classes and functions written (all from the 2013 bytes)

| File | What | PLAYER | STUDIO |
|---|---|---|---|
| WindowsClient/GameVerbs.cpp/.h | `RecordToggleVerb::doIt`: OutputDebugString, then if recording `job = bind(&stopAction, this)`, else only under graphics mode Automatic (1) or Direct3D (3) `job = bind(&startAction, this)`, otherwise `PostMessage(doc's hWnd, WM_COMMAND, 40003, 0)` and return; then `jobWait.Set(); jobDone.Wait()` (no 2016 counterpart for the graphics-mode test) | STUB 0x004396c0, 85.1 | n/a |
| | `startAction` / `stopAction` declared, STUB bodies (their addresses are bound by doIt) | STUB 0x00439370 / 0x00439450 | |
| Win/VideoControl.cpp/.h | The ctor: `capture.reset`, `RBXASSERT(verb)` on line 36, `RBXASSERT(rbxView)` on line 39, then `path[0] = 0; strcpy(path, FileSystem::getUserDirectory(true, DirVideo, NULL).c_str())`, `recorded = false`, `videoQuality = -1`, `setVideoQuality(GameSettings::singleton().getVideoQualitySetting())` (an inline forward to the capture slot, no store). `m_unk0x0c` is now `char path[0x100]` | **FUNCTION 0x00464c30, 100** | **FUNCTION 0x0048c7a0, 100** |

doIt's remaining two differences:
- 2013 reads the graphics mode with a folded `mov eax,[eax+0x68]`; ours does `add eax,0x64; mov eax,[eax+4]`.
  A probe compile (`/FAs`, no `/GL`) shows the fold appears only for a direct member access
  (`singleton().graphicsMode`), not through the inline `CRenderSettings::getGraphicsMode()`, and not
  through a `CRenderSettings&` (that adds a null test). So 2013 read the member directly, through a
  friend, a public member or a CRenderSettingsItem-scope accessor. The shell has no out-of-line
  `[ecx+0x68]` getter. CRenderSettings is GfxBase's class, so I left the stand-in's access as it is.
- `job = bind(...)`: 2013's `function<void()>::operator=(bind_t)` 0x0043af20 calls `function0<void>::swap`
  0x0043b140 with the standard convention (ecx + stack), because engine code (0x00540fd0, 0x0069c690)
  calls the same shell copy. Our swap has only shell callers, so LTCG gives it a register convention
  and operator= takes `this` in ebx, not edi.

## 4. Stand-ins (task item 5)

Added to `Client/Win/placeholder.cpp` (both shells), each A/B measured and documented in the file
header:
- the intrusive_ptr and CStringT entries above;
- `RBX::DebugSettings::singleton()` (7/0 shell/native engine calls to 0x00407700): 7 PLAYER entries;
- an `RBX::Http` temporary: gains `~Http` 0x00403cd0; the Http ctor 0x00402b50 stays native (69.1);
- `RunningAverage<double,double>::sample` 0x004612a0: now the shell copy, 21.3 → 30.0. The rest is
  `RBX::isFinite`, which 2013 inlines (integer test on 0x7ff00000) and ours calls out of line from Base.

Tried and removed:
- `Security::Context::current()` to reach `ptr()` (3/0): it also moved the engine's own out-of-line
  `current()` 0x0053ac40 into the shell (−1).
- `ServiceProvider::find<T>` / `create<T>` for RunService, ScriptContext, GuiService, SoundService, plus
  `FactoryProduct<T>::className()` for 13 services in a TU that includes `Util/Object.h` first:
  **−620**. find<T>'s shared helpers and every `Described<T>::classDescriptor` moved into the shell.
  Only the 3 `getClassName` copies spelled `Util/Object.h` (ServerStorage, ReplicatedStorage,
  ServerScriptService) rose. Kept in `build-sh/w13/tried/placeholder.batch2.cpp`.

`tools/shell_twin_callers.py` on link D (`build-sh/w13/twins-D.csv`) lists 82 shell-owned symbols
whose copy of ours is native. The ones still below 100 that look reachable without the twin
problems: `unique_lock<recursive_mutex>::lock` 0x004366c0 (8/0, 59.5), `ConstProperty` ctor 0x00441d40
(3/0, 87.5), `use_facet<numpunct<char>>` 0x0041aaa0 (3/0), the 13 `getClassName` copies (92.6, include
spelling) and the three `safe_static_do_get_mutex` (52.1).

No stand-in was removed: no new real class instantiates an entry already listed in placeholder.cpp.

## 5. Tried and reverted

**View::SetFullscreen 0x00467880 with app-13's GameBasicSettings** (`build-sh/w13/tried/View.cpp`, plus
View.cpp in the player_client sources): −57 / +8, reverted with the CMakeLists edit. Two causes:
- LTCG still inlines SetFullscreen (and its STUB callees) into ToggleFullscreenVerb::doIt (64.9 → 47.6).
  2013's SetFullscreen realigns its frame (`and esp,-8`), most likely for changeResolution /
  restoreResolution (0x00467ae0, 0x004676a0: 747 and 468 bytes). Those are not written.
- `setFullScreen` makes the shell instantiate `signal<void(bool)>::operator()`. 2013 keeps two shell
  copies, 0x00439b20 (bool) and 0x00456d20 (PropertyDescriptor), which differ only in a per-type static
  (`call_once` flag 0x00e2505c vs 0x00e25cd8) inside their callees 0x00439fe0 / 0x00459dd0. Our signal
  has no per-type static, so the two fold. The CRenderSettingsItem setters (13), the Lighting setters (9)
  and a dozen `sp_counted_impl_pd` vftables then pair with the wrong twin.
- Also, app-13's `GameBasicSettings::setFullScreen` stores before it signals; 2013 (0x004678c1) fires the
  signal first and stores `fullscreen` after.

**ReflectionMetadata writeProperty / writeEvent (task item 5)**: not raised (91.1 / 89.1). LTCG still
inlines `get(PropertyDescriptor)` / `get(EventDescriptor)` (each 100 on its own, one caller each).
Everything else in writeProperty matches. I did not find what kept them out of line in 2013.

## Not done (task items 3 and 4)

- **UserInput** (asserts 148–892, WindowsClient/UserInput.cpp): needs App's UserInputBase header
  (there is only `Client/App/util/UserInputBase.cpp`) and DirectInput state. Not started.
- **VideoControl::startRecording** 0x00464f30 (asserts 111/127): needs SoundService's three callbacks
  and two ViewBase virtuals. **DSVideoCaptureEngine**: not started.
- **RecordToggleVerb ctor** 0x00439000: it needs `view->VTable0x60()` (the FrameRateManager), a
  ViewBase member at +0x60 whose +0x24 object gets the VideoControl through slot 0x144 (an
  IFrameDataCallback setter), DSVideoCaptureEngine (0x78 bytes, ctor 0x004324c0) and
  `RecordToggleVerb::action` 0x00438fa0. GfxBase/ViewBase.h is empty, so this is blocked on Rendering.
- **ScreenshotVerb** ctor 0x004382f0 / doIt 0x00438460: doIt is
  `if (dm) dm->submitTask(bind(&DataModel::TakeScreenshotTask, weak_ptr<DataModel>(dm)), DataModelJob::Write)`.
  App's DataModel.h declares a non-static `void TakeScreenshotTask()`. 2013 binds a function of a
  `weak_ptr<DataModel>` (0x0063e240), so the header needs fixing first. The ctor connects
  `DataModel::screenshotReadySignal` (+0xad0) to `screenshotFinished` 0x004385c0.
- **ToggleFullscreenVerb::doIt** (59.5): FLog::Verbs is still unresolved data, and SetFullscreen is blocked
  (section 5).

## Checks

- `python3 -I tools/check_2016_copies.py`: silent.
- `reccmp_addr_padding`: 1,690 files, 0 bad.
- `reccmp-decomplint` on Client/Win, Client/WindowsClient, Client/ClientBase and Client/ClientShared, for
  PLAYER and STUDIO: clean.
- VideoControl.cpp's asserts are on lines 36 and 39.
- Both shells build (`player_client`, `studio_client`).
- The final scored links are `build-sh/w13/H` (PLAYER) and `build-sh/w13/SH` (STUDIO). Since then only
  comments changed (the placeholder.cpp header, the VideoControl.h and GameVerbs.h headers) and the
  VideoControl ctor markers went STUB → FUNCTION at measured 100.

## Files changed

- Client/Win/placeholder.cpp (stand-ins, header log)
- Client/Win/VideoControl.cpp/.h (ctor, `path`, `setVideoQuality`)
- Client/WindowsClient/GameVerbs.cpp/.h (RecordToggleVerb::doIt, startAction/stopAction STUBs)
- Client/ClientBase/RenderSettingsItem.h (two TEMPLATE markers)
- Saved:
  - runs: `build-sh/w13/runs/` (`{A,D,H,SA,SH}-{a,b,p,q,r}.json`, `q-*.json`)
  - links: `build-sh/w13/{B..H,SD,SG,SH}`
  - CSVs: `build-sh/w13/csv-{A,D,H,SA,SH}`
  - scripts: `build-sh/w13/{link,score,ab,quick}.sh`, `cmp.py`, `diff.py`
  - tried: `build-sh/w13/tried/`

## Unresolved

1. **Integrator:**
   - Regenerate the shared CSVs. The new TEMPLATE markers and the ehhandler rows of singleton / the
     VideoControl ctor need it: both were 99.x until the generator added their `__ehhandler$` rows.
   - LogManager's GetAppVersion / getCrashEventName (FLog::CrashReporterInit) and, in STUDIO, the
     RobloxCrashReporter ctor, WriteCrashDump and ~MainLogManager (CrashReporter pairing) are at 98–99
     in both A-r and H-r. Other agents' FastLogSettings.cpp and CrashReporter edits moved those markers
     under the HEAD objects. Recheck them on the integrated build; they are not shell changes.
   - The Lua no-.reloc coincidences (symbexec, luaV_execute, patchtestreg, …: an immediate equal to the
     ProcessPerfCounter assert string's address) flip with any shell size change. Link D had them,
     G and H do not.
2. **Base owner:** `rbx::signals` needs a per-type static inside `signal_with_args<1,…>::operator()`'s
   callee (2013 0x00439fe0 vs 0x00459dd0 differ only in a `call_once` flag). Without it the bool and
   PropertyDescriptor copies fold, and View::SetFullscreen cannot land. Also `RBX::isFinite` is inline
   in 2013 (RunningAverage::sample 0x004612a0).
3. **App owner (app-13):**
   - `GameBasicSettings::setFullScreen` should fire `fullscreenChangedSignal(value)` before storing
     `fullscreen` (PLAYER 0x004678c1–0x004678ca).
   - `DataModel::TakeScreenshotTask` takes a `weak_ptr<DataModel>` in 2013 (bound at 0x00438460).
   - A UserInputBase header for UserInput.
4. **Rendering contributor:** ViewBase (slot 0x60 = getFrameRateManager; a member at +0x60 whose +0x24
   object takes the frame-data callback through slot 0x144), and how CRenderSettings exposes
   `graphicsMode` (RecordToggleVerb::doIt reads it as `[item+0x68]` directly).
5. **Shell, next:** View::restoreResolution / changeResolution (then SetFullscreen, once item 2 lands),
   RecordToggleVerb::startAction / stopAction / action, the remaining shell-owned stand-ins listed in
   section 4, UserInput, DSVideoCaptureEngine.
