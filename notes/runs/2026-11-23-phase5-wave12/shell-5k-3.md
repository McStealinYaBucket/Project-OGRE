# phase5-wave12 shell-5k-3: Win helpers, VideoControl, LeaveGameVerb, STUDIO LogManager

Builds: `build-sh/` (PLAYER) and `build-sh-studio/` (STUDIO). Other agents' in-progress Network edits
broke the normal exe links (`LNK2005` Players ctor/dtor, a 0-byte-at-HEAD `ConcurrentRakPeer.cpp` that
did not compile yet). So every scored exe is a private relink of the build's objects
(`build-sh/k/mkrsp.py`, `build-sh/w12/link.sh`: the CMake link line plus `/FORCE:MULTIPLE`, with
`ConcurrentRakPeer.cpp.obj` left out). Scoring used wave 11's fixed protocol: shared CSVs, 2 runs,
regenerate, 2 runs, regenerate, final run `<name>r`, on private roots (`build-sh/w12/ab.sh`,
`snap.sh`, `cmp.py`, `mk.py`).

I ran `gen_link_stubs` logic on those private links, using a driver that feeds it the link log, and
it rewrote `Client/_generated/{player,studio}`. I did not write the shared `reccmp/`, open IDA,
edit `Client/Rendering`, or commit anything. Every heavy command ran through `tools/capped.sh` and
none hit the cap.

## Results

| | entries at 100% (final run) | |
|---|---|---|
| PLAYER sweep18 (integrated, wave 11) | 11,673 | |
| PLAYER E (final tree, `build-sh/w12/snaps/E`) | **11,708** | the whole tree, including other agents' edits |
| STUDIO sweep18 | 11,071 | |
| STUDIO SE (final tree, `snaps/SE`) | **11,158** | |

Step deltas inside this wave (consecutive links that differ only in my objects):
- B→C (LeaveGameVerb, a comment line): +11.
- C→D (VideoControl): PLAYER +10 / −1, STUDIO +18 / −1.
- D→E (Network's CrashReporter.h replaces my stand-in): +2 / −6.

Every loss outside my files is another agent's source moving while it was being scored: Network
Server's vftable slot 0x70, and Http::ThrowIfFailure (its Http.cpp lines moved, which I confirmed
with `--verbose`).

My A/B with HEAD objects (A 11,687 → B 11,713) overstates my gain. HEAD-compiled objects cannot
pair their markers against the current source lines. So I measured the gains below per function,
against sweep18.

**Row 5k** (`tools/row_coverage.py --row 5k`, PLAYER): sweep18 923/3,493 (26.4%) → E
**949/3,493 (27.2%)**, +26.

## 1. Task item 1: wave-11 callees, ~MainLogManager, STUDIO LogManager

- `gen_link_stubs` now binds G3D::Log::getCommonLogFilename, the G3D hook setters,
  View::SetFullscreen and format_string. I also wrote CVersionInfo, IsVistaPlus and
  DumpErrorUploader myself.
- **~MainLogManager 0x00440600: 100%** (FUNCTION). Also raised to 100%:
  - GetAppVersion 0x0043eb80 (CVersionInfo is now real, with its 2013 ctor store order).
  - RobloxCrashReporter::ProcessException 0x0043f660.
- LogManager: `// SIZE 0x28` added. STUDIO VTABLE markers added for LogManager 0x00d0cc04,
  MainLogManager 0x00d0cc10/0x00d0cc18, RobloxCrashReporter 0x00d0cc24 and ThreadLogManager
  0x00d0cc30.
- STUDIO GLOBALs added: mainLogManager 0x00fa4fec, fastLogChannelsLock 0x00fa57b8,
  threadLogManagers 0x00fa4fd8.
