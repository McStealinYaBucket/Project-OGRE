# ida-sync-24 (workflow phase5-wave21)

The wave 21 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0/.nam/.til left behind.
- Every IDA run was headless, one process per database, one at a time, through tools/capped.sh at 8G. None hit the cap.
- I did not touch ida/Archive.zip, Client/, gen_reccmp_csv.py or the shared reccmp/ CSVs, and committed nothing. check_2016_copies is silent.
- `fix_selectors.py nosave` ran before and after the session on both databases. All segment bases are 0 (PLAYER 9 segments, STUDIO 4), the function counts did not change, and STUDIO has 0 calls without their cref (PLAYER 19, as before). The nosave runs left the canonical files byte-identical (`cmp`).

## 1. Snapshots

ida/snapshots/PLAYER-30-wave21-sync.i64 and STUDIO-30-wave21-sync.i64 were taken before any change. Both are `cmp`-identical to the canonical files as they were then.

## 2. Names from the scored runs

Inputs: build/sweep28/P-final.json and build-studio/sweep28/S-final.json, each joined only with the build it scored (build*/sweep28/scored/).

- **Functions at effective 100%:** PLAYER 10,202, STUDIO 9,581 (sweep27: 10,068 / 9,470).
- **PLAYER renamed:** 69 functions (54 match:, 8 ios:, 5 src2016:, 1 prior:, 1 str:) and 29 data names.
- **STUDIO renamed:** 50 functions (43 match:, 6 ios:, 1 prior:) and 29 data names.
- **Conflicts:** PLAYER 131, STUDIO 128 (unchanged). After the apply, a rerun of match_names leaves only the two twin skips pending. The selftest passes.
- **New match: names (wave 21's code):**
  - UserInput: getWindowRect, getGameCursorPositionInternal, getGameCursorPositionExpandedInternal, onMouseInside, getWindowsCursorPositionInternal (0x00460320..0x00460780).
  - Document::FUN_0041df60 (the verb release).
  - The shell create<ScriptContext> chain: create, doGetClassIndex, callDoGetClassIndex, and the shared_ptr / shared_count Deleter ctors (STUDIO too).
  - The shell copies of `G3D::Rect2D::xyxy` 0x0045df70 and `Math::expandVector2` 0x0045dff0.
  - shared_ptr / shared_count<UserInputSleepingJob> 0x00461d20 / 0x004621f0.
  - ServerReplicator::readClientQuotaUpdate 0x00572390 / 0x006547d0.
  - `~ConnectionStats` 0x005c3d40 / 0x0061ec00, plus the StatsMap find_node_impl / delete_nodes and the callback map's find_node.
  - The 28 tool `Name::doDeclare<&sXTool>` / `callDoDeclare<&sXTool>` functions. 18 of them replace IDA's `j_` jump-thunk names.
  - STUDIO: `shared_from_polymorphic_downcast<Stats::Item>`, `shared_polymorphic_downcast<Stats::Item>`, `shared_static_cast<Instance>` (the shared_from twin integrate fixed).
- **Skipped (the same symbol on two originals; IDA names must be unique):** PLAYER 0x00462100 (the name sits on the engine copy 0x0070fb30) and STUDIO 0x0054b3b0 (the name sits on the shell copy 0x004f3770). The convention I propose is in section 6.

**Facts** (tools/ida/match_names.py: the wave21 block and the `TTOOL_NAMES` table). Each was checked from the exe bytes (capstone over the PE, string reads):

| PLAYER | STUDIO | Name | Tag | Evidence |
|---|---|---|---|---|
| 0x004601c0 | none | `UserInput::sendMouseEvent(int, const Vector2&)` (our decoration) | src2016: | builds a Vector2int16, calls getWindowSize 0x00460370, then sendEvent 0x00460210 |
| 0x00460210 | none | `RBX::UserInput::sendEvent` | src2016: | DataModel::processUiEvent 0x0063df50 on the overlay DataModel, then on the game DataModel |
| 0x00460370 | none | `RBX::UserInput::getWindowSize` | src2016: | GetClientRect, Rect2D::xyxy, two `__ftol2_sse` |
| 0x004609d0 | none | `RBX::UserInput::processInput` | str: | RobloxCritSecLoc "processInput" 0x00be4d20; calls acquireKeyboard |
| 0x0041d1c0 | none | `RBX::Document::PrepareShutdown` | src2016: | RemoveJobs 0x00466a30, getDataModel, verb release 0x0041df60, then 0x00467010 |
| 0x00466a30 | none | `?RemoveJobs@View@RBX@@QAEXXZ` | src2016: | three TaskScheduler removes, each followed by a shared_ptr reset |
| 0x007145b0 | none | `ServiceClient<Selection>::createService() const` | ios: | fills the cached shared_ptr at +4 through 0x006de0b0 |
| 0x006de0b0 | none | `ServiceProvider::create<Selection>(const Instance*)` | ios: | root ancestor, isA ServiceProvider, then `jmp create<Selection>() const` 0x00656f10 |
| 0x005c5d70 | 0x00620fd0 | `ConnectionStats::ConnectionStats()` | ios: | mtuSize 0x5d4 (1492), two RunningAverage<> with weight 0.1 |
| 0x005c4050 | 0x0061ef10 | ConnectionStats copy ctor | ios: | `rep movsd` 0x3c dwords, then both averages. STUDIO by position before updateStats, same size (224) |
| 0x005c4190 | 0x0061f050 | `StatsUpdateJob::updateStats(pair&, RakPeerInterface*)` | ios: | three RakPeer virtual calls, two RunningAverage::sample |
| 0x005d25f0 | 0x00670da0 | `StreamJob::receiveInstanceGcMessage(const Guid::Data&)` | ios: | lookupByGuid first, ret 4 |
| 0x005a8bd0 | 0x00618890 | `SharedScopeDictionary::receive(BitStream&, Guid::Scope&)` | prior: | reads a string, Name::declare, ret 8 (no iOS symbol) |
| 0x00573350 | 0x006551c0 | `ServerReplicator::setBasicFilteringEnabled(bool)` | ios: | new(4), NetworkFilter ctor folded into Faces::Faces(int), dtor folded into `ret` 0x00528720 |
| 0x005a8030 | 0x00617cb0 | `RBX::operator>><short>(BitStream&, short&)` | ios: | calls Read<unsigned short> 0x005a7080, returns the stream |
| 0x00d98b40 / 0x00d98b44 / 0x00e14628 | 0x00eb3b94 / 0x00eb3b98 / 0x00f694f0 | `AdvArrowToolBase::advManualJointMode` / `advManualJointType` / `advGridMode` (data1 / data4 / data4) | ios: | TurnOnManualJointCreation, SetManualJoint and SetGrid doIts test and store them |
| 26 globals in .rdata (0x00b8d388..0x00bb0668) | 26 (0x00beec18..0x00c72898) | `const char* const RBX::sXTool` (data4) | str: | each points to its tool string (all 52 checked); the 100% doDeclare pushes it (0x0070d948 pushes 0x00b8d590) |

These got guess: comments only, with no name: View 0x00465e70 (2016's View::Stop has its shape), 0x00466cf0, 0x00466e50, 0x00467010; the Selection size helper 0x0070eb00 (utilities.h:71 assert; iOS has no Selection::size); the ServerReplicator +0x88 signal ctor 0x005733e0 and its base ctor 0x0057bcd0.

Already named by an earlier sync: updateClientQuota 0x005d1310 / 0x0066fb00 and Read<unsigned short> 0x005a7080.

## 3. Types

import_types.py, dumped from the scored PDBs. Every size equals the tree's.

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| OverlayDataModel::DualLegacyLock | 0xc (DECOMP_SIZE_ASSERT) | ctor 0x0068c1f0 / 0x0084b050 |
| BoolPropertyVerb | 0x28 | ctor 0x00859130 / 0x007839f0 |
| Document (PLAYER only) | 0x28 (Document.h SIZE comment) | dtor 0x0041d020 |
| View (PLAYER only) | 0xcc (View.h SIZE comment) | dtor 0x00465b90 |

- **Local types:** PLAYER 396 → 400 (161 listed), STUDIO 378 → 380 (149).
- **`this` typed:** PLAYER 615 → 632 (382 ctor/dtor), STUDIO 568 → 571 (350).
- No size mismatch and no type removed. PLAYER still logs "NOT IN PDB RBX::UnsecuredStudioGame" (a STUDIO-only class, as before).
- **Not imported:**
  - STUDIO Game: ctor 0.9965, dtor 0.9907.
  - UserInput (0x2f8): ctor 0.46, dtor 0.991.
  - PropSync::Master (0x50): its ctor is inline, nothing of its own scores 100%, and the Base<MasterItem> ctor 0x005becb0 is unpaired. Its "SIZE 0x50." comment is also not on the line above the class.
  - LegacyLock::Implementation::Events: its ctor is at 100%, but the tree's `Events` 0x60 belongs to ReflectionMetadata.h.

## 4. Unnamed functions (PLAYER-app-unnamed-by-class.csv regenerated)

| Block | Functions | ios:/mac:/match:, before → after | Still without one |
|---|---|---|---|
| App (0x00606a00–0x009ad3a0) | 21,631 | 8,876 → 8,912 | 12,719 (58.8%) |
| Network (0x0051f6b0–0x00606a00) | 5,336 | 2,373 → 2,384 | 2,952 (55.3%) |
| Shell (0x00401000–0x0051f6b0) | 5,761 | 1,938 → 1,957 | 3,804 (66.0%) |

Largest App groups:

| Group | Functions |
|---|---|
| No caller with a known class | 3,945 (582 KB) |
| Ogre::RbxSceneManager (vftable slots) | 306 |
| FactoryProduct<>::Creator | 212 |
| EnumDesc<> | 178 |
| DataModel | 160 |
| RBX free functions | 151 |
| ScriptContext | 116 |
| Ogre::RbxSceneNode | 104 |
| Ogre::ManualObject | 95 |
| Humanoid | 92 |
| ScriptDebugger, Creatable<> | 86 each |
| PartInstance, RemoteFunction | 82 each |
| Workspace, SceneUpdater | 80 each |
| InsertService | 79 |
| Lua::Bridge<> | 68 |
| OverlayDataModel, TestService | 67 each |

Largest Network groups: no caller with a known class 821, Replicator 373, Players 127, Player 111, ServerReplicator 77, FactoryProduct<>::Creator 48, Replicator::StreamJob 46, InterpolatingPhysicsReceiver 39, ErrorCompPhysicsSender 38, PhysicsPacketCache 37.

## 5. Coverage (functions with a non-dummy name)

| | Before | After |
|---|---|---|
| PLAYER named (of 49,091) | 15,738 | 15,789 |
| match: | 7,752 | 7,810 |
| ios: | 6,254 | 6,262 |
| src2016: | 29 | 34 |
| str: | 24 | 25 |
| prior: | 24 | 21 |
| ida: | 1,076 | 1,058 |
| PLAYER data rows | 40,465 | 40,494 |
| STUDIO named (of 44,815) | 16,171 | 16,202 |
| match: | 7,357 | 7,400 |
| ios: | 6,308 | 6,314 |
| prior: | 5 | 6 |
| ida: | 1,805 | 1,786 |
| STUDIO data rows | 41,827 | 41,856 |

prior: dropped by 3 in PLAYER because three prior: names are now reccmp 100% and became match:. ida: dropped because the `j_` thunk names of the doDeclare targets were replaced by the callDoDeclare match: names.

## 6. Twin names: proposed convention (not applied; gen_reccmp_csv.py is not mine)

There are two cases where one symbol has two originals, a shell (LTCG) copy and an engine copy:
- PLAYER: 0x00462100 (shell) and 0x0070fb30 (engine).
- STUDIO: 0x004f3770 (shell) and 0x0054b3b0 (engine).

Today IDA keeps the name on whichever address had it first. The generator then needs MATCH_DROP entries to stop a stale `match:` row from pairing.

**Proposal:**
1. In IDA, the engine copy keeps the plain symbol. The copy in the shell range (PLAYER 0x00401000–0x0051f6b0, STUDIO's shell objects) gets the same symbol plus a `.shell` suffix. `.` is legal in IDA names and never appears in an MSVC decorated name. `$` is not safe because our PDB already uses `name$0` for EH funclets.
2. match_names.py writes the `.shell` form whenever the reccmp pair's original sits in the shell range and the plain name is already taken by the engine copy. It would also do this for future twins.
3. gen_reccmp_csv.py strips `.shell` and pairs the row only with a body of ours from a shell object (`Client/Win`, `Client/WindowsClient`, `Client/ClientShared`, …). It pairs the plain row only with an engine object.

With this, PLAYER MATCH_DROP 0x0070fb30 and STUDIO MATCH_DROP 0x00431860 become unnecessary: when our link lacks a copy, its row finds no body on its side, instead of pairing across. This needs the generator owner's A/B before match_names starts writing the suffix.

## Files

- tools/ida/match_names.py: the wave21 FACTS block and the TTOOL_NAMES table.
- tools/ida/import_types.py: four classes added to CLASSES (Document and View in PLAYER_ONLY).
- ida/exports/{PLAYER,STUDIO}-{names,functions,types.h,proven-types.json,db-updates,match-conflicts}.csv, plus PLAYER-app-unnamed-by-class.csv. Function starts and sizes are unchanged against HEAD, so the evidence export, body_match, pair_names and lib-resolved were not rerun.
- ida/LOG.md.
- Logs: ida/logs/<T>-30-*.log and <T>-30-db-updates.tsv.
- Snapshots: ida/snapshots/<T>-30-wave21-sync.i64.

## Unresolved

- **Twin names:** see section 6. The generator owner has to decide. Until then PLAYER 0x00462100 and STUDIO 0x0054b3b0 stay unnamed.
- **STUDIO addresses not found:**
  - ServiceClient<Selection>::createService: STUDIO 0x00701c50 and 0x00701d00 are both 168 bytes, and neither calls a proven create<Selection>.
  - create<Selection>(const Instance*).
  - The Selection size helper.
  - Only PLAYER has these names.
- **OverlayDataModel 0x00652f00 / 0x007cba50:** it is still a STUB (0.349), so it keeps its guess: comment.
- **Shell layouts:** UserInput (ctor 0.46, dtor 0.991) and STUDIO Game (ctor 0.9965, dtor 0.9907) are still below 100%, so their layouts were not imported.
- **Network:** PropSync::Master has no ctor or dtor of its own to prove its layout. Its "SIZE 0x50." comment needs a `// SIZE 0x50` line directly above `class Master` before an import can pick it up. The ServerReplicator ctor 0x0057c3b0 would prove it.
