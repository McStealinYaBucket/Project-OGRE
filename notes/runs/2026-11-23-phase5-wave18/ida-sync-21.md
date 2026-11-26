# ida-sync-21 (workflow phase5-wave18)

The wave 18 IDA sync is done. ida/PLAYER.i64 and ida/STUDIO.i64 are saved packed and closed, with no .id0/.nam/.til left behind.
- Every IDA run was headless, one process per database, one at a time, through tools/capped.sh at 8G. None hit the cap.
- I did not touch ida/Archive.zip, Client/, gen_reccmp_csv.py or the shared reccmp/ CSVs, and committed nothing. check_2016_copies is silent.

## 1. Snapshots

ida/snapshots/PLAYER-27-wave18-sync.i64 and STUDIO-27-wave18-sync.i64 were taken before any change; both `cmp`-identical to the canonical databases as they were then. All three apply passes of this session came after them.

## 2. Names from the scored runs

Inputs: build/sweep25/P-final.json and build-studio/sweep25/S-final.json, each joined only with the build it scored (build*/sweep25/scored/).

- **Functions at effective 100%:** PLAYER 9,926, STUDIO 9,346 (sweep24: 9,771 / 9,226). Library-owned rows (1,747 / 1,648) stay with apply_lib_names.py.
- **PLAYER renamed:** 138 match: (including the shutdownDM fact), 12 rtti: (9 vftable slots, 3 facet facts), 10 str:, 3 ios:, 3 src2016:, 3 prior:, plus 36 data names.
- **STUDIO renamed:** 127 match: (15 through the PLAYER body match, 3 of them undone, see below), 3 rtti:, 10 str:, 2 ios:, 2 src2016:, plus 36 data names. FUNC_LIB was cleared on 1 (0x009eda40, the FLIRT `fpos` ctor, now `Lua::YieldingThreads` ctor).
- **New match: names (wave 18's code):** Game::doClearVerbs / clearVerbs / setDataModel, the SecurePlayerGame ctor, the signal<void(DataModel*)> ctor / dtor / disconnectAll / safe_static_init_mutex (the Game+0x30 member that wave 17 left as the Game::Unk0x30 guess), the LuaAllocator ctor and dtor, ContentProvider::getBaseUrl, LegacyLock::Implementation::safe_static_init_currentJob (0x0063a370 / 0x00758fc0, which were IDA `j_` names), View::HandleWindowsMessage, and the UserInputSleepingJob safe_queue::push / deque::push_back (0x00460ee0 / 0x00461700).
- **Swap:** 0x0066fb00 (prior: openScriptLibrary) is now `RBX::FUN_0066fb00`; 0x0066fb80 is `RBX::openScriptLibrary`. Both are reccmp 100%.
- **Conflicts:** PLAYER 131, STUDIO 127. New ones: the folded copyValue / equalValues ContentId and TextureId vs SoundId oracle spellings, panic, duplicateIndexRange vs ScriptContext::load. All are folded-original or same-function, so the oracle name stays.
- **Post-apply rerun:** no rename pending except STUDIO 0x0054b3b0 (as before). Selftest passes.

**Facts** (tools/ida/match_names.py), each checked from the exe bytes:

| PLAYER | STUDIO | Name | Tag | Evidence |
|---|---|---|---|---|
| 0x00653600 | (PLAYER only) | `shutdownDM<DataModel>` | match: | reccmp 100% vs a body of ours that folds both spellings, as 2013 does; ~Game calls it twice, the DataModel first (app-18). The bulk rule would have taken OverlayDataModel. |
| 0x00654400 | (none) | `Game::shutdownGameDataModel()` | ios: | only caller View::resetScheduler at 0x00466ed9; our STUB pairs at 0.76 |
| 0x00654dc0 | (none) | `signal_with_args<1,void(DataModel*)>::operator()` | prior: | only caller setDataModel (100%) |
| 0x004265c0 | 0x007cb980 | `signal<void(DataModel*)>::safe_static_do_get_mutex` | prior: | reached from init_mutex, the ctor and disconnectAll (all 100%), which push its once flag; STUDIO is 100% |
| 0x0041cf40 | 0x00461ce0 | shell `Impersonator(Identities)` | ios: | called by doClearVerbs (100%) and 8 more |
| 0x0041cf80 | 0x004619a0 | shell `~Impersonator()` | src2016: | 15 callers, doClearVerbs among them |
| 0x00526e10 | (PLAYER only) | `Network::initWithPlayerSecurity()` | ios: | SecurePlayerGame's ctor (100%) pushes it, with flag 0x00de68b4, to call_once at 0x00656283 |
| 0x0072b220 | 0x0082a210 | `SleepingJob::wake()` | src2016: | after SleepingJob::error; spin_mutex at +0x218; only caller PostUserInputMessage. In STUDIO: same position, instructions equal up to addresses. Decoration from unresolved.txt, so gen_link_stubs can bind it. |
| 0x0045e300 | (none) | `UserInput(HWND, View*, shared_ptr<Game>)` | prior: | only caller View::initializeInput after `new` |
| 0x0045e630 | (none) | `UserInput::setGame(shared_ptr<Game>)` | src2016: | only caller the ctor |
| 0x004627b0 | (none) | UserInputSleepingJob ctor (comment only) | rtti: | stores vftable 0x00be4e18 after the SleepingJob base ctor; its parameters are unproven, so no rename |
| 0x00477aa0 / 0x00477c30 / 0x00477dd0 | (none) | program_options `utf8_codecvt_facet::do_in / do_out / do_length` | rtti: | slots 4/5/7 of the RTTI-named vftable 0x00be64d0. 2013 links no filesystem facet; the filesystem names came from our link's identical copy. |
| (none) | 0x004337a0 | `ContentId(const std::string&)` | ios: | STUDIO's /GS shell copy: same callees as PLAYER's 0x004c4780 (getNullName, CorrectBackslash); 59 callers |
| 0x00e24fac / 0x00de58b8 | 0x00f8930c / 0x00f84b4c | the signal mutex and LegacyLock currentJob once_flags (data8) | prior: | each pushed with its init function to call_once |
| 0x00e0c968 / 0x00e0c7ac | 0x00f8330c / 0x00f8314c | `ClassRegistrar<ControllerService>` / `<FriendService>::registrar` (data4) | prior: | the `inc dword` operand in their Described ctors; our GLOBAL markers |

- **Lua Bridges (new `BRIDGES` table, str:).** Each 2013 `registerClassLibrary` is `mov eax,[className]; push esi; mov esi,[esp+8]; push classLibrary; ...`. The className global it reads names the function, its luaL_Reg array and the global.
  - 2013 order: Color3, Ray, Region3, Region3int16, Vector3 (the only array with FromNormalId / FromAxis), Vector3int16, Vector2int16, Vector2, BrickColor, CFrame, UDim, UDim2, Faces, Axes, CellId, InputObject.
  - Renamed: 10 register functions per target (PLAYER 0x008819c0–0x00881f90, STUDIO 0x0092ba90–0x0092c030). The other six already had their names and keep match:, with the check added to the comment.
  - Data names: 32 per target (the 16 classLibrary arrays and the 16 className globals).
  - This is the cause of integrate's wrong classLibrary pairing: reccmp's position pairing had put 10 PLAYER functions under a neighbour's name.
- **Removed facts:** the wave-17 Game::Unk0x30 guess comments, and the STUDIO 0x007cc250 unname, which would have overridden the 100% signal ctor name.
- **Undone carries:** three PLAYER→STUDIO carries (the deque / safe_queue / _Allocate of `UserInputSleepingJob::Message`, STUDIO 0x0041a490 / 0x0048a260 / 0x00501f30) named a class STUDIO does not have. guess: facts unname them again.

Tool change: in match_names.py the STUDIO carry now skips a PLAYER name that spells a class whose RTTI PLAYER has and STUDIO lacks.

**Carried-over items now closed:**
- MemberDescriptorContainer compare 0x00614cd0, singleton 0x00407230 and createInstance 0x00409020 already carry match: names.
- STUDIO 0x004337a0 and the three program_options facet slots are named (above).
- STUDIO-PLAYER-bodymatch.csv now has isUpperRoot (section 4).

## 3. Types

import_types.py, dumped from the scored PDBs. Every size equals the tree's // SIZE.

| Class | Size | Proof (PLAYER / STUDIO) |
|---|---|---|
| RBX::LuaAllocator | 0x24 | ctor 0x0086b300 / 0x009ad320, dtor 0x0086b240 / 0x009ad260, vftable 0x00b7f5f8 / 0x00c52fa8 |
| RBX::Network::PacketReceiveJob | 0x230 | ctor 0x005b92c0 (STUDIO 0.986), dtor 0x005b94b0 / 0x0064b220 |
| RBX::RobloxCriticalSection (PLAYER only) | 0x54 | shell ctor 0x0045e190 |

- **Changed members from the tree:**
  - Game +0x30 is now `signal<void(DataModel*)>`.
  - Player +0x168 is killPlayerSignal.
  - Players +0x1b4 / +0x1b8 are testPlayerNameId / testPlayerUserId.
  - STUDIO Replicator +0x128c is removingInstance.
- **GuiBuilder:** the STUDIO PDB now has a single 0x1c GuiBuilder (the PDB is deleted before each link), so fix_2013's GuiBuilder case no longer fires.
- **Counts:**
  - Local types: PLAYER 372 → 375 (140 listed), STUDIO 356 → 358 (132).
  - `this` typed: PLAYER 565 → 579 (349 ctor/dtor), STUDIO 526 → 535 (322).
  - No size mismatch and no type removed.
- **Not imported:**
  - STUDIO Game (ctor 0.9965, dtor 0.9907);
  - Application (0x140; ctor 0.505, dtor 0.559);
  - UserInput (ctor 0.427);
  - ConcurrentRakPeer::StatsUpdateJob (ctor 0.462, dtor 0.127);
  - SecurePlayerGame, UnsecuredStudioGame, LegacyLock::Implementation and Lua::YieldingThreads (ctor or dtor at 100%, but no size annotation).

## 4. STUDIO evidence and body match

- **Evidence:** export_evidence.py on STUDIO (read-only). The old export had 44,814 functions and lacked isUpperRoot.
- **Body match:** body_match.py STUDIO vs PLAYER gives 20,129 accepted pairs (was 20,115). New: 0x00a0e170 ↔ 0x008f90b0 (isUpperRoot). Lost: 0x007d82a0 ↔ 0x0066ffd0 (now ambiguous; both unnamed). Replanning with it changed no rename.
- **pair_names.py (STUDIO): rerun, then reverted.** Its gates now see the match: names in the evidence, so it drops 452 proposal rows and adds 61. Committing that would move match_names' conflict evidence, so the committed proposals stay until those rows are reviewed. The new file is kept at /tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/STUDIO-names-proposed.new.csv.

## 5. Unnamed functions (PLAYER-app-unnamed-by-class.csv regenerated)

| Block | Functions | ios:/mac:/match:, before → after | Still without one |
|---|---|---|---|
| App (0x00606a00–0x009ad3a0) | 21,631 | 8,774 → 8,844 | 12,787 (59.1%) |
| Network (0x0051f6b0–0x00606a00) | 5,336 | 2,311 → 2,351 | 2,985 (55.9%) |
| Shell (0x00401000–0x0051f6b0) | 5,761 | 1,877 → 1,905 | 3,856 (66.9%) |

Largest App groups:

| Group | Functions |
|---|---|
| No caller with a known class | 3,991 (586 KB) |
| Ogre::RbxSceneManager (vftable slots) | 306 |
| FactoryProduct<>::Creator | 212 |
| EnumDesc<> | 178 |
| DataModel | 160 |
| RBX free functions | 151 |
| ScriptContext | 116 (was 191) |
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

Largest Network groups:

| Group | Functions |
|---|---|
| Replicator | 343 |
| Players | 129 |
| ServerReplicator | 116 |
| Player | 111 |
| FactoryProduct<>::Creator | 48 |
| Replicator::StreamJob | 47 |
| InterpolatingPhysicsReceiver | 39 |
| ErrorCompPhysicsSender | 38 |
| PhysicsPacketCache | 37 |

## 6. Coverage, before → after

Functions with a non-dummy name. Function starts are unchanged (49,091 / 44,815). Types export: PLAYER 806 declarations (was 803), STUDIO 697 (was 695).

| | PLAYER | STUDIO |
|---|---|---|
| Named | 15,435 → 15,601 | 15,939 → 16,077 |
| match: | 7,545 → 7,681 | 7,180 → 7,304 |
| ios: | 6,234 → 6,237 | 6,293 → 6,295 |
| rtti: | 170 → 181 | 165 → 168 |
| src2016: | 21 → 24 | 20 → 22 |
| str: | 8 → 18 | 7 → 17 |
| prior: | 14 → 16 | 5 → 4 |
| flirt: | 97 | 171 → 170 |
| ida: | 1,075 → 1,076 | 1,804 → 1,803 |
| Data names | 40,421 → 40,457 | 41,788 → 41,824 |

## Files changed
- ida/PLAYER.i64, ida/STUDIO.i64; ida/snapshots/{PLAYER,STUDIO}-27-wave18-sync.i64
- ida/LOG.md
- tools/ida/match_names.py: about 25 new facts and the BRIDGES table; the wave-17 Unk0x30 facts are removed; the STUDIO carry has an RTTI guard.
- tools/ida/import_types.py: LuaAllocator, PacketReceiveJob and RobloxCriticalSection (PLAYER_ONLY) added.
- ida/exports/{PLAYER,STUDIO}-names.csv, -functions.csv, -types.h, -proven-types.json, -db-updates.csv, -match-conflicts.csv; STUDIO-PLAYER-bodymatch.csv; PLAYER-app-unnamed-by-class.csv; STUDIO-evidence/ (gitignored)
- ida/logs/{PLAYER,STUDIO}-27-*.log / .tsv

## Unresolved
- **Integrator (gen_reccmp_csv.py):** regenerate the CSVs from the new names.csv.
  - The 16 Bridge register functions, classLibrary arrays and className globals now carry their 2013 identity, which should clear the wrong classLibrary datacmp rows and the 0.94 register pairings.
  - shutdownDM 0x00653600 now carries the DataModel spelling.
- **App:** the 2013 Bridge registerClassLibrary order (above) differs from LuaAtomicClasses.cpp.
- **App:** `Game::m_unk0x30` is `signal<void(DataModel*)>` in the PDB; rename the member.
- **pair_names.py:** regenerating the STUDIO proposals against today's evidence drops 452 rows and adds 61. Review those before committing (the file is kept in the scratchpad).
- **Still open:**
  - The UserInputSleepingJob ctor 0x004627b0 has no decoration (its parameters are unproven).
  - 0x009b7020 is still a dummy.
  - OverlayDataModel 0x00652f00 has a guess: comment only.
  - CSHA1 x6 and TCPInterface/PacketizedTCP x4 have no 2013 body.
