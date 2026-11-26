# ida-sync-18 (workflow phase5-wave15)

The wave 15 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0/.nam/.til left behind.
- Every IDA run was headless, one process per database, one at a time, through tools/capped.sh at 8G. None hit the cap.
- I did not touch ida/Archive.zip, Client/, gen_reccmp_csv.py or the shared reccmp/ CSVs, and committed nothing. check_2016_copies is silent.

## 1. Snapshots

ida/snapshots/PLAYER-24-wave15-sync.i64 and STUDIO-24-wave15-sync.i64 were taken before any change. Both are `cmp`-identical to the canonical databases as they were at that point.

## 2. Names from the scored runs

The inputs were build/sweep22/P-final.json and build-studio/sweep22/S-final.json. Each was joined only with the build it scored, in build*/sweep22/scored/.

- **Functions at effective 100%:** PLAYER 9,542, STUDIO 9,029 (sweep21: 9,298 / 8,842). The library-owned rows (1,747 / 1,654) are left to apply_lib_names.py.
- **PLAYER: 207 functions and 2 data names renamed.**
  - 203 `match:`. These include one IDA `j_` thunk (0x00657bc0, now `callDoDeclare<sSettings>`) and the Selection D0 at 0x00869fd0. That D0 had the ida: Itanium name and now carries `??_ESelection@RBX@@UAEPAXI@Z`.
  - 2 `rtti:` vftable slots.
  - 2 `ios:` facts.
- **STUDIO: 172 functions and 2 data names renamed.** 168 are `match:`, 24 of them taken from PLAYER through body match. The rest are 2 `rtti:` and 2 `ios:`.
- **Integrate's generator fixes are now in the databases.** reccmp's own pairing in sweep22 carries them, so no fact was needed:

| PLAYER address | New name | Old name |
|---|---|---|
| 0x006ee210 | `FactoryProduct<SoundService>::getClassName` | the shell className name |
| 0x0041c8b0 | `boost::function<void()>::~function` | the `function<void(shared_ptr<Instance>, string)>` dtor name |
| 0x0073da60, 0x0073db90, 0x0074c040, 0x00844de0, 0x00844f60 | DebrisService's `function0<void>` / `basic_vtable0<void>` / `weak_ptr<Instance>` spellings | the weak_ptr<DataModel> spellings |

- **New match: names are this wave's code:**
  - the GlobalAdvancedSettings ctor 0x00658580 / 0x00740d10;
  - Selection's D0 and onAncestryChanged;
  - the create / find / declare templates for Selection and for Instance-, Cluster- and PhysicsPacketCache;
  - the NetworkOwnerJob, Replicator, PacketReceiveJob and CRenderSettingsItem ctors (plus the NetworkOwnerJob and Replicator dtors);
  - View::calcDefaultResolution and ScreenshotVerb::doIt.
- **22 PLAYER rtti: slot names are now proven.** The names are unchanged; their source is now match:.
- **Conflicts:** PLAYER 124 (99 folded-original, 25 same-function), STUDIO 122.
  - The new PLAYER conflicts are four EventDescImpl::fireEvent oracle spellings and 0x00656320 (below). The oracle name is kept in each case.
  - The STUDIO 0x0054b3b0 skip is unchanged.
- **Three STUDIO shell copies keep their dummy names: 0x0042f1d0, 0x00430a80, 0x00431ac0.**
  - Through body match, PLAYER offers them the weak_ptr<DataModel> function1 names.
  - STUDIO already uses those names at its engine copies 0x0098a930, 0x0098a7b0 and 0x005e3f00. STUDIO's own run pairs those engine copies with our DataModel spelling, because STUDIO's link has only DataModel symbols at those recomp addresses.
  - match_names.py now skips such a carry ("PLAYER name kept at another STUDIO address"), so apply_db_updates no longer has to refuse it.
- **Post-apply rerun:** nothing is pending except the no-op unnames and STUDIO 0x0054b3b0. The selftest passes.

**Facts** (tools/ida/match_names.py; app-15.md "IDA owner (names)"). Each was checked from the exe bytes, and the decorations come from our scored map.

