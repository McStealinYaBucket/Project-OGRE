# integrate (workflow phase5-wave21), 2026-11-27

This integrates the three phase5-wave21 Work agents: app-21, shell-5k-12 and network-5f-12. All three finished. Their uncommitted work builds from clean in both targets with 0 failures, and all of it stays.

| | sweep27 | Lost | Gained | **sweep28** |
|---|---|---|---|---|
| PLAYER | 13,117 | 0 | 154 | **13,271** (+154) |
| STUDIO | 12,280 | 0 | 111 | **12,391** (+111) |

- Both targets end above every agent's private figure: app-21 13,266 / 12,381, network-5f-12 13,262 / 12,351, shell-5k-12 13,259 (PLAYER).
- **Nothing at 100% in sweep27 is lost in either target.** The one interim STUDIO loss (step 4) was fixed in the generator.
- Housekeeping:
  - Every heavy command ran through tools/capped.sh, one build at a time, and none hit the cap.
  - No commits and no IDA database opened. Nothing under Client/Rendering/ was edited.
  - The shared reccmp/ was written only by the generator, apart from the one row deleted in step 3.

## What I did

1. **Saved the agents' tree** to build/sweep28/ (pre.diff, pre-status.txt, pre-untracked.tgz).
   - **CMakeLists.txt is unchanged.** The new files (tool/{AxisRotate,Clone,Game,Grab,Hammer}Tool.cpp, v8datamodel/Tools{Model,Part}.cpp, Network/NetworkFilter.h) and the formerly empty tool files come in through the `ogre_prior_sources` globs once the build is reconfigured.
   - shell-5k-12 linked no new .cpp.
