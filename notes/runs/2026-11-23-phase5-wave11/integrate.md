# integrate (workflow phase5-wave11), 2026-11-24

I integrated the three phase5-wave11 Work agents: app-headers, shell-5k-2 and network-5f-2. All three finished, and their uncommitted work compiles in both targets from clean, so nothing was reverted. I kept shell-5k-2's two CMakeLists.txt lines (`Client/Win/LogManager.cpp` in `OGRE_CLIENT_COMMON_SOURCES`, `Client/WindowsClient/GameVerbs.cpp` in `player_client`); they belong in a small separate commit.

I rebuilt both targets from clean, ran gen_link_stubs, regenerated the CSVs until a round gained nothing, swept the markers and ran every check. I also made two generator fixes that the shell agent asked for (see Generator). Both targets are stable:
- PLAYER held **11,673** across P21–P24 and PA1–PA4.
- STUDIO held **11,071** across S15–S18 and SA1–SA4.

| | sweep17 | Lost | Gained | sweep18 |
|---|---|---|---|---|
| PLAYER | 11,435 | 8 | 246 | **11,673** (+238) |
| STUDIO | 10,908 | 0 | 163 | **11,071** (+163) |

Every loss was read with `--verbose` (see Losses). None needed a fix I could make inside my files: 6 are the Lua `0x7fffff` no-.reloc coincidence, 1 was a wrong-name pairing in sweep17, and 1 is an LTCG frame-size change in the shell.

- Every heavy command ran through tools/capped.sh. Only one build ran at a time; scoring and regeneration overlapped with the other target's build. Nothing hit the cap.
- No commits, no IDA database opened, nothing under Client/Rendering/ edited. The 2016-copy check is silent.

## What I did

**Clean rebuilds**
- `app_sweep set FUNCTION` first (547 PLAYER and 550 STUDIO markers flipped).
- PLAYER: build/ clean, 685 steps, 0 failures.
- STUDIO: build-studio/ clean, `--target studio`, 677 steps, 0 failures.

**Link stubs.** For each target I ran `gen_link_stubs.py --build <dir> <T>` (it deletes the exe itself), then relinked.
- PLAYER has 49 unresolved symbols (was 31) and STUDIO 47. Both gain 2 stubs, 11 → 13 addresses:
  - PLAYER: `FileSystem::getUserDirectory(bool, FileSystemDir, const char*)` 0x006ea4c0 and `G3D::Log::Log` 0x009d5b70.
  - STUDIO: 0x006aa840 and 0x00772800.
- The other new shell callees stay `unknown` because ida/exports/PLAYER-names.csv has no row that names them. These are CrashReporter (5), CVersionInfo (2), IsVistaPlus, DumpErrorUploader, View::SetFullscreen, G3D::Log::common/getCommonLogFilename/println, the G3D hook setters and CSHA1. gen_link_stubs reads only names.csv and has no override, so they need `match:`/`prior:` rows from the IDA owner (see Unresolved).
- `RBX::operator<<(ostream&, const Name&)` is now defined (app-headers) and has left the unresolved list.

**Generator** (tools/ida/gen_reccmp_csv.py, my file):
1. **Unnamed `__ehhandler$` stubs.** `aligned_refs` now pairs a pushed EH-handler address when both sides point at a `/GS` handler stub (`mov edx,[esp+8]; lea eax,[edx+N]`, constant `EH_STUB`) and ours has no PDB name. `data_rows` then writes the original address as a 1-byte anonymous global, the same row the relocation path already wrote.
   - The relocation path now treats a nameless PDB entry the same as no entry. The LTCG stubs (for example 0x0072c3d0 for wstring::_Construct) are listed in the PDB without a name, so before this change they were never written once their function reached 100%.
   - The first version left that gap open. The result oscillated: 10 entries alternated between 11,663 and 11,673 for 4 rounds (P15–P18), until the relocation-path change fixed it.
   - All 13 near misses shell-5k-2 listed are now 100%: 0x004010f0, 0x0040c2c0, 0x00413600, 0x00415700, 0x004198a0, 0x00440f90, 0x00441b90, 0x00449d40, 0x00474df0, 0x00474e90, 0x004777e0, 0x00477880, 0x0047a820. So are 0x0043d1d0 (`shared_count<GameSettings*>`) and 0x009be050 (`shared_count<filesystem_error::m_imp>`).
   - `tools/ehhandler_near_misses.py` now finds 0 such entries in the shell range.
