# ida-sync-9 (workflow phase5-wave6)

The wave 6 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0, .nam or .til files left behind. Every IDA run was headless, one process per database, through tools/capped.sh at 8G. None hit the cap. ida/Archive.zip was not touched, and nothing under Client/ was edited.

**1. Snapshots.** Before any change I copied both databases to ida/snapshots/PLAYER-15-wave6-sync.i64 and STUDIO-15-wave6-sync.i64. cmp showed each copy identical to its canonical database.

**2. Names from the scored runs.** Sources: build/sweep13/P-final.json and build-studio/sweep13/S-final.json, joined only with build/sweep13/scored/ and build-studio/sweep13/scored/ (the exe, pdb, map and pdbview they scored).
- Functions at effective 100%: PLAYER 7,508, STUDIO 7,269. Library-owned rows (1,740 / 1,645) are left to apply_lib_names.py.
- PLAYER: 93 renamed `match:`. 75 were dummies, 2 were IDA `j_` thunk names, and 16 were stale match: names.
- STUDIO: 85 renamed `match:`. 65 were dummies, 1 more came from PLAYER through the body match, 2 were `j_` names and 16 were stale match: names. One FLIRT false positive had its FUNC_LIB flag cleared: 0x0060af40, `??1strstreambuf@std@@UAE@XZ_5`, is the `rbx::signals::signal<void(string,int,string)>` ctor.
- **The stale names, all requested by settings-serialization.md:**
  - The settings items' FactoryProduct members (getClassName, Creator::getClassName, static_getCreator) for Debug, Game, Network and TaskSchedulerSettings now spell the base `VItem@GlobalAdvancedSettings@2@`.
  - `??_E?$GlobalAdvancedSettingsItem@VDebugSettings…OBM` (PLAYER 0x0065fac0 / STUDIO 0x00778a40) and `??_ETaskSchedulerSettings@RBX@@WBM` (0x0065eda0 / 0x00778b70).
  - PLAYER 0x00644360 / STUDIO 0x0073fce0 is the folded ServiceProvider/DataModel slot-0 thunk, `??_E?$DescribedNonCreatable@VDataModel…WBM`, no longer `??_EGlobalBasicSettings`.
  - `singleton_pool<Body,0xEC>::free` 0x00756970 / 0x00895c40 (Body is 0xec now).
  - The `j_` names: `callDoDeclare<sGlobalAdvancedSettings>` 0x006581a0 / 0x00740930, and `~IndexArray<IndexedTree,getIndex>` 0x00708a80 / 0x00a04f10.
- **A fix in match_names.py.** Our linker folds several names at one recompiled address. The script took the one reccmp reports only when reccmp printed it demangled, so a decorated report fell back to the first name alphabetically. Before the fix, PLAYER 0x00708a80 was planned as `~IndexArray<Contact,steppingIndexFunc>`. It now gets reccmp's `~IndexArray<IndexedTree,getIndex>`, which matches IDA's own thunk target `j_??1?$Array@PAVIndexedTree`. The selftest passes.
- **Conflicts:** PLAYER 121 (96 folded-original, 21 same-function, 4 oracle names kept by a fact). STUDIO 122 (99 / 18 / 3 kept by a fact / 1 body-wins, which is the FLIRT case / 1 entry point). The STUDIO skip at 0x0054b3b0 stays (its name sits on the ICF twin 0x004f3770).
- **Facts** (FACTS rows in tools/ida/match_names.py, checked against both exes with capstone):

