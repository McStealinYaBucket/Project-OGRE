# ida-sync-8 (workflow phase5-wave5)

The wave 5 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0, .nam or .til files left behind. Every IDA run was headless, one process per database, through tools/capped.sh at 8G. None hit the cap.

**1. Snapshots.** Before any change I copied both databases to ida/snapshots/PLAYER-14-wave5-sync.i64 and STUDIO-14-wave5-sync.i64. cmp showed each copy identical to its canonical database.

**2. Names from the scored runs.** Sources: build/sweep12/P-final.json and build-studio/sweep12/S-final.json, joined only with build/sweep12/scored/ and build-studio/sweep12/scored/ (the exe, pdb, map and pdbview they scored).
- Functions at effective 100%: PLAYER 7,392, STUDIO 7,167. Library-owned rows (1,740 / 1,645) are left to apply_lib_names.py.
- PLAYER: 249 renamed `match:`. 172 were dummies, 72 were IDA `j_` thunk names and 5 already had names.
- STUDIO: 250 renamed `match:`. 176 were dummies, 67 were `j_` names and 5 already had names; 2 were FLIRT false positives, whose FUNC_LIB flag was cleared. 6 of the 250 came from PLAYER through the body match.
- The `j_` thunks are the `Name::callDoDeclare<sX>` functions (`jmp doDeclare<sX>`). They reached 100% once base-blockers flipped Name.h.
- Functions that already had a name and were renamed:
  - The `DataModel::GenericJob` ctor, PLAYER 0x0063ee60 / STUDIO 0x0075c930, went from prior: to match:.
  - `WeakThreadRef::Node::eraseAllRefs`, PLAYER 0x00871990, went from prior: to match:.
  - The three FactoryProduct<LuaSettings> functions now spell `VItem@GlobalAdvancedSettings@2@`, as base-blockers asked: PLAYER 0x0085db70 / 0x0085dc20 / 0x0085e300, STUDIO 0x009481a0 / 0x00948250 / 0x00948920. The thunk is `??_E?$GlobalAdvancedSettingsItem@VLuaSettings…@OBM@AEPAXI@Z`.
  - STUDIO 0x00828830 is now `RobloxExtraSpace::createNewNode`. It replaces the mac: name `intrusive_ptr<Node>::operator=(Node*)`, which only the callgraph supported. The body takes no argument and is `node = new Node()`.
  - The two STUDIO FLIRT false positives are 0x007dbcb0 (`strstreambuf` dtor → the `signal<void(string,string,shared_ptr<Instance>)>` ctor) and 0x00966460 (a Concurrency function → `PartInstance::getPartPrimitive`).
- Conflicts: PLAYER 121 (96 folded-original, 21 same-function, 4 oracle names kept by a fact). STUDIO 121 (99 / 18 / 3 kept by a fact / 1 entry point). The STUDIO skip at 0x0054b3b0 stays (its name sits on the ICF twin 0x004f3770).
- Three facts, checked against both exes with capstone (new FACTS rows in tools/ida/match_names.py):

| Name | PLAYER | STUDIO | Tag | Evidence |
|---|---|---|---|---|
| `??_GLuaSettings@RBX@@UAEPAXI@Z` (LuaSettings scalar deleting dtor) | 0x0085e310 | 0x00948930 | match: | reccmp 100%. Our linker folds it with `GlobalAdvancedSettingsItem<LuaSettings>`'s ??_G, and the bulk rule took that PUBLICS name. It clears the singleton slot 0x00de68a8 / 0x00f89334. It is reached through the adjustor{28} thunk 0x0085e300, which is slot 0 of both the LuaSettings and the GlobalAdvancedSettingsItem<LuaSettings> +0x1c vftables (0x00b7d008, 0x00b7cf70). base-blockers names it LuaSettings |
| `doDeclare<Stats::sStatsItem>` (ios: name kept) | 0x00480e90 | 0x0043f250 | ios: | The 100% body is ours, but our tree spells the template argument `RBX::sStatsItem`. The 2013 RTTI `.?AV?$NonFactoryProduct@VInstance@RBX@@$E?sStatsItem@Stats@2@3QBDB@RBX@@` says `RBX::Stats`, so the bulk rule's body-wins would have put a misspelt name in place of a correct one |
| `??$callDoDeclare@$E?sStatsItem@Stats@RBX@@3QBDB@Name@RBX@@SAXXZ` | 0x00482680 | 0x0043f240 | match: | reccmp 100% against our callDoDeclare<sStatsItem>, with the namespace taken from the RTTI. Body: `jmp` to doDeclare. IDA demangles it as `RBX::Name::callDoDeclare<RBX::Stats::sStatsItem>` |

