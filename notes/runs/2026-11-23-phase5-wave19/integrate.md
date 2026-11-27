# integrate (workflow phase5-wave19), 2026-11-26

This integrates the three phase5-wave19 Work agents: app-19, shell-5k-10 and network-5f-10. All three finished. Their uncommitted work builds from clean in both targets with 0 failures, and all of it stays.

| | sweep25 | Lost | Gained | **sweep26** |
|---|---|---|---|---|
| PLAYER | 12,944 | 0 | 95 | **13,039** (+95) |
| STUDIO | 12,155 | 7 | 68 | **12,216** (+61) |

- Both targets end at or above every agent's private figure: app-19 13,033 / 12,215, network-5f-10 13,018 / 12,157, shell-5k-10 12,936 PLAYER.
- All 7 STUDIO losses are no-.reloc coincidences, read with `--verbose` (see "Losses explained").
- Housekeeping:
  - Every heavy command ran through tools/capped.sh, one build at a time, and none hit the cap.
  - No commits and no IDA database opened. Nothing under Client/Rendering/ was edited.
  - Only the generator wrote the shared reccmp/.

## What I did

1. **Saved the agents' tree** to build/sweep26/ (pre.diff, pre-status.txt, pre-untracked.tgz).
2. **CMakeLists.txt:**
   - Added `Client/WindowsClient/Document.cpp` to `player_client`, after Teleporter.cpp.
   - Added `target_compile_definitions(player_client PRIVATE OGRE_ATOMIC_CTOR_NOINLINE)`, with a comment citing the Application ctor and 0x0040eb40, as shell-5k-10 asked.
   - The new App sources (CommonVerbs.cpp, SleepingJob.cpp, the Commands.cpp changes) come in through the App glob on reconfigure.
3. **NetworkSettings rename** (asked by app-19 and network-5f-10), applied in one change to three files with no line moved:
   - `m_unk0x78` becomes `isQueueErrorComputed` at NetworkSettings.h:54 and NetworkSettings.cpp:91.
   - ConcurrentRakPeer.cpp:293 reads it, and the comment on line 288 is updated.
   - The name is the src2016 name; the PacketJob ctor 0x005c5610 copies the value into its own `isQueueErrorComputed`.
4. **Sweep and builds:**
   - `app_sweep set FUNCTION` set 624 PLAYER and 623 STUDIO markers.
   - Clean builds (reconfigure, `--target clean`, delete the exe PDB, build): PLAYER 718 steps, STUDIO 699, 0 failures.
5. **`gen_link_stubs.py`** (it deletes the exe first), then the exe PDB was deleted and the exe relinked:
   - PLAYER: 32 unresolved, 23 stubs, 12 unknown.
     - `SleepingJob::wake` and `Network::initWithPlayerSecurity` are resolved now.
     - New: `RBX::rot13` (unknown: no PLAYER name; Network/API.cpp `initVersion1` calls it).
     - Also new: four DirectInput data symbols (`GUID_SysKeyboard`, `GUID_SysMouse`, `c_dfDIKeyboard`, `c_dfDIMouse2`), unparsed because there is no dinput8.lib or dxguid.lib.
   - STUDIO: 23 unresolved, 20 stubs, 10 unknown, no change.
6. **Regeneration rounds** (score twice, regenerate from the pair, stop when a round gains nothing). Entries at effective 100%:
   - PLAYER 12,949 → 13,019 → 13,029 → 13,033 → 13,038/13,039 → 13,039 (P1–P14).
   - STUDIO 12,157 → 12,199 → 12,208 → 12,211 → 12,215 → 12,216 → 12,216 (S1–S14).
7. **Sweep:** `app_sweep apply P14 S14`. Against HEAD (markers.py):
   - PLAYER: 6 promoted, 29 new FUNCTION, 37 new STUB.
   - STUDIO: 5 promoted, 27 new FUNCTION, 37 new STUB, 3 demoted.
   - The 3 STUDIO demotions are no-.reloc coincidences, measured below 100 in this link, so they are STUB again: Math.cpp isDenormal 0x00898bc0, isNanInfDenormVector3 0x00898cc0, and WinHttp.cpp httpGetPostWinHttp 0x0091cd80. They go back to FUNCTION when the coincidence moves.
