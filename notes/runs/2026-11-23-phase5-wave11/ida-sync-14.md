# ida-sync-14 (workflow phase5-wave11)

The wave 11 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed (no .id0/.nam/.til left). Every IDA run was headless, one process per database, one at a time, through tools/capped.sh at 8G; none hit the cap. ida/Archive.zip, Client/ and the shared reccmp/ CSVs were not touched; nothing was committed.

**1. Snapshots.** ida/snapshots/PLAYER-20-wave11-sync.i64 and STUDIO-20-wave11-sync.i64, `cmp`-identical to the canonical databases before any change.

**2. Names from the scored runs** (build/sweep18/P-final.json and build-studio/sweep18/S-final.json, joined only with build*/sweep18/scored/).
- Functions at effective 100%: PLAYER 8,852, STUDIO 8,432 (sweep17: 8,649 / 8,292). Library-owned rows (1,739 / 1,653) left to apply_lib_names.py.
- PLAYER renamed 184: 166 `match:` (153 dummies, 5 IDA `j_` names, 8 stale names), 8 `rtti:` (5 vftable slots, 3 facts), 7 `src2016:`, 3 `str:`.
- STUDIO renamed 145: 129 `match:` (123 dummies, 28 of them via PLAYER body match; 6 stale), 7 `rtti:`, 7 `src2016:`, 2 `str:`.
- The stale names follow this wave's tree spellings: `FactoryProduct<Network::Server, Network::Peer, ...>` (was `<..., Instance, ...>`), `XmlElement::addAttribute<const Name*>`, the `DescribedCreatable<Network::Player,...,$0BD@>` ??_E, `resume_adapter<...>`, and `safe_static_init_mutex` over IDA's `j_safe_static_do_get_mutex`.
- **Conflicts:** PLAYER 121 (101 folded-original, 20 same-function), STUDIO 120 (100 / 19 / 1 keep-entry); no body-wins. New ones: 0x0044db50 (the rtti: fact name stays, see Unresolved), 0x0090e3d0 (`Name::getNullName` kept, folded with `ContentId()`), and two `thread_specific_ptr` same-function rows. STUDIO 0x0054b3b0 skipped as before.

**Facts for gen_link_stubs** (tools/ida/match_names.py). I named the integrate report's shell callees with the decorated names our tree declares, so every unresolved shell function with an original address now has an exact names.csv row: 13 in PLAYER, 12 in STUDIO. I checked each one from the exe bytes:

| PLAYER | STUDIO | Name | Tag | Evidence |
|---|---|---|---|---|
| 0x00520070 | 0x005d78c0 | CrashReporter::CrashReporter() | rtti: | stores `??_7CrashReporter@@6B@` 0x00b119e0 |
| 0x005200f0 | 0x005d7940 | CrashReporter::~CrashReporter() | rtti: | stores the same vftable; 'Closing crashreporter thread on destroy' (STUDIO had the FLIRT name `~_Locimp`) |
| 0x0051fcb0 | 0x005d7500 | CrashReporter::ProcessException | rtti: | vftable slot 1; MiniDumpWriteDump |
| 0x005205a0 | 0x005d7da0 | CrashReporter::Start | src2016: | SetUnhandledExceptionFilter, a boost::thread over `mf0<void,CrashReporter>` |
| 0x0051fad0 | 0x005d7320 | CrashReporter::GenerateDmpFileName | src2016: | its two callers are ProcessException and CreateFakeCrashDump |
| 0x009d5d10 | 0x006aa9e0 | G3D::Log::getCommonLogFilename | src2016: | 36 bytes, calls Log::common, directly after it |
| 0x009d63b0 | 0x006ab080 | G3D::setAssertionHook | src2016: | stores to 0x00db54b0, initialised to `_handleDebugAssert_` 0x009d6510 ('Assertion Failure') (STUDIO had the lib: name `Vector3(const Vector3int32&)`) |
| 0x009d63c0 | 0x006ab090 | G3D::setFailureHook | src2016: | stores to 0x00db54b4, initialised to `_handleErrorCheck_` 0x009d6780 |
| 0x0046a2e0 | 0x0048d690 | IsVistaPlus | src2016: | GetVersionExA, returns platform 2 && major >= 6 |
| 0x00435f10 | 0x0041ab40 | DumpErrorUploader::UploadCrashEventFile | str: | 'Crash Event post' strings |
| 0x00463f50 | 0x0048bb90 | CVersionInfo::Load(HINSTANCE) | src2016: | GetModuleFileNameW + GetShortPathNameW |
| 0x00464370 | 0x0048bf40 | CVersionInfo::GetFileVersionAsString | str: | 'FileVersion' |
| 0x00467880 | — | RBX::View::SetFullscreen(bool) | str: | calls View::changeResolution / restoreResolution, which their strings name |

