# ida-sync-10 (workflow phase5-wave7)

The wave 7 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0, .nam or .til files left behind. Every IDA run was headless, one process per database, through tools/capped.sh at 8G. None hit the cap. ida/Archive.zip was not touched, and nothing under Client/ was edited. The interrupted earlier run had left nothing in my files (no -16 snapshot, no -16 logs, ida/ and tools/ida/ clean against HEAD), so there was nothing to review or revert.

**1. Snapshots.** Before any change I copied both databases to ida/snapshots/PLAYER-16-wave7-sync.i64 and STUDIO-16-wave7-sync.i64. cmp showed each copy identical to its canonical database.

**2. Names from the scored runs.** Sources: build/sweep14/P-final.json and build-studio/sweep14/S-final.json, joined only with build/sweep14/scored/ and build-studio/sweep14/scored/ (the exe, pdb, map and pdbview they scored).
- Functions at effective 100%: PLAYER 7,674, STUDIO 7,417 (sweep13: 7,508 / 7,269). Library-owned rows (1,740 / 1,645) are left to apply_lib_names.py.
- PLAYER: 108 renamed `match:`: 100 dummies, 1 IDA `j_` thunk name, 7 stale match: names. 1 unname (an existing fact row).
- STUDIO: 98 renamed `match:`: 91 dummies, 1 `j_` name, 4 stale match: names, and 2 FLIRT false positives with FUNC_LIB cleared:
  - `??1strstreambuf@std@@UAE@XZ_23` 0x007958a0 is the `rbx::signals::signal<void(StarterGuiService::CoreGuiType,bool)>` ctor.
  - `??1strstreambuf@std@@UAE@XZ_44` 0x0086c8b0 is the `signal<void(std::string,ContentId)>` ctor.
- **The stale names all come from util-target.md section 3** (`StatsItem` became `Stats::Item`, `sStatsService` became `Stats::sStats`). The 2013 PLAYER RTTI spells both `sStats@Stats` and `sStatsItem@Stats`, so the new names are the 2013 ones:
  - `doDeclare` / `callDoDeclare<Stats::sStats>` PLAYER 0x00480f00 / 0x00482690 (STUDIO callDoDeclare 0x00445d90).
  - `NonFactoryProduct<Instance,Stats::sStatsItem>::getClassName` 0x00485b10 / 0x0043e8f0 and the `<Stats::sStats>` one 0x00485b40 / 0x00445250. Both were the static `className`; reccmp now pairs the virtual `getClassName` (folded tiny bodies in our link).
  - `Described<Stats::Item,...>` ctor 0x0048d440, `fastDynamicCast<Stats::Item>` 0x00641940 / 0x0075f1f0, and the `Described<Stats::Item>::classDescriptor` static's atexit 0x00a901e0.
- The `j_` names: `callDoDeclare<sInsertService>` PLAYER 0x0055ad80 and `callDoDeclare<Network::sClient>` STUDIO 0x005ba110.
- **Conflicts:** PLAYER 117 (95 folded-original, 19 same-function, 3 oracle names kept by a fact). STUDIO 122 (99 / 17 / 3 / 2 body-wins, which are the FLIRT cases / 1 entry point). The STUDIO skip at 0x0054b3b0 stays (its name sits on the ICF twin 0x004f3770).
- **Facts changed in tools/ida/match_names.py:**
  - Removed the `keep` row on `doDeclare<Stats::sStatsItem>` 0x00480e90 / 0x0043f250. It held the iOS name only because our tree spelled `RBX::sStatsItem`. The tree now spells `RBX::Stats::sStatsItem`, and reccmp's 100% name is the RTTI's (`??$doDeclare@$E?sStatsItem@Stats@RBX@@3QBDB@Name@RBX@@SAABV01@XZ`). The iOS name agrees by signature, and the function now carries a `p4 match:` line. The selftest passes.
  - Kept the three `Stats::Item::formatValue<T>` keep rows, because our `formatValueId/If/Ii` are still the prior tree's invented spellings.

**3. Types.** New classes. Each has a constructor or destructor at effective 100% in sweep14 in both targets, and a PDB size equal to the tree's DECOMP_SIZE_ASSERT or `// SIZE` (checked for every one):

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| NetworkSettings | 0x120 | ctor 0x005b7cf0 / 0x00609e80 (was 0.474 in sweep13) |
| InsertService | 0x188 | ctor 0x007db980 (STUDIO 0x008775b0 at 0.993), dtor 0x007dbbf0 / 0x00877820 |
| Profiling::CodeProfiler | 0xc040 | ctor 0x00745230 / 0x007d7a90 (base Profiling::Profiler 0xc040) |
| Profiling::Mark | 0x30 | ctor 0x007454f0 / 0x007d7d50 |
| StepJointsStage | 0x20 | ctor 0x008ef410 / 0x00a0d190, dtor 0x008ef340 / 0x00a0d0c0 |
| SpatialHash<Primitive,Contact,ContactManager,4>::TreeNode | 0x30 | dtor 0x007b36f0 / 0x008cf770 (base NodeBase 0x14) |