8. **Shell markers** (shellcheck):
   - **`Application::Application` 0x00403000 is at 100% in PLAYER.** The OGRE_ATOMIC_CTOR_NOINLINE define brings it to 96.7, and its three callees (CEvent, variables_map, the thread bind) pair once the CSVs are regenerated.
   - I flipped its marker to FUNCTION (Application.cpp:26) and reworded the comment line above it, with no line added.
   - No other shell FUNCTION is below 100. No STUB outside Rendering is at 100 (stubcheck).
9. **Reorder: tried and reverted.**
   - The plan called `App/script/LuaAtomicClasses.cpp` fixable (PLAYER 9 → 0 descents, STUDIO 11 → 2), so I applied it and rebuilt both targets (PC1–4 / SC1–4).
   - STUDIO held at 12,216. PLAYER fell to 13,033: symbexec, luaV_execute, constructor, patchtestreg, removevalues and luaK_setreturns went to 97.8–99.9.
   - `--verbose` shows the usual Lua mask immediate (`test/and reg, <imm>`) now reading as the `DescribedNonCreatable<FaceInstance>` vftable. The move shifted `.rdata` so that this vftable lands on the immediate.
   - Reverting the file (A/B, PR1) gave back exactly those 6, and STUDIO stays at 12,216 either way. **The file is byte-identical to the agents' version.** The reordered copy is in build/sweep26/LuaAtomicClasses.cpp.reordered for a later wave.
   - No other file in the plan is newly fixable. ConcurrentRakPeer.cpp (+1) and Replicator.cpp (+2) are assert-pinned or studio-only chunks, as network-5f-10 reported.
10. **Stability:**
    - After the revert I rebuilt both targets (PDBs deleted) and scored PR1, PR2 and SR1, SR2.
    - Then I regenerated the CSVs from those two runs of the current link and scored again (PR3 / SR3).
    - **PLAYER scored 13,039 and STUDIO 12,216 in every run.** `app_sweep apply` on the final runs changes nothing (the md5 of `git diff` is the same before and after).

## Losses explained (against build/sweep25/P-final.json and build-studio/sweep25/S-final.json)

- **PLAYER: none.**
- **STUDIO: 7, all no-.reloc coincidences.** `--verbose` on each shows only an immediate that our link now reads as an address:

  | Address | Function | Score | Diff |
  |---|---|---|---|
  | 0x0065f060 | RakNet IsOlderOrderedPacket (lib) | 95.0 | `lea edx,[eax-imm]` reads as the GuiLabel `{for GuiTarget}` vftable |
  | 0x00898bc0 | Math::isDenormal | 96.9 | `and eax,0x7fffff` |
  | 0x00898cc0 | Math::isNanInfDenormVector3 | 97.6 | `and eax,0x7fffff` |
  | 0x0091cd80 | Http::httpGetPostWinHttp | 99.9 | `and edx,imm` reads as the GuiLabel vftable |
  | 0x00a5dcc0 | luaK_setoneret (lib) | 96.3 | `and edx,0x7fffff` |
  | 0x00a5e020 | luaK_nil (lib) | 98.3 | `and eax,0x7fffff` |
  | 0x00a5ee90 | luaK_posfix (lib) | 99.6 | `and ecx,0x7fffff` |

  app-19 reported the same 7.
- **The six sweep25 STUDIO Lua coincidences:** the `"Points        : "` string moved. symbexec, luaV_execute, patchtestreg and removevalues are back at 100. constructor 0x00a35730 (99.07) and luaK_setreturns 0x00a5dc30 (97.92) still read `and reg,0x7fffff` as an address. These are library rows with no marker, so there is nothing to re-sweep.