2. **Sweep and clean builds:**
   - `app_sweep set FUNCTION` set 666 PLAYER and 638 STUDIO markers.
   - Then reconfigure, `--target clean`, delete the exe PDB and build: PLAYER 734 steps, STUDIO 715, 0 failures in either.
   - `gen_link_stubs.py` (it deletes the exe first), then the PDB was deleted and the exe relinked:
     - PLAYER: 32 unresolved, 24 stubs, 11 unknown. `FLog::PlayerShutdownLuaTimeoutSeconds` is new in unresolved.txt (shell-5k-12's Document::Shutdown; App has to define it).
     - STUDIO: 23 unresolved, 20 stubs, 10 unknown. No change.
3. **Generator inputs, before the first regeneration:**
   - **STUDIO `MATCH_DROP` 0x00445440 deleted.** shell-5k-12's Client/ClientShared/placeholder.cpp instantiates `shared_from<DataModel>` and `<Instance>`, and they fold with `shared_from<RunService>` into one shell body, as in 2013. All five STUDIO `shared_from` twin rows the agents reported are back at 100 (PacketJob 0x00620870, StatsUpdateJob 0x006211c0, PacketReceiveJob 0x0064b030, YieldFunctionStateObject 0x009deec0, RunService::start 0x0079af80). The shell's own 0x00445440 is at 100 too. I did not repeat the A/B: shell-5k-12 measured it on identical objects (12,307 without the entry, 12,298 with it).
   - **PLAYER `MATCH_DROP` 0x0070fb30 kept.** The map still has only the shell's `shared_ptr<RunService>(p, Deleter)` (UserInput.cpp.obj 0x0041b280), with no engine copy.
   - **The sticky player-libdata.csv row for 0x00669fd0 deleted** (it carried the copyValue<SoundId> symbol; app-21's item 4). No generator change was needed: with the row gone, the call-site votes pair 0x00669fd0 with our copyValue<ContentId> and 0x006bc800 with copyValue<SoundId>, both at 100. The vftables 0x00b3388c, 0x00b33904 and 0x00b44c2c are at 100 as well.
4. **Regeneration rounds** (score twice, regenerate from the pair, stop when a round gains nothing):
   - PLAYER 13,159 → 13,233 → 13,267 → 13,269 → 13,271 → 13,271 (P1–P14).
   - STUDIO 12,287 → 12,360 → 12,390 → 12,390 (S1–S8).
   - **The only loss against sweep27 was STUDIO `shared_ptr<RunService>(p, Deleter)` 0x0082cdf0 (96.97).** `--verbose`: its call to `shared_count<RunService*, Deleter>` went unpaired. OGRE_VOTES shows our only copy (engine, ChatOutput.obj 0x004ed6b0) has 3 call-site votes for the engine original 0x0082bf00. The IDA wave-20 sync gave the shell original 0x00431860 that symbol as a `match:` name, though, and our STUDIO shell has no copy.
   - **Fix: `MATCH_DROP` ("STUDIO", 0x00431860)**, with the evidence in the comment. A/B on the same link (SM1/SM2 against S7/S8): **12,390 → 12,391, +1 / −0**, and 0 lost against sweep27.
5. **Sweep:** `app_sweep apply P14 SM2`.
   - Against HEAD: PLAYER 14 promoted, 12 new FUNCTION, 12 new STUB; STUDIO 4 promoted, 22 new FUNCTION, 15 new STUB. Nothing demoted.
   - Against the agents' working tree (every marker word they left), 6 promotions and 0 demotions:
     - **httpGetPostWinHttp is FUNCTION again in both targets** (PLAYER 0x00826990, STUDIO 0x0091cd80): the `and edx, imm` coincidence moved off our image.
     - ScriptContext::setTimeout PLAYER 0x0067eee0.
     - Commands.cpp PLAYER 0x00856580 / STUDIO 0x00780bb0.
     - RunService::start STUDIO 0x0079af80 (step 3).
   - shellcheck finds no shell FUNCTION below 100, and stubcheck finds no STUB (outside Rendering) at 100.
6. **Stability:**
   - After `apply`, I scored both targets twice (PF1/PF2, SF1/SF2), regenerated from those runs and scored again (PF3/SF3). **PLAYER scored 13,271 and STUDIO 12,391 in every run, with 0 lost and 0 gained against P14/SM2.**
   - Re-running `apply` on PF3/SF3 changes nothing (the md5 of `git diff` is the same).
   - No rebuild after `apply`: it rewrites only the marker word, with no line moved.
7. **Reorder: nothing to apply.**
   - `reorder_markers.py plan` (build/sweep28/reorder-plan-pre.txt) lists no fixable file. Every file with descents is pinned by assert lines, has a studio-only chunk, or its descents cross runs.
   - This wave's descents:
     - PartInstance.cpp: PLAYER 26 → 27. `fromPrimitive` is at the end of a file whose asserts are pinned (app-21 reported it).
     - DataModel.cpp: PLAYER 18 → 17.
     - Replicator.cpp: network-5f-12's createPhysicsSender STUB stays below ~Replicator on purpose, to keep asserts 348/350 on their lines.
   - **ServerReplicator.cpp, STUDIO-only descent (STUB 0x006502d0): left as it is.** In STUDIO both 0x006502d0 and 0x00650b30 sit below the file's first function (0x00654010). A per-target order would mean two copies of each body under `#if OGRE_TARGET_*`, which moves lines in both targets, so it is not cost-free.

## Final figures (sweep27 in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| **Entries at effective 100%** | **13,271** (13,117) | **12,391** (12,280) |
| Non-library entries | 10,508 (10,356) | 9,781 (9,670) |
| **Functions at 100%** | **10,132** (9,998) | **9,531** (9,420) |
| **Non-library functions at 100%** | **7,436** (7,304) | **6,968** (6,857) |
| Functions at 100%: owner / Rendering (minus g3d) / third party | 7,424 / 2 / 2,706 (7,293 / 2 / 2,703) | 6,959 / 3 / 2,569 (6,846 / 3 / 2,571) |
| Vftables at 100%: owner / Rendering / third party | 3,057 / 1 / 11 (3,035 / 1 / 13) | 2,801 / 1 / 8 (2,802 / 1 / 7) |
| Vftables at 100% / paired | 3,069 / 3,942 (3,049 / 3,942) | 2,810 / 3,864 (2,810 / 3,864) |
| reccmp Implemented, with libraries | 11,615 / 40,551 (11,505) | 11,127 / 42,384 (11,053) |
| reccmp Implemented, `--nolib` | 8,683 / 37,582 (8,575) | 8,381 / 39,527 (8,306) |
| App markers: FUNCTION + FOLDED + STUB | 1,368 + 29 + 563 (1,347 + 29 + 576) | 1,315 + 28 + 548 (1,294 + 28 + 552) |
| Base markers | 93 + 0 + 19 (no change) | 87 + 0 + 20 (no change) |
| Network markers | 207 + 3 + 76 (203 + 3 + 68) | 193 + 2 + 64 (189 + 2 + 52) |
| Shell markers (FUNCTION + STUB) | 124 + 47 (113 + 38) | 58 + 8 (no change) |

The owner / Rendering / third-party split goes by the .obj that the link map puts at each recompiled address, so a few rows move between columns from link to link. No STUDIO row was lost.

**Where the gains are**, by the .obj of the recompiled body:
- PLAYER 154 (134 functions, 20 vftables): CommonVerbs 81, Commands 9, UserInput 16, Document 7, View 1, ConcurrentRakPeer 6, ServerReplicator 2, Mechanism 2, FunctionMarshaller 2, and one each in Streaming, DataModel, ScriptContext, OverlayDataModel, Sound, PartInstance, WinHttp and lcode.c (luaK).
- The 20 PLAYER vftables are app-21's verb vftables plus the three copyValue vftables (step 3). The map lists them under atlstr.obj because they sit in .rdata.
- STUDIO 111 (all functions): CommonVerbs 81, Commands 9, ConcurrentRakPeer 6, Peer 2, ServerReplicator 2, Mechanism 2, and one each in placeholder, Streaming, ScriptContext, DataModel, RunService, PartInstance, OverlayDataModel, WinHttp and lcode.c.

**Rows** (PLAYER, tools/row_coverage.py on the final link; per-function CSVs are in build/sweep28/row-*.csv):

| Row | At 100% / total | Share | sweep27 |
|---|---|---|---|
| 5a | 139/155 | 89.7% | 139/155 |
| 5b | 1,297/3,324 | 39.0% | 1,296/3,324 |
| 5c | 105/158 | 66.5% | 105/158 |
| 5d | 477/1,554 | 30.7% | 475/1,554 |
| 5e | 588/1,337 | 44.0% | 587/1,337 |
| 5f | 970/3,372 | 28.8% | 961/3,371 |
| 5g (row_coverage_5g.py) | 334/613 | 54.5% | 333/613 |
| 5h | 550/1,359 | 40.5% | 549/1,359 |
| 5k | 1,180/3,493 | 33.8% | 1,154/3,493 |

None of the row file sets contains the Commands/CommonVerbs block, so most of app-21's gains show up in no row.

**Near misses** (markers outside Rendering not at 100%; app_sweep report, build/sweep28/app-sweep.csv):

| Target | Not at 100% | inlining-size | ebp-frame | other | unpaired | unpaired-callee | x87-vs-sse | data-ref | assert-file-line | other shapes | ≥99 / ≥95 / ≥90% |
|---|---|---|---|---|---|---|---|---|---|---|---|
| PLAYER | 663 (675) | 228 | 220 | 133 | 48 | 14 | 20 | 3 | 4 | 0 | 7 / 36 / 59 |
| STUDIO | 635 (633) | 205 | 201 | 134 | 37 | 35 | 20 | 3 | 4 | stack-cookie 1, regalloc-sched 1 | 18 / 60 / 87 |

The shape counts include a few Rendering markers, as in sweep27. The "not at 100%" column and the ≥ thresholds exclude them.

**Closest PLAYER misses:**

| Function | Address | Score | Shape |
|---|---|---|---|
| TaskScheduler::remove | 0x009b52b0 | 99.49 | data-ref |
| TextXmlWriter::serializeNode | 0x00628a60 | 99.39 | other |
| Players::onChildAdded (waits on the Impersonator pairing) | 0x0056ff40 | 99.28 | unpaired-callee |
| TaskScheduler::setThreadCount | 0x009b7ec0 | 99.26 | unpaired-callee |
| World::destroyAutoJoints | 0x00749340 | 99.23 | other |
| TaskScheduler::getJobsInfo | 0x009b75f0 | 99.21 | unpaired-callee |
| GuiService::openBrowserWindow | 0x0069a540 | 99.08 | unpaired-callee |

**Closest STUDIO misses:**

| Function | Address | Score | Shape |
|---|---|---|---|
| DataModel::getJobsExtendedStats | 0x00768d70 | 99.75 | unpaired-callee |
| ScriptContext::ypcall | 0x007ed020 | 99.73 | unpaired-callee |
| Game::Game | 0x007cddd0 | 99.65 | unpaired-callee |
| TaskScheduler::remove | 0x00a6d0a0 | 99.49 | data-ref |
| ScriptContext::setTimeout | 0x007e4db0 | 99.41 | unpaired-callee |
| TextXmlWriter::serializeNode | 0x0074c8d0 | 99.39 | other |
| InsertService::InsertService | 0x008775b0 | 99.28 | unpaired-callee |

## Checks

| Check | PLAYER (sweep27) | STUDIO (sweep27) |
|---|---|---|
| decomplint: function_out_of_order | 319 (319) | 323 (323) |
| decomplint: duplicate_offset | 12 (12) | 4 (4) |
| reccmp-vtable: traceback | none | none |
| reccmp-vtable: "Problem with adjuster thunk" lines | 13 (13) | 15 (15) |
| reccmp-datacmp: traceback | none | none |
| reccmp-datacmp: variables | 3,335 (3,223) | 3,188 (3,080) |
| reccmp-datacmp: issues | 74 (75) | 78 (79) |
| check_folded: annotated addresses | 3,310 (3,270) | 2,976 (2,943) |
| check_folded: folding problems | 0 | 0 |
| check_folded: "Debug data out of sync" lines | 19 (19) | 19 (19) |
| reccmp_addr_padding | 1,726 files, 0 bad | |
| check_2016_copies | silent | |
| gen_reccmp_csv.py --selftest | ok | |

- decomplint per file (ooo.py against sweep27): PartInstance.cpp PLAYER 26 → 27, DataModel.cpp PLAYER 18 → 17, and nothing else. As in every sweep, decomplint exits 1 under `--warnfail`.

## Saved

- **Final JSONs:**
  - build/sweep28/P-final.json (= PF3) and build-studio/sweep28/S-final.json (= SF3).
  - The scored exe, pdb, map and pdbview.tsv are in build/sweep28/scored/ and build-studio/sweep28/scored/.
- **Runs:**
  - PLAYER: P1–P14 and PF1–PF3.
  - STUDIO: S1–S8, SM1–SM2 (the MATCH_DROP A/B) and SF1–SF3.
  - Each run has its .txt log, plus the gen logs, rounds-*.log, build1/build2 and stubs.log.
- **Check outputs:**
  - P/S-nolib, -vtable, -datacmp and -folded.
  - {PLAYER,STUDIO}-decomplint.txt, app-sweep.csv, row-*.csv and reorder-plan-pre.txt.
- **Pre-change copies:** build/sweep28/player-libdata.pre.csv (before the 0x00669fd0 row was deleted), build-studio/sweep28/reccmp-S8gen/ (the STUDIO CSVs before the 0x00431860 MATCH_DROP) and pre-apply.diff.
- **Scripts:** the sweep27 set retargeted to sweep28, plus sab.sh (regenerate STUDIO from a pair, then score twice).

## Files changed by me

- tools/ida/gen_reccmp_csv.py: `MATCH_DROP = {("PLAYER", 0x0070fb30), ("STUDIO", 0x00431860)}`. STUDIO 0x00445440 is deleted. The evidence and measured effect are in the comment.
- reccmp/: regenerated player-libdata, player-symbols, studio-libdata, studio-stl and studio-symbols. The sticky player-libdata row 0x00669fd0 was deleted once, by hand, before regeneration.
- Marker words, through app_sweep; no line moved.
- Client/_generated/player/unresolved.txt, through gen_link_stubs.
- CMakeLists.txt, tools/app_sweep.py and tools/reorder_markers.py: no change. No function was reordered.

## Unresolved

- **App, FastLogSettings.cpp:** define `FLog::PlayerShutdownLuaTimeoutSeconds` (shell-5k-12: .bss PLAYER 0x00de3f92, initializer 0x00a663b0). Until then it is the 33rd /FORCE:UNRESOLVED symbol.
- **STUDIO shell stand-in for `shared_count<RunService*, Creatable<Instance>::Deleter>`** (Client/ClientShared/placeholder.cpp). With it, the shell original 0x00431860 pairs too, and the new STUDIO MATCH_DROP entry can go. Measure A/B.
- **PLAYER `shared_ptr<RunService>` engine copy:** the MATCH_DROP entry for 0x0070fb30 stays until our link keeps an engine copy again.
- **ServerReplicator.cpp STUDIO-only descent** (STUB 0x006502d0, and 0x00650b30 also sits before the file's first function in STUDIO): the targets order this .obj differently. Leave it unless a per-target layout comes for free.
- **PartInstance.cpp:** the `fromPrimitive` descent stays until the file's asserts can be laid out.
- **From the agents' lists, not integrator work:**
  - app-21: ModelInstance's 2013 layout (0x128) and the Workspace bases; Selection's layout; requireEdit 0x00856010; openState; the DataModel unnamed group; `PartInstance::primitive` at +0xb8 (network-5f-12's checkDistributed* at 97–99%); `Guid::Scope` default ctor without setNull.
  - shell-5k-12: UIEvent and OverlayDataModel::dataModelShutdownFinished for ProcessUserInputMessage; Document::Initialize / executeScript; Document::Shutdown's 64-byte frame.
  - network-5f-12: the ServerReplicator ctor 0x0057c3b0, readRegionRemoval, the ConnectionStats ctor and updateStats; the empty-body twin for ~NetworkFilter (setBasicFilteringEnabled 97.6%); `Read<short>` not folded into `Read<unsigned short>` (88.9%).
- **IDA owner:** the names in app-21.md, shell-5k-12.md and network-5f-12.md. Also: STUDIO 0x00431860's `match:` name is a shell copy our link lacks, so keep it, but the generator now drops it (step 4).
- **DirectInput (user's call):** vendoring dinput8.lib and dxguid.lib. Unchanged.