- GlobalAdvancedSettingsItem<LuaSettings>'s ctor 0x0085e220 / 0x00948850 already had the correct ios: name. It stays a STUB at 74.5%.
- **Fixed in match_names.py.** A `keep` fact on an address with no reccmp row wrote "p4 ios:; …", and export_names read the source as `ios:;` (STUDIO 0x0043f250). It now writes "p4 ios: …". I replanned and reapplied STUDIO; that second pass changed comments only.

**3. Types.** New classes. Each has a constructor or destructor at effective 100% in sweep12, and a PDB size equal to the tree's `// SIZE`:

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| World | 0xb8 | ctor 0x0074ae30 (100%) / 0x0088b780 (99.1%); dtor 0x00749ae0 / 0x0088a430 (both 100%) |
| ContactManager | 0x6c | dtor 0x007bb5e0 / 0x008d7f80 (both 100%). The ctor is at 79.4% because ContactManagerSpatialHash is 0xa4 against 0xdc in 2013. That class is held by pointer at +0x28, so ContactManager's own layout is unaffected |

- **World's members** follow world-5d.md: autoJoinSignal/autoDestroySignal/primitiveCollideSignal 0x0/0x4/0x8, frmThrottle 0x10, eThrottle 0x14, touchReporting 0x20, primitives 0x3c, movingPrimitives 0x50, breakableJoints 0x60, tempPrimitives 0x78, tempJoints 0x88, the five profiling scoped_ptrs 0x98–0xa8, contactManager 0xac (now a typed `ContactManager*`), sendPhysics 0xb0, cleanStage 0xb4.
- **ContactManager** derives Voxel::CellChangeListener and has concurrencyValidator 0x04, spatialHash 0x28, world 0x30 and profiler 0x68.
- **typed `this`:** 6 World functions in each target, plus PLAYER's ContactManager ctor and the ContactManager dtor in both.
- **The tree's settings rewrite carried through:** CRenderSettingsItem's base is now `GlobalAdvancedSettingsItem<CRenderSettingsItem, sRenderSettings>`, and GlobalAdvancedSettings::Item derives `NonFactoryProduct<Instance, sSettingsItem>`. I deleted the old non-template `RBX::GlobalAdvancedSettingsItem` type with del_types.py. On the first run del_types kept it as "still referenced": it treated `RBX::GlobalAdvancedSettingsItem<…>` as a use, because that type name contains it. It now matches whole type names only.
- **Local types:** PLAYER 198 (74 listed), STUDIO 189 (72); they were 194 and 185. `this` is typed on 260 PLAYER / 248 STUDIO functions (160 / 145 of them ctor or dtor); it was 248 / 237. Every size equals the PDB's, and no type failed to apply.
- **Still not imported:**
  - MultiJoint (dtor 0x009741c0 at 100%, but the tree has no size assertion).
  - LuaSettings (DECOMP_SIZE_ASSERT 0x88, but the ctor 0x0085e360 is at 73.9%; only the ??_G is at 100%).
  - StarterGear and the v8xml classes (no size assertion).
  - STUDIO ScreenGui (0.982) and RunService (0.937 / 0.899).
  - Workspace, TaskScheduler, SleepStage (0x140 against 0x14c in 2013) and Primitive (ctor 0.634, dtor 0.925).