| Name | PLAYER | STUDIO | Tag | Evidence |
|---|---|---|---|---|
| `?__TBB_LockByte@@YAIAAE@Z` (respelled from `…YAEAAE…`) | 0x00402240 | 0x00443510 | lib: | TBB 2.1's `__TBB_LockByte` returns uintptr_t. reccmp scores it 100% against our function of that exact name. The callers keep `mov ebx,eax … mov [esi],bl` (settings-serialization.md §6) |
| `??0ContentId@RBX@@QAE@PBD@Z` (`RBX::ContentId::ContentId(char const*)`) | 0x004c4720 | 0x004885a0 | ios: | iOS 1029 has `_ZN3RBX9ContentIdC2EPKc`. The body is byte for byte the twin of ContentId(const string&) 0x004c4780 (match:) except for the import it calls: basic_string(const char*) `[0x00aa130c]` (STUDIO `[0x00b4e520]`, the shell's /GS copy) against basic_string(const string&) `[0x00aa12f4]`. ContentProvider::readContent builds it in the return slot |

- `FactoryProduct<LuaSettings,…>::createInstance` 0x00654190 was already a dummy at 100% in sweep13, so the bulk pass named it `match:`.

**3. Types.** New classes. Each has a constructor or destructor at effective 100% in sweep13 in both targets, and a PDB size equal to the tree's DECOMP_SIZE_ASSERT (checked for every one):

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| SleepStage | 0x14c | ctor 0x008e7f50 / 0x00a06180, dtor 0x008e7160 / 0x00a053a0 |
| Primitive | 0x170 | dtor 0x00755540 / 0x00894860 (ctor 0.989) |
| MultiJoint | 0xcc | dtor 0x009741c0 / 0x009bf3a0 |
| DebugSettings | 0x7c | ctor 0x0065fa10 / 0x00778980 |
| GameSettings | 0x94 | ctor 0x006f6030 / 0x00792df0 |
| TaskSchedulerSettings | 0x68 | ctor 0x0065fb60 / 0x00778ae0 |
| LuaSettings | 0x88 | ctor 0x0085e360 / 0x00948980 |
| StarterGear | 0x60 | ctor 0x006cb3e0 / 0x007b33e0 |
| MemoryBinder | 0x20 | ctor 0x006fb6e0 / 0x00749c70 |
| ArchiveBinder | 0x2c | ctor 0x006faeb0 / 0x00749620, dtor 0x006fb460 / 0x007499f0 |
| XmlWriter | 0x14 | ctor 0x00628a20 / 0x0074c890 |
| XmlElement | 0x28 | ctor 0x00567110 / 0x005cac50 |

- **Bases imported with them:** the `GlobalAdvancedSettingsItem<T, sT>` (0x64), FactoryProduct, Described and DescribedCreatable chains, Joint (0x94), IndexedMesh, IndexedTree, SpanningNode, SpanningEdge, BasicSpatialHashPrimitive, MergeBinder, IReferenceBinder, XmlNameValuePair, and Parent/Sibling<XmlElement>.
- **IStage is now 0xc**, against 0x10 before: its `world` pointer moved to IWorldStage. SleepStage's 100% ctor and dtor prove the new layout. IWorldStage and ScriptContext also changed members, following this wave's headers.
- **Nothing to delete:** no previously imported type left the set, so del_types.py was not needed.
- **XmlNameValuePair's union members** overlap at +0x8. The importer keeps the first one and logs the rest.
- **Local types:** PLAYER 242 (86 listed), STUDIO 233 (84); they were 198 and 189. `this` is now typed on 330 / 318 functions (205 / 191 of them ctor or dtor); it was 260 / 248. No type failed to apply, and there were no size mismatches.
- **Still not imported:** NetworkSettings (ctor 0.474), GlobalAdvancedSettings (no ctor row; singleton 0.381), STUDIO ScreenGui and RunService, Workspace, TaskScheduler.

**4. Unnamed functions.** I regenerated ida/exports/PLAYER-app-unnamed-by-class.csv.
- PLAYER App block (0x00606a00-0x009ad3a0, 21,631 functions): those with an ios:/mac:/match: name went from 7,982 to 8,043, and 13,588 (62.8%) still have none.
- Network block (0x0051f6b0-0x00606a00, 5,336 functions): 1,853 to 1,865, and 3,471 (65.0%) still have none.

Largest App-block groups still unnamed:

| Group | Functions |
|---|---|
| No caller with a known class | 4,107 (600 KB) |
| Ogre::RbxSceneManager | 306 (vftable slots) |
| FactoryProduct<>::Creator | 212 |
| ScriptContext | 191 (was 202) |
| EnumDesc<> | 180 |
| DataModel | 180 |
| RBX free functions | 151 |
| Ogre::RbxSceneNode | 104 |
| Ogre::ManualObject | 95 |
| InsertService | 94 |
| Humanoid | 93 |
| Scripting::ScriptDebugger | 88 |
| Creatable<> | 88 |

ContactManager went from 63 to 61 and World from 45 to 44.

**5. Name coverage, before → after.** I regenerated the names, functions and types exports (PLAYER 671 type declarations, STUDIO 570), and ida/LOG.md has the session note.

| | PLAYER (49,091 functions) | STUDIO (44,815 functions) |
|---|---|---|
| Named | 13,519 → 13,597 | 14,264 → 14,333 |
| match: | 5,684 → 5,761 | 5,523 → 5,593 |
| ios: | 6,209 → 6,210 | 6,269 → 6,270 |
| ida: | 1,066 → 1,066 | 1,815 → 1,815 |
| flirt: | unchanged | 183 → 182 |
| rtti: | unchanged | 170 → 169 (0x00740940 is now match:) |

The ida: count stays the same because 2 `j_` names were replaced and IDA added 2 new ones, for thunks to newly named functions. Globals are unchanged in both databases.

## Files changed

- ida/PLAYER.i64, ida/STUDIO.i64
- ida/snapshots/PLAYER-15-wave6-sync.i64, ida/snapshots/STUDIO-15-wave6-sync.i64
- ida/LOG.md
- tools/ida/match_names.py (two wave6 FACTS changes: TBB respelled, ContentId(const char*) added; the folded-name pick now also accepts a decorated reccmp name)
- tools/ida/import_types.py (12 wave6 classes in CLASSES)
- ida/exports/{PLAYER,STUDIO}-names.csv, -functions.csv, -types.h, -proven-types.json, -db-updates.csv, -match-conflicts.csv
- ida/exports/PLAYER-app-unnamed-by-class.csv
- ida/logs/{PLAYER,STUDIO}-15-db-updates.log/.tsv, -15-types.log, -15-export-names.log

## Unresolved

- **Stale pairing data:** the pair_names.py proposals and STUDIO-PLAYER-bodymatch.csv still predate STUDIO isUpperRoot 0x00a0e170. No function start changed this session.
- **STUDIO 0x004337a0** is ContentId(const string&), the shell's /GS copy. reccmp pairs our engine copy with it at 0.64. I left it unnamed because no report asked for it; the evidence is the same import-slot test as above.
- **Stats owner:** our tree still declares `sStatsItem` in `RBX`, but 2013 has `RBX::Stats::sStatsItem` (and `Stats::Item`). A keep fact holds the oracle name until the tree moves it.
- **NetworkSettings and GlobalAdvancedSettings** import once a ctor reaches 100% (0.474, and no ctor row).
