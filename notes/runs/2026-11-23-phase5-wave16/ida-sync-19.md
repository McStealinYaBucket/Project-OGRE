# ida-sync-19 (workflow phase5-wave16)

The wave 16 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0/.nam/.til left behind.
- Every IDA run was headless, one process per database, one at a time, through tools/capped.sh at 8G. None hit the cap.
- I did not touch ida/Archive.zip, Client/, gen_reccmp_csv.py or the shared reccmp/ CSVs, and committed nothing. check_2016_copies is silent.

## 1. Snapshots

ida/snapshots/PLAYER-25-wave16-sync.i64 and STUDIO-25-wave16-sync.i64 were taken before any change. Both are `cmp`-identical to the canonical databases as they were then.

## 2. Names from the scored runs

Inputs: build/sweep23/P-final.json and build-studio/sweep23/S-final.json, each joined only with the build it scored (build*/sweep23/scored/).

- **Functions at effective 100%:** PLAYER 9,615, STUDIO 9,101 (sweep22: 9,542 / 9,029). The library-owned rows (1,747 / 1,644) stay with apply_lib_names.py.
- **PLAYER: 64 functions renamed.** 57 match:, 3 rtti: vftable slots, 2 ios: and 2 src2016: facts.
- **STUDIO: 65 functions renamed.** 58 match: (4 of them through the PLAYER body match), 3 rtti:, 2 ios:, 2 src2016:. FUNC_LIB was cleared on 4 functions, including the FLIRT false positive `??1strstreambuf@std@@UAE@XZ_33` at 0x0080a360. That function is the `signal<void(int,int,FriendEventType)>` ctor.
- **Integrate's MATCH_FIX now reaches the databases through reccmp's own pairing:**

| PLAYER / STUDIO | New name | Old name |
|---|---|---|
| 0x006aaf50 / 0x00809d30 | `??_G?$DescribedCreatable@VFriendService@...` (the base D0) | `??_GFriendService@RBX@@UAEPAXI@Z` |
| 0x006aaf40 / 0x00809d20 | its `{24}` adjustor thunk `??_E?$DescribedCreatable@...WBI@...` | `??_EFriendService@RBX@@WBI@AEPAXI@Z` |
| 0x006b2b30 / 0x00811830 | `??_GFriendService@RBX@@UAEPAXI@Z` | dummy |
| 0x006b2970 / 0x00811670 | `??1FriendService@RBX@@UAE@XZ` | dummy |
| 0x00656390 / 0x0073eed0 | `RBX::globalBasicSettingsFile` (app-16 took the 2016 spelling) | `RBX::basicSettingsFile` |