## Final figures (sweep25 in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| **Entries at effective 100%** | **13,039** (12,944) | **12,216** (12,155) |
| Non-library entries | 10,277 (10,181) | 9,612 (9,558) |
| **Functions at 100%** | **9,927** (9,856) | **9,356** (9,296) |
| **Non-library functions at 100%** | **7,232** (7,160) | **6,799** (6,746) |
| Functions at 100%: owner / Rendering (minus g3d) / third party | 7,224 / 2 / 2,701 (7,149 / 2 / 2,705) | 6,788 / 3 / 2,565 (6,736 / 3 / 2,557) |
| Vftables at 100%: owner / Rendering / third party | 3,027 / 1 / 14 (3,007 / 1 / 10) | 2,801 / 1 / 8 (2,800 / 1 / 8) |
| Vftables at 100% / paired | 3,042 / 3,872 (3,018 / 3,857) | 2,810 / 3,795 (2,809 / 3,781) |
| reccmp Implemented, with libraries | 11,411 / 40,551 (11,359) | 10,934 / 42,384 (10,890) |
| reccmp Implemented, `--nolib` | 8,481 / 37,584 (8,429) | 8,188 / 39,526 (8,154 / 39,536) |
| App markers: FUNCTION + FOLDED + STUB | 1,313 + 29 + 542 (1,289 + 29 + 517) | 1,259 + 28 + 552 (1,239 + 28 + 523) |
| Base markers | 93 + 0 + 19 (no change) | 87 + 0 + 20 (no change) |
| Network markers | 193 + 3 + 58 (182 + 3 + 54) | 176 + 2 + 51 (167 + 2 + 47) |
| Shell markers (FUNCTION + STUB) | 109 + 40 (106 + 35) | 58 + 8 (no change) |

The owner / Rendering / third-party split comes from split.py: the .obj that the link map puts at each recompiled address.

**Rows** (PLAYER, tools/row_coverage.py on the final link; per-function CSVs are in build/sweep26/row-*.csv):

| Row | At 100% / total | Share | sweep25 |
|---|---|---|---|
| 5a | 139/155 | 89.7% | 139/155 |
| 5b | 1,291/3,305 | 39.1% | 1,288/3,305 |
| 5c | 105/158 | 66.5% | 105/158 |
| 5d | 475/1,554 | 30.6% | 475/1,554 |
| 5e | 587/1,337 | 43.9% | 564/1,337 |
| 5f | 951/3,371 | 28.2% | 917/3,327 |
| 5g (row_coverage_5g.py) | 334/613 | 54.5% | 334/613 |
| 5h | 549/1,359 | 40.4% | 549/1,359 |
| 5k | 1,134/3,493 | 32.5% | 1,119/3,493 |

**Near misses** (markers not at 100%; app_sweep report, build/sweep26/app-sweep.csv):

| Target | Not at 100% | ebp-frame | inlining-size | other | unpaired | unpaired-callee | x87-vs-sse | assert-file-line | data-ref | stack-cookie | ≥99 / ≥95 / ≥90% |
|---|---|---|---|---|---|---|---|---|---|---|---|
| PLAYER | 631 (602) | 205 | 207 | 131 | 48 | 12 | 21 | 4 | 3 | 0 | 9 / 34 / 56 |
| STUDIO | 632 (599) | 195 | 200 | 135 | 37 | 35 | 21 | 4 | 4 | 1 | 21 / 62 / 89 |

The "not at 100%" count rose because this wave added 37 new STUB markers per target (Commands.cpp, CommonVerbs, Replicator read* and others). None of these is a demotion except the 3 coincidences.

**Closest PLAYER misses:**

