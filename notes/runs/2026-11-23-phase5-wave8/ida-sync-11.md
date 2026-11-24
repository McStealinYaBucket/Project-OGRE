# ida-sync-11 (workflow phase5-wave8)

The wave 8 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0, .nam or .til files left behind. Every IDA run was headless, one process per database, through tools/capped.sh at 8G; none hit the cap. ida/Archive.zip was not touched and nothing under Client/ was edited.

**1. Snapshots.** Before any change both databases were copied to ida/snapshots/PLAYER-17-wave8-sync.i64 and STUDIO-17-wave8-sync.i64; `cmp` showed each identical to its canonical database.

**2. Names from the scored runs.** Sources: build/sweep15/P-final.json and build-studio/sweep15/S-final.json, joined only with build/sweep15/scored/ and build-studio/sweep15/scored/.
- Functions at effective 100%: PLAYER 8,052, STUDIO 7,699 (sweep14: 7,674 / 7,417). Library-owned rows (1,738 / 1,645) are left to apply_lib_names.py.
- PLAYER renamed 276: 256 dummies, 10 IDA `j_` thunk names, 10 named. STUDIO renamed 216: 199 dummies (39 via PLAYER body match), 9 `j_`, 8 named.
- The `j_` names are `Name::callDoDeclare<sX>` thunks for the services datamodel-5b-6.md wrote or fixed (FriendService, UserInputService, ServerStorage, ReplicatedStorage, ServerScriptService, TimerService, TestService, ChangeHistoryService, PhysicsSettings).
- The named renames follow datamodel-5b-6.md's 29 template-argument fixes from the 2013 RTTI: the `Described<ScriptService>` / `<PhysicsService>` ctors and their classDescriptor atexits, the Camera `DescribedCreatable` ??_G (`$0BL@` -> `$0BJ@`), TouchTransmitter ??_E/??_G (`$02`), and the HttpService ??_E (now `??_EHttpService@RBX@@WBM@`, STUDIO `WBI@`). PLAYER 0x00587560 went from `shared_polymorphic_downcast` to `shared_static_cast<Instance,DescribedBase>`: identical folded bodies, reccmp's pick.
- **Conflicts:** PLAYER 118 (98 folded-original, 20 same-function), STUDIO 119 (100 / 18 / 1 entry point). New ones are all folded-original or same-function (e.g. `IndexArray<SimBody,getFreeFallBodyIndex>::fastRemove` 0x0082f810, same function). The STUDIO skip at 0x0054b3b0 stays.
- **Facts changed in tools/ida/match_names.py:**
  - Dropped the three `Stats::Item::formatValue<T>` keep rows (PLAYER 0x0073b3f0 / 0x0073b420 / 0x0073b450). The tree now spells them `??$formatValue@N/M/H@Item@Stats@RBX@@`, so the iOS names agree by signature and get a `p4 match:` line.
  - New rename (rtti:) PLAYER 0x00474040 = `??_Eutf8_codecvt_facet@detail@program_options@boost@@UAEPAXI@Z`. It is slot 0 of the vftable 0x00be64d0 whose COL 0x00c75d38 names `.?AUutf8_codecvt_facet@detail@program_options@boost@@`. It had held `??_Ewindows_file_codecvt` (match:) since an earlier sweep. With program_options built /GL, reccmp now pairs our windows_file_codecvt deleting dtor with 0x009befb0, slot 0 of `??_7windows_file_codecvt` 0x00bbb00c. The first pass skipped 0x009befb0 ("name in use"); a second pass with the fact swapped the two. STUDIO has no such facet, so the fact is PLAYER only.
  - Selftest passes.

**3. Types.** New classes; each has a ctor or dtor at effective 100% in sweep15 and a PDB size equal to the tree's DECOMP_SIZE_ASSERT or SIZE annotation:

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| TimerService | 0x78 | ctor 0x007a6d10 / 0x008bb430 |
| PhysicsSettings | 0x70 | ctor 0x0085da60 / 0x009424e0 |
| StarterGuiService | 0x98 | ctor 0x0070c150 / 0x00798860 (100% with reccmp patch 0002) |
| Network::ItemQueue | 0x10 | dtor 0x005d42a0 / 0x0066d630 |
| Stats::Item | 0x88 | ctor 0x0048e470, PLAYER only (STUDIO 0x0043e560 at 0.655); size from StatsItem.h's SIZE comment (TypedStatsItem<int> is new 0xa8 with a member at 0x88) |