| PLAYER | STUDIO | Name | Tag | Evidence |
|---|---|---|---|---|
| 0x00659170 | 0x00773bf0 | `?gfxcard@DebugSettings@RBX@@QBE?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ` | ios: | Calls EnumDisplayDevicesA and the SetupDi* class enumeration, and uses the strings 'HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\' + '\DriverVersion', ' drv=' and 'unknown'. The only caller is View::calcDefaultResolution 0x00467dd0. |
| 0x00659910 | 0x00774340 | `?videoMemory@DebugSettings@RBX@@QBEHXZ` | ios: | 90 bytes. The only caller is calcDefaultResolution. |
| 0x00d30edd | 0x00ec17c9 | `_ZN4FLog13RobloxWndInitE` (data1) | ios: | The initial byte is 1 in .data in both exes. |
| 0x00de3f88 | 0x00f84e62 | `_ZN6DFFlag15UseClusterCacheE` (data1) | ios: | In bss. Registered with kind 1 (dynamic). |

- **Kept:** `globalAdvancedSettingsFile` 0x00656320 / 0x0073ee60 (src2016:), not our tree's `RBX::advancedSettingsFile`, even though that body is at reccmp 100%.
  - The resolver judges it same-function and keeps the 2016 spelling. That matches the 2016 name of its twin, `globalBasicSettingsFile`.
  - Our tree's name is now a line in the fact's comment.
- **Selection ctor:** PLAYER 0x0086b070 already carries the mac: name `_ZN3RBX9SelectionC2Ev`, and STUDIO 0x0079f000 carries the ios: name, so nothing changed. app-15 had reported 0x0086b070 as unnamed.
- **The STUDIO GlobalAdvancedSettings ctor 0x00740d10** was named by the bulk rule.

## 3. Types

import_types.py, dumped from the scored PDBs. Each size below equals the tree's // SIZE or DECOMP_SIZE_ASSERT.

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| RBX::GlobalAdvancedSettings (+ its DescribedNonCreatable / Described / NonFactoryProduct chain over Settings) | 0xb4 | ctor 0x00658580 / 0x00740d10 |
| RBX::Network::NetworkOwnerJob | 0x230 | ctor 0x005f7370 / 0x0066af00, dtor 0x005f74b0 / 0x0066b040 |
| RBX::Network::Replicator (+ its chain over IdSerializer), PLAYER only | 0x2aa4 | ctor 0x005a5e70, dtor 0x005a55a0 (STUDIO is another revision) |

- **Changed members** (from the tree):
  - Network::Server: +0xac8 players, +0xad0 networkOwnerJob, +0xad8 itemAddedConnection.
  - NetworkSettings: +0x79 usePhysicsPacketCache, +0x7a useInstancePacketCache, +0xdc distributedPhysicsEnabled, +0xf0 networkOwnerRate.
- **Counts:**
  - Local types: PLAYER 341 → 350 (127 listed), STUDIO 330 → 335 (121 listed).
  - `this` typed: PLAYER 522 → 532 functions (323 ctor/dtor), STUDIO 488 → 495 (300 ctor/dtor).
  - No size mismatch and no type removed.
- **Not imported:**
  - Replicator::ReplicationData: the dtor 0x005837d0 is at 100% and the PDB size is 0x10, but there is no size annotation. The SIZE 0x34 in Replicator.h belongs to ClusterReplicationData.
  - PacketReceiveJob: ctor 0x005b92c0 is at 100%, but there is no size annotation.
  - Selection (0x90): its ctor and dtor are STUBs.
  - Game: ctor 0x00655e70 is unpaired.
  - ScreenshotVerb: ctor at 0.759.
  - VistaAPIs: no size annotation.

## 4. Unnamed functions

I regenerated ida/exports/PLAYER-app-unnamed-by-class.csv.

| Block | Functions | Named (ios:/mac:/match:), before → after | Still without one |
|---|---|---|---|
| PLAYER App (0x00606a00–0x009ad3a0) | 21,631 | 8,578 → 8,692 | 12,939 (59.8%) |
| Network (0x0051f6b0–0x00606a00) | 5,336 | 2,152 → 2,236 | 3,100 (58.1%) |

Largest App-block groups:

