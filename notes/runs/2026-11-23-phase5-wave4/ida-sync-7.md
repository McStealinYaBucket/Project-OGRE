# ida-sync-7 (workflow phase5-wave4)

The wave 4 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0, .nam or .til files left behind. Every IDA run was headless, one process per database, through tools/capped.sh at 8G. None hit the cap.

**1. Snapshots.** Before any change I copied both databases to ida/snapshots/PLAYER-13-wave4-sync.i64 and STUDIO-13-wave4-sync.i64. cmp showed each copy identical to its canonical database.

**2. Names from the scored runs.** Sources: build/sweep11/P-final.json and build-studio/sweep11/S-final.json, joined only with build/sweep11/scored/ and build-studio/sweep11/scored/ (the exe, pdb, map and pdbview they scored).
- Functions at effective 100%: PLAYER 7,086, STUDIO 6,861. Library-owned rows (1,739 / 1,645) are left to apply_lib_names.py.
- PLAYER: 160 renamed `match:` (159 unnamed, 1 stale match: name).
- STUDIO: 154 renamed `match:` (149 unnamed, 1 stale match: name, 5 carried over from PLAYER through the body match). The STUDIO skip at 0x0054b3b0 stays, because its name already sits on the ICF twin 0x004f3770.
- No new vftable-slot (`rtti:`) names this time.
- The stale match: name is the one script-5e-2.md asked for. The `Described<ScriptContext,…>` ctor (PLAYER 0x0067fcd0 / STUDIO 0x007e58c0) is now spelled with `FactoryProduct<ScriptContext,Instance,sScriptContext,Instance>` and `$00@`, not `NonFactoryProduct<Instance,sScriptContext>` and `$0BB@`. ida/exports/<T>-names.csv carries the new spelling.
- `ScriptContext::scriptShouldRun` (0x00682810 / 0x007e8660) went from `rtti:` to `match:` because it is now at 100%.
- Conflicts: PLAYER 117 (96 folded-original, 18 same-function, 3 oracle names kept by hand facts). STUDIO 118 (99 / 15 / 3, plus 1 entry point). None changed a name: every same-function or folded row keeps its oracle name.
- One fact from the reports, checked against both exes with capstone (a new FACTS row in tools/ida/match_names.py):

| Name | PLAYER | STUDIO | Tag | Evidence |
|---|---|---|---|---|
| `RBX::Name::lookup(char const* const&)` `_ZN3RBX4Name6lookupERKPKc` | 0x00613be0 | 0x007034b0 | ios: | cdecl. It loads the pointer through its reference argument. On null it runs call_once(initNullName 0x00613a70) and returns the null Name. Otherwise it builds a std::string and calls the string lookup 0x00613ab0 / 0x00703380. iOS 1029 has the symbol. 5c-serialization.md: it is the last function before XmlElement.obj |

  0x00613ab0 stays unnamed. It is probably iOS's `lookup(std::string const&)`, but the bytes do not prove that.

**3. Types.** New classes. Each has its constructor and destructor at effective 100% in sweep11 in both targets, and a PDB size equal to the tree's `// SIZE`:

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| Lighting | 0x1f0 | ctor 0x00707ba0 / 0x007d39e0, dtor 0x00707e40 / 0x007d3c80 |
| PhysicsJob (local to RunService.cpp) | 0x228 | ctor 0x0072b5e0 / 0x00798fe0, dtor 0x0072b780 / 0x00799180 |
| HeartbeatTask (local to RunService.cpp) | 0x230 | ctor 0x0072d040 / 0x0079acc0, dtor 0x0072d250 / 0x0079aed0 |

- **Lighting's imported members match datamodel-5b-4.md:** skyParameters 0x68, hasSky 0x160, the three Color4 colours clear/shadow/fog at 0x164/0x174/0x184, fogStart/fogEnd 0x194/0x198, timeOfDay 0x1a0, the colour shifts 0x1a8/0x1b4, globalBrightness 0x1c0, the two ambients 0x1c4/0x1d0, globalShadows 0x1dc, outlines 0x1dd, lightingChangedSignal 0x1e0, sky 0x1e4. 12 PLAYER Lighting functions now have a typed `this` (ctor, dtor and deleting dtor among them), plus 2 DescribedCreatable<Lighting> ones.
- **ScriptContext's base chain changed in the tree** (script-5e-2.md): it is now `Described<…, FactoryProduct<…>, 1, 0>` / `DescribedCreatable`. import_types.py only adds and replaces types, so the three old `NonFactoryProduct<Instance,sScriptContext>` base types would have stayed in the database. The new tools/ida/del_types.py deleted them from both databases.
- `RunningAverage<double,double>` keeps its offsets. Only its member names follow the rewritten header (lastValue, mean, meanSquareDev, unsampled).
- Local types: PLAYER 194 (72 listed), STUDIO 185 (70); they were 188 and 179. `this` is typed on 248 PLAYER / 237 STUDIO functions (155 / 141 of them ctor or dtor); it was 226 / 212. Every size equals the PDB's, and no type failed to apply.
- Still not imported:
  - The v8xml classes (XmlElement 0x28, XmlParser, XmlWriter, MemoryBinder 0x20, ArchiveBinder 0x2c). Their ctors are at 100% (for example XmlWriter 0x00628a20, MemoryBinder 0x006fb6e0, ArchiveBinder 0x006faeb0 and its dtor 0x006fb460), but the tree has no DECOMP_SIZE_ASSERT or `// SIZE` to compare with.
  - StarterGear (ctor 0x006cb3e0 at 100%, no size assertion).
  - STUDIO ScreenGui and RunService (0.982 and 0.937 / 0.899), which stay PLAYER-only.
  - Workspace (0.19 / 0.45) and TaskScheduler (0.978 / 0.995) in both targets.