- Every LogManager function now has a STUDIO marker. I matched each one by comparing its
  disassembly with PLAYER's. To keep the ctor's assert on line 354, the 17 new lines above it came
  out of the padding block.
  - FUNCTION in STUDIO (all measured at 100% in SB/SE): GetAppVersion 0x0041c6d0, fastLogMessage
    0x0041c790, getSessionId 0x0041c8e0, getLog 0x0041cfc0, provideLog 0x0041d090,
    RobloxCrashReporter ctor 0x0041d0c0, ProcessException 0x0041d1b0, logEvent 0x0041d260,
    ~MainLogManager 0x0041dfa0, ~LogManager 0x0041e120, handleG3DDebugAssert 0x0041e1d0,
    handleG3DFailure 0x0041e2a0, ReportEvent 0x0041e370, initThreadLogManagers 0x0041e4d0,
    getCurrent 0x0041e560.
  - STUB in STUDIO: the ctor 0x0041d5b0 (59.4), purecall 0x0041d510 (81.7), handleDebugAssert
    0x0041e1f0 (78.1), handleFailure 0x0041e2c0 (78.5), CullLogs 0x0041dd40, and the CString
    helpers (getCrashEventName 0x0041c970 and the others).
- **Network/CrashReporter.h has landed** (`Client/Network/include/Network/CrashReporter.h`).
  LogManager.h now includes it, and my stand-in CrashReporter / CrashReportControls are gone.
  - The stand-in had GenerateDmpFileName with four parameters. 2013 has three (`ret 0xc` at
    0x0051fca3).
  - With the fix, CreateFakeCrashDump goes 70.4 → 78.6.
- `LogManagerExterns.h` now includes `Util/FileSystem.h`, where app-12 now declares FileSystemDir
  and the 2013 getUserDirectory, instead of declaring them itself. It also includes the new
  VistaTools.h and DumpErrorUploader.h. Only the G3D::Log / hook stand-ins remain.

## 2. Classes and functions written (all from the 2013 bytes)

| File | What | PLAYER | STUDIO |
|---|---|---|---|
| Win/VistaTools.cpp/.h (new) | IsVistaPlus (OSVERSIONINFOA `= {0}`, GetVersionExA) | 0x0046a2e0 100% | 0x0048d690 100% |
| Win/DumpErrorUploader.cpp/.h (new) | UploadCrashEventFile: 2013 has no analytics, and uses `catch (...)` (FuncInfo 0x00cdbe80, HT_IsStdDotDot) | 0x00435f10 100% | 0x0041ab40 100% |
| | statics, initialised in the order crashEventRequest, crashEventData("Crash happened!"), crashEventResponse (initialisers 0x00a5b3d0..) | GLOBAL 0x00e25178 / 0x00e25198 / 0x00e2517c | 0x00fa5714 / 0x00fa5718 / 0x00fa579c |
| Win/VersionInfo.cpp/.h (new .cpp) | Load(HMODULE): GetModuleFileNameW → GetShortPathNameW → `Load(utf8_encode(wstring))` | 0x00463f50 100% | 0x0048bb90 100% |
| | Load(const std::string&): copies the name and calls utf8_decode once per Win32 call; LTCG passes `this` in esi and the name in ecx | 0x00464050 100% | 0x0048bc90 100% |
| | GetValue (format_string 0x00629910, VerQueryValueA) | 0x00464200 100% | 0x0048be40 100% |
| | GetFileVersionAsString | 0x00464370 100% | 0x0048bf40 100% |
| Win/VideoControl.cpp (new) / .h | **2013 VideoControl derives Ogre::IFrameDataCallback** (RTTI; vftable 0x00be517c, dtor + 4 pure slots). Layout from ctor 0x00464c30, size 0x120: recorded +4, videoQuality +8, a 0x100-byte path buffer +0xc, capture +0x10c, soundState +0x110, rbxView +0x114, frameRateManager +0x118, verb +0x11c. IVideoCapture has 10 slots. SoundState is three boost::functions (0x60) | ~VideoControl 0x00464dd0; the four frame callbacks 0x00464e50 / 0x00464e90 / 0x00464ed0 / 0x00464f00; SYNTHETIC 0x00464db0, 0x00464b30; SoundState dtor 0x00464b60; vftables 0x00be5164, 0x00be517c. All 100% | 0x0048c940, 0x0048c9c0 / 0x0048ca00 / 0x0048ca40 / 0x0048ca70, 0x0048c920, 0x0048d660, 0x0048c6e0; vftables 0x00d19d84, 0x00d19d9c. All 100% |
| WindowsClient/GameVerbs | LeaveGameVerb (holds the **Document**, not the View; doIt only posts WM_CLOSE to `doc→view→hWnd`; no user dtor, so its scalar deleting dtor is the shared 0x004382b0); SIZE 0x10 | ctor 0x00438220, doIt 0x004382d0, vftable 0x00be1bcc. All 100% | n/a (Qt verbs) |
| | RecordToggleVerb: 2013 layout (doc +0xc, game +0x10, scoped_ptr<VideoControl> +0x14, fileName +0x18, flags +0x34/+0x35, helper thread +0x38, boost::function job +0x40 (8-aligned buffer), three CEvents +0x60..+0x68, SIZE 0x70); the dtor (no abortCapture in 2013) | dtor 0x00439230 100%, isEnabled 0x00439330 100% (now that the vftable is emitted), SYNTHETIC 0x00439210 100% | |
| WindowsClient/View.h, Document.h (new) | View: hWnd +0xc, fullscreen +0x45, desireFullscreen +0x46, restoreWindowStyle +0x94, gfx view +0xa0. Document: View* +0 (placeholder name), Game's pointer +4 | | |

