# ida-sync-22 (workflow phase5-wave19)

The wave 19 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0/.nam/.til left behind.
- Every IDA run was headless, one process per database, one at a time, through tools/capped.sh at 8G. None hit the cap.
- I did not touch ida/Archive.zip, Client/, gen_reccmp_csv.py or the shared reccmp/ CSVs, and committed nothing. check_2016_copies is silent.
- **The main finding is damage to the STUDIO database, now repaired (section 4).** From the wave9 sync onwards, STUDIO recorded near call and jump targets 0x10 bytes short. That damage is what made the regenerated STUDIO pair_names proposals drop 452 rows. After the repair they drop none.

## 1. Snapshots

- ida/snapshots/PLAYER-28-wave19-sync.i64 and STUDIO-28-wave19-sync.i64 were taken before any change. Both are `cmp`-identical to the canonical files as they were then.
- ida/snapshots/STUDIO-28-wave19-pre-selector-fix.i64 was taken right before the repair, after steps 2 and 3.

## 2. Names from the scored runs

Inputs: build/sweep26/P-final.json and build-studio/sweep26/S-final.json. Each was joined only with the build it scored (build*/sweep26/scored/).

- **Functions at effective 100%:** PLAYER 9,997, STUDIO 9,406 (sweep25: 9,926 / 9,346). Library-owned rows (1,747 / 1,648) stay with apply_lib_names.py.
- **PLAYER renamed:** 56 functions (43 match:, 4 ios:, 3 rtti:, 3 src2016:, 3 prior:) and 8 data names.
- **STUDIO renamed:** 37 functions (34 match:, 2 of them through the PLAYER body match; 2 ios:; 1 prior:) and 3 data names.
- **New match: names (wave 19's code):**
  - Application ctor 0x00403000 and its boost::thread bind / make_thread_info / heap_new chain;
  - CEvent(int,int) 0x0041b060, intrusive_ptr_target<islot>::operator new;
  - SleepingJob ctor and sleep (0x0072b000 / 0x0072b0e0);
  - the YieldingThreads::WaitingThread ctors and their priority_queue / heap templates (0x008a8fe0 to 0x008a97f0);
  - the CommonVerbs shared_ptr chain, CameraVerb::~CameraVerb 0x00653700, Game::globalExit 0x00653780 / 0x007cc240;
  - TaskScheduler::removeBlocking (shell copy), RobloxCriticalSection dtor, the UserInputSleepingJob D0 and deque _Tidy;
  - Players::killPlayer, ServerReplicator::canUseProtocolVersion, Replicator::requestDisconnect and its bind templates;
  - the ConcurrentRakPeer PacketJob queue templates, initVersion1, and callDoDeclare<sHumanoid> (it was an IDA `j_` name).
- **Conflicts:** PLAYER 131, STUDIO 127. Two verdicts per target moved from same-function to folded-original, and the oracle name stays in both.
- **Post-apply rerun:** no rename is pending except STUDIO 0x0054b3b0, as before. The match_names selftest passes.

**Facts** (tools/ida/match_names.py, wave19 block), each checked from the exe bytes:

| PLAYER | STUDIO | Name | Tag | Evidence |
|---|---|---|---|---|
| 0x004627b0 | (none) | `UserInputSleepingJob(UserInput*, shared_ptr<DataModelArbiter>)` | prior: | LTCG frame with `this` on the stack. [esp+0x34] is stored to +0x234 after the vftable store. The shared_ptr goes to the SleepingJob base ctor 0x0072b000. `ret 0x10` = this + a pointer + a shared_ptr. **Closes the ida-sync-21 open item.** |
| 0x0045e990 | (none) | `UserInput::~UserInput()` | rtti: | Stores ??_7UserInput 0x00be4d9c, then calls removeJobs. Our STUB scores 0.935. |
| 0x0041d020 / 0x0041d270 | (none) | `Document::~Document` / `Document::Shutdown` | src2016: | Application::Shutdown calls Shutdown (0x00404f82), then the dtor (0x00404f92). |
| 0x00437180 | (none) | `FunctionMarshaller::ReleaseWindow(FunctionMarshaller*)` | ios: | call_once mutex, window map erase, DestroyWindow |
| 0x00466840 | (none) | `View::initializeInput()` | src2016: | news the UserInput and calls its ctor, then create<ControllerService> |
| 0x0041ce60 | 0x004d5770 | `Game::getOverlayDataModel() const` | prior: | The shell copy returns Game+0x0c (proven layout). It is the tree's name. 28 call sites. |
| 0x00468f30 | 0x004f9c20 | `ServiceProvider::create<ControllerService>() const` | ios: | `this` in ecx; calls find<> then Creatable::create<> |
| 0x0047b790 | (none) | `variables_map::variables_map()` | rtti: | Stores the abstract_variables_map vftable, then the variables_map vftable. Only caller is the Application ctor. |
| 0x0074fcc0 | (none) | `RBX::rot13(std::string)` | ios: | isalpha, then tolower −'a' < 13 ? +13 : −13. Only caller initVersion1. The decoration is from unresolved.txt, so gen_link_stubs can bind it. STUDIO has no copy. |
| 0x00876cc0 | 0x009db8e0 | `ObjectBridge::lockInstance` | ios: | the "Lock" slot of the Instance classLibrary |
| 0x00b80464 | 0x00c5f184 | `registerInstanceClassLibrary`'s `classLibrary` (data4) | str: | `{new, Lock, Unlock}`, pushed with "Instance" by openState at 0x00688860 |
| 0x009b7020 | (none) | `vector<shared_ptr<TaskScheduler::Thread>>::push_back` | prior: | A folded body: disableThreads, getJobsByName, LoadZ and TextureCompositor all call it. Replaces the wave-9 unname. **Closes the open "dummy" item.** |
| 0x00de68d0 | 0x00f8935c | `RBX::s_profanityFilter` (data8) | src2016: | globalExit is `mov ecx,0xde68d0; jmp reset` |
| 0x00df4e48 | 0x00f8d5b8 | `FFlag::LastWakeTimeSleepingJobError` (data1) | str: | read by SleepingJob::error and wake |
| 0x00bc3628 / 0x00bc35c8 / 0x00bc35d8 | (none) | `_IID_IDirectInput8A`, `_GUID_SysKeyboard`, `_GUID_SysMouse` (data16) | lib: | the GUID values in .rdata |
| 0x00bc3824 / 0x00bc3a2c | (none) | `_c_dfDIMouse2`, `_c_dfDIKeyboard` (data24) | lib: | DIDATAFORMAT {0x18, 0x10, 2, 0x14 or 0x100, 11 or 256 objects, …} |

- **Comment facts:**
  - **unlockInstance** (`xor eax,eax; ret`) folds into PLAYER 0x0096bde0, where the ios: topHashCode name stays, and into STUDIO 0x00a150d0, where the match: GetIPPROTO name stays.
  - Integrate's `_DLRealloc_Ex` name comes from our own link folding the same body with RakNet's, not from the IDA database.
  - The shell `~basic_string` import thunk 0x0041c8a0 also gets a comment.
- **Tool changes:**
  - apply_db_updates.py now accepts any `dataN` action (an N-byte array).
  - A comment fact's tag becomes the export's source, so each comment fact now carries the tag of the name it sits on. A first version tagged str: briefly relabelled two names; that was found and fixed in the same session.

## 3. Types

import_types.py, dumped from the scored PDBs. Every size equals the tree's // SIZE or DECOMP_SIZE_ASSERT.

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| DataModel::LegacyLock::Implementation | 0x20 | dtor 0x0063a380 / 0x00758fd0 |
| Lua::YieldingThreads | 0x18 | ctor 0x008a9e20 / 0x009eda40 |
| SleepingJob | 0x230 | ctor 0x0072b000 / 0x00829ff0 |
| CameraVerb | 0x1c | dtor 0x00653700 / 0x00781c80 |
| UnsecuredStudioGame (STUDIO) | 0x34 | ctor 0x007ce190 (it brings STUDIO's Game in as its base) |
| Application (PLAYER) | 0x140 | ctor 0x00403000 |
| SecurePlayerGame (PLAYER) | 0x34 | ctor 0x00656250 |
| UserInputSleepingJob (PLAYER) | 0x260 | D0 0x004628d0 (vftable 0x00be4e18 at 100%) |

- **Changed member:** NetworkSettings +0x78 is now isQueueErrorComputed.
- **Counts:**
  - Local types: PLAYER 375 → 382 (147 listed), STUDIO 358 → 364 (137).
  - `this` typed: PLAYER 579 → 597 (360 ctor/dtor), STUDIO 535 → 551 (331).
  - No size mismatch and no type removed.
- **Not imported:**
  - STUDIO Game as a listed class (ctor 0.9965, dtor 0.9907);
  - UserInput (0x2f8; ctor 0.46, dtor 0.935);
  - ToggleViewMode (ctor 0.986);
  - CommonVerbs (opaque);
  - StatsUpdateJob (not declared in the tree).

## 4. STUDIO database repair (new tools/ida/fix_selectors.py)

- **Symptom:** in STUDIO, Name::declare 0x00703190 had an empty callee list in the evidence export.
  - Its bytes `e8 e3 fc ff ff` at 0x007032a8 are `call 0x702f90`.
  - IDA decoded the target as 0x702f80 and stored that cref.
  - export_evidence drops a callee that is not a function start, so the call disappeared.
- **Cause:** STUDIO's selectors 1/2/3 had become paragraphs 1/2/3, so the segment bases were 0x10/0x20/0x30 (PLAYER's are 0).
  - Every instruction IDA re-decoded after that (functions retyped or renamed and then reanalysed) got near targets 0x10 short.
  - Data references were not affected.
- **When:** I checked the snapshots.
  - STUDIO-00 through STUDIO-18 have base 0 and no short crefs.
  - STUDIO-19 through STUDIO-28 have base 0x10, with 2,053 growing to 2,962 calls 0x10 short of a function start.
  - So it happened during the **wave9 sync** session. No tool in tools/ida sets selectors, so the exact cause is unknown.
- **Repair:**
  - Every selector was set back to paragraph 0.
  - 3,661 call crefs and 1,953 jump crefs were moved to their decoded operands, and those heads were re-planned.
  - Measured afterwards: 0 near operands lack their cref, 0 crefs are short, and calls to a function start number 122,697, the same as the clean snapshots. There are still 44,815 functions.
  - I tested it first on a scratch copy (dry run, then a wet run and a re-check), then applied it to the canonical database.
- **Effects downstream:**
  - **Evidence:** STUDIO evidence re-exported. Functions with a callee went from 32,963 to 33,476.
  - **Body match:** body_match.py gives 20,129 pairs, unchanged.
  - **pair_names (STUDIO), the review ida-sync-21 left open:** against the repaired evidence it drops **0** committed rows (wave 18's run dropped 452). The 452 are fully explained by the damage:
    - Empty callee lists made graph_ok reject body-match seeds. Example: Name::declare's ios1021/mac seed failed because the oracle's callee Name::Name C2 is paired, but the damaged STUDIO side "did not reach" it.
    - The callgraph rows those seeds grew were then lost with them: 179 of the 452 hung off Name::declare alone and 28 off Described<Instance>::classDescriptor.
  - **New proposals:** they add 6 rows: 5 rtti-slot rows on folded or already match:-named slots, and isUpperRoot 0x00a0e170 (string-unique, the same name as in the database). Four GuiTextButton rows move from rtti-slot to callgraph/body-match with the same names (its vftable 0 now counts as a slot mismatch).
  - **Committed:** the new proposals are in the tree (no git commit). match_names reruns with nothing pending, and only 2 conflict verdicts per target change (to folded-original; the oracle names stay).

## 5. Unnamed functions (PLAYER-app-unnamed-by-class.csv regenerated)

| Block | Functions | ios:/mac:/match:, before → after | Still without one |
|---|---|---|---|
| App (0x00606a00–0x009ad3a0) | 21,631 | 8,844 → 8,866 | 12,765 (59.0%) |
| Network (0x0051f6b0–0x00606a00) | 5,336 | 2,351 → 2,361 | 2,975 (55.8%) |
| Shell (0x00401000–0x0051f6b0) | 5,761 | 1,905 → 1,920 | 3,841 (66.7%) |

Largest App groups:

| Group | Functions |
|---|---|
| No caller with a known class | 3,985 (586 KB) |
| Ogre::RbxSceneManager (vftable slots) | 306 |
| FactoryProduct<>::Creator | 212 |
| EnumDesc<> | 178 |
| DataModel | 160 |
| RBX free functions | 151 |
| ScriptContext | 116 |
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
| Lua::Bridge<> | 68 |
| OverlayDataModel | 68 |

Largest Network groups:

| Group | Functions |
|---|---|
| Replicator | 339 |
| Players | 128 |
| ServerReplicator | 115 |
| Player | 111 |
| FactoryProduct<>::Creator | 48 |
| Replicator::StreamJob | 47 |
| InterpolatingPhysicsReceiver | 39 |
| ErrorCompPhysicsSender | 38 |
| PhysicsPacketCache | 37 |

Largest Shell groups:

| Group | Functions |
|---|---|
| No caller with a known class | 1,163 |
| ATL CComContainedObject<CAxHostWindow> | 163 |
| FactoryProduct<>::Creator | 111 |
| ViewRbxGfx | 89 |
| FastClusterMeshGenerator | 86 |
| FastCluster | 84 |
| LightGrid | 76 |
| TextureCompositor | 74 |

## 6. Coverage, before → after

Functions with a non-dummy name. Function starts and sizes are unchanged (49,091 / 44,815). Types export: PLAYER 806 → 817 declarations, STUDIO 697 → 703.

| | PLAYER | STUDIO |
|---|---|---|
| Named | 15,601 → 15,656 | 16,077 → 16,113 |
| match: | 7,681 → 7,725 | 7,304 → 7,338 |
| ios: | 6,237 → 6,241 | 6,295 → 6,297 |
| rtti: | 181 → 183 | 168 |
| src2016: | 24 → 27 | 22 |
| prior: | 16 → 19 | 4 → 5 |
| ida: | 1,076 → 1,075 | 1,803 → 1,802 |
| Data names | 40,457 → 40,465 | 41,824 → 41,827 |

## Files changed

- ida/PLAYER.i64, ida/STUDIO.i64
- ida/snapshots/{PLAYER,STUDIO}-28-wave19-sync.i64, ida/snapshots/STUDIO-28-wave19-pre-selector-fix.i64
- ida/LOG.md
- tools/ida/match_names.py: the wave19 fact block; the 0x009b7020 unname fact became a rename.
- tools/ida/apply_db_updates.py: accepts `dataN`.
- tools/ida/import_types.py: 8 classes added; Application, SecurePlayerGame and UserInputSleepingJob are PLAYER_ONLY.
- tools/ida/fix_selectors.py (new)
- ida/exports/:
  - {PLAYER,STUDIO}-names.csv, -functions.csv, -types.h, -proven-types.json, -db-updates.csv, -match-conflicts.csv;
  - STUDIO-names-proposed.csv, STUDIO-rtti-mismatch-{ios1021,mac}.csv;
  - PLAYER-app-unnamed-by-class.csv;
  - STUDIO-evidence/ (gitignored).
- ida/logs/{PLAYER,STUDIO}-28-*.log / .tsv

## Unresolved

- **Integrator (gen_reccmp_csv.py):** regenerate the CSVs from the new names.csv.
  - rot13 0x0074fcc0 now carries the decoration from unresolved.txt, so gen_link_stubs can bind it.
  - The five DirectInput data rows exist for when dinput8.lib and dxguid.lib are vendored (that is the user's call).
  - The Instance classLibrary 0x00b80464 is named.
  - 0x009b7020 now carries the Thread spelling of push_back.
- **STUDIO selector damage:** the wave9-session cause is unknown. Running `fix_selectors.py nosave` on a copy prints the segment bases, and is a cheap check after any session that runs a new kind of script.
  - Any STUDIO analysis between wave 9 and now that read IDA xrefs directly saw short targets on about 2.4% of calls. The STUDIO evidence was only re-exported in wave 18, so of the exports only that one (now replaced) was affected.
- **Still open:**
  - OverlayDataModel 0x00652f00 keeps its guess: comment (our STUB pairs at 0.35).
  - CSHA1 ×6 and TCPInterface/PacketizedTCP ×4 have no 2013 body.
  - STUDIO 0x0054b3b0 stays skipped.
