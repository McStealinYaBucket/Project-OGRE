# integrate (workflow phase5-wave8), 2026-11-24

I integrated the three phase5-wave8 Work agents: base-pairing, world-5d-5 and datamodel-5b-6. All three finished and all of their uncommitted work compiles in both targets from clean, so nothing was reverted.

Then I took three integrator items from their Unresolved lists:
- applied reccmp patch 0002
- built program_options /GL in PLAYER
- fixed three marker or layout items (below)

After that I rebuilt both targets from clean, regenerated the CSVs until a round gained nothing, swept the markers and ran every check. Both targets are stable:
- PLAYER held **10,729** across PA1–PA5.
- STUDIO held **10,192** across S9–S12, SSW and SA1–SA5.

Against sweep14 (10,076 / 9,803), PLAYER gained 702 entries and lost 49, and STUDIO gained 389 and lost **0**. All 49 PLAYER losses are pairing-only, and every one traces to building program_options /GL, the 2013 config (see Losses).

Every heavy command ran through tools/capped.sh, one build at a time, and nothing hit the cap. There are no commits, no IDA database was opened, nothing under Client/Rendering/ was edited, and the 2016-copy check is silent.

## What I did

**Integrator items from the agents' Unresolved lists**
- **reccmp patch 0002** (base-pairing's one-past-end pointer rule): `sh tools/reccmp-patches/apply.sh` patched the shared venv (`compare/asm/parse.py`, `replacement.py`). Patch 0001 was already applied. StarterGuiService ctor 0x0070c150 is now 100% (datamodel-5b-6 had it at 97.44%).
- **build/ reconfigured with `-DOGRE_BOOST_PO_GL=ON`.** program_options is now built with the shell's /GL flags, which is the 2013 configuration.
- **Client/App/include/Util/IndexArray.h** (world-5d-5's two items):
  - I removed 5 blank lines between `operator[](int)` and `operator[](unsigned)`, so the asserts fall on the 2013 lines :103/:108/:113/:118. I added 5 blank lines after the const unsigned overload, so every later line keeps its number.
  - The fastRemove nameref under PLAYER 0x00830340 / STUDIO 0x009a29a0 now names the `Connector/getRealTimeIndex` instance, the one our PDB keeps after folding.
  - Result: the 9 const IndexArray accessor instances (0x0082e250–0x0082e9d0, 95.5% before), fastRemove 0x00830340, IndexedTree::setIndexedTreeParent 0x00999850 and KernelData::removeConnector 0x00831f80 are all at 100%.
- **Client/App/include/Util/AsyncHttpQueue.h, the TEMPLATE nameref under PLAYER 0x0086bc70.**
  - **Cause:** datamodel-5b-6 asked for a generator row, but the real cause was the marker. Its nameref named `list<FailedUrl>::_Incsize`, which our PDB no longer has as a PUBLICS name, because our link folds it with TimerService's `list<Item>::_Incsize` as the original does.
  - **Fix:** the nameref now names TimerService's instance.
  - **Result:** `_Incsize` 0x0086bc70, `push_front` 0x0086c5b0 and TimerService::delay 0x007a6a30 all reached 100%, the three datamodel-5b-6 predicted.
  - **The generator is unchanged.** I tried a MATCH_FIX entry first. It had no effect because the marker held the address, so I reverted it.

**Clean rebuilds**
- PLAYER: build/ took 680 steps with 0 failures.
- STUDIO: build-studio/ with `--target studio` took 675 steps with 0 failures.

**Link stubs.** I deleted each exe first, then ran gen_link_stubs on each target. Each now has 31 unresolved symbols (was 32) and 11 stub addresses: unknown 17, data 10, unparsed 4. Client/_generated is unchanged.

**Regeneration rounds.** Each round pools the two latest runs of the current link (build/sweep15/rounds.sh). Figures are entries at effective 100%:
- **PLAYER:**
  - P0 with the stale CSVs scored 10,000.
  - The rounds then went 10,000 → 10,526 → 10,666 → 10,716 → 10,725 → 10,726 → 10,726.
  - After the sweep and the nameref fix: PSW 10,726, then PA1/PA2 10,729, PA3/PA4 10,729 and PA5 10,729.
- **STUDIO:**
  - The rounds went 9,724 → 10,029 → 10,132 → 10,173 → 10,192 → 10,192.
  - After the sweep: SSW 10,192, then SA1–SA5 10,192.
- PLAYER signature pairing: 1,389 by signature and 61 by folded name. Vftables paired by RTTI name: 3,412 of 7,333.

**Sweep**
- `app_sweep set FUNCTION` flipped 576 PLAYER and 578 STUDIO markers.
- I rebuilt both and scored PSW and SSW. Both equalled their pre-flip figures. Then I ran `apply`.
- Swept scope:

  | | FUNCTION | FOLDED | STUB |
  |---|---|---|---|
  | PLAYER | 1,289 | 49 | 580 |
  | STUDIO | 1,247 | 49 | 579 |

- Against HEAD, in Client/App, Base and Network (build/sweep15/markers.py HEAD):

  | | New FUNCTION | New STUB | Promoted | Demoted |
  |---|---|---|---|---|
  | PLAYER | 26 | 8 | 23 | **4** |
  | STUDIO | 26 | 7 | 18 | 0 |

  The four PLAYER demotions are TaskScheduler::disableThreads 0x009b7140, getJobsInfo 0x009b75f0, getJobsByName 0x009b78f0 and setThreadCount 0x009b7ec0. They are pairing-only (see Losses).
- stubcheck: no STUB marker outside Rendering sits on an address at effective 100%.

**Reorder.** `reorder_markers plan` finds only type.cpp and LuaInstanceBridge.cpp fixable. Their reorders break the compile, as in every earlier wave, so I left them; nothing was applied.

New descents against sweep14, per decomplint file counts (PLAYER and STUDIO):

| File | Added descents (both targets) |
|---|---|
| Kernel.cpp | +1 |
| SimBody.cpp | +1 |
| SleepStage.cpp | +2 |
| World.cpp | +1 |

All of them are world-5d-5's 2013 assert-line layouts, and the plan reports them as pinned.

## Final figures (sweep14 in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| **Entries at effective 100%** | **10,729** (10,076) | **10,192** (9,803) |
| Non-library entries | 8,164 (7,593) | 7,750 (7,359) |
| **Functions at 100%** | **7,983** (7,605) | **7,650** (7,368) |
| **Non-library functions at 100%** | **5,485** (5,189) | **5,255** (4,971) |
| Functions at 100%: owner side / Rendering (minus g3d) / third party | 5,474 / 2 / 2,507 (5,178 / 2 / 2,425) | 5,242 / 3 / 2,405 (4,959 / 3 / 2,406) |
| Vftables at 100%: owner / Rendering / third party | 2,665 / 1 / 11 (2,389 / 1 / 12) | 2,485 / 1 / 7 (2,377 / 1 / 8) |
| Vftables at 100% | 2,677 / 3,493 (2,402 / 3,375) | 2,493 / 3,430 (2,386 / 3,312) |
| reccmp Implemented, with libraries | 9,680 / 40,550 (9,355) | 9,350 / 42,383 (9,104) |
| reccmp Implemented, `--nolib` | 6,925 / 37,756 (6,677 / 37,832) | 6,761 / 39,672 (6,513 / 39,670) |
| App markers: FUNCTION + FOLDED + STUB | 1,146 + 27 + 535 (1,100 + 27 + 551) | 1,107 + 27 + 539 (1,066 + 27 + 551) |
| Base markers | 86 + 0 + 24 (90 + 0 + 20) | 83 + 0 + 22 (83 + 0 + 22) |
| Network markers | 54 + 1 + 9 (54 + 1 + 9) | 54 + 1 + 9 (54 + 1 + 9) |
| Sweep scope: markers at 100% / short | 1,338 / 580 (1,296 / 592) | 1,296 / 579 (1,255 / 591) |

- The vftable total grew from 3,375 to 3,493 in PLAYER because the /GL program_options build brings its vftables into the pairing. Base-pairing counted 165 `sp_counted_impl_pd` and program_options vftables among its gains.
- The integrated tree is above every agent's private figure:

  | Agent | PLAYER | STUDIO |
  |---|---|---|
  | base-pairing | 10,712 (10,718 with patch 0002) | 10,011 |
  | world-5d-5 | 10,450 | 10,006 |
  | datamodel-5b-6 | 10,427 | 10,175 |

**Rows** (PLAYER). The sweep14 column is sweep14's P-final scored on the current ranges. Per-function CSVs are in build/sweep15/row-*.csv.

| Row | At 100% / total | Share | sweep14 |
|---|---|---|---|
| 5a | 138/155 | 89.0% | 138 |
| 5b | 699/1,923 | 36.3% | 662 |
| 5c | 88/152 | 57.9% | 87 |
| 5d | 421/1,553 | 27.1% | 368 |
| 5e | 479/1,335 | 35.9% | 481 |
| 5f | 584/3,280 | 17.8% | 544 |
| 5g (row_coverage_5g.py) | 324/609 | 53.2% | 323 |
| 5h | 525/1,359 | 38.6% | 520 |
| 5k | 664/3,493 | 19.0% | 546 |

- **5e:** lost 5 and gained 3. The 5 losses are the program_options `_Tree` twins inside ScriptContext.cpp's range and `Bridge<Library>::on_tostring`, all pairing-only (see Losses).
- **5k:** most of its +118 is the /GL program_options block.

**Near misses** (app_sweep report, build/sweep15/app-sweep.csv):

| Target | Short | inlining-size | ebp-frame | other | unpaired | unpaired-callee | x87-vs-sse | stack-cookie | assert-file-line | data-ref | At 90% or more | Pairing-only |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| PLAYER | 579 | 168 | 168 | 129 | 49 | 38 | 20 | 2 | 4 | 1 | 76 | 39 |
| STUDIO | 579 | 169 | 166 | 133 | 38 | 43 | 20 | 5 | 4 | 1 | 89 | 44 |

data-ref fell from 6 to 1 in both targets: patch 0002 cleared the prime_list end pointers.

Closest (PLAYER):

| Function | Address | Score | Pairing-only? |
|---|---|---|---|
| ~DataModel | 0x0064d850 | 99.76 | yes |
| DataModel ctor | 0x0064ec50 | 99.70 | yes |
| AsyncHttpQueue::asyncRequest | 0x00870430 | 99.59 | yes |
| ~TaskScheduler | 0x009b4e00 | 99.52 | yes |
| TaskScheduler::remove | 0x009b52b0 | 99.49 | yes |
| HeartbeatTask::stepDataModelJob | 0x0072dc30 | 99.45 | yes |
| TextXmlWriter::serializeNode | 0x00628a60 | 99.39 | no |
| RunService::start | 0x0072d300 | 99.37 | yes |
| MemoryBinder::processID | 0x006fc500 | 99.33 | yes |
| ~Job | 0x009ba1f0 | 99.28 | yes |
| TaskScheduler::setThreadCount | 0x009b7ec0 | 99.26 | yes |
| DebrisService::onServiceProvider | 0x00845380 | 99.22 | yes |
| Job::computeStandardSleepTime | 0x009b9a90 | 99.21 | yes |
| ServiceProvider::onChildAdded | 0x006a9770 | 99.19 | yes |
| World::createAutoJoints | 0x0074aa80 | 99.14 | yes |

- The engine-copy versus shell-copy `~shared_ptr` / STL twins that base-pairing described need the 2013 shell's own instantiations.

## Losses (against sweep14's P-final / S-final, summ.py, each read with --verbose)

**STUDIO: none.**

**PLAYER: 49 entries.** All of them are pairing-only, and all come from program_options now being built /GL (base-pairing measured that switch at +292/−48 on one source snapshot). The link moves which of two identical bodies survives under which name.

- **37 library entries, program_options and its STL/boost instantiations:**
  - `_Tree<string, variable_value>` members, `vector<basic_option>`, `function1<...>` and `shared_ptr<options_description>` helpers, and `to_internal`.
  - `boost::any::holder<string>`'s deleting destructor 0x0047a530 (45%). Our /GL body inlines `~holder` where the original calls it out of line, so the original is now paired with the LTCG copy.
  - `table_impl<...>::operator[]` 0x008f0630 (99.15%). Its callee `to_internal` (folded with `make_tuple<string>` in the original) now pairs with the LTCG copy.
- **7 engine template instances whose only diff is one callee twin:**
  - ScriptStats `_Tree::erase` 0x00679740 (96.3%): `_Inc` and `_Min` pair with the `LuaProfiler` map copies instead of the program_options ones.
  - `_Uninit_copy`/`_Uninit_fill_n` of PriorityTask 0x0082a4a0 and 0x0082a7b0, and of CallbackWrapper 0x0086cab0, 0x0086cbc0 and 0x0086d0f0 (98.8%): `~function1<void,bool const&>` pairs with `allocator<CallbackWrapper>::destroy`.
  - `Bridge<Library>::on_tostring` 0x0087f770 (96.9%): `to_internal` versus `make_tuple<string>`.
- **5 in Base, the per-.obj STL copies base-pairing found:**
  - `vector<shared_ptr<Instance>>::push_back` 0x009b7020 (now unpaired).
  - TaskScheduler disableThreads, getJobsInfo, getJobsByName and setThreadCount (97–99%). Their `push_back` call goes to the other twin (0x009ba960), because our link folds the two instantiations the 2013 linker kept apart.

**I kept /GL program_options despite these 49:** it is the 2013 configuration and nets +653 here (base-pairing measured +292/−48 on one source snapshot). Reverting it would trade 49 pairing-only entries for several hundred real ones.

## Checks

| Check | PLAYER (sweep14) | STUDIO (sweep14) |
|---|---|---|
| decomplint: function_out_of_order | 305 (300) | 312 (307) |
| decomplint: duplicate_offset | 12 (12) | 4 (4) |
| reccmp-vtable: traceback | none | none |
| reccmp-vtable: "Problem with adjuster thunk" lines | 11 (11) | 13 (13) |
| reccmp-datacmp: traceback | none | none |
| reccmp-datacmp: variables | 2,588 (2,502) | 2,447 (2,371) |
| reccmp-datacmp: issues | 57 (55) | 60 (58) |
| check_folded: annotated addresses | 2,649 (2,579) | 2,499 (2,430) |
| check_folded: folding problems | 0 | 0 |
| check_folded: "Debug data out of sync" lines | 19 (19) | 19 (19) |

- **decomplint** exits 1 under `--warnfail`, as in every sweep. The 5 added descents per target are the assert-pinned world-5d-5 ones in the Reorder table.
- **reccmp-datacmp:** the two new issues per target are `FFlag::FreeFallOptimization` 0x00d30efe and `FFlag::ImpulseSolverForContact` 0x00d30eff. These variables paired for the first time this wave through world-5d-5's KernelData code. The original's initial byte is 1, and Client/App/util/FastFlags.cpp:34–35 initialises them to `false`.
- **reccmp_addr_padding:** 1,669 files, 0 bad.
- **check_2016_copies:** silent.

## Saved

- Final JSONs:
  - build/sweep15/P-final.json (= PA5)
  - build-studio/sweep15/S-final.json (= SA5)
- Alongside them:
  - P/S-nolib.json and .txt, -vtable.txt, -datacmp.txt, -folded.txt
  - build/sweep15/PLAYER- and STUDIO-decomplint.txt
  - app-sweep.csv and row-*.csv
- Scored exe, pdb, map and pdbview: build/sweep15/scored/ and build-studio/sweep15/scored/.
- Every round's JSONs and gen logs: build*/sweep15/.
- Pre-edit IndexArray.h: build/sweep15/IndexArray.h.bak.
- Helper scripts: the sweep14 set, with paths moved to sweep15, in build/sweep15/.

## Files changed by me

- reccmp/: regenerated, +1,809 / −744 lines.
  - player-boost.csv, player-libdata.csv, player-symbols.csv
  - studio-libdata.csv, studio-stl.csv, studio-symbols.csv
- Client/App/include/Util/IndexArray.h: blank-line layout, plus one nameref.
- Client/App/include/Util/AsyncHttpQueue.h: one nameref.
- Marker words in Client/App, Base and Network, through app_sweep set/apply.
- The shared venv's reccmp, through tools/reccmp-patches/apply.sh (patch 0002).

## Unresolved

- **FastFlags.cpp:** `FFlag::FreeFallOptimization` and `FFlag::ImpulseSolverForContact` default to true in the original (initial byte 1), so FastFlags.cpp:34–35 should initialise them to `true`. Check the KernelData.h functions that read them after changing it.
- **Base, the per-.obj STL copies:**
  - The 2013 linker kept apart `push_back` 0x009b7020 and 0x009ba960, whose `_Insert_n` callees differ only in their EH handler.
  - Until they are reproduced, TaskScheduler disableThreads, getJobsInfo, getJobsByName and setThreadCount stay at 97–99% and STUB in PLAYER. In STUDIO, disableThreads and setThreadCount are at 100%; getJobsInfo and getJobsByName were already short in sweep14 (99.2% and 99.0%).
- **Client shells:** the engine-copy versus shell-copy twins (~DataModel, DataModel ctor, ~TaskScheduler, ~Job, computeStandardSleepTime, RunService::start, onChildAdded, HeartbeatTask, createAutoJoints) need the 2013 shell's own instantiations.
- **program_options /GL twin pairings (37 library and 7 engine entries).**
  - These need the original's two copies of each folded STL/boost body, the shell's LTCG copy and the engine's native one, to exist apart in our link, as with the trivial-body twins in CLAUDE.md's Gotchas.
  - `boost::any::holder<string>` D0 0x0047a530 also needs `~holder` out of line in the copy the engine pairs with.
- **reorder_markers:** type.cpp and LuaInstanceBridge.cpp are still "fixable" by plan but break the compile. Fixing them needs `--strict` runs or a manual move.
- **Agents' remaining items,** carried over unchanged:
  - base-pairing: Profiling Mark::~Mark, raiseHeartbeat, the volatile second atomic class, the Stats STUBs.
  - world-5d-5: FFlag::SymmetricContact, the JointInstance IJointOwner base, ContactManager::createContact pools, unwritten World and SleepStage bodies, STUDIO markers that would move asserts.
  - datamodel-5b-6: DataModelJob EH state, LocalBackpack/PlayerHUD RelativePanel, InsertService reflection block, PhysicsInstructions radius setters (Network owner), Rendering's IAdornable 0x18.