| Function | Address | Score |
|---|---|---|
| TaskScheduler::remove | 0x009b52b0 | 99.49 |
| ScriptContext::setTimeout | 0x0067eee0 | 99.41 |
| TextXmlWriter::serializeNode | 0x00628a60 | 99.39 |
| Players::onChildAdded (waits on the Impersonator pairing) | 0x0056ff40 | 99.28 |
| TaskScheduler::setThreadCount | 0x009b7ec0 | 99.26 |
| LegacyLock::Implementation ctor | 0x00641e60 | 99.25 |
| World::destroyAutoJoints | 0x00749340 | 99.23 |
| TaskScheduler::getJobsInfo | 0x009b75f0 | 99.21 |

**Closest STUDIO misses:**

| Function | Address | Score |
|---|---|---|
| httpGetPostWinHttp (coincidence) | 0x0091cd80 | 99.89 |
| DataModel::getJobsExtendedStats | 0x00768d70 | 99.75 |
| ScriptContext::ypcall | 0x007ed020 | 99.73 |
| Game::Game | 0x007cddd0 | 99.65 |
| TaskScheduler::remove | 0x00a6d0a0 | 99.49 |

## Checks

| Check | PLAYER (sweep25) | STUDIO (sweep25) |
|---|---|---|
| decomplint: function_out_of_order | 328 (317) | 331 (321) |
| decomplint: duplicate_offset | 12 (12) | 4 (4) |
| reccmp-vtable: traceback | none | none |
| reccmp-vtable: "Problem with adjuster thunk" lines | 13 (13) | 15 (15) |
| reccmp-datacmp: traceback | none | none |
| reccmp-datacmp: variables | 3,207 (3,197) | 3,077 (3,068) |
| reccmp-datacmp: issues | 73 (74) | 77 (78) |
| check_folded: annotated addresses | 3,179 (3,106) | 2,894 (2,832) |
| check_folded: folding problems | 0 | 0 |
| check_folded: "Debug data out of sync" lines | 19 (19) | 19 (19) |
| reccmp_addr_padding | 1,715 files, 0 bad | |
| check_2016_copies | silent | |

- **decomplint per file** (ooo.py):
  - PLAYER: LuaAtomicClasses.cpp 0 → 9, SecurityContext.cpp 1 → 0, ConcurrentRakPeer.cpp 0 → 1, Replicator.cpp 2 → 4.
  - STUDIO: LuaAtomicClasses.cpp 2 → 11, SecurityContext.cpp 2 → 0, ConcurrentRakPeer.cpp 0 → 1, Replicator.cpp 2 → 4.
  - LuaAtomicClasses.cpp is fixable but costs 6 coincidental entries (step 9).
  - The others are the line-70 / 2750 asserts and the read* STUBs that network-5f-10 explained.
  - Players.cpp keeps its one descent (it moved from line 388 to 401).
  - As in every sweep, decomplint exits 1 under `--warnfail`.
- **datacmp:**
  - **The classLibrary rows now pair the right arrays.** orig 0x00b81704 is `Vector2int16Bridge::classLibrary` (`{"new", newVector2int16}`), and 0x00b81714 is Vector2's.
  - All 13 remaining classLibrary diffs per target are only `.func` "Unknown pointer" slots, the function pointers reccmp cannot see without `.reloc`.
  - The exception is the unnamed `classLibrary` at 0x00b80464: its slot [2] points to 0x0096bde0, which the export calls `_DLRealloc_Ex`. That is probably a misnamed ObjectBridge function (IDA owner).
  - **`FFlag::DebugCrashEnabled` is cleared** in both targets, since app-19 set it to true.

## Saved

- **Final JSONs:**
  - build/sweep26/P-final.json (= PR3) and build-studio/sweep26/S-final.json (= SR3).
  - The scored exe, pdb, map and pdbview.tsv are in build/sweep26/scored/ and build-studio/sweep26/scored/.
- **Per-round output:**
  - P1–14, PC1–4, PE1, PR1–3; S1–14, SC1–4, SR1–3.
  - The gen logs, rounds-*.log and build logs (build1, build2, build-PC/SC, build-PR/SR).
- **Check outputs:**
  - P/S-nolib, -vtable, -datacmp and -folded.
  - {PLAYER,STUDIO}-decomplint.txt, app-sweep.csv, row-*.csv, reorder-plan-pre.txt and stubs.log.