- Bases came with them: the Described / DescribedCreatable / DescribedNonCreatable / FactoryProduct / NonFactoryProduct / GlobalAdvancedSettingsItem chains (STUDIO also HeartbeatInstance).
- KernelData (already imported) was refreshed: eleven `IndexArray<T,&T::getXIndex>` (world-5d.md), still 0xb0.
- Local types: PLAYER 281 (97 listed), STUDIO 269 (94); were 263 / 254. `this` typed on 375 / 363 functions (230 / 217 ctor/dtor); was 351 / 339. No size mismatch; no type left the set, so del_types.py was not needed.
- Not imported: Kernel (ctor 0x00831590 / dtor 0x0082f670 at 100%, but no size assertion), SpatialHash (no size assertion), AssemblyHistory, GlobalAdvancedSettings, STUDIO ScreenGui/RunService, Workspace, TaskScheduler.

**4. Unnamed functions.** ida/exports/PLAYER-app-unnamed-by-class.csv regenerated.
- PLAYER App block (0x00606a00-0x009ad3a0, 21,631 functions): ios:/mac:/match: 8,126 -> 8,262; 13,369 (61.8%) still without one.
- Network block (0x0051f6b0-0x00606a00, 5,336): 1,876 -> 1,890; 3,446 (64.6%) without.

Largest App-block groups still unnamed:

| Group | Functions |
|---|---|
| No caller with a known class | 4,076 (597 KB), was 4,082 |
| Ogre::RbxSceneManager | 306 (vftable slots) |
| FactoryProduct<>::Creator | 212 |
| ScriptContext | 190 |
| EnumDesc<> | 180 |
| DataModel | 172, was 179 |
| RBX free functions | 159 |
| Ogre::RbxSceneNode | 104 |
| Ogre::ManualObject | 95 |
| Humanoid | 93 |
| Scripting::ScriptDebugger | 88 |
| Creatable<> | 86, was 88 |
| EventDescImpl<> | 83 |
| PartInstance | 82 |
| InsertService | 79, was 85 |

**5. Name coverage, before -> after.** Names, functions and types exports regenerated (PLAYER 710 type declarations, STUDIO 606; were 692 / 591); ida/LOG.md has the session note.

| | PLAYER (49,091 functions) | STUDIO (44,815 functions) |
|---|---|---|
| Named | 13,702 -> 13,959 | 14,429 -> 14,629 |
| match: | 5,862 -> 6,125 | 5,687 -> 5,893 |
| ida: | 1,070 -> 1,061 | 1,819 -> 1,811 |
| rtti: | 173 -> 176 | 169 -> 171 |
| others (ios:, mac:, flirt:, lib:, prior:) | unchanged | unchanged |

Globals unchanged in both. pair_names.py, body_match.py and lib_consistency.py were not rerun (no function start changed).

## Files changed

- ida/PLAYER.i64, ida/STUDIO.i64
- ida/snapshots/PLAYER-17-wave8-sync.i64, ida/snapshots/STUDIO-17-wave8-sync.i64
- ida/LOG.md
- tools/ida/match_names.py (formatValue keep rows dropped; utf8_codecvt_facet fact)
- tools/ida/import_types.py (5 wave8 classes in CLASSES; Stats::Item in PLAYER_ONLY)
- ida/exports/{PLAYER,STUDIO}-names.csv, -functions.csv, -types.h, -proven-types.json, -db-updates.csv, -match-conflicts.csv
- ida/exports/PLAYER-app-unnamed-by-class.csv
- ida/logs/{PLAYER,STUDIO}-17-db-updates.log/.tsv, PLAYER-17-db-updates-pass2.log/.tsv, -17-types.log, -17-export-names.log

## Unresolved

- **Stale pairing data:** pair_names.py proposals and STUDIO-PLAYER-bodymatch.csv still predate STUDIO isUpperRoot 0x00a0e170.
- **STUDIO 0x004337a0**, ContentId(const string&) (shell /GS copy), still unnamed.
- **program_options facet slots:** the shell copies of `do_in` / `do_out` / `do_length` (0x00477aa0 / 0x00477c30 / 0x00477dd0, slots of 0x00be64d0) still carry the filesystem `utf8_codecvt_facet` names (same .ipp source, kept as folded-original).
- **Type candidates that need a tree change first:** Kernel and SpatialHash (add a size assertion), Stats::Item in STUDIO (ctor 0.655), AssemblyHistory, GlobalAdvancedSettings and Workspace (ctors not at 100%).
