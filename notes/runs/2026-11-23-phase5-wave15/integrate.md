# integrate (workflow phase5-wave15), 2026-11-26

This integrates the three phase5-wave15 Work agents: app-15, shell-5k-6 and network-5f-6. All three finished. Their uncommitted work builds from clean in both targets with 0 failures, so all of it stays and nothing was reverted.

On top of their work I made three changes:
- **A CMake source-order change** for STUDIO (app-15 item 4).
- **Generator name fixes** (shell-5k-6's renames, plus two more that the same analysis turned up).
- **One TEMPLATE nameref** in DebrisService.h.

| | sweep21 | Lost | Gained | **sweep22** |
|---|---|---|---|---|
| PLAYER | 12,200 | **0** | 296 | **12,496** (+296) |
| STUDIO | 11,547 | **0** | 233 | **11,780** (+233) |

Both targets ended above every agent's private figure:

| Agent's private figure | PLAYER | STUDIO |
|---|---|---|
| app-15 | 12,394 | 11,704 |
| shell-5k-6 | 12,420 | 11,734 |
| network-5f-6 | 12,453 | 11,735 |

No 100% was lost against build/sweep21/P-final.json or build-studio/sweep21/S-final.json. The agents' shared losses are all recovered:
- the four DebrisService bind rows;
- AsyncHttpQueue::dispatchGenericCallback;
- STUDIO EnumDesc<UploadSetting>::convertToValue;
- the SoundService vftables.

The Lua/Math/RakNet/WinHttp no-.reloc coincidences the agents saw do not show up on this link.

Housekeeping:
- Every heavy command ran through tools/capped.sh, one build at a time, and nothing hit the cap.
- No commits and no IDA database opened.
- Nothing under Client/Rendering/ was edited.
- The shared reccmp/ was written only by the generator.

## What I did

1. **Saved the agents' tree** (build/sweep22/pre.diff, pre-status.txt, pre-untracked.tgz).
   - `app_sweep set FUNCTION` flipped 572 PLAYER and 564 STUDIO markers.
   - CMakeLists.txt: GameSettings.cpp now links ahead of GameBasicSettings.cpp in STUDIO (below).
2. **Clean builds** (reconfigure, `--target clean`, build). PLAYER build/ took 705 steps and STUDIO build-studio/ took about 690, both with 0 failures.
   - Every new Network .cpp is picked up by the reconfigure, including the untracked ReplicatorStats.cpp. That is the empty-at-configure-time caveat network-5f-6 reported.
   - shell-5k-6 linked nothing privately (it made no CMakeLists change), so there is nothing to add.
3. **`gen_link_stubs.py`, then a relink** (build/sweep22/pipe2.sh).
   - PLAYER: 30 unresolved, 23 stub addresses (20 at sweep21), unknown 10 (13).
   - The three IDA-sync-17 names are now bound to stubs ("exact 3"): `UserInput::PostUserInputMessage` 0x0045f1b0, `CRenderSettings::getResolutionPreset` 0x009c5e10 and `VideoControl::startRecording` 0x00464f30.
   - `FLog::RobloxWndInit` is no longer unresolved: app-15 defined it.
   - No View.cpp address carries a generated stub.
   - STUDIO: 27 unresolved, 20 stubs, unknown 10. STUDIO has no unresolved startRecording reference, so 0x0048caa0 needed no stub.
   - New unresolved data in both targets: `ClassRegistrar<X>::registrar` for ClusterPacketCache, InstancePacketCache, PhysicsPacketCache and Replicator. Network's new code references them and nothing defines them (see Unresolved).
4. **Regeneration rounds** (build/sweep22/rounds.sh: score twice, regenerate from the pair, until a round gains nothing). Entries at effective 100%:
   - PLAYER: 12,191 → 12,328 → 12,434 → 12,468 → 12,480 → 12,488 → 12,489 → 12,491 → 12,492 → 12,492, then (after the 0x0041c8b0 fix) 12,493 → 12,496 → 12,496 (P1–P26).
   - STUDIO: 11,534 → 11,659 → 11,745 → 11,770 → 11,780 → 11,780 (S1–S12). STUDIO 0x0073cfc0 is at 100% from S5 on.
5. **Generator name fixes** (tools/ida/gen_reccmp_csv.py, my file; `--selftest` passes). All are `match:` names corrected in MATCH_FIX or the new MATCH_SUB, with comments in the file.
   - **0x006ee210 → `FactoryProduct<SoundService>::getClassName`** (shell-5k-6). It is the className/getClassName fold. Our /OPT:NOREF link keeps an unreferenced /GL shell copy of className, which took the className name.
     - Result: SoundService vftables 0x00b44908 / 0x00b451d0 are 100 again, and 0x006ee210 is 100 against the engine body 0x00545a80.
   - **0x0073da60, 0x0073db90, 0x0074c040 (assign_functor, found here), 0x00844de0 and 0x00844f60 → the DebrisService `function0` / `basic_vtable0` / `weak_ptr<Instance>` symbols.**
     - shell-5k-6's suggested `basic_vtable1<void,DataModel*>` + `weak_ptr<Instance>` names do not exist in our link, so they could not pair.
     - The 2013 bodies are ICF-folded across Script.cpp's `function1<void,DataModel*>` bind and DebrisService::addItem's bind. At sweep21 our Script.cpp copy folded with our DebrisService copy at 0x004e5d10.
     - Now the /GL shell's GameVerbs.cpp owns the DataModel symbols (ScreenshotVerb::doIt's callees), so the engine body survives in our link only under DebrisService's names.
     - Result: all five are 100. **DebrisService::addItem 0x00845000 rose from 98.98 to 100** (it was a "pairing only" near miss at sweep21) and the sweep promoted it. ScreenshotVerb::doIt 0x00438460 stays 100.
   - **0x0041c8b0 → `~function<void()>`.** It is a /GL shell copy of the folded boost::function destructor.
     - Its only paired caller, AsyncHttpQueue::dispatchGenericCallback 0x0086d480 (engine code), routes to the shell copy in 2013. Ours routes to the shell's ~function<void()> 0x0042a950 (GameVerbs.cpp).
     - Under the old native alias name it paired with the native fold 0x0063ba00, and 0x0086d480 fell to 98.87.
     - The other 13 original callers of 0x0041c8b0 are unpaired, so nothing loses. Result: 0x0086d480 is 100 again.
   - **DebrisService.h TEMPLATE 0x00844f60:** the nameref is now the `function0<void>` / `weak_ptr<Instance>` ctor name our link has. The marker held the address, so the generator row could not take it. This is a comment-only nameref change, and no line moved.
   - wave 13 already added the `__sehtable$` rows (network-5f-6's note on this is stale). CrashReporter's two `__except` functions are 100 in both targets.
6. **Sweep.** `app_sweep apply P26.json S12.json`, then a rebuild (419 PLAYER steps, incremental STUDIO, 0 failures) and rounds PA1–PA4 / SA1–SA4.
   - Result: 12,496 / 11,780, with 0 lost and 0 gained against P26 / S12. It is stable.
   - No demotions against HEAD (build/sweep22/markers.py HEAD).
     - PLAYER: 3 promoted, 14 new FUNCTION and 22 new STUB.
     - STUDIO: 1 promoted, 9 new FUNCTION and 10 new STUB.
   - Against the agents' own marker words (pre.diff applied to HEAD), apply changed one marker: **DebrisService::addItem PLAYER 0x00845000, STUB → FUNCTION** (100 after the name fix). Every FUNCTION the agents wrote is at effective 100% in its target.
   - Shell markers (build/sweep22/shellcheck.py): PLAYER 93 FUNCTION + 17 STUB, STUDIO 58 + 8, with no mismatch. stubcheck finds no STUB outside Rendering at effective 100%.
7. **Reorder: not needed.**
   - `reorder_markers plan` (build/sweep22/reorder-plan-pre.txt) lists no fixable file.
   - The only difference from sweep21's plan is Item.cpp (P 2 → 3 descents, "pinned by assert lines"). network-5f-6 reported that descent, at Item.cpp:71, which the 2013 asserts at lines 52/61/73 pin.
   - decomplint: function_out_of_order 313 → 314 in PLAYER and 317 → 318 in STUDIO. Item.cpp's descent is the only new finding in either module. duplicate_offset is unchanged (12 / 4).

### STUDIO GameSettings.cpp link order (CMakeLists.txt)

- **The change:** `ogre_prior_sources` moves GameSettings.cpp ahead of GameBasicSettings.cpp in App, for `OGRE_TARGET=STUDIO` only. The deciding function and the reason are in a comment there.
- **The 2013 evidence:** STUDIO's shared `EnumDesc<GameSettings::UploadSetting>::convertToValue` 0x0073cfc0 inlines singleton(), so it is GameSettings.cpp's copy. The glob put GameBasicSettings.obj first, so our link kept that copy (0.864).
- **Result:** 0x0073cfc0 is 100 from S5 on.
- **Scope:** PLAYER keeps the glob order. PLAYER's 2013 copy is the out-of-line form, and it was already 100 there.

## Final figures (sweep21 in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| **Entries at effective 100%** | **12,496** (12,200) | **11,780** (11,547) |
| Non-library entries | 9,792 (9,500) | 9,200 (8,957) |
| **Functions at 100%** | **9,472** (9,228) | **8,979** (8,792) |
| **Non-library functions at 100%** | **6,835** (6,595) | **6,446** (6,249) |
| Functions at 100%: owner / Rendering (minus g3d) / third party | 6,823 / 2 / 2,647 (6,586 / 2 / 2,640) | 6,435 / 3 / 2,541 (6,238 / 3 / 2,551) |
| Vftables at 100%: owner / Rendering / third party | 2,941 / 1 / 12 (2,888 / 1 / 13) | 2,742 / 1 / 8 (2,696 / 1 / 8) |
| Vftables at 100% / paired | 2,954 / 3,793 (2,902 / 3,736) | 2,751 / 3,723 (2,705 / 3,666) |
| reccmp Implemented, with libraries | 10,926 / 40,550 (10,721) | 10,509 / 42,384 (10,333) |
| reccmp Implemented, `--nolib` | 8,055 / 37,640 (7,856 / 37,646) | 7,797 / 39,556 (7,612 / 39,546) |
| App markers: FUNCTION + FOLDED + STUB | 1,261 + 28 + 510 (1,257 + 28 + 506) | 1,214 + 27 + 517 (1,211 + 27 + 512) |
| Base markers | 93 + 0 + 19 (93 + 0 + 19) | 87 + 0 + 20 (87 + 0 + 20) |
| Network markers | 139 + 2 + 37 (126 + 2 + 22) | 124 + 1 + 24 (117 + 1 + 20) |
| Shell markers (FUNCTION + STUB) | 93 + 17 (90 + 20) | 58 + 8 (58 + 8) |

- **Split:** the owner / third-party split comes from the .obj in the link map (build/sweep22/split.py).
- **Third party, one vftable fewer (PLAYER) and ten fewer functions (STUDIO):** this is reclassification by the .obj our link now keeps, not a loss. summ.py finds 0 lost by original address.
- **Rendering column:** only the two or three pre-existing Rendering bodies sit there, as before.

**Rows** (PLAYER, tools/row_coverage.py on P-final; per-function CSVs in build/sweep22/row-*.csv):

| Row | At 100% / total | Share | sweep21 |
|---|---|---|---|
| 5a | 139/155 | 89.7% | 139 |
| 5b | 1,126/2,979 | 37.8% | 1,109 |
| 5c | 105/158 | 66.5% | 105 |
| 5d | 475/1,554 | 30.6% | 475 |
| 5e | 513/1,335 | 38.4% | 504 |
| 5f | 827/3,306 | 25.0% | 748/3,294 |
| 5g (row_coverage_5g.py) | 332/612 | 54.2% | 331 |
| 5h | 544/1,359 | 40.0% | 534 |
| 5k | 1,048/3,493 | 30.0% | 1,037 |

- **5f:** network-5f-6 reported 818/3,306 on its private link; the integrated, pooled regeneration gives 827 (row-5f.csv lists which functions; I did not attribute the extra 9 one by one).
- **5b, 5e and 5h:** app-15's Event.h/property.h line and cast fixes, plus Selection and GlobalAdvancedSettings.
- **5k:** +11: shell-5k-6's ScreenshotVerb::doIt, calcDefaultResolution, the CRenderSettingsItem ctor and their bind copies.

**Near misses** (app_sweep report, build/sweep22/app-sweep.csv; markers not at 100%):

| Target | Short | ebp-frame | inlining-size | other | unpaired | unpaired-callee | x87-vs-sse | assert-file-line | data-ref | stack-cookie | At 90% or more | Pairing-only |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| PLAYER | 578 | 183 | 183 | 126 | 48 | 11 | 20 | 4 | 3 | 0 | 52 | 14 |
| STUDIO | 570 | 175 | 170 | 132 | 37 | 28 | 20 | 4 | 3 | 1 | 78 | 31 |

"Short" grew by 19 / 9 because the agents added new STUB markers (pairing targets for the Replicator, the packet caches, Selection and so on). No FUNCTION was demoted.

The closest PLAYER near misses are unchanged from sweep21, except that DebrisService::addItem and createAutoJoints have left the list:

| Function | Address | Score | Shape |
|---|---|---|---|
| TaskScheduler::remove | 0x009b52b0 | 99.49 | pairing only |
| ScriptContext::setTimeout | 0x0067eee0 | 99.41 | pairing only |
| TextXmlWriter::serializeNode | 0x00628a60 | 99.39 | other |
| TaskScheduler::setThreadCount | 0x009b7ec0 | 99.26 | pairing only |
| World::destroyAutoJoints | 0x00749340 | 99.23 | other |
| TaskScheduler::getJobsInfo | 0x009b75f0 | 99.21 | pairing only |
| GuiService::openBrowserWindow | 0x0069a540 | 99.08 | pairing only |
| TaskScheduler::getJobsByName | 0x009b78f0 | 98.99 | pairing only |
| InsertService::insert | 0x007d6a20 | 98.90 | pairing only |
| TaskScheduler ctor | 0x009b55e0 | 98.88 | other |
| DataModel::onlyJobsLeftForThisArbiterAreGenericJobs | 0x0063a120 | 98.68 | other |
| Peer::~Peer | 0x005b8bf0 | 98.68 | pairing only (the `ret` twin) |
| TaskScheduler::setJobsExtendedStatsWindow | 0x009b7760 | 98.55 | pairing only |
| Joint::Joint | 0x00821410 | 98.47 | pairing only |
| parseContent | 0x00665810 | 98.39 | other |

## Checks

| Check | PLAYER (sweep21) | STUDIO (sweep21) |
|---|---|---|
| decomplint: function_out_of_order | 314 (313) | 318 (317) |
| decomplint: duplicate_offset | 12 (12) | 4 (4) |
| reccmp-vtable: traceback | none | none |
| reccmp-vtable: "Problem with adjuster thunk" lines | 13 (13) | 15 (15) |
| reccmp-datacmp: traceback | none | none |
| reccmp-datacmp: variables | 3,124 (3,061) | 2,991 (2,948) |
| reccmp-datacmp: issues | 57 (55) | 59 (58) |
| check_folded: annotated addresses | 2,967 (2,928) | 2,725 (2,706) |
| check_folded: folding problems | 0 | 0 |
| check_folded: "Debug data out of sync" lines | 19 (19) | 19 (19) |
| reccmp_addr_padding | 1,696 files, 0 bad | |
| check_2016_copies | silent | |

- **decomplint** exits 1 under `--warnfail`, as in every sweep. The one new finding per module is Item.cpp's assert-pinned descent.
- **datacmp, PLAYER (+2 issues):** two more `stored_vtable` DIFFs. These are boost::function stored vtables that newly pair through the Network code, and the "unknown pointer" into .text is the no-.reloc limit. A `parser_error_base` catchable-type WARN replaces an `__ehfuncinfo` WARN.
- **datacmp, STUDIO:** +1, of the same kind.

## Saved

- **Final JSONs:**
  - build/sweep22/P-final.json (= PA4)
  - build-studio/sweep22/S-final.json (= SA4)
- **Scored exe, pdb, map and pdbview:** build/sweep22/scored/ and build-studio/sweep22/scored/.
- **Alongside the finals:**
  - every round's JSON and .txt (P1–P26, PA1–4; S1–S12, SA1–4), gen logs, and rounds-*.log;
  - gen-votes.log (OGRE_VOTES=41c8b0);
  - P/S-nolib.json and .txt, -vtable.txt, -datacmp.txt and -folded.txt;
  - {PLAYER,STUDIO}-decomplint.txt (+ .norm), app-sweep.csv, row-*.csv, reorder-plan-pre.txt, stubs.log and PA4-vs-s21.txt;
  - build logs (build1, build2, build-PA/SA).
- **Helper scripts** in build/sweep22/: the sweep21 set with paths moved to sweep22, plus `pipe2.sh` (stubs, relink and rounds for both targets).

## Files changed by me

- **CMakeLists.txt:** STUDIO-only GameSettings.cpp-before-GameBasicSettings.cpp ordering in `ogre_prior_sources`.
- **tools/ida/gen_reccmp_csv.py:**
  - MATCH_FIX for PLAYER 0x006ee210 and 0x0041c8b0;
  - the new MATCH_SUB (the five DebrisService bind rows) and its application in the names.csv loop;
  - the docstring updated to match.
- **reccmp/:** regenerated player-* and studio-* (libdata, symbols, studio-stl).
- **Client/_generated/{player,studio}/** stubs_00.cpp and unresolved.txt, through gen_link_stubs.
- **Client/App/include/V8DataModel/DebrisService.h:** the TEMPLATE PLAYER 0x00844f60 nameref (comment only, no line moved).
- **Marker words** through app_sweep (DebrisService::addItem promoted). No body changed and no line moved.
- No change to tools/app_sweep.py or tools/reorder_markers.py.

## Unresolved

- **IDA owner:** carry the generator name fixes into ida/PLAYER.i64 so the exports agree:
  - 0x006ee210 getClassName;
  - 0x0073da60, 0x0073db90, 0x0074c040, 0x00844de0 and 0x00844f60: the 2013 names are folds. The Script.cpp DataModel names are as true as the DebrisService names, but only the latter exist in our link;
  - 0x0041c8b0 ~function<void()>.
  - Until then the generator's MATCH_FIX / MATCH_SUB keep them. Drop each entry once names.csv carries it.
  - Also app-15's list of renames: Selection ctor 0x0086b070, DebugSettings::gfxcard / videoMemory, FLog::RobloxWndInit, DFFlag::UseClusterCache, advancedSettingsFile, and the STUDIO GlobalAdvancedSettings ctor 0x00740d10.
- **Network:**
  - **`ClassRegistrar<ClusterPacketCache / InstancePacketCache / PhysicsPacketCache / Replicator>::registrar` are unresolved data in both links.** Each needs its registration (the `RBX_REGISTER_CLASS`-style static) in the class's .cpp, at the 2013 position.
  - Server::onServiceProvider's 8-byte frame (88.99), the STUDIO Replicator layout, and Players::onChildRemoving/onChildAdded (blocked on the FriendService Creator stand-in): unchanged from network-5f-6.
- **Shell:**
  - The FriendService `className()` / Creator stand-in from a TU whose first Object.h include is spelled `util/object.h` (app-15, network-5f-6). That is the key to Players.cpp including FriendService.h.
  - The ScreenshotVerb ctor's 8-byte frame (75.9), initializeSizes (74.2), and the rest of View.cpp / UserInput / WndProc.
- **App:**
  - EventDescImpl fireEvent by-value Variant fetch;
  - Settings::verifyAddDescendant (InvalidDescendentDetector);
  - the FriendService layout;
  - the Game ctor 0x00655e70;
  - openState;
  - the Event.h sendEvent/getSignalPtr assert lines (187 against 159, 1108 against 797);
  - NetworkSettings +0x100 for Replicator::getAdjustedMtuSize / getPhysicsMtuSize (network-5f-6).
- **Rendering contributor:** FrameRateManager::ResumeAutoAdjustment and ViewBase's frame-data callback (shell-5k-6).
- **Generator twins:** the 0x0041c8b0 fix is a hand entry. A general rule ("a shell original pairs with our shell twin when its paired callers route there") would cover the next case automatically. Not written.
- **gen_link_stubs unknowns:** PLAYER 10 and STUDIO 10, the CSHA1 ×6 and TCPInterface / PacketizedTCP ×4, as before.
- **prime_list constructors:** still FUNCTION through no-.reloc layout luck. Nothing flipped this wave.