All VideoControl markers are FUNCTION and measured. Its slot names are placeholders (VTable0x04..):
no Mac or iOS build has the class.

CMakeLists.txt (shared, one edit): `OGRE_CLIENT_COMMON_SOURCES` gains Client/Win/DumpErrorUploader.cpp,
VersionInfo.cpp, VideoControl.cpp and VistaTools.cpp. It belongs in a small separate commit.

## 3. ReflectionMetadata (task item 3)

Not raised. writeProperty 0x00448050 (91.7) and writeEvent 0x00448750 (89.8) still get
get(Property) / get(Event) inlined. The original keeps both out of line even though each has only
one caller (I checked the whole binary). The bytes show isScriptable() is an inline flag test
(`[d+0x1c] >> 5 & 1`), so that is not the cause either. writeProperty's original frame is
8-aligned (`and esp,-8`), which ours may not be. I did not find the cause. The singleton
0x00447970 (90.6 PLAYER, 47.3 STUDIO) is still blocked on the CString/CPath helper conventions.

## 4. Stand-in entries (task item 4)

None removed. No new class instantiates a symbol listed in `Client/Win/placeholder.cpp` or
`Client/ClientShared/placeholder.cpp`: the RecordToggleVerb dtor destroys its boost::function
inline, and the stand-in covers only the copy ctor. The only thing retired is the LogManager.h
CrashReporter stand-in (section 1).

## Tried and reverted

View::SetFullscreen 0x00467880, written without its GameBasicSettings tail. LTCG then inlined it
into ToggleFullscreenVerb::doIt (64.9 → 58.2), and the generated stub kept the pairing (F vs E:
0 / 0). I reverted it. The fields it proved stay in View.h, with the reason noted there.

## Checks

- `check_2016_copies`: silent.
- `reccmp_addr_padding`: 1,689 files, 0 bad.
- `reccmp-decomplint` on Client/Win, Client/WindowsClient and Client/ClientBase, for PLAYER and
  STUDIO: clean.
- A GLOBAL on a constructor-call definition needs a comment nameref: crashEventData carries
  `// DumpErrorUploader::crashEventData`.
- LogManager.cpp's assert is still on line 354.
- Both shells build: PLAYER `--target player_client`, STUDIO `--target studio_client`.
- The final scored links are `build-sh/w12/linkE` and `build-sh-studio/w12/linkE`. The only change
  since is comment and members in View.h, with no codegen change.

