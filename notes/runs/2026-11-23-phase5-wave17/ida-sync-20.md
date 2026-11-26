# ida-sync-20 (workflow phase5-wave17)

The wave 17 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0/.nam/.til left behind.
- Every IDA run was headless, one process per database, one at a time, through tools/capped.sh at 8G. None hit the cap.
- I did not touch ida/Archive.zip, Client/, gen_reccmp_csv.py or the shared reccmp/ CSVs, and committed nothing. check_2016_copies is silent.

## 1. Snapshots

ida/snapshots/PLAYER-26-wave17-sync.i64 and STUDIO-26-wave17-sync.i64 were taken before any change. Both are `cmp`-identical to the canonical databases as they were then.

## 2. Names from the scored runs

Inputs: build/sweep24/P-final.json and build-studio/sweep24/S-final.json, each joined only with the build it scored (build*/sweep24/scored/).

- **Functions at effective 100%:** PLAYER 9,771, STUDIO 9,226 (sweep23: 9,615 / 9,101). The library-owned rows (1,747 / 1,654) stay with apply_lib_names.py.
- **PLAYER: 152 functions and 1 data name renamed.** 138 match: (including the two facts below), 9 ios: facts, 5 rtti: Replicator vftable slots (writeChangedProperty, writeChangedRefProperty, filterChangedProperty, onPropertyChanged, AddCommonRGSReplacements).
- **STUDIO: 116 functions and 1 data name renamed.** 103 match: (8 of them through the PLAYER body match), 9 ios:, 4 rtti:. One unname (0x007cc250). FUNC_LIB was cleared on 3 functions.
- **New match: names (wave 17's code):**
  - WinMain's ATL rows: the CComModule D0, CAtlModule::Lock / Unlock / GetGITPtr, CRegObject (dtor, QueryInterface, AddRef, AddReplacement), AtlAdd / AtlMultiply, UpdateRegistryFromResourceD/S;
  - Application::OnGetMinMaxInfo and AboutToShutdown, View::AboutToShutdown;
  - ConcurrentRakPeer::Send with its PacketJob dtor, SendData copy ctor, timestamped_safe_queue / deque templates and the five `sp_pointer_construct<...Job>` copies;
  - Replicator::ItemSender (ctor, dtor, openPacket, closePacket, send, getNumberOfBytesUsed), the ClusterReplicationData ctor, the InstancePacketCache ctor, Replicator filterReceivedParent / isLegalReceiveProperty / shouldDelayAddingToWorld / requestInstances;
  - Instance::unlockParent and isDescendantOf;
  - Players::friendStatusChanged / friendEventFired, Player::onFriendStatusChanged, FriendService's bind / storage / set templates and weak_from;
  - the Game::setupDataModel bind chain; ControllerService's getClassName, callDoDeclare and classDescriptor atexit.
- **Renamed spellings:** the `signal<void(shared_ptr<Instance>, int)>` names on 10 PLAYER / 10 STUDIO functions became the FriendStatus / FriendEventType spellings (they were match: names from the int placeholder).
- **Conflicts:** PLAYER 127, STUDIO 126 after the rerun. New: JoinCommand D0 vs NullVerb D0 (both targets) and STUDIO UnsecuredStudioGame D0 vs Game D0 at 0x007ce200. All are folded-original, so the oracle name is kept.
- **Post-apply rerun:** nothing is pending except the no-op unnames and STUDIO 0x0054b3b0 (as before). The selftest passes.

**Facts** (tools/ida/match_names.py). Each was checked from the exe bytes; sizes are equal in both targets.

| PLAYER | STUDIO | Name | Tag | Evidence |
|---|---|---|---|---|
| 0x005bd080 | (PLAYER only) | `_internal_accept_owner<NetworkOwnerJob>` of `enable_shared_from_this<TaskScheduler::Job>` | match: | reccmp 100% with our shell /GL body 0x00494a30, a fold of the NetworkOwnerJob / HeartbeatTask / PacketReceiveJob / StatsUpdateJob spellings. The bulk rule would have taken StatsUpdateJob. |
| 0x005baa50 | (PLAYER only) | `_internal_accept_owner<YieldFunctionStateObject>` | match: | reccmp 100% with our native body 0x006be170 (folded with Registry / Thread / FWBase). The bulk rule would have taken Registry. |
| 0x00652d70 | 0x007cb930 | `ScriptInformationProvider::setAssetUrl(std::string)` | ios: | Game::Game (100%) calls it with baseUrl + '/asset/'; our STUB pairs here. |
| 0x0069ff10 | 0x0083c6d0 | `UserInputService::setProcessToolCommandsSeparately(bool)` | ios: | 13 bytes, stores the bool at +0x80; called by Game::Game. |
| 0x0065fe50 | 0x0078a2c0 | `ContentProvider::setBaseUrl(std::string)` | ios: | called by Game::Game. |
| 0x0072db60 | 0x0079b7e0 | `RunService::setRunState(RunState)` | ios: | called by Game::Game with RS_RUNNING. Decorated with iOS's `RunState` (`W4RunState@2@`); our tree declares `int`. |
| 0x00655d30 | 0x007cdc90 | `Game::~Game()` | ios: | called by the 100% D0 0x00656230 / 0x007ce200. |
| 0x006ea020 | 0x00817010 | `ControllerService::ControllerService()` | ios: | its only caller is the shell create below, after new(0x68). |
| 0x00469490 | 0x004fa420 | `Creatable<Instance>::create<ControllerService>()` | ios: | /GS shell copy: `push 0x68`, operator new, the ctor. |
| 0x006b2c80 | 0x00811980 | `FriendService::ProcessBulkFriendResponse(weak_ptr, int, set<int>, string*, exception*)` | ios: | bound and called by playerAdded (100%). |
| 0x006b1500 | 0x00810200 | `FriendService::storeAndReplicateFriendStatus(int, int, FriendStatus)` | ios: | called by friendStatusReplicationChanged (100%). |
| 0x00de98d0 | 0x00f8e390 | `_ZN5FFlag23OverlayDataModelEnabledE` (data1) | ios: | STUDIO Game::Game reads it with `cmp bl,[0x00f8e390]` at 0x007cde3e. |
| 0x00653790 / 0x00654e80 | 0x007cc250 / 0x007cd5c0 | Game::Unk0x30 ctor / dtor (comment only) | guess: | Our placeholder; the class is unknown. STUDIO 0x007cc250 also loses its FLIRT false positive `??1strstreambuf@std@@UAE@XZ_28`. |
| 0x00652f00 | 0x007cba50 | OverlayDataModel FUN_00652f00 (comment only) | guess: | Inline in 2013 with no iOS symbol. |

createOverlayDataModel 0x00693310 / 0x00850680 already carried its iOS name, so it is unchanged.

Tool changes:
- match_names.py: a STUDIO address of 0 in FACTS now means the fact is PLAYER only.
- apply_db_updates.py: a fact unname whose tag is not lib: now clears FUNC_LIB (a FLIRT false positive).

## 3. Types

import_types.py, dumped from the scored PDBs. Every size equals the tree's // SIZE or DECOMP_SIZE_ASSERT.

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| RBX::Network::ConcurrentRakPeer | 0x1c | ctor 0x005c6770 / 0x006219d0, dtor 0x005c4330 / 0x0061f1f0 |
| RBX::Network::ConcurrentRakPeer::PacketJob | 0x248 | dtor 0x005c5cc0 / 0x00620f20 |
| RBX::Network::Replicator::ReplicationData | 0x10 | dtor 0x005837d0 / 0x00625ad0 (now has its size assert) |
| RBX::Network::Replicator::ClusterReplicationData | 0x34 | ctor 0x005a0090 / 0x006438a0 |
| RBX::Network::Replicator::ItemSender | 0x1c | ctor 0x005da3f0 / 0x00672030, dtor 0x005da520 / 0x00672160 |
| RBX::Game (PLAYER only) | 0x34 | ctor 0x00655e70 (STUDIO 0x007cddd0 is at 0.9965) |
| RBX::Network::InstancePacketCache (PLAYER only, + its DescribedNonCreatable chain) | 0x98 | ctor 0x005d7cb0 (STUDIO 0x00669180 is at 0.985) |

- **Changed members from the tree:** NetworkSettings +0x64 is now printSplitMessages and +0xd8 dataSendPriority; Players +0x194 is players.
- **STUDIO DataModel now comes straight from the STUDIO PDB** (its ctor and dtor are at 100%, with no networkStatsWindowsOn and parentDataModel at 0xe08), so fix_2013's STUDIO DataModel case no longer fires.
  - The STUDIO PDB holds two GuiBuilder records, one of 0x20 and one of 0x1c. The dump takes the 0x20 one because the tree's // SIZE is PLAYER's.
  - fix_2013 already cut GuiBuilder to 0x1c; it now also sets DataModel's guiBuilder member to 0x1c. Without that, the member would have overlapped parentDataModel.
- **Counts:**
  - Local types: PLAYER 362 → 372 (137 listed), STUDIO 351 → 356 (130 listed).
  - `this` typed: PLAYER 541 → 565 (342 ctor/dtor), STUDIO 507 → 526 (317 ctor/dtor).
  - No size mismatch and no type removed.
- **Not imported:** ControllerService (0x68; ctor STUB), ReplicatorStats and Selection (ctor/dtor STUB), PacketReceiveJob (no size annotation), ScreenshotVerb (ctor 0.759), Application (0x140; ctor STUB).

## 4. Unnamed functions

I regenerated ida/exports/PLAYER-app-unnamed-by-class.csv.

| Block | Functions | Named (ios:/mac:/match:), before → after | Still without one |
|---|---|---|---|
| PLAYER App (0x00606a00–0x009ad3a0) | 21,631 | 8,734 → 8,774 | 12,857 (59.4%) |
| Network (0x0051f6b0–0x00606a00) | 5,336 | 2,253 → 2,311 | 3,025 (56.7%) |

Largest App groups:

| Group | Functions |
|---|---|
| No caller with a known class | 4,005 (588 KB) |
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
| OverlayDataModel | 69 |

Largest Network groups:

| Group | Functions |
|---|---|
| Replicator | 348 |
| Players | 149 |
| ServerReplicator | 116 |
| Player | 113 |
| FactoryProduct<>::Creator | 48 |
| Replicator::StreamJob | 47 |
| InterpolatingPhysicsReceiver | 39 |
| ErrorCompPhysicsSender | 38 |
| PhysicsPacketCache | 37 |

## 5. Coverage, before → after

I regenerated the names, functions and types exports. The types export has 803 declarations for PLAYER and 695 for STUDIO. Function starts are unchanged (49,091 / 44,815), so I did not rerun pair_names.py, body_match.py or lib-resolved. The session note is in ida/LOG.md.

| | PLAYER | STUDIO |
|---|---|---|
| Named | 15,293 → 15,435 | 15,834 → 15,939 |
| match: | 7,418 → 7,545 | 7,086 → 7,180 |
| ios: | 6,225 → 6,234 | 6,284 → 6,293 |
| rtti: | 165 → 170 | 162 → 165 |
| flirt: | 97 | 172 → 171 |
| ida: | 1,074 → 1,075 | 1,804 |

## Files changed
- ida/PLAYER.i64, ida/STUDIO.i64
- ida/snapshots/{PLAYER,STUDIO}-26-wave17-sync.i64
- ida/LOG.md
- tools/ida/match_names.py: 16 new facts. A STUDIO address of 0 now means PLAYER only.
- tools/ida/apply_db_updates.py: a fact unname now clears FUNC_LIB unless its tag is lib:.
- tools/ida/import_types.py: 7 classes added (2 of them PLAYER_ONLY). fix_2013 now sets STUDIO DataModel's guiBuilder member to 0x1c.
- ida/exports/{PLAYER,STUDIO}-names.csv, -functions.csv, -types.h, -proven-types.json, -db-updates.csv, -match-conflicts.csv
- ida/exports/PLAYER-app-unnamed-by-class.csv
- ida/logs/{PLAYER,STUDIO}-26-db-updates.log/.tsv, -26-types.log, -26-export-names.log

## Unresolved
- **Integrator (gen_reccmp_csv.py):** names.csv now carries both wave-17 MATCH_FIX names (PLAYER 0x005bd080 NetworkOwnerJob, 0x005baa50 YieldFunctionStateObject). Both entries are now no-ops (the first replaces a name with itself; the second adds a row that already exists), so they can be dropped. Confirm at the next regeneration.
- **App:** the STUDIO PDB has a 0x20 GuiBuilder record next to the real 0x1c one. Some STUDIO TU sees GuiBuilder.h without the OGRE_TARGET_STUDIO member removal (an ODR mismatch inside one link).
- **App:** setRunState is `(RunState)` in iOS; our RunService.cpp declares `(int)`. The IDA name uses the iOS type.
- **Still open:**
  - Game::Unk0x30 (class unknown) and OverlayDataModel 0x00652f00 carry only guess: comments.
  - `MemberDescriptorContainer<T>::compare` 0x00614cd0 and `GlobalAdvancedSettingsItem<CRenderSettingsItem,sRenderSettings>::singleton` 0x00407230 / createInstance 0x00409020 are unnamed.
  - STUDIO 0x004337a0 is unnamed, and 0x009b7020 is still a dummy.
  - The program_options facet shell copies 0x00477aa0 / 0x00477c30 / 0x00477dd0 still carry filesystem names.
  - pair_names.py proposals and STUDIO-PLAYER-bodymatch.csv still predate STUDIO isUpperRoot (0x00a0e170).