**4. Unnamed functions.** I regenerated ida/exports/PLAYER-app-unnamed-by-class.csv.
- PLAYER App block (0x00606a00-0x009ad3a0, 21,631 functions): those with an ios:/mac:/match: name went from 7,705 to 7,824, and 13,807 (63.8%) still have none.
- Network block (0x0051f6b0-0x00606a00, 5,336 functions): 1,812 to 1,816, and 3,520 (66.0%) still have none.

Largest App-block groups still unnamed:

| Group | Functions |
|---|---|
| No caller with a known class | 4,160 (602 KB) |
| Ogre::RbxSceneManager | 306 |
| ScriptContext | 214 |
| FactoryProduct<>::Creator | 213 |
| DataModel | 192 |
| EnumDesc<> | 181 |
| RBX free functions | 151 |
| Ogre::RbxSceneNode | 104 |
| Ogre::ManualObject | 95 |
| InsertService | 94 |
| Humanoid | 93 |
| Scripting::ScriptDebugger | 90 |

Largest Network-block groups still unnamed: Replicator 385, no known class 949, Players 161, Player 120, ServerReplicator 119, FactoryProduct<>::Creator 63, DataModel 57, Server 56, Replicator::StreamJob 51.

**5. Name coverage, before → after.** I regenerated the names, functions and types exports, and ida/LOG.md has the session note.

| | PLAYER (49,091 functions) | STUDIO (44,815 functions) |
|---|---|---|
| Named | 13,151 → 13,311 | 13,906 → 14,060 |
| match: | 5,277 → 5,437 | 5,121 → 5,275 |
| ios: | 6,208 → 6,209 | 6,268 → 6,269 |
| rtti: | 175 → 174 (promoted to match:) | 172 → 171 |

The other sources are unchanged. Globals are unchanged too: per database, 6 ios:, 10 src2016: and 3 guess:.

## Files changed

- ida/PLAYER.i64, ida/STUDIO.i64
- ida/snapshots/PLAYER-13-wave4-sync.i64, ida/snapshots/STUDIO-13-wave4-sync.i64
- ida/LOG.md
- tools/ida/match_names.py (1 wave4 FACTS row), tools/ida/import_types.py (3 wave4 CLASSES), tools/ida/del_types.py (new)
- ida/exports/{PLAYER,STUDIO}-names.csv, -functions.csv, -types.h, -proven-types.json, -db-updates.csv, -match-conflicts.csv
- ida/exports/PLAYER-app-unnamed-by-class.csv
- ida/logs/{PLAYER,STUDIO}-13-db-updates{,-pass2}.log/.tsv, -13-types.log, -13-del-types{,-2}.log, -13-export-names.log

## Unresolved

- v8xml owner: add `// SIZE` or DECOMP_SIZE_ASSERT for XmlElement (0x28), XmlAttribute (0x18), XmlNameValuePair (0x10), MemoryBinder (0x20) and ArchiveBinder (0x2c), and StarterGear's owner for StarterGear. Their ctors are already at 100%, so the next sync can import them once the tree asserts a size.
- World: the 2013 size is 0xb8 against our 0x7c (datamodel-5b-4.md scout). Nothing to import until the world owner fixes the layout.
- 0x00613ab0 / 0x00703380 (the string-keyed Name lookup under a lock) is unnamed. iOS has `RBX::Name::lookup(std::string const&)`. Name it when a match or a decisive call site proves it.
- pair_names.py proposals and STUDIO-PLAYER-bodymatch.csv still predate STUDIO isUpperRoot 0x00a0e170. No function start changed this session either.
- The STUDIO skip at 0x0054b3b0 (name on the ICF twin 0x004f3770) remains.