- **Bases imported with them:** the NetworkSettings `GlobalAdvancedSettingsItem` / FactoryProduct / Described chain, InsertService's Described / DescribedCreatable chain, and the `boost::intrusive::list_base_hook` types for StepJointsStage and MovingAssemblyStage.
- **Changed layouts that were already imported:**
  - SpanningEdge is now 0x4 (no data members; Joint's list hooks are its own bases, world-5d.md). Joint stays 0x94.
  - MovingAssemblyStage, Primitive and Joint changed members but kept their sizes. PLAYER RunService also changed members, at the same 0xb0.
- **A fix in tools/ida/import_types.py.** For sizes, array lengths and member offsets of 32 KB and more, cvdump prints a numeric leaf, e.g. `Size = (LF_USHORT) 49216`. The dump's regexes did not accept that form, so CodeProfiler's definition counted as missing (KeyError). The three regexes now accept it.
- **Local types:** PLAYER 263 (92 listed), STUDIO 254 (90); they were 242 and 233. `this` is now typed on 351 / 339 functions (221 / 207 of them ctor or dtor); it was 330 / 318. No type failed to apply and there were no size mismatches. No type left the set, so del_types.py was not needed.
- **Still not imported:**
  - SpatialHash itself (ctor 0x008eef80 is at 100%, but the tree has no size assertion).
  - StarterGuiService (0x98, ctor 0.974 because of the prime_list end pointer).
  - AssemblyHistory (0x20, ctor 0.08).
  - GlobalAdvancedSettings, STUDIO ScreenGui and RunService, Workspace and TaskScheduler.

**4. Unnamed functions.** I regenerated ida/exports/PLAYER-app-unnamed-by-class.csv.
- PLAYER App block (0x00606a00-0x009ad3a0, 21,631 functions): those with an ios:/mac:/match: name went from 8,043 to 8,126. 13,505 (62.4%) still have none.
- Network block (0x0051f6b0-0x00606a00, 5,336 functions): 1,865 to 1,876. 3,460 (64.8%) still have none.

Largest App-block groups still unnamed:

| Group | Functions |
|---|---|
| No caller with a known class | 4,082 (597 KB), was 4,107 |
| Ogre::RbxSceneManager | 306 (vftable slots) |
| FactoryProduct<>::Creator | 212 |
| ScriptContext | 191 |
| EnumDesc<> | 180 |
| DataModel | 179, was 180 |
| RBX free functions | 159 |
| Ogre::RbxSceneNode | 104 |
| Ogre::ManualObject | 95 |
| Humanoid | 93 |
| Scripting::ScriptDebugger | 88 |
| Creatable<> | 88 |
| InsertService | 85, was 94 |

ContactManager went from 61 to 59 and World from 44 to 34.

**5. Name coverage, before → after.** I regenerated the names, functions and types exports (PLAYER 692 type declarations, STUDIO 591), and ida/LOG.md has the session note.

| | PLAYER (49,091 functions) | STUDIO (44,815 functions) |
|---|---|---|
| Named | 13,597 → 13,702 | 14,333 → 14,429 |
| match: | 5,761 → 5,862 | 5,593 → 5,687 |
| ida: | 1,066 → 1,070 | 1,815 → 1,819 |
| flirt: | unchanged | 182 → 180 |
| others (ios:, mac:, rtti:, lib:, prior:) | unchanged | unchanged |

ida: went up because IDA adds `j_` names for thunks to newly named functions. Globals are unchanged in both databases. pair_names.py, body_match.py and lib_consistency.py were not rerun, because no function start changed.

## Files changed

- ida/PLAYER.i64, ida/STUDIO.i64
- ida/snapshots/PLAYER-16-wave7-sync.i64, ida/snapshots/STUDIO-16-wave7-sync.i64
- ida/LOG.md
- tools/ida/match_names.py (the doDeclare<Stats::sStatsItem> keep row removed)
- tools/ida/import_types.py (6 wave7 classes in CLASSES; numeric-leaf sizes and offsets accepted)
- ida/exports/{PLAYER,STUDIO}-names.csv, -functions.csv, -types.h, -proven-types.json, -db-updates.csv, -match-conflicts.csv
- ida/exports/PLAYER-app-unnamed-by-class.csv
- ida/logs/{PLAYER,STUDIO}-16-db-updates.log/.tsv, -16-types.log, -16-export-names.log

## Unresolved

- **Stale pairing data:** the pair_names.py proposals and STUDIO-PLAYER-bodymatch.csv still predate STUDIO isUpperRoot 0x00a0e170.
- **STUDIO 0x004337a0**, ContentId(const string&) (the shell's /GS copy), is still unnamed.
- **Stats owner:**
  - `TypedStatsItem<int>::update` 0x0048e7a0 and `<bool>` 0x0048e7d0 have no body in the tree. They get names once util-target.md's TypedStatsItem rewrite lands.
  - The `formatValue<T>` keep facts stay until the tree spells them as the template.
- **Type candidates that need a tree change first:** SpatialHash (add a size assertion), StarterGuiService (the prime_list pairing), and AssemblyHistory, GlobalAdvancedSettings and Workspace (their ctors are not at 100%).