2. **Byte index tables.** `true_size` now extends a function over the byte index table VC9 puts after the dword jump table (bytes below the dword count, up to the int3 padding). The rule is in the function's docstring.
   - Example: LogManager::ReportEvent 0x00440a70 goes 328 → 345 and is now 100%.
   - The change moved 40 PLAYER and 37 STUDIO size rows. I checked 0x0041b990 (72 → 153, an 81-byte table of indices 0–3) and 0x00444020 against the original bytes.
   - No entry was lost to it: the 8 PLAYER losses are the same 8 that the agents measured with the old generator.

**Regeneration rounds** (build/sweep18/rounds.sh). Entries at effective 100%:
- PLAYER: 11,407 → 11,541 → 11,608 → 11,634 → 11,657 → 11,663 → 11,673 / 11,663 (oscillating, fixed above) → P21–P24 11,673.
- STUDIO: 10,882 → 10,961 → 11,005 → 11,025 → 11,058 → 11,063 → 11,070 → 11,071 → 11,071.
- After apply: PA1–PA4 11,673 and SA1–SA4 11,071, 0 lost and 0 gained against P24/S18.

**Sweep.** I ran `app_sweep apply P24.json S18.json`. In the swept scope it wrote:

| | FUNCTION | STUB | FOLDED |
|---|---|---|---|
| PLAYER | 1,390 | 553 | 49 |
| STUDIO | 1,330 | 555 | 49 |

Beyond the agents' own marker words, it changed:
- `genericConvert<ProtectedString>`, PLAYER 0x0066f7f0 / STUDIO 0x00780350 (reflection/Type.h:300): promoted STUB → FUNCTION in both targets.
- The four new stubs in Client/_generated: STUB.