**4. Unnamed functions.** I regenerated ida/exports/PLAYER-app-unnamed-by-class.csv.
- PLAYER App block (0x00606a00-0x009ad3a0, 21,631 functions): those with an ios:/mac:/match: name went from 7,824 to 7,982, and 13,649 (63.1%) still have none.
- Network block (0x0051f6b0-0x00606a00, 5,336 functions): 1,816 to 1,853, and 3,483 (65.3%) still have none.

Largest App-block groups still unnamed:

| Group | Functions |
|---|---|
| No caller with a known class | 4,117 (601 KB) |
| Ogre::RbxSceneManager | 306 |
| FactoryProduct<>::Creator | 212 |
| ScriptContext | 202 (was 214) |
| EnumDesc<> | 181 |
| DataModel | 181 (was 192) |
| RBX free functions | 151 |
| Ogre::RbxSceneNode | 104 |
| Ogre::ManualObject | 95 |
| InsertService | 94 |
| Humanoid | 93 |
| Scripting::ScriptDebugger | 89 |

ContactManager went from 66 to 63 and World from 57 to 45.

Largest Network-block groups still unnamed: Replicator 384, no known class 934, Players 161, ServerReplicator 119, Player 119, FactoryProduct<>::Creator 61, DataModel 57, Server 56, Replicator::StreamJob 51.

**5. Name coverage, before → after.** I regenerated the names, functions and types exports, and ida/LOG.md has the session note.

| | PLAYER (49,091 functions) | STUDIO (44,815 functions) |
|---|---|---|
| Named | 13,311 → 13,519 | 14,060 → 14,264 |
| match: | 5,437 → 5,684 | 5,275 → 5,523 |
| ida: | 1,102 → 1,066 | 1,854 → 1,815 |
| prior: | 16 → 14 | 6 → 5 |
| rtti: | 174 → 173 | 171 → 170 |
| flirt: | unchanged | 185 → 183 |
| ios: / mac: | unchanged | 6,269 → 6,268 / 249 → 248 |

Globals are unchanged in both databases.

## Files changed

- ida/PLAYER.i64, ida/STUDIO.i64
- ida/snapshots/PLAYER-14-wave5-sync.i64, ida/snapshots/STUDIO-14-wave5-sync.i64
- ida/LOG.md
- tools/ida/match_names.py (3 wave5 FACTS rows; the `keep` comment-line fix)
- tools/ida/import_types.py (World and ContactManager in CLASSES)
- tools/ida/del_types.py (whole-name use test)
- ida/exports/{PLAYER,STUDIO}-names.csv, -functions.csv, -types.h, -proven-types.json, -db-updates.csv, -match-conflicts.csv
- ida/exports/PLAYER-app-unnamed-by-class.csv
- ida/logs/{PLAYER,STUDIO}-14-db-updates.log/.tsv, STUDIO-14-db-updates-pass2.log/.tsv, -14-types.log, -14-del-types{,-2}.log, -14-export-names.log

## Unresolved

- **Stats owner:** our tree declares `sStatsItem` in `RBX`, but the 2013 RTTI has `RBX::Stats::sStatsItem`. Bytes are unaffected, but the PDB names are wrong: reccmp and match_names then propose misspelt names such as `doDeclare<sStatsItem>`. A keep fact holds the oracle name for now.
- **World-kernel owner:** a `// SIZE` for MultiJoint would let its 100% dtor 0x009741c0 import the layout.
- **v8xml and StarterGear owners:** a `// SIZE` would let these import too (their ctors are already at 100%).
- **LuaSettings:** it imports once its ctor 0x0085e360 (73.9%) or the item ctor 0x0085e220 (74.5%) reaches 100%. The item ctor pushes the "LuaSettings" literal where ours reads `sLuaSettings`.
- **Stale pairing data:** the pair_names.py proposals and STUDIO-PLAYER-bodymatch.csv still predate STUDIO isUpperRoot 0x00a0e170. No function start changed this session.
- **STUDIO plan record:** the second-pass STUDIO-db-updates.csv no longer lists the 6 renames carried over through the PLAYER body match, because those addresses are no longer dummies. They are recorded in ida/logs/STUDIO-14-db-updates.tsv.