- **lib-resolved** (tools/lib/lib_consistency.py). The wave 10 rtti-fact rule now also covers `str:` and `src2016:` facts (`FACT_TAGS`). Exactly one status changed: STUDIO 0x006ab080 went from located to conflict, so the next CSV regeneration drops its Vector3 library row. Selftest passes.

**3. Types** (import_types.py, dumped from the scored PDBs). Each class below has a ctor at 100% in sweep18, and its PDB size equals the tree's size annotation.

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| Reflection::Descriptor | 0x10 | ctor 0x00455710 / 0x004113e0 (was imported only as Type's base) |
| Reflection::Metadata::Item | 0x80 (// SIZE) | ctor 0x00402d10 / 0x00474000 |
| Reflection::Metadata::Class | 0xa4 (// SIZE) | ctor 0x00402e20 / 0x00474110 |
| XmlAttribute | 0x18 (DECOMP_SIZE_ASSERT) | 3 ctors each (0x00566f00 / 0x0061d2e0 / 0x006289c0) |
| ToggleFullscreenVerb (PLAYER only) | 0x14 (// SIZE) | ctor 0x00439750 |
| Network::Player (PLAYER only) | 0x1d8 (DECOMP_SIZE_ASSERT) | ctor 0x00550d40 (STUDIO's is below 100%) |

- Their Described/FactoryProduct chains came in as bases.
- Type, EnumDescriptor and SignatureDescriptor::Item now have size assertions in the tree, so I updated the CLASSES note (app-headers' request).
- Counts: local types PLAYER 307 (109 listed), STUDIO 293 (105); before, 290 / 282. `this` is typed on 460 / 424 functions (283 / 265 ctor/dtor); before, 427 / 416. No size mismatch and no type removed.
- 0x004382b0 is LeaveGameVerb's deleting destructor, ICF-folded and named for ToggleFullscreenVerb, so its `this` is now ToggleFullscreenVerb*. Both are Verb subclasses, so the type is close but not exact.
- **Not imported:**
  - LogManager: its dtor 0x00440780 is at 100%, but it has no size annotation.
  - MainLogManager: ctor at 84.7.
  - Network::Players and Server: ctor and dtor below 100%.
  - Still waiting from earlier waves: AssemblyHistory, GlobalAdvancedSettings, TaskScheduler, JointInstance, BallBall/BallBlockContact, Workspace.

**4. Unnamed functions** (ida/exports/PLAYER-app-unnamed-by-class.csv regenerated).
- PLAYER App block (0x00606a00–0x009ad3a0, 21,631 functions): ios:/mac:/match: 8,444 -> 8,473; 13,158 (60.8%) still without one.
- Network block (5,336 functions): 1,981 -> 2,005.

Largest App-block groups:

| Group | Functions |
|---|---|
| No caller with a known class | 4,062 (596 KB) |
| Ogre::RbxSceneManager | 306 (vftable slots) |
| FactoryProduct<>::Creator | 212 |
| ScriptContext | 191 |
| EnumDesc<> | 178 (was 180) |
| DataModel | 164 |
| RBX free functions | 159 |
| Ogre::RbxSceneNode | 104 |
| Ogre::ManualObject | 95 |
| Humanoid | 92 |
| ScriptDebugger | 86 |
| Creatable<> | 86 |
| PartInstance | 82 |
| RemoteFunction | 82 |
| SceneUpdater | 81 |

Network-block groups: Replicator 375, Players 160, Player 122, ServerReplicator 118, Server 56.

**5. Coverage, before -> after.** I regenerated the names, functions and types exports (PLAYER 737 type declarations, STUDIO 631; before, 720 / 620). Function starts are unchanged (49,091 / 44,815), so I did not rerun pair_names.py or body_match.py. The session note is in ida/LOG.md.

| | PLAYER | STUDIO |
|---|---|---|
| Named | 14,468 -> 14,645 | 15,136 -> 15,274 |
| match: | 6,620 -> 6,787 | 6,383 -> 6,515 |
| rtti: | 190 -> 189 (9 Metadata `Creator::create` slot names are now match:) | 188 -> 185 |
| src2016: | 3 -> 10 | 3 -> 10 |
| str: | 1 -> 4 | 1 -> 3 |
| ida: | 1,063 -> 1,064 | 1,813 -> 1,814 |
| lib: | unchanged | 47 -> 46 |

Globals are unchanged.

## Files changed
- ida/PLAYER.i64, ida/STUDIO.i64; ida/snapshots/{PLAYER,STUDIO}-20-wave11-sync.i64
- ida/LOG.md
- tools/ida/match_names.py (13 facts), tools/ida/import_types.py (6 classes, PLAYER_ONLY, CLASSES note), tools/lib/lib_consistency.py (`FACT_TAGS`)
- ida/exports/{PLAYER,STUDIO}-names.csv, -functions.csv, -types.h, -proven-types.json, -db-updates.csv, -match-conflicts.csv, -lib-resolved.csv; ida/exports/PLAYER-app-unnamed-by-class.csv
- ida/logs/{PLAYER,STUDIO}-20-db-updates.log/.tsv, -20-types.log, -20-export-names.log

## Unresolved
- **Integrator:** rerun tools/gen_link_stubs.py for both targets (delete the exe first). The CrashReporter, CVersionInfo, IsVistaPlus, DumpErrorUploader, View::SetFullscreen, G3D getCommonLogFilename and hook-setter callees now resolve. ~MainLogManager 0x00440600 should then reach 100%. Regenerate the CSVs so STUDIO 0x006ab080's Vector3 library row goes.
- **Not named (no address in the reports):** G3D::Log::println, CSHA1 (6 methods), and FLog::CrashReporterInit is data. These need an address first.
- **0x0044db50 (generator, gen_reccmp_csv.py):** lib-resolved has no row at this address. The `library` row with the std::exception name comes from the generator (reccmp/player-libdata.csv line 614), and reccmp pairs it with our std::exception thunk at 0x00706580 (mapped_file.obj). Our build has the right twin, `??_E?$error_info_injector@Vbad_placement_any_cast@rbx@@@exception_detail@boost@@WM@AEPAXI@Z`, at 0x00413ed0 (RenderSettingsItem.obj). 0x0044d400's lib row is already `conflict`, and libdata now carries the right name there.
- **Type candidates that need a tree change:**
  - LogManager: a size annotation (0x28 per shell-5k-2).
  - MainLogManager: its ctor.
  - Network::Players/Server: their ctors/dtors.
  - STUDIO Network::Player: its ctor.
  - Still waiting from earlier waves: TaskScheduler, JointInstance, BallBall/BallBlockContact, AssemblyHistory, GlobalAdvancedSettings, Workspace.
- **Carried from wave 10:**
  - pair_names.py proposals and STUDIO-PLAYER-bodymatch.csv predate STUDIO isUpperRoot 0x00a0e170.
  - STUDIO 0x004337a0 is still unnamed.
  - The program_options facet shell copies 0x00477aa0 / 0x00477c30 / 0x00477dd0 still carry filesystem names.
  - 0x009b7020 is still a dummy.