Against HEAD (build/sweep18/markers.py HEAD), PLAYER has 3 promoted, 10 new FUNCTION and 6 new STUB markers; STUDIO has 3, 6 and 2. There are no demotions.
- **Shell markers** (outside app_sweep's scope, build/sweep18/shellcheck.py): I flipped two of shell-5k-2's markers in Client/Win/LogManager.cpp to FUNCTION, because both are at effective 100% in P-final:
  - `MainLogManager::fastLogMessage` 0x0043ec40: effective, raw 79.8.
  - `LogManager::ReportEvent` 0x00440a70: 100% after the size fix.

  The shell now holds PLAYER 50 FUNCTION + 27 STUB and STUDIO 23 + 4, with no FUNCTION below 100% and no STUB at 100%.
- stubcheck: no STUB marker outside Rendering sits on an address at effective 100%.

**Reorder.** `reorder_markers plan` finds no fixable file: 77 files have descents, and each is pinned by asserts, crosses runs or has a STUDIO-only chunk (build/sweep18/reorder-plan.txt). decomplint shows two new descents, both pinned:
- PhysicsInstructions.cpp (PLAYER and STUDIO): the radius changers sit on 2013 assert lines 55/63 and VC9 emits them before the constructor. This is the out-of-source-order exception that CLAUDE.md allows.
- Network/Player.cpp (PLAYER): registerLocalPlayerNotIdle is held at its 2013 assert line 838.

So nothing was moved, and step 2 needed no rebuild.

## Final figures (sweep17 in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| **Entries at effective 100%** | **11,673** (11,435) | **11,071** (10,908) |
| Non-library entries | 9,045 (8,841) | 8,545 (8,426) |
| **Functions at 100%** | **8,782** (8,579) | **8,382** (8,242) |
| **Non-library functions at 100%** | **6,221** (6,052) | **5,903** (5,807) |
| Functions at 100%: owner / Rendering (minus g3d) / third party | 6,209 / 2 / 2,571 (6,041 / 3 / 2,535) | 5,891 / 3 / 2,488 (5,796 / 3 / 2,443) |
| Vftables at 100%: owner / Rendering / third party | 2,809 / 1 / 11 (2,773 / 1 / 12) | 2,632 / 1 / 6 (2,607 / 1 / 8) |
| Vftables at 100% / paired | 2,821 / 3,656 (2,786 / 3,619) | 2,639 / 3,594 (2,616 / 3,568) |
| reccmp Implemented, with libraries | 10,298 / 40,551 (10,140) | 9,931 / 42,384 (9,804) |
| reccmp Implemented, `--nolib` | 7,495 / 37,709 (7,363 / 37,734) | 7,264 / 39,595 (7,173 / 39,631) |
| App markers: FUNCTION + FOLDED + STUB | 1,211 + 27 + 506 (1,203 + 27 + 508) | 1,165 + 27 + 514 (1,158 + 27 + 516) |
| Base markers | 92 + 0 + 19 (unchanged) | 86 + 0 + 20 (unchanged) |
| Network markers | 84 + 1 + 16 (79 + 1 + 11) | 76 + 1 + 12 (74 + 1 + 11) |
| Shell markers (FUNCTION + STUB) | 50 + 27 (37 + 7) | 23 + 4 (23 + 4) |

The owner/third-party split comes from the .obj in the link map. A third-party vftable or Rendering function moving between columns is a reclassification of the obj, not a lost entry: summ.py lists every loss, and none of them is a vftable.

**Agents' private figures, against the integrated tree:**

| Agent | PLAYER | STUDIO |
|---|---|---|
| app-headers | 11,647 | 10,981 |
| shell-5k-2 (final link D) | 11,600 | n/a |
| network-5f-2 | 11,653 | 10,997 |
| **Integrated** | **11,673** | **11,071** |

**Rows** (PLAYER, tools/row_coverage.py on P-final; per-function CSVs in build/sweep18/row-*.csv). The "sweep17" column is sweep17's P-final measured with today's ranges, so both columns cover the same functions. Row 5b's range grew from 2,452 to 2,916 functions since wave 10's report.

| Row | At 100% / total | Share | sweep17, same ranges |
|---|---|---|---|
| 5a | 139/155 | 89.7% | 139 |
| 5b | 1,079/2,916 | 37.0% | 1,053 |
| 5c | 105/158 | 66.5% | 105 |
| 5d | 465/1,554 | 29.9% | 464 |
| 5e | 502/1,335 | 37.6% | 502 |
| 5f | 678/3,281 | 20.7% | 653 |
| 5g (row_coverage_5g.py) | 322/609 | 52.9% | 322 |
| 5h | 533/1,359 | 39.2% | 532 |
| 5k | 923/3,493 | 26.4% | 810 |

- 5f 678 is 1 above network-5f-2's 677.
- 5k 923 is well above shell-5k-2's 873. The ehhandler and jump-table rows account for 16 of the difference; the rest is app-headers' Object.h/Name work reaching shell templates.
- 5b +26 comes mostly from app-headers: Descriptor, Type<T> singletons, isInRole and canAdd.

**Near misses** (app_sweep report, build/sweep18/app-sweep.csv):

| Target | Short | inlining-size | ebp-frame | other | unpaired | unpaired-callee | x87-vs-sse | assert-file-line | data-ref | stack-cookie | At 90% or more | Pairing-only |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| PLAYER | 553 | 166 | 168 | 127 | 48 | 17 | 20 | 4 | 3 | 0 | 63 | 20 |
| STUDIO | 555 | 164 | 164 | 130 | 37 | 32 | 20 | 4 | 3 | 1 | 85 | 35 |

The closest PLAYER near misses have not changed since sweep17:

| Function | Address | Score | Cause |
|---|---|---|---|
| LegacyContentTable ctor | 0x008f08a0 | 99.97 | prime_list string coincidence |
| TaskScheduler::remove | 0x009b52b0 | 99.49 | pairing only |
| ScriptContext::setTimeout | 0x0067eee0 | 99.41 | pairing only |
| DataModel ctor | 0x0064ec50 | 99.40 | prime_list string coincidence |
| TextXmlWriter::serializeNode | 0x00628a60 | 99.39 | |
| TaskScheduler::setThreadCount | 0x009b7ec0 | 99.26 | pairing only |
| TaskScheduler::getJobsInfo | 0x009b75f0 | 99.21 | pairing only |
| World::createAutoJoints | 0x0074aa80 | 99.14 | pairing only |
| GuiService::openBrowserWindow | 0x0069a540 | 99.08 | pairing only |
| TaskScheduler::getJobsByName | 0x009b78f0 | 98.99 | pairing only |
| DebrisService::addItem | 0x00845000 | 98.98 | pairing only |
| InsertService::insert | 0x007d6a20 | 98.90 | pairing only |
| TaskScheduler ctor | 0x009b55e0 | 98.88 | |
| World::destroyAutoJoints | 0x00749340 | 98.84 | |

## Losses (against sweep17's P-final / S-final, summ.py, each read with --verbose)

STUDIO lost none. The STUDIO losses app-headers saw (11 Network::Player functions and one `enable_current_exception` twin) are gone in the integrated tree, which has network-5f-2's finished Player.

PLAYER lost 8:
- **Lua, 6 entries:** symbexec 0x008a8330, luaV_execute 0x0098ad00, constructor 0x0098f9a0, patchtestreg 0x009a6800, removevalues 0x009a6880 and luaK_setreturns 0x009a6c30.
  - Each diff is one `and edx, 0x7fffff`-style immediate that our side prints as `crc_table[1][36] (OFFSET)`. It is the no-.reloc coincidence CLAUDE.md describes: one of our data addresses now equals the immediate.
  - All three agents measured the same 6 before my generator change. Not a code fix.
- **`boost::throw_exception` 0x0044d400 (84.0).**
  - In sweep17 it scored 100% against our `throw_exception<std::exception>` (mapped_file.cpp.obj). That pairing came from the wrong `library` name wave 10 reported.
  - It now pairs with the right body, our shell `throw_exception<rbx::bad_placement_any_cast>` (RenderSettingsItem.cpp.obj, LTCG). That body's frame is 4 bytes larger (`sub esp,0x50` against 0x4c).
  - So the sweep17 100% was a false match. This entry is not a regression.
- **`GlobalAdvancedSettingsItem<CRenderSettingsItem>` ctor 0x00456ee0 (84.3).**
  - Shell LTCG (RenderSettingsItem.cpp.obj). The only diff is the frame: ours is 4 bytes smaller (`sub esp,0x28` against 0x2c), with every `[esp+N]` shifted.
  - The body calls only the Described ctor, `std::string(const char*)`, `Instance::setName` and `RBX::runtime_error`, not `Creatable::create`. So the Object.h `&create` change is not an obvious cause.
  - It is an LTCG frame-packing change from one of this wave's header edits. I could not isolate it in the time available: that needs a rebuild per candidate header. Kept, because the same edits gain 246.

## Checks

| Check | PLAYER (sweep17) | STUDIO (sweep17) |
|---|---|---|
| decomplint: function_out_of_order | 309 (307) | 314 (313) |
| decomplint: duplicate_offset | 12 (12) | 4 (4) |
| reccmp-vtable: traceback | none | none |
| reccmp-vtable: "Problem with adjuster thunk" lines | 13 (13) | 15 (15) |
| reccmp-datacmp: traceback | none | none |
| reccmp-datacmp: variables | 2,937 (2,881) | 2,819 (2,782) |
| reccmp-datacmp: issues | 56 (55) | 59 (58) |
| check_folded: annotated addresses | 2,785 (2,734) | 2,563 (2,553) |
| check_folded: folding problems | 0 | 0 |
| check_folded: "Debug data out of sync" lines | 19 (19) | 19 (19) |
| reccmp_addr_padding | 1,679 files, 0 bad | |
| check_2016_copies | silent | |

- decomplint exits 1 under `--warnfail`, as in every sweep. The new descents are the pinned PhysicsInstructions.cpp (both targets) and Player.cpp (PLAYER) entries above.
- **The one new datacmp issue in each target is `FLog::Network`**: the original's byte at PLAYER 0x00d30f1b is **6**, and ours is 1. app-headers wrote `LOGVARIABLE(Network, 1)` in ScriptContext.cpp:81, so it should be `LOGVARIABLE(Network, 6)`. That line is outside my files, so I left it for the script owner.
- The Name.h "out of sync" line moved from 36 to 37 (the header grew by a line). The count is unchanged.

## Saved

- Final JSONs: build/sweep18/P-final.json (= PA4) and build-studio/sweep18/S-final.json (= SA4).
- Scored exe, pdb, map and pdbview: build/sweep18/scored/ and build-studio/sweep18/scored/.
- Alongside:
  - every round's JSON and .txt, gen logs, rounds-*.log
  - P/S-nolib.json and .txt, -vtable.txt, -datacmp.txt, -folded.txt, {PLAYER,STUDIO}-decomplint.txt
  - app-sweep.csv, row-*.csv, reorder-plan.txt, markers-before.txt / markers-after.txt, stubs.log
  - pre.diff / post.diff / pre-status.txt / pre-untracked.tgz (the agents' tree before my changes)
- Helper scripts: the sweep17 set with paths moved to sweep18, plus shellcheck.py, all in build/sweep18/.

## Files changed by me

- tools/ida/gen_reccmp_csv.py: `EH_STUB` and the aligned-ref / nameless-entry pairing of `__ehhandler$` stubs; `true_size` over byte index tables.
- reccmp/: regenerated player-* and studio-* (boost, function-size, g3d, libdata, libzip, lua, raknet, symbols, zlib).
- Client/_generated/{player,studio}/stubs_00.cpp and unresolved.txt, through gen_link_stubs.
- Marker words:
  - genericConvert<ProtectedString> in reflection/Type.h:300, through app_sweep.
  - Client/Win/LogManager.cpp:62 and :474, STUB → FUNCTION.
  - Every other marker word app_sweep wrote is the same as the agents left it.
- CMakeLists.txt: shell-5k-2's two source-list lines kept (no change of my own).
- No change to tools/app_sweep.py or tools/reorder_markers.py.

## Unresolved

- **FLog::Network default is 6, not 1** (datacmp, PLAYER 0x00d30f1b; STUDIO the same). This is one token in Client/App/script/ScriptContext.cpp:81 (script/app owner).
- **GlobalAdvancedSettingsItem<CRenderSettingsItem> ctor 0x00456ee0**: 4 bytes less LTCG frame than the original since this wave. Not isolated; bisect the wave's header edits (Name.h, Object.h, Descriptor.h, SecurityContext.h) with one rebuild each.
- **throw_exception<rbx::bad_placement_any_cast> 0x0044d400** (84.0, now correctly paired): our shell LTCG body has 4 bytes more frame. The IDA owner should also drop the wrong `library` names at 0x0044d400 and 0x0044db50 in lib-resolved, as wave 10 said.
- **New shell callees without a stub** (IDA owner: name them in ida/exports/PLAYER-names.csv / STUDIO-names.csv so gen_link_stubs binds them):
  - CrashReporter: ctor 0x00520070, dtor 0x005200f0, ProcessException 0x0051fcb0, Start 0x005205a0, GenerateDmpFileName 0x0051fad0
  - G3D::Log: common 0x009d5c60, getCommonLogFilename 0x009d5d10
  - the G3D hook setters 0x009d63b0 / 0x009d63c0
  - IsVistaPlus 0x0046a2e0
  - DumpErrorUploader::UploadCrashEventFile 0x00435f10
  - CVersionInfo::Load 0x00463f50 and GetFileVersionAsString 0x00464370
  - View::SetFullscreen 0x00467880

  ~MainLogManager 0x00440600 (99.1) waits on the CrashReporter dtor stub.
- **Lua `0x7fffff` coincidence** (6 PLAYER entries): a data address coincidence; it moves with any change in our data layout.
- **prime_list string coincidence** (DataModel ctor 0x0064ec50, LegacyContentTable 0x008f08a0 and others): unchanged from wave 10.
- **Carried from the agents** (owner edits outside this task):
  - app-headers: Type<Region3int16> needs `Reflection/Type.h` in the BasicPartInstance TU; the Variant converts at 62–83%; Name::operator<; ObjectBridge::newInstance 67.5; import_types.py CLASSES note.
  - shell-5k-2: App's ATL helper callers (0x006ea4c0, 0x006ea260, 0x006eab90, 0x00624210, 0x00664260, 0x00623230); Util/FileSystem.h's FileSystemDir; network/CrashReporter.h; G3D Log.cpp (Rendering); ReflectionMetadata writeProperty/writeEvent/writeAPI; the remaining shell classes and STUDIO LogManager markers.
  - network-5f-2: Player dtor 91.3; setCharacter and the `__declspec(noinline)` stand-ins; the missing Players/Server vftable overrides; the Players ctor/dtor; Server ctor/dtor; IdSerializer/Replicator.