## Files changed

- New:
  - Client/Win/DumpErrorUploader.h/.cpp
  - Client/Win/VersionInfo.cpp
  - Client/Win/VideoControl.cpp
  - Client/Win/VistaTools.h/.cpp
  - Client/WindowsClient/Document.h
- Changed:
  - Client/Win/LogManager.cpp (STUDIO markers and GLOBALs, flips) and LogManager.h (SIZE, STUDIO
    vtables, the Network header)
  - LogManagerExterns.h
  - VersionInfo.h (2013 ctor and dtor out of line, TRANSLATION)
  - VideoControl.h
  - Client/WindowsClient/GameVerbs.h/.cpp
  - View.h
  - CMakeLists.txt (one edit)
  - Client/_generated/{player,studio}/stubs_00.cpp and unresolved.txt (regenerated)
- Saved:
  - runs: `build-sh/w12/runs/{A,B,C,D,E,F,SB,SD,SE}*.json`
  - snapshots: `build-sh/w12/snaps/*`
  - scripts: `build-sh/w12/{ab.sh,snap.sh,link.sh,cmp.py,mk.py,sc.py,v.sh}`

## Unresolved

1. **Integrator:**
   - The CMakeLists.txt edit.
   - Rerun gen_link_stubs on the integrated build: my `_generated` files come from private links
     that left out the in-progress ConcurrentRakPeer.cpp.
2. **App/Base owner:**
   - `RBX::utf8_encode` 0x009b8060 and `RBX::utf8_decode` 0x009b8100 (Base, StringConv) have no
     name rows and no source. VersionInfo.cpp declares them, and `format_string` 0x00629910,
     locally.
   - `FLog::Verbs` and `FLog::CrashReporterInit` are still unresolved data, so every FASTLOG test
     in the shell reads 0. In 2016 they come from App's FastLogSettings.cpp.
   - App's GameSettings.h names +0x90 `postImageSetting`. VideoControl's ctor and startRecording
     pass +0x90 to IVideoCapture::setVideoQuality, which suggests the 2013 order differs. That
     blocks the VideoControl ctor.
3. **App owner, blocking the remaining verbs:**
   - `Game` (v8datamodel/Game.h; 2013 `getDataModel` has an out-of-line shell copy, 0x0041ce40,
     taking `this` in ecx and the result in eax). Needed by the RecordToggleVerb ctor 0x00439000,
     the ScreenshotVerb ctor/doIt 0x004382f0 / 0x00438460, and startAction / stopAction.
   - `GameBasicSettings` (fullscreen +0x7c, signal +0x64; singleton 0x00439bf0). Needed by
     View::SetFullscreen, then ToggleFullscreenVerb::doIt (64.9).
4. **Network owner:** CrashReporter.cpp's unoptimized region appears to instantiate
   `intrusive_ptr<thread_data_base>::~intrusive_ptr` unoptimized. That /Od copy is what 0x004069d0
   (STUDIO 0x004256d0) now pairs with, at 5.7%. My A link with HEAD shell objects had the same
   loss, so it is not from the shell. The RobloxCrashReporter ctor 0x0043f570 is 98.6 PLAYER
   only because CrashReporter's own ctor 0x00520070 is not paired yet.
5. **Rendering contributor:**
   - VideoControl.h declares `Ogre::IFrameDataCallback` (RTTI-proven, 5 slots) until RbxOgre has
     the header.
   - G3D Log.cpp and the hook setters are still unbuilt; LogManagerExterns.h declares them.
6. **Shell, next:**
   - The VideoControl ctor and startRecording (asserts 36/39/111/127), after item 2.
   - DSVideoCaptureEngine (Win, asserts 326–1233).
   - UserInput (0x0045e9xx–0x0046xxxx, asserts 148–892, DirectInput).
   - The remaining LogManager STUBs (CString helper conventions, after app-12's ATL callers land;
     remeasure then).
   - ReflectionMetadata writeProperty / writeEvent.