- **Scripts:** the sweep25 set retargeted to sweep26, plus pipe2.sh (stubs and relink for both targets, then parallel rounds) and final-checks.sh.
- **Reorder material:** LuaAtomicClasses.cpp.pre (= the current file) and LuaAtomicClasses.cpp.reordered.

## Files changed by me

- CMakeLists.txt: Document.cpp added to `player_client`, plus the `OGRE_ATOMIC_CTOR_NOINLINE` definition for `player_client`.
- Client/App/include/V8DataModel/NetworkSettings.h:54, Client/App/v8datamodel/NetworkSettings.cpp:91, Client/Network/ConcurrentRakPeer.cpp:288/293: the `isQueueErrorComputed` rename.
- Client/WindowsClient/Application.cpp:25–26: the comment reworded, and the marker set to FUNCTION.
- Marker words through app_sweep; no line moved.
- reccmp/: regenerated player-libdata, player-symbols, studio-libdata, studio-stl and studio-symbols.
- Client/_generated/player/unresolved.txt, through gen_link_stubs.
- No change to tools/ida/gen_reccmp_csv.py, tools/app_sweep.py or tools/reorder_markers.py.

## Unresolved

- **Reorder:** LuaAtomicClasses.cpp's 9 PLAYER / 9 STUDIO new descents can be fixed by tools/reorder_markers.py. Applying it moves a vftable onto the Lua `0x7fffff`-style mask immediates and costs 6 PLAYER Lua library rows (a no-.reloc coincidence). Apply it in a later wave once the data layout moves again, and measure A/B.
- **No-.reloc coincidences:**
  - STUDIO isDenormal 0x00898bc0, isNanInfDenormVector3 0x00898cc0 and httpGetPostWinHttp 0x0091cd80 are STUB in this sweep. Their code is unchanged and byte-identical, so flip them back when the immediate stops matching.
  - Plus four STUDIO library rows (IsOlderOrderedPacket and three luaK_*) and the leftover constructor / luaK_setreturns.
- **Generator, STUDIO `shared_from` twin** (network-5f-10), not attempted:
  - filterChangedProperty 0x00638330 (98.77) and the PacketJob ctor 0x00620870 (99.19) call orig 0x00445440, which the generator gives to our shell `shared_from<RunService>` (recomp 0x4255f0, studio_client). Our engine `shared_from<DataModel>` is a separate body.
  - Needs a MATCH_FIX or a vote change, measured A/B.
- **IDA owner:**
  - `RBX::rot13`'s PLAYER address, so gen_link_stubs can bind it (it is unresolved now).
  - The 0x0096bde0 `_DLRealloc_Ex` name in the unnamed classLibrary at 0x00b80464.
  - The names listed in app-19.md, shell-5k-10.md and network-5f-10.md.
- **DirectInput (user's call):** vendoring dinput8.lib and dxguid.lib from a DirectX SDK. Six DirectInput symbols are unresolved now. Once they are linked, generator rows for the five data addresses that shell-5k-10 lists take createMouse and createKeyboard to 100.
- **App** (from the agents):
  - Move FLog::NetworkReadItem and FLog::JoinSendExtraItemCount into ScriptContext.cpp, deleting the Network copies in the same change.
  - Add `Game::getOverlayDataModel` and ControllerService's `hardwareDevice` / `setHardwareDevice`, which unblocks View::initializeInput and UserInput::setGame.
  - The Impersonator ctor/dtor (Players::onChildAdded stays at 99.28).
  - The CommonVerbs members, the Commands.cpp bodies, openState, shutdownGameDataModel, and STUDIO ~Game / Game::Game.
- **Shell and Network:** the next-pass lists in shell-5k-10.md and network-5f-10.md are unchanged.
- **Rendering contributor:** `ViewBase::FUN_00487d50` and `CRenderSettings::minGameWindowSize` are still unresolved.