| Group | Functions |
|---|---|
| No caller with a known class | 4,026 (591 KB) |
| Ogre::RbxSceneManager (vftable slots) | 306 |
| FactoryProduct<>::Creator | 212 |
| ScriptContext | 191 |
| EnumDesc<> | 178 |
| DataModel | 162 |
| RBX free functions | 151 |
| Ogre::RbxSceneNode | 104 |
| Ogre::ManualObject | 95 |
| Humanoid | 92 |
| ScriptDebugger | 86 |
| Creatable<> | 86 |
| PartInstance | 82 |
| RemoteFunction | 82 |
| Workspace | 80 |
| SceneUpdater | 80 |
| InsertService | 79 |

Largest Network-block groups:

| Group | Functions |
|---|---|
| Replicator | 341 (was 363) |
| Players | 154 |
| ServerReplicator | 118 |
| Player | 111 |
| Replicator::StreamJob | 50 |
| FactoryProduct<>::Creator | 48 |
| ErrorCompPhysicsSender | 40 |

## 5. Coverage, before → after

I regenerated the names, functions and types exports. Type declarations went from 772 to 781 in PLAYER and from 669 to 674 in STUDIO. Function starts are unchanged (49,091 / 44,815), so I did not rerun pair_names.py, body_match.py or lib-resolved. The session note is in ida/LOG.md.

| | PLAYER | STUDIO |
|---|---|---|
| Named | 15,029 → 15,230 | 15,604 → 15,775 |
| match: | 7,137 → 7,355 | 6,834 → 7,023 |
| ios: | 6,221 → 6,223 | 6,280 → 6,282 |
| rtti: | 191 → 171 (22 became match:) | 185 → 166 |
| ida: | 1,071 → 1,072 | 1,806 → 1,805 |

## Files changed
- ida/PLAYER.i64, ida/STUDIO.i64
- ida/snapshots/{PLAYER,STUDIO}-24-wave15-sync.i64
- ida/LOG.md
- tools/ida/match_names.py: 4 new facts (2 functions, 2 globals); the DebrisService / globalAdvancedSettingsFile comments updated; the STUDIO carry skips names STUDIO already uses elsewhere
- tools/ida/import_types.py: 3 classes added; Replicator added to PLAYER_ONLY
- ida/exports/{PLAYER,STUDIO}-names.csv, -functions.csv, -types.h, -proven-types.json, -db-updates.csv, -match-conflicts.csv
- ida/exports/PLAYER-app-unnamed-by-class.csv
- ida/logs/{PLAYER,STUDIO}-24-db-updates.log/.tsv, -24-types.log, -24-export-names.log

## Unresolved
- **Integrator (gen_reccmp_csv.py, not mine):** names.csv now carries every wave-15 MATCH_FIX / MATCH_SUB name: 0x006ee210, 0x0041c8b0, 0x0073da60, 0x0073db90, 0x0074c040, 0x00844de0 and 0x00844f60. Those entries can be dropped. Measure the effect at the next regeneration.
- **App owner:**
  - 0x00656320 keeps 2016's `globalAdvancedSettingsFile`, and its twin 0x00656390 keeps our `basicSettingsFile`. The tree's `advancedSettingsFile` / `basicSettingsFile` are invented; the 2016 spellings are `globalAdvancedSettingsFile` / `globalBasicSettingsFile`.
  - Replicator::ReplicationData and PacketReceiveJob need a size annotation before their layouts can be imported.
- **STUDIO naming:** the shell copies 0x0042f1d0, 0x00430a80 and 0x00431ac0 stay dummies. In PLAYER the shell copies have the DataModel names and the engine copies the Instance names; in STUDIO the engine copies have the DataModel names. Both spellings are true of the folded body.
- **Not named:**
  - The Game ctor helper 0x00653790.
  - The STUDIO checkLookup copies 0x006db970 / 0x00703620.
  - MemberDescriptorContainer<T>::compare 0x00614cd0.
  - GlobalAdvancedSettingsItem<CRenderSettingsItem, sRenderSettings>::singleton 0x00407230 and its createInstance 0x00409020.
  - CSHA1 ×6 and TCPInterface/PacketizedTCP ×4 have no 2013 body.
- **Carried:**
  - pair_names.py proposals and STUDIO-PLAYER-bodymatch.csv predate STUDIO isUpperRoot 0x00a0e170.
  - STUDIO 0x004337a0 is still unnamed.
  - The program_options facet shell copies 0x00477aa0 / 0x00477c30 / 0x00477dd0 still carry filesystem names.
  - 0x009b7020 is still a dummy.
