# ida-sync-23 (workflow phase5-wave20)

The wave 20 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0/.nam/.til left behind.
- Every IDA run was headless, one process per database, one at a time, through tools/capped.sh at 8G. None hit the cap.
- I did not touch ida/Archive.zip, Client/, gen_reccmp_csv.py or the shared reccmp/ CSVs, and committed nothing. check_2016_copies is silent.
- `fix_selectors.py nosave` ran before and after the session on both databases. All segment bases are 0, the function counts did not change, and STUDIO has 0 calls without their cref (PLAYER has 19, as before).

## 1. Snapshots

ida/snapshots/PLAYER-29-wave20-sync.i64 and STUDIO-29-wave20-sync.i64 were taken before any change. Both are `cmp`-identical to the canonical files as they were then. The nosave selector runs left the canonical files byte-identical, which I checked with `cmp`.

## 2. Names from the scored runs

Inputs: build/sweep27/P-final.json and build-studio/sweep27/S-final.json. Each was joined only with the build it scored (build*/sweep27/scored/).

- **Functions at effective 100%:** PLAYER 10,068, STUDIO 9,470 (sweep26: 9,997 / 9,406). The 1,746 library-owned rows stay with apply_lib_names.py.
- **PLAYER renamed:** 82 functions: 27 match:, 28 rtti:, 14 ios:, 6 str:, 5 prior:, 2 src2016:.
- **STUDIO renamed:** 56 functions: 18 match:, 27 rtti:, 11 ios:. FUNC_LIB was cleared on 2, including the FLIRT `_Locimp` false positive on ~RunStateVerb 0x00781a70.
- **New match: names (wave 20's code):**
  - Application::Shutdown 0x00404f10.
  - FunctionMarshaller: safe_static_init_staticData 0x00436ce0, plus the window map's erase / _Eqrange / _Distance2.
  - ~View 0x00465b90.
  - The shell create<RunService> and create<ControllerService> chains: doDeclare, doGetClassIndex, the shared_ptr / shared_count Deleter ctors.
  - Players::friendServiceRequest.
  - Event::getInstance and EventInvocation::replicateEvent (0x00572d90 / 0x00572dc0).
  - ServerReplicator::rebroadcastEvent and RakNetToRbxAddress 0x005c6c50.
  - The StatsUpdateJob StatsMap delete_nodes.
  - checked_delete<CommonVerbs>.
  - The RunStateVerb ctor and dtor, ~EditSelectionVerb, and the DeleteBase ctor (both targets).
  - Replicator::OnReceive 0x0059d0e0, named from its rtti: slot.
- **Skipped (the same symbol on two originals; IDA names must be unique):**
  - PLAYER 0x00462100: the shell `shared_ptr<RunService>(p, Deleter)`. The name already sits on the engine copy 0x0070fb30, which is integrate's MATCH_DROP pair.
  - STUDIO 0x0054b3b0: the open item from ida-sync-22. It is the engine copy of `_Tree<set<shared_ptr<Job>>>::_Erase`, and the name sits on the shell copy 0x004f3770.
  - Both are documented in LOG.md rather than given suffixed names.
- **Conflicts:** PLAYER 131, STUDIO 128. The post-apply rerun leaves only those two skips and the old no-op unnames pending. The match_names selftest passes.

**Facts** (tools/ida/match_names.py, wave20 block plus the TTOOLS table). Each was checked from the exe bytes (capstone over the PE, RTTI COL walk):

| PLAYER | STUDIO | Name | Tag | Evidence |
|---|---|---|---|---|
| 0x00436c70 | none | `FunctionMarshaller::safe_static_do_get_staticData` | prior: | guard 0x00e2500c, calls 0x00436d70, atexit 0x00a8f980, returns 0x00e25010; 0x00436ce0 (100%) jumps here |
| 0x00436d70 | none | `StaticData::StaticData()` | prior: | /GL copy with `this` = 0x00e25010 propagated: new map head → 0x00e25014, zeroes, returns 0x00e25010 |
| 0x00437690, 0x00a8f980 | none | `~StaticData`, its atexit thunk `??__Fvalue@…` | ios: / prior: | the thunk is `jmp 0x00437690`; the dtor walks the map at 0x00e25014 |
| 0x0045f2a0 | none | `UserInput::ProcessUserInputMessage(UINT,WPARAM,LPARAM)` | str: | RobloxCritSecLoc "ProcessUserInputMessage" |
| 0x0045edf0 / 0x0045ee20 | none | `setKeyboardDesiredInternal(bool)` / `setKeyboardDesired(bool)` | src2016: / str: | "setKeyboardDesired" critsec; calls acquireKeyboard |
| 0x0045eec0 / 0x0045ef20 / 0x0045f050 / 0x0045f120 | none | `acquireMouseInternal` / `acquireMouseInternalBase(const Vector2&)` / `unAcquireMouse` / `acquireKeyboard` | src2016: / str: | UserInput.cpp asserts 269 "!isMouseAcquired", 300 "isMouseAcquired", 340 "!isKeyboardAcquired"; access from the 2016 header (private, except setKeyboardDesired) |
| 0x0045e0c0 | none | `~UserInputBase` (shell copy) | ios: | destroys the signal at +0x34, the shared_ptr at +0x30, the cursors and a ContentId. **Replaces the wrong ios: `scoped_ptr<VirtualHardwareDevice>::~scoped_ptr`** |
| 0x00462a50 / 0x00462b00 | none | `safe_queue<UserInputSleepingJob::Message>::pop_if_present` / `peek_if_present` | prior: | our link's names (shell-5k-11) |
| 0x00643100 / 0x0062daf0 | 0x00760980 / 0x0074dd60 | `DataModel::scoped_write_request` ctor / dtor | ios: | ctor locks +0xd50 and bumps +0xca0; dtor undoes that. 18 PLAYER stepDataModelJob bodies call the pair. The STUDIO pair are the common callees of its HeartbeatTask / PhysicsJob / GcJob / WaitingScriptsJob steps, with equal sizes |
| 0x0068c1f0 | 0x0084b050 | `OverlayDataModel::DualLegacyLock` ctor | ios: | the iOS spelling; Windows passes two shared_ptrs by value. LegacyLock at +0, scoped_write_request at +4, 0x006431d0 at +8 |
| 0x0041d550 | none | `~DualLegacyLock` (shell /GS copy) | ios: | destroys +8, +4 (0x0062daf0), +0 (~LegacyLock 0x00642370) |
| 0x0041dfd0 | none | `RBX::Document::executeScript` | str: | "Exception occured in Document::executeScript: %s". The VMProtect marker region from 0x0041e0d9 cuts IDA's function at 270 bytes. **app-20's "shell function at 0x0041e302" is one of its mutated fragments.** It inlines the Impersonator dtor at 0x0041e4ef / 0x0041e5c8 |
| 0x00861c70..0x00864320 (26) | 0x009d37a0..0x009d5d60 (26) | `TToolVerb<T, RunStateVerb>::TToolVerb(DataModel*, bool)` | rtti: | each stores its `.?AV?$TToolVerb@V<T>@…` vftable; the iOS 1029 signature (all 26 checked against the symtab) |
| 0x00859130 | 0x007839f0 | `BoolPropertyVerb` ctor (our decoration) | rtti: | stores ??_7BoolPropertyVerb 0x00b7c454 |
| 0x0071b970 | 0x007b8f90 | `PartInstance::fromPrimitive` (fromConstPrimitive folds here) | ios: | `p ? owner(+0x10c) - 0x68 : 0` |
| 0x007fa010 | 0x009155a0 | `Mechanism::getRootMovingPrimitive` | ios: | `jmp getConstRootMovingPrimitive`; was the ida: name of the jump target |
| 0x0058b260 | 0x0062f8d0 | `Replicator::createPhysicsSender(PhysicsSendMethod)` | ios: | called by onSentMarker in both targets |
| 0x00574f90 | 0x006502d0 | `PropSync::detail::Base<MasterItem>::expireItems` | ios: | called by dataOutStep (STUDIO 0x00650b39) |
| 0x005becb0 | 0x00650210 | `Base<MasterItem>::Base(Time::Interval)` | ios: | called by the ServerReplicator and ClientReplicator ctors (Base<SlaveItem> folds here) |
| 0x00578e70 | 0x00658120 | `PropSync::Master::onReceivedPropertyChanged` | ios: | next to Master::onPropertyChanged |
| 0x005d29f0 | 0x006711a0 | `Replicator::StreamJob::readInstanceRemoval` | ios: | readItem case 15 |
| 0x00572050 | 0x00654490 | `ServerReplicator::checkDistributedReceive` | ios: | calls getRootMovingPrimitive, fromPrimitive, RakNetToRbxAddress |

Already named, so unchanged: EditSelectionVerb and RotateAxisCommand ctors (mac:/ios:), Player::loadCharacter, checkDistributedSend / SendFast / onSentMarker, the StatsUpdateJob ctor and D0, Master::onPropertyChanged, RakNetAddressToString. Integrate's `_DLRealloc_Ex` / unlockInstance item was already done in wave 19 (comment fact on 0x0096bde0).

## 3. Types

import_types.py, dumped from the scored PDBs. Every size equals the tree's // SIZE or DECOMP_SIZE_ASSERT.

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| CommonVerbs | 0x724 | ctor 0x00864450 / 0x009d5e90, dtor 0x006531e0 / 0x007cbbb0 |
| RunStateVerb | 0x1c | ctor 0x00857780 / 0x00781ee0, dtor 0x00857430 / 0x00781a70 |
| EditSelectionVerb | 0x24 | dtor 0x008574b0 / 0x00781af0 (ctor PLAYER 100%, STUDIO 0.977) |
| DeleteBase | 0x28 | ctor 0x00858d10 / 0x00783740 |
| RotateAxisCommand | 0x24 | ctor 0x008590c0 / 0x00783980 |
| ToggleViewMode | 0x10 | ctor 0x00856130 / 0x00780750 |
| ConcurrentRakPeer::StatsUpdateJob | 0x290 | ctor 0x005c5f60 / 0x006211c0 |
| Network::GuidRegistryService | 0x6c | ctor 0x005baf10 / 0x0061d900, dtor 0x005ba990 / 0x0061cf70 |
| Network::SharedScopeDictionary | 0x101c | ctor 0x005aa320 / 0x0061a150, dtor 0x00599140 / 0x0061a1b0 |
| Profiling::Profiler | 0xc040 | ctor 0x007451d0 / 0x007d7a30 |

- **Counts:**
  - Local types: PLAYER 382 → 396 (157 listed), STUDIO 364 → 378 (147).
  - `this` typed: PLAYER 597 → 615 (377 ctor/dtor), STUDIO 551 → 568 (348).
  - No size mismatch and no type removed.
- **Not imported:**
  - STUDIO Game (ctor 0.9965, dtor 0.9907).
  - UserInput (ctor 0.46, dtor 0.987).
  - Document and View: their dtors are 100%, but the tree has no size assertion for either. Document is 0x28 by its own layout comment.
  - The name `Events` matched a // SIZE 0x60 in ClientBase/ReflectionMetadata.h, which is a different class, so it was not imported.

## 4. Unnamed functions (PLAYER-app-unnamed-by-class.csv regenerated)

| Block | Functions | ios:/mac:/match:, before → after | Still without one |
|---|---|---|---|
| App (0x00606a00–0x009ad3a0) | 21,631 | 8,866 → 8,876 | 12,755 (59.0%) |
| Network (0x0051f6b0–0x00606a00) | 5,336 | 2,361 → 2,373 | 2,963 (55.5%) |
| Shell (0x00401000–0x0051f6b0) | 5,761 | 1,920 → 1,938 | 3,823 (66.4%) |

The rtti:/str:/prior:/src2016: names (TToolVerb, UserInput, …) do not count in the oracle column. App rtti: went from 118 to 145.

Largest App groups:

| Group | Functions |
|---|---|
| No caller with a known class | 3,971 (582 KB) |
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

Largest Network groups:

| Group | Functions |
|---|---|
| No caller with a known class | 821 |
| Replicator | 375 |
| Players | 127 |
| Player | 111 |
| ServerReplicator | 80 |
| FactoryProduct<>::Creator | 48 |
| Replicator::StreamJob | 47 |
| InterpolatingPhysicsReceiver | 39 |
| ErrorCompPhysicsSender | 38 |
| PhysicsPacketCache | 37 |

## 5. Coverage (functions with a non-dummy name)

| | Before | After |
|---|---|---|
| PLAYER named (of 49,091) | 15,656 | 15,738 |
| match: | 7,725 | 7,752 |
| ios: | 6,241 | 6,254 |
| rtti: | 183 | 211 |
| str: | 18 | 24 |
| prior: | 19 | 24 |
| src2016: | 27 | 29 |
| STUDIO named (of 44,815) | 16,113 | 16,171 |
| match: | 7,338 | 7,357 |
| ios: | 6,297 | 6,308 |
| rtti: | 168 | 194 |
| flirt: | 170 | 169 |

ida: rose by 1 in PLAYER and 3 in STUDIO. These are IDA's own `j_` thunk names for newly named targets.

## Exports

Regenerated:
- names, functions and types (PLAYER 831 declarations, STUDIO 717);
- proven-types JSON;
- db-updates and match-conflicts;
- PLAYER-app-unnamed-by-class.csv.

Function starts and sizes are unchanged (checked against HEAD). So the evidence export, body_match, pair_names and lib-resolved were not rerun.

## Files

- tools/ida/match_names.py: the wave20 FACTS block and the TTOOLS table.
- tools/ida/import_types.py: ten classes added to CLASSES.
- ida/exports/{PLAYER,STUDIO}-{names,functions,types.h,proven-types.json,db-updates,match-conflicts}.csv, plus PLAYER-app-unnamed-by-class.csv.
- ida/LOG.md.
- Logs: ida/logs/<T>-29-*.log and <T>-29-db-updates.tsv.
- Snapshots: ida/snapshots/<T>-29-wave20-sync.i64.

## Unresolved

- **Twin names:** PLAYER 0x00462100 / 0x0070fb30 and STUDIO 0x0054b3b0 / 0x004f3770 each hold one symbol on two originals, and IDA keeps it on the address that had it first. If a shell or engine suffix convention is wanted, the generator owner should decide it, because gen_reccmp_csv reads these names.
- **OverlayDataModel 0x00652f00 / 0x007cba50:** it stores arg0 at +0xe58 and assigns a by-value shared_ptr to +0xe5c (`ret 8`). There is no oracle name, our STUB scores 0.349, and it keeps its guess: comment.
- **Shell:** add a `// SIZE` for Document (0x28 per its layout comment) and View, so the next sync can import them (their dtors are 100%).
- **Shell:** 0x0041e302 is not a function. It is a VMProtect-mutated fragment of Document::executeScript 0x0041dfd0, so the "real caller" lead for the Impersonator frame is executeScript, and its body is partly mutated code.
- **App:** STUDIO Game (ctor 0.9965, dtor 0.9907) and UserInput (ctor 0.46) are still below 100%, so their layouts were not imported.