- **Other new match: names (wave 16's code):**
  - the FriendService ctor 0x006b2780 / 0x00811480 and its signal, map and set templates;
  - Settings::InvalidDescendentDetector (`invalid`, `operator()`) and `visitDescendants<InvalidDescendentDetector>`;
  - the `Variant::cast_unk0x0058f560<T>` placeholders (0x0058f560 for CoordinateFrame) and the by-value any_casts;
  - the PhysicsPacketCache and ClusterPacketCache ctors, `~ReplicatorJob`, `Replicator::disableProcessPackets`, `ConcurrentRakPeer::GetBufferHealth` and `Players::findConstLocalCharacter`;
  - `View::resetScheduler` and `RegisterWindowClass`.
- **Unchanged:** onServiceProvider 0x006b2b50 and verifyAddDescendant 0x00656a80 / 0x0073f440 already carry their iOS names, which have the same signature (comment only).
- **Conflicts:** PLAYER 126, STUDIO 124 after the rerun.
  - New in both: the iOS names `Replicator::SendDataJob` D0 and `sleepTime`, on bodies folded with ReplicatorJob and NetworkOwnerJob. The verdict is folded-original, so the oracle name is kept.
  - The STUDIO 0x0054b3b0 skip is unchanged.
- **Post-apply rerun:** nothing is pending except the no-op unnames and STUDIO 0x0054b3b0. The selftest passes.

**Facts** (tools/ida/match_names.py). Each was checked from the exe bytes.

| PLAYER | STUDIO | Name | Tag | Evidence |
|---|---|---|---|---|
| 0x006b2950 | 0x00811650 | `??_EFriendService@RBX@@WBI@AEPAXI@Z` | match: | `sub ecx,18h; jmp` the D0; vftable slot 0x00b39754. Its 8 bytes equal our thunk at 0x005c5ad0, jmp offset included. reccmp does not score thunks. |
| 0x006b2960 | 0x00811660 | `??_EFriendService@RBX@@WBM@AEPAXI@Z` | match: | `sub ecx,1Ch; jmp` the D0; slot 0x00b39744. Equal to our thunk at 0x005c5ae0. |
| 0x006b14e0 | 0x008101e0 | `?friendEventReplicationChanged@FriendService@RBX@@AAEXHHW4FriendEventType@12@@Z` | ios: | iOS 1029 symbol. Bound in onServiceProvider (push at 0x006b2c09). Our STUB pairs here. |
| 0x006b2440 | 0x00811140 | `?friendStatusReplicationChanged@FriendService@RBX@@AAEXHHW4FriendStatus@12@@Z` | ios: | iOS 1029 symbol. Bound in onServiceProvider (push at 0x006b2ba4). Our STUB pairs here. |
| 0x009c7aa0 | 0x006c3e40 | `?PauseAutoAdjustment@FrameRateManager@RBX@@QAEXXZ` | src2016: | /Od body `mov byte [this+0Ch],0`. Only caller: VideoControl::startRecording at 0x00465186 / 0x0048ccf6. STUDIO's old name was the FLIRT false positive `?ResetOverflow@TaskStack@details@Concurrency@@QAEXXZ_0`. |
| 0x009c7ac0 | 0x006c3e60 | `?ResumeAutoAdjustment@FrameRateManager@RBX@@QAEXXZ` | src2016: | `mov byte [this+0Ch],1`. Only caller: 0x00439477 / 0x004693e4. The decoration comes from unresolved.txt, so gen_link_stubs can now bind it. |

match_names.py change: a fact rename on a FLIRT-named address now becomes `rename-lib`, which clears FUNC_LIB.

## 3. Types

import_types.py, dumped from the scored PDBs. Every size equals the tree's // SIZE.

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| RBX::FriendService (+ its DescribedCreatable / Described / FactoryProduct chain) | 0x14c | ctor 0x006b2780 / 0x00811480, dtor 0x006b2970 / 0x00811670 |
| RBX::Network::PhysicsPacketCache (+ its DescribedNonCreatable chain) | 0x94 | ctor 0x005d74c0 / 0x00668990 |
| RBX::Network::ClusterPacketCache (+ its DescribedNonCreatable chain) | 0x98 | ctor 0x005d9a10 / 0x00669e30 |
| RBX::Network::Replicator, now in STUDIO too (removed from PLAYER_ONLY) | PLAYER 0x2aa8 (was 0x2aa4), STUDIO 0x2328 | STUDIO ctor 0x0064a0f0 and dtor 0x006497b0 are at 100% |

- **Changed members:** NetworkSettings +0xfc is now physicsMtuAdjust and +0x100 replicationMtuAdjust.
- **Counts:**
  - Local types: PLAYER 350 → 362 (130 listed), STUDIO 335 → 351 (125 listed).
  - `this` typed: PLAYER 532 → 541 (329 ctor/dtor), STUDIO 495 → 507 (308 ctor/dtor).
  - No size mismatch and no type removed.
- **Not imported:**
  - The Game ctor 0x00655e70 is unpaired.
  - InstancePacketCache: its ctor is at 0.655.
  - ReplicatorStats: its ctor and dtor are STUBs.
  - Replicator::ReplicationData and PacketReceiveJob have no size annotation.
  - Selection: its ctor and dtor are STUBs.
  - ScreenshotVerb: its ctor is at 0.759.
  - The other shell classes with a size annotation were already imported (LogManager, RobloxCrashReporter, MainLogManager, VideoControl, SoundState, the GameVerbs, CRenderSettingsItem, Metadata::Item and Class).

## 4. Unnamed functions

I regenerated ida/exports/PLAYER-app-unnamed-by-class.csv.

| Block | Functions | Named (ios:/mac:/match:), before → after | Still without one |
|---|---|---|---|
| PLAYER App (0x00606a00–0x009ad3a0) | 21,631 | 8,692 → 8,734 | 12,897 (59.6%) |
| Network (0x0051f6b0–0x00606a00) | 5,336 | 2,236 → 2,253 | 3,083 (57.8%) |

Largest App groups:

| Group | Functions |
|---|---|
| No caller with a known class | 4,016 (590 KB) |
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

FriendService (60 at the last sync) is no longer among them.

Largest Network groups:

| Group | Functions |
|---|---|
| Replicator | 341 |
| Players | 154 |
| ServerReplicator | 118 |
| Player | 111 |
| Replicator::StreamJob | 48 |
| FactoryProduct<>::Creator | 48 |
| ErrorCompPhysicsSender | 40 |
| InterpolatingPhysicsReceiver | 39 |
| PhysicsPacketCache | 38 |

## 5. Coverage, before → after

I regenerated the names, functions and types exports. The types export has 793 declarations for PLAYER and 690 for STUDIO. Function starts are unchanged (49,091 / 44,815), so I did not rerun pair_names.py, body_match.py or lib-resolved. The session note is in ida/LOG.md.

| | PLAYER | STUDIO |
|---|---|---|
| Named | 15,230 → 15,293 | 15,775 → 15,834 |
| match: | 7,355 → 7,418 | 7,023 → 7,086 |
| ios: | 6,223 → 6,225 | 6,282 → 6,284 |
| rtti: | 171 → 165 (now match:) | 166 → 162 |
| src2016: | 19 → 21 | 18 → 20 |
| flirt: | 97 | 175 → 172 |
| ida: | 1,072 → 1,074 | 1,805 → 1,804 |

## Files changed
- ida/PLAYER.i64, ida/STUDIO.i64
- ida/snapshots/{PLAYER,STUDIO}-25-wave16-sync.i64
- ida/LOG.md
- tools/ida/match_names.py: 6 new facts (both targets). A fact rename on a FLIRT name now becomes rename-lib.
- tools/ida/import_types.py: FriendService, PhysicsPacketCache and ClusterPacketCache added. Replicator removed from PLAYER_ONLY.
- ida/exports/{PLAYER,STUDIO}-names.csv, -functions.csv, -types.h, -proven-types.json, -db-updates.csv, -match-conflicts.csv
- ida/exports/PLAYER-app-unnamed-by-class.csv
- ida/logs/{PLAYER,STUDIO}-25-db-updates.log/.tsv, -25-types.log, -25-export-names.log

## Unresolved
- **Integrator (gen_reccmp_csv.py):** names.csv now carries the wave-16 MATCH_FIX names (PLAYER 0x006aaf50 / 0x006aaf40, STUDIO 0x00809d30 / 0x00809d20), so those entries can be dropped. Measure it at the next regeneration.
- **Integrator:** `?ResumeAutoAdjustment@FrameRateManager@RBX@@QAEXXZ` 0x009c7ac0 is now in names.csv, so gen_link_stubs should bind it as a stub. PauseAutoAdjustment 0x009c7aa0 is named too.
- **Network:** size annotations for Replicator::ReplicationData and PacketReceiveJob are still needed. InstancePacketCache and ReplicatorStats layouts wait on their ctors.
- **App:** the Game ctor 0x00655e70 (and its helper 0x00653790) and the Selection ctor/dtor are still unproven.
- **Not named:**
  - MemberDescriptorContainer<T>::compare 0x00614cd0.
  - GlobalAdvancedSettingsItem<CRenderSettingsItem, sRenderSettings>::singleton 0x00407230 and its createInstance 0x00409020.
  - The STUDIO checkLookup copies 0x006db970 / 0x00703620.
  - CSHA1 ×6 and TCPInterface/PacketizedTCP ×4 have no 2013 body.
- **Carried:**
  - pair_names.py proposals and STUDIO-PLAYER-bodymatch.csv predate STUDIO isUpperRoot 0x00a0e170.
  - STUDIO 0x004337a0 is still unnamed.
  - The program_options facet shell copies 0x00477aa0 / 0x00477c30 / 0x00477dd0 still carry filesystem names.
  - 0x009b7020 is still a dummy.
  - The STUDIO shell copies 0x0042f1d0 / 0x00430a80 / 0x00431ac0 are still dummies.
