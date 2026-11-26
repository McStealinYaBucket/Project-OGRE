# integrate (workflow phase5-wave17), 2026-11-26

This integrates the three phase5-wave17 Work agents: app-17, shell-5k-8 and network-5f-8. All three finished. Their uncommitted work builds from clean in both targets with 0 failures, so all of it stays and nothing was reverted.

| | sweep23 | Lost | Gained | **sweep24** |
|---|---|---|---|---|
| PLAYER | 12,597 | 0 | 169 | **12,766** (+169) |
| STUDIO | 11,880 | 0 | 134 | **12,014** (+134) |

Both targets ended above every agent's private figure (app-17 12,727 / 11,977; shell-5k-8 12,698 PLAYER; network-5f-8 12,744 / 11,999). **No 100% is lost against sweep23 in either target.** The PLAYER surplus over network-5f-8 comes mostly from the shell agent's `_internal_accept_owner` names, which only the generator could apply (below).

Housekeeping: every heavy command ran through tools/capped.sh, one build at a time, no cap hits. No commits, no IDA database opened, nothing under Client/Rendering/ edited. The shared reccmp/ was written only by the generator.

## What I did

1. **Saved the agents' tree** to build/sweep24/: pre.diff, pre-status.txt, pre-untracked.tgz.
2. **CMakeLists.txt:** dropped `Client/WindowsClient/placeholder.cpp` from `player_client` (comment-only now; WinMain is in main.cpp) and deleted the file (unstaged deletion). No other private source needed adding: Game.cpp and Replicator.ItemSender.cpp come in through the App/Network globs.
3. **Marker pair for the sleepTime fold** (network-5f-8's request): ConcurrentRakPeer.cpp's `StatsUpdateJob::sleepTime` now carries `FUNCTION: PLAYER 0x008ef650 FOLDED` / `STUDIO 0x00a0d3d0 FOLDED`, and ContentProviderJob.cpp's two markers say FOLDED. The three comment lines it replaced kept the file's line count. check_folded: 0 problems.
4. **`app_sweep set FUNCTION`** (594 PLAYER, 593 STUDIO), then clean builds (reconfigure, `--target clean`, build): PLAYER 712 steps, STUDIO 696, 0 failures.
5. **`gen_link_stubs.py` + relink:** PLAYER 26 unresolved, 24 stubs, unknown 10; STUDIO 24 unresolved, 20 stubs, unknown 10.
   - **`FrameRateManager::ResumeAutoAdjustment` 0x009c7ac0 is now bound** (names.csv carries it), so it left the unknown list.
   - **New unresolved data in both targets:** `ClassRegistrar<ControllerService>::registrar` (app-17's new ControllerService has no registrar definition; see Unresolved).
6. **Generator (tools/ida/gen_reccmp_csv.py, `--selftest` ok):**
   - **New MATCH_FIX PLAYER 0x005bd080 → `_internal_accept_owner<NetworkOwnerJob>` (the Job types' shell/shell body), and PLAYER 0x005baa50 → the YieldFunctionStateObject name** (shell-5k-8's names.csv fix, done in the generator since names.csv is the IDA owner's). names.csv has no row for 0x005baa50, so a MATCH_FIX entry whose address has no names.csv row now adds one (docstring updated).
   - Result in the final run: 0x005bd080, 0x005baa50, 0x006678b0, 0x006684b0 and the five `sp_pointer_construct<Job>` copies (0x005b1970, 0x005b8e10, 0x00633950, 0x0072bc60, 0x0072bcd0) are all at 100. These were sweep23's five PLAYER losses against sweep22.
   - **Dropped the wave-16 FriendService MATCH_FIX entries** (PLAYER 0x006aaf50 / 0x006aaf40, STUDIO 0x00809d30 / 0x00809d20). Proof: regenerating both targets from P15+P16 / S15+S16 with and without them gives byte-identical reccmp/ CSVs (build/sweep24/csv-with/). names.csv now names them `DescribedCreatable<FriendService>`, which our link folds with the FactoryProduct base D0, so the folded PUBLICS name pairs either way.
   - MATCH_DROP (STUDIO 0x0042b260 / 0x0042e370) stays: the STUDIO shell still has no GameVerbs.cpp.
7. **Regeneration rounds** (score twice, regenerate from the pair, until a round gains nothing), entries at effective 100%:
   - PLAYER 12,588 → 12,661 → 12,705 → 12,738 → 12,756 → 12,762 → 12,766 → 12,766 (P1–P16).
   - STUDIO 11,870 → 11,928 → 11,964 → 11,985 → 11,996 → 12,004 → 12,004 (S1–S16).
8. **Sweep:** `app_sweep apply P16 S16`. Against HEAD (markers.py): PLAYER 9 promoted, 18 new FUNCTION, 33 new STUB, 1 new FOLDED; STUDIO 10 promoted, 18 new FUNCTION, 34 new STUB, 1 new FOLDED. No demotion.
9. **Reorder:** `reorder_markers plan` differs from sweep23's in one file: GuiBuilder.cpp, P 3→2 / S 3→2 (app-17's GuiBuilder edits opened one run). Applied to that file only, which moved addCustomStat, removeCustomStat and restyleStatRow below the build* functions; no assert line moved. Network's Item.cpp (3, pinned), Player.cpp (1, cross-run) and Players.cpp (1, studio-only chunk) descents were already there at sweep23.
10. **Rebuild + stability rounds** PC1–4 / SC1–4: PLAYER 12,766 every run; STUDIO **12,014** every run (+10 over S16). The +10 STUDIO are the no-.reloc coincidences of sweep23 going away with the new layout: `IsOlderOrderedPacket`, `isDenormal`, `isNanInfDenormVector3`, `httpGetPostWinHttp`, `constructor`, `luaK_setreturns`, `luaK_setoneret`, `luaK_nil`, `exp2reg`, `luaK_posfix`.
11. **Re-sweep of the STUDIO no-.reloc demotions:** `app_sweep apply PC4 SC4` promoted exactly those three (Math.cpp 0x00898bc0, 0x00898cc0; WinHttp.cpp 0x0091cd80) back to FUNCTION. Marker words only, so no rebuild; final rounds PD1–4 / SD1–4 gave 12,766 / 12,014 in every run (stable).

## Losses explained

None against build/sweep23/P-final.json or build-studio/sweep23/S-final.json in the final runs.

Intermediate only: STUDIO `exp2reg` 0x00a5e430 (lib, 99.02) was lost in S1–S16 (`or ebp,0x800002` read as one of our data addresses, `--verbose`), the same coincidence network-5f-8 reported. It came back with the post-reorder link.

## Final figures (sweep23 in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| **Entries at effective 100%** | **12,766** (12,597) | **12,014** (11,880) |
| Non-library entries | 10,017 (9,888) | 9,412 (9,300) |
| **Functions at 100%** | **9,701** (9,545) | **9,176** (9,051) |
| **Non-library functions at 100%** | **7,019** (6,903) | **6,621** (6,518) |
| Functions at 100%: owner / Rendering (minus g3d) / third party | 7,003 / 2 / 2,696 (6,893 / 2 / 2,650) | 6,611 / 3 / 2,562 (6,508 / 3 / 2,540) |
| Vftables at 100%: owner / Rendering / third party | 2,983 / 1 / 11 (2,968 / 1 / 13) | 2,779 / 1 / 8 (2,771 / 1 / 7) |
| Vftables at 100% / paired | 2,995 / 3,826 (2,982 / 3,806) | 2,788 / 3,754 (2,779 / 3,737) |
| reccmp Implemented, with libraries | 11,155 / 40,551 (10,998 / 40,550) | 10,720 / 42,384 (10,592 / 42,384) |
| reccmp Implemented, `--nolib` | 8,240 / 37,598 (8,121 / 37,634) | 7,985 / 39,535 (7,869 / 39,546) |
| App markers: FUNCTION + FOLDED + STUB | 1,272 + 29 + 520 (1,267 + 28 + 511) | 1,226 + 28 + 526 (1,217 + 27 + 521) |
| Base markers | 93 + 0 + 19 (93 + 0 + 19) | 87 + 0 + 20 (87 + 0 + 20) |
| Network markers | 175 + 3 + 50 (154 + 2 + 35) | 162 + 2 + 41 (141 + 1 + 25) |
| Shell markers (FUNCTION + STUB) | 98 + 28 (95 + 23) | 58 + 8 (58 + 8) |

The owner / third-party split is by the .obj in the link map (build/sweep24/split.py). PLAYER's third-party vftables went 13 → 11 although no vftable at 100% was lost, so two moved sides by the .obj the map now puts at their recompiled address (none is a library row).

Marker checks on the final runs: shellcheck finds no shell FUNCTION below 100 and no STUB at 100; stubcheck finds no STUB outside Rendering at effective 100%.

**Rows** (PLAYER, tools/row_coverage.py on PC4 = final link; per-function CSVs in build/sweep24/row-*.csv):

| Row | At 100% / total | Share | sweep23 |
|---|---|---|---|
| 5a | 139/155 | 89.7% | 139/155 |
| 5b | 1,243/3,305 | 37.6% | 1,130/2,979 |
| 5c | 105/158 | 66.5% | 105/158 |
| 5d | 475/1,554 | 30.6% | 475/1,554 |
| 5e | 515/1,335 | 38.6% | 515/1,335 |
| 5f | 906/3,327 | 27.2% | 858/3,319 |
| 5g (row_coverage_5g.py) | 332/612 | 54.2% | 332/612 |
| 5h | 548/1,359 | 40.3% | 545/1,359 |
| 5k | 1,088/3,493 | 31.1% | 1,054/3,493 |

5b's total grew by 326 with app-17's new TUs and declarations (Game, ControllerService, the DataModel group); 5f +48 is network-5f-8 plus the five sp_pointer_construct copies; 5k +34 is the shell's AboutToShutdown/OnGetMinMaxInfo, the accept_owner fold and the stand-ins.

**Near misses** (app_sweep report, build/sweep24/app-sweep.csv; markers not at 100%):

| Target | Not at 100% | ebp-frame | inlining-size | other | unpaired | unpaired-callee | x87-vs-sse | assert-file-line | data-ref | stack-cookie | At 90% or more |
|---|---|---|---|---|---|---|---|---|---|---|---|
| PLAYER | 601 | 195 | 189 | 130 | 48 | 11 | 21 | 4 | 3 | 0 | 53 |
| STUDIO | 596 | 186 | 182 | 132 | 37 | 30 | 21 | 4 | 3 | 1 | 79 |

The closest PLAYER near misses are unchanged: TaskScheduler::remove 0x009b52b0 99.49 (pairing only), ScriptContext::setTimeout 0x0067eee0 99.41, TextXmlWriter::serializeNode 0x00628a60 99.39, TaskScheduler::setThreadCount 0x009b7ec0 99.26, World::destroyAutoJoints 0x00749340 99.23, getJobsInfo 0x009b75f0 99.21, GuiService::openBrowserWindow 0x0069a540 99.08, getJobsByName 0x009b78f0 98.99, InsertService::insert 0x007d6a20 98.90, the TaskScheduler ctor 0x009b55e0 98.88; new near the top: DataModel::onlyJobsLeftForThisArbiterAreGenericJobs 0x0063a120 98.68 and Peer::~Peer 0x005b8bf0 98.68. STUDIO Game::Game 0x007cddd0 is 99.65 (twin pairing, app-17). Shell: View::HandleWindowsMessage 95.4, WndProc 76.4, WinMain 52.7 (all STUB).

## Checks

| Check | PLAYER (sweep23) | STUDIO (sweep23) |
|---|---|---|
| decomplint: function_out_of_order | 314 (315) | 318 (319) |
| decomplint: duplicate_offset | 12 (12) | 4 (4) |
| reccmp-vtable: traceback | none | none |
| reccmp-vtable: "Problem with adjuster thunk" lines | 13 (13) | 15 (15) |
| reccmp-datacmp: traceback | none | none |
| reccmp-datacmp: variables | 3,162 (3,141) | 3,031 (3,014) |
| reccmp-datacmp: issues | 59 (57) | 61 (59) |
| check_folded: annotated addresses | 3,058 (2,997) | 2,804 (2,752) |
| check_folded: folding problems | 0 | 0 |
| check_folded: "Debug data out of sync" lines | 19 (19) | 19 (19) |
| reccmp_addr_padding | 1,704 files, 0 bad | |
| check_2016_copies | silent | |

- decomplint: the only per-file change is GuiBuilder.cpp 3 → 2 in both modules. It exits 1 under `--warnfail`, as in every sweep.
- datacmp +2 issues per target: two more `stored_vtable` statics of boost::function (bind helpers from the new FriendService and Replicator code), the known class of function-pointer slots reccmp cannot see without `.reloc`. Not code.

## Saved

- **Final JSONs:** build/sweep24/P-final.json (= PD4) and build-studio/sweep24/S-final.json (= SD4). Scored exe, pdb, map and pdbview.tsv: build/sweep24/scored/ and build-studio/sweep24/scored/. The final link is the one PC/SC and PD/SD scored (PD/SD only rescored after the marker-word re-apply).
- Every round's JSON and .txt (P1–16, PC1–4, PD1–4; S1–16, SC1–4, SD1–4), gen logs, rounds-*.log, csv-with/ (the MATCH_FIX drop proof) with gen_with.py.bak, P/S-nolib, -vtable, -datacmp, -folded, {PLAYER,STUDIO}-decomplint.txt(.norm), ooo-*.txt, app-sweep.csv, row-*.csv, reorder-plan-pre.txt / -post-sweep.txt, GuiBuilder.cpp.pre, stubs.log, build logs.
- Helper scripts: the sweep23 set retargeted to sweep24, plus final.sh.

## Files changed by me

- CMakeLists.txt: `Client/WindowsClient/placeholder.cpp` out of `player_client`; the file deleted.
- tools/ida/gen_reccmp_csv.py: MATCH_FIX for PLAYER 0x005bd080 / 0x005baa50 and row insertion for a MATCH_FIX address without a names.csv row; the four wave-16 FriendService entries removed; docstring.
- reccmp/: regenerated player-* and studio-* (libdata, raknet, symbols, crt, stl).
- Client/_generated/{player,studio}/: unresolved.txt and stubs through gen_link_stubs.
- Marker words through app_sweep (no line moved); the sleepTime FOLDED pair (ConcurrentRakPeer.cpp, ContentProviderJob.cpp); GuiBuilder.cpp function order (reorder_markers).
- No change to tools/app_sweep.py or tools/reorder_markers.py.

## Unresolved

- **IDA owner:**
  - PLAYER-names.csv: 0x005bd080 → `_internal_accept_owner<NetworkOwnerJob>` (enable_shared_from_this<Job>), and add 0x005baa50 with the YieldFunctionStateObject name; then drop the two generator MATCH_FIX entries (regenerate with/without and compare, as for FriendService).
  - Push app-17's names (Game callees 0x00653790 / 0x00654e80 / 0x00652f00, setAssetUrl 0x00652d70, setProcessToolCommandsSeparately 0x0069ff10, setBaseUrl 0x0065fe50, setRunState 0x0072db60, FriendService 0x006b2c80 / 0x006b1500, ControllerService ctor 0x006ea020 / STUDIO 0x00817010, FFlag OverlayDataModelEnabled 0x00de98d0).
- **App:** `ClassRegistrar<ControllerService>::registrar` is unresolved data in both links (FactoryRegistration.cpp needs it, or the reference has to go); STUDIO `Described<FriendService>::classDescriptor` 0x00734740 (0.86) needs FactoryRegistration.cpp to supply that copy (network-5f-8); the 0xe0c → 0xe10 comments in DataModel.h / GuiBuilder.h; Game::setupDataModel, ~Game, SecurePlayerGame ctor, openState (0.691); STUDIO Game::Game 99.65 (operator= twin).
- **Shell:** UserInput (PostUserInputMessage 0x0045f1b0) for WndProc / HandleWindowsMessage; Application ctor 0x00403000; STUDIO InstancePacketCache ctor 0x00669180 (98.5) needs the shell's `list<connection>::_Buynode` instantiated; a STUDIO shell `bind<weak_ptr<DataModel>>` would retire MATCH_DROP.
- **Network:** Players::onChildAdded / onChildRemoving (88%), Server::onServiceProvider (88.99), the 15 Replicator virtual STUBs, postProcessPacket / filterPhysics vftable slots (twin folding).
- **Rendering contributor:** `CRenderSettings::minGameWindowSize` (unresolved data in PLAYER), the RbxOgre function 0x00487d50 UploadSessionLogs calls, the real ViewBase.
- **No-.reloc churn:** this link has none of sweep23's 13 STUDIO coincidences; expect them to come and go with layout.
- **gen_link_stubs unknowns:** PLAYER 10 (CSHA1 ×6, TCPInterface / PacketizedTCP ×4), STUDIO 10.
