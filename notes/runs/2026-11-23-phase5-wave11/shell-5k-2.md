# phase5-wave11 shell-5k-2: LogManager, the fullscreen/record verbs, one stand-in retired

Builds: `build-sh/` (PLAYER, full build at 13:37, incremental relinks after) and `build-sh-studio/`
(STUDIO `--target studio`, 14:21, compiles the new LogManager.cpp cleanly). Every figure is a relink of
one set of objects with only my shell objects swapped (`build-sh/k/mkrsp.py`, `k/link.sh`), scored with
wave 10's fixed protocol (shared CSVs, 2 runs, regenerate, 2 runs, regenerate, final run `<name>r`) on
private roots `build-sh/w11/rootA|rootB` (`w11/ab.sh`, `w11/snap.sh`, `w11/cmp.py`). The shared
`reccmp/` was not written, no IDA database was opened, nothing under Client/Rendering was edited, nothing
was committed, every heavy command ran through tools/capped.sh (none hit the cap).

## Results

| PLAYER (final run) | entries at 100% | against previous |
|---|---|---|
| A: tree without LogManager.obj / GameVerbs.obj | 11,505 | |
| B: + LogManager.cpp + GameVerbs.cpp | **11,556** | **+51 / −0** |
| C: rebuilt tree at 14:26 (other agents' header edits in), old Win/placeholder.obj | 11,600 | |
| D: C with the StandardOut stand-in entry removed (final tree) | **11,600** | identical to C (0 / 0) |

Sweep17 was 11,435; C/D include the other wave-11 agents' unfinished edits, so the delta that is mine is
A→B (+51) and C→D (0, the retirement is free).

**Row 5k** (tools/row_coverage.py --row 5k, PLAYER): sweep17 810/3,493 (23.2%); A 835 → B **871**
(+36); final D **873/3,493 (25.0%)**.

The 51 gains: 13 FUNCTION markers below, 9 vftables (LogManager, MainLogManager ×2, ThreadLogManager,
RobloxCrashReporter, ToggleFullscreenVerb, thread_specific_ptr delete_data, tss_cleanup_function,
sp_counted_impl_pd), 4 scalar deleting destructors (LogManager, MainLogManager + its `W3` thunk,
ThreadLogManager, the verbs' shared 0x004382b0), InitPath, the thread_specific_ptr<ThreadLogManager>
templates (reset, heap_new, delete_data, do_heap_delete, sp_counted_impl_pd dtor, the static's atexit),
and native code that now has a paired caller: Verb::Verb 0x006f9300, Verb::~Verb 0x006f91f0,
Verb::getText 0x00438150, Guid::generateRBXGUID 0x006f93b0 / generateStandardGUID 0x006f9340, and the
GameSettings singleton/Creator chain (0x00439860, 0x00439a10, 0x00439f50, 0x0043afb0, 0x0043ca10).

## 1. Classes written

### Win/LogManager.cpp (new; LogManager.h, LogManagerExterns.h, VersionInfo.h)
2013 path and line from the assert `mainLogManager == NULL`, line 354 (PLAYER and STUDIO), reproduced
by layout. Layouts from the bytes: LogManager 0x28 (vfptr, log +4, threadID +8, name +0xc),
MainLogManager 0x64 (ILogProvider +0, LogManager +4, crashReporter +0x2c, fastLogChannels +0x30,
crashExtention +0x40, crashEventExtention +0x44, guid +0x48; no gameState in 2013),
RobloxCrashReporter 0x268 (CrashReporter::controls at +0x20), CVersionInfo 0x14.

Against 2016, 2013: creates G3D's common log (`<log>_G3D.txt`) and routes G3D's assertion/failure hooks
to two wrappers; ~MainLogManager deletes that log; getLog has no logsEnabled; DoGetPath prints the logs
folder once (a static `first` flag); both CullLogs calls keep 128 files (folded into CullLogs, which
takes the folder in ecx); ReportEvent has no EVENTLOG_WARNING_TYPE case (jump table 0x00440bb8 maps 2 to
the default); ThreadLogManager::getCurrent keeps a thread_specific_ptr made by a call_once initialiser.

FUNCTION (100%), PLAYER: getSessionId 0x0043ed90, getLog 0x0043f470, provideLog 0x0043f540,
RobloxCrashReporter::logEvent 0x0043f710, ~LogManager 0x00440780, handleG3DDebugAssert 0x00440830,
handleG3DFailure 0x00440900, initThreadLogManagers 0x00440bd0, ThreadLogManager::getCurrent 0x00440c60.
GLOBAL: mainLogManager 0x00e2510c, fastLogChannelsLock 0x00e2525c, threadLogManagers 0x00e25098.

STUB, with the cause of each remaining diff (read with --verbose on D):

| Function | Address | Score | What is left |
|---|---|---|---|
| ~MainLogManager | 0x00440600 | 99.1 | only the call to the unresolved `CrashReporter::~CrashReporter` (needs a link stub) |
| RobloxCrashReporter ctor | 0x0043f570 | 98.6 | unresolved CrashReporter ctor and IsVistaPlus |
| getCrashEventName / getLogFileName / MakeLogFileName | 0x0043ee20 / 0x0043efb0 / 0x0043f230 | 93.5 / 93.1 / 93.3 | ATL helper conventions (below) + unpaired CloneData twin |
| ProcessException | 0x0043f660 | 89.1 | unresolved CrashReporter::ProcessException / DumpErrorUploader |
| ReportEvent | 0x00440a70 | 88.3 | code identical; the function-size row stops before the in-function jump table (generator) |
| ThreadLogManager::getLogFileName, getFastLogFileName, handleDebugAssert, handleFailure, purecallHandler, WriteCrashDump | | 78–87 | CString::Format convention (below) |
| fastLogMessage | 0x0043ec40 | 79.8 effective | vector<Log*>::resize's LTCG convention differs (eax/edx vs ecx/eax); kept STUB |
| MainLogManager ctor | 0x0043fa50 | 84.7 | CullLogs/ATL conventions, unresolved G3D::Log ctor and hooks |
| CreateFakeCrashDump | 0x0043f8c0 | 73.2 | unresolved GenerateDmpFileName |
| CullLogs | 0x004403a0 | 62.7 | ATL operator+/AddBackslash/Append conventions, list<WIN32_FIND_DATA> helpers |
| GetAppVersion | 0x0043eb80 | 38.8 | CVersionInfo::Load / GetFileVersionAsString unwritten (custom conventions) |

**The ATL convention blocker.** The original's shell copies of CStringT::Format (0x00441660),
CPathT::Append (0x00436b10), CloneData (0x004414f0), AddBackslash (0x00440f00) and operator+(CString,
PCXSTR) (0x00440db0) keep standard conventions because native App code calls those same copies:
0x00624210, 0x00664260, 0x00623230, FileSystem's 0x006ea260 / getUserDirectory 0x006ea4c0 /
0x006eab90. None of those App functions exists in the tree, so our LTCG gives the helpers custom
conventions (Format takes `this` in edi) and every LogManager caller differs. Same mechanism as the
Creator::create/Object.h case of wave 10.

### WindowsClient/GameVerbs.cpp (new; GameVerbs.h, View.h, Win/VideoControl.h)
No 2013 `__FILE__` names this file (no asserts), so the path is 2016's (noted in the header).
ToggleFullscreenVerb: videoControl +0xc, view +0x10 (View::fullscreen +0x45); RecordToggleVerb:
videoControl +0x14, videoUploadingEnabled +0x34; VideoControl's capture at +0x10c (IVideoCapture
isRunning = slot 3); FLog::Verbs channel (ToggleViewMode 0x008561d0 uses the same byte).

FUNCTION: ToggleFullscreenVerb ctor 0x00439750, isChecked 0x004397e0, isEnabled 0x004397f0;
RecordToggleVerb::isChecked 0x00439360 (effective; its vftable is not emitted yet).
STUB: ToggleFullscreenVerb::doIt 0x00439810 (View::SetFullscreen 0x00467880 unwritten, takes the flag
in bl), RecordToggleVerb::isEnabled 0x00439330 (34.8: without the ctor no vftable is emitted, so LTCG
gives the slot a custom convention; the body is right).

Lesson: an LTCG virtual is only `__thiscall` in our link once its vftable is emitted, i.e. once the
class's ctor (or dtor) is compiled. logEvent went 36% → 100% when the RobloxCrashReporter ctor was
added, ToggleFullscreenVerb's slots went effective → 100% with its ctor.

Not written (no time / blocked): LeaveGameVerb (doIt reads [[this+0xc]+0]+0xc as the HWND; the type of
+0xc is not proven), ScreenshotVerb, RecordToggleVerb's ctor/dtor/doIt, UserInput, ViewUpdateJob,
RenderJob (ctor 0x0044e360 needs DataModelJob's 2013 ctor signature; step 0x0044ec20 jumps into
.vmp0), VideoControl, RbxWebView, the WindowsClient application classes, DSVideoCaptureEngine.

## 2. ReflectionMetadata STUBs
Not raised. writeProperty 0x00448050 (91.1) and writeEvent 0x00448750 (89.1): ours inlines
get(Property)/get(Event) (with Classes::get folded in), the original calls the out-of-line copies
(both 100%) although each has one caller; writeFunction/writeYieldFunction/writeCallback inline theirs
in both builds. The only source difference between the two groups is the isScriptable() early return;
the cause is not found and noinline would not be 2013 source. writeAPI 0x00448df0 (75.0) and the
singleton 0x00447970 (91.9) are blocked as in wave 10 (command-line handler 0x004054b8; the CString
operator+ convention above). Note: another agent changed descriptor `name->str` to `name.str` in
ReflectionMetadata.cpp this wave (Descriptor::name is now a reference); kept.

## 3. Stand-in entries
Removed `boost::shared_ptr<RBX::StandardOut>` from Client/Win/placeholder.cpp: LogManager's
ReportEvent now instantiates `~shared_ptr<StandardOut>` (PLAYER map: LogManager.cpp.obj). C vs D:
11,600 = 11,600, 0 / 0. Every other entry stays (no real shell code instantiates them yet).
Client/ClientShared/placeholder.cpp unchanged (FrameRateManagerStatsItem still unwritten).

## 4. Shell near misses that differ only by an unpaired `__ehhandler$` (for the integrator)
New tool `tools/ehhandler_near_misses.py JSON` (self-check in `__main__`); on B's final run, 13 shell
entries (generator / libdata rows, not source):

| Address | Score | Function | Original operand |
|---|---|---|---|
| 0x004010f0 | 97.75 | boost::system::system_error::what | 0x00a522e9 (+ a catch funclet return) |
| 0x0040c2c0 | 99.39 | boost::call_once<void(*)()> | 0x00a52069 |
| 0x00413600 | 98.33 | std::_Uninit_fill_n<std::string*> | 0x00a51f31 |
| 0x00415700 | 98.00 | shared_count<Script*, Creatable::Deleter> | 0x00a4f820 |
| 0x004198a0 | 98.21 | std::_Tree<map<string, …>> | 0x00a51d61 |
| 0x00440f90 | 97.96 | ~thread_specific_ptr<ThreadLogManager> | 0x00a54448 |
| 0x00441b90 | 97.92 | shared_count<thread_specific_ptr<…>::delete_data> | 0x00a504e0 |
| 0x00449d40 | 99.63 | vector<MergeBinder::IDREFItem>::_Insert_n | 0x00a56150 |
| 0x00474df0 | 98.08 | wstring::_Construct | 0x00a4ff90 |
| 0x00474e90 | 98.00 | string::_Construct<char*> | 0x00a4ff70 |
| 0x004777e0 | 98.11 | shared_count<program_options::value_semantic const> | 0x00a4fdc0 |
| 0x00477880 | 98.11 | shared_count<program_options::option_description> | 0x00a4fda0 |
| 0x0047a820 | 97.83 | vector<string>::_Insert_n | 0x00a56760 |

Also for the generator: ReportEvent 0x00440a70's function-size row should extend over its jump tables
(0x00440ba0–0x00440bc9).

## Checks
- check_2016_copies: silent. reccmp_addr_padding: 1,679 files, 0 bad.
- reccmp-decomplint Client/Win, Client/WindowsClient, Client/ClientBase (PLAYER): clean (GetAppVersion
  moved above fastLogMessage for ascending order; the assert stays on 354).
- Both shells build (PLAYER 13:44/14:26, STUDIO 14:21). The last edit after the scored D link is one
  marker word (STUB → FUNCTION on getSessionId), no line change.

## Files changed
- New: Client/Win/LogManager.h, LogManager.cpp, LogManagerExterns.h, VersionInfo.h, VideoControl.h;
  Client/WindowsClient/GameVerbs.h, GameVerbs.cpp, View.h; tools/ehhandler_near_misses.py.
- Client/Win/placeholder.cpp: StandardOut stand-in entry and its include removed, comment updated.
- **CMakeLists.txt (shared, outside my ownership; two lines):** `OGRE_CLIENT_COMMON_SOURCES` gains
  Client/Win/LogManager.cpp; `player_client` gains Client/WindowsClient/GameVerbs.cpp. A small separate
  commit for the integrator.
- Saved: runs/JSONs `build-sh/w11/runs/` (A, B, C, D, B0–B2), scored exe/pdb/map `build-sh/w11/snaps/*`,
  links `build-sh/w11/link{A,B,C,D}`, scripts `build-sh/w11/{ab.sh,snap.sh,cmp.py,score_list.py}`.

## Unresolved
1. **Integrator:** the two CMakeLists.txt lines; rerun tools/gen_link_stubs.py (delete the exe first) —
   new unresolved functions with original addresses: CrashReporter ctor 0x00520070 / dtor 0x005200f0 /
   ProcessException 0x0051fcb0 / Start 0x005205a0 / GenerateDmpFileName 0x0051fad0, G3D::Log ctor
   0x009d5b70 / common 0x009d5c60 / getCommonLogFilename 0x009d5d10, G3D::setAssertionHook 0x009d63b0 /
   setFailureHook 0x009d63c0, FileSystem::getUserDirectory(bool, FileSystemDir, const char*) 0x006ea4c0,
   IsVistaPlus 0x0046a2e0, DumpErrorUploader::UploadCrashEventFile 0x00435f10, CVersionInfo::Load
   0x00463f50 / GetFileVersionAsString 0x00464370, View::SetFullscreen 0x00467880; data
   FLog::CrashReporterInit (0x00d30ede). ~MainLogManager (99.1) should reach 100% with the stubs.
2. **App owner (native callers that fix the shell's ATL conventions):** write 0x006ea4c0
   (FileSystem::getUserDirectory, 2013 overload), 0x006ea260, 0x006eab90, 0x00624210, 0x00664260,
   0x00623230. Until then CString::Format/CPath helpers get LTCG conventions in our link and about 12
   LogManager functions (and ReflectionMetadata's singleton) stay below 100%.
3. **App owner, Util/FileSystem.h:** declare `enum FileSystemDir` and
   `std::string getUserDirectory(bool create, FileSystemDir dir, const char* sub)`; LogManagerExterns.h
   declares them meanwhile.
4. **Network owner:** network/CrashReporter.h (layout in Win/LogManager.h: controls at +0x20, size
   0x268 with RobloxCrashReporter); delete the stand-in declaration then.
5. **Rendering contributor:** G3D 8.00 Log.cpp and the debugAssert hook setters are not built
   (SerializerV2 already references G3D::Log::println); LogManagerExterns.h declares them.
6. **Generator:** the 13 `__ehhandler$` near misses (section 4) and ReportEvent's jump-table size row.
7. **Shell, next:** RecordToggleVerb ctor (emits its vftable, raises isEnabled), LeaveGameVerb/
   ScreenshotVerb (need View's 2013 layout: View.cpp has one assert, line 739), View::SetFullscreen,
   VersionInfo.cpp, VistaTools.cpp, DumpErrorUploader.cpp, RenderJob (DataModelJob's 2013 ctor),
   UserInput (asserts 148–892), VideoControl (36–127), WebBrowserAxDialog (359), the command-line
   handler 0x004054b8 (writeAPI), STUDIO markers for LogManager (STUDIO ctor 0x0041d5b0, handlers
   0x0041e1f0/0x0041e2c0, purecall 0x0041d510, fastLogMessage 0x0041c790).
