# integrate (workflow phase5-wave9), 2026-11-24

I integrated the three phase5-wave9 Work agents: shell-twins (shell-base), world-5d-6 and script-5e-5. All three finished, and their uncommitted work compiles in both targets from clean, so nothing was reverted.

I also took the integrator item from shell-base's Unresolved list: the generator now pairs the shell and native twins (see Generator).

Then I rebuilt both targets from clean, regenerated the CSVs until a round gained nothing, swept the markers and ran every check. Both targets are stable:
- PLAYER held **10,909** across Q1–Q4 and PA1–PA4.
- STUDIO held **10,373** across T3–T6 and SA1–SA4.

| | sweep15 | Lost | Gained |
|---|---|---|---|
| PLAYER | 10,729 | 15 | 195 |
| STUDIO | 10,192 | 9 | 190 |

Every loss was read with `--verbose`; none has a code cause in this wave's source (see Losses).

- Every heavy command ran through tools/capped.sh, one build at a time, and nothing hit the cap.
- There are no commits, no IDA database was opened, and nothing under Client/Rendering/ was edited.
- The 2016-copy check is silent.

## What I did

**Clean rebuilds**
- PLAYER: build/ took 680 steps with 0 failures.
- STUDIO: build-studio/ with `--target studio` took 675 steps with 0 failures.
- Both caches have `OGRE_BOOST_PO_GL=ON`.
- I ran `app_sweep set FUNCTION` before the STUDIO build and before the PLAYER relink, so every scoring run below saw every in-scope marker. The flip touched 571 PLAYER and 570 STUDIO markers.

**Link stubs.** I deleted each exe first, then ran gen_link_stubs on each target.
- Each target now has 30 unresolved symbols (was 31). `Variant::convert<shared_ptr<const ValueMap>>` dropped out, as script-5e-5 expected.
- Stubs are unchanged: 11 addresses each. Only `Client/_generated/{player,studio}/unresolved.txt` changed.

**Reorder.** `reorder_markers plan` found one fixable file: Client/Base/rbx/TaskScheduler.cpp.
- I applied it. `getShortestSleepTime` moved above `sampleRunningJobCount`; no assert line moved.
- Descents went from 7 to 6 in both targets.
- Every other file with descents is pinned by asserts, has a studio-only chunk, or has descents that cross runs.

**Generator (tools/ida/gen_reccmp_csv.py), shell/native twins.** This was shell-base's integrator item.
- **Why the twins never paired:** the previous CSV had paired the original's shell copy with our native body:
  - `~shared_ptr` 0x00405b50 with our `~shared_ptr<spin_mutex>` 0x005d34d0
  - `_Destroy_range` 0x00427f30 with our `_Destroy_range<GuiBase>` 0x00685070

  A reccmp run then scored those identical bodies at 100%, and that pair counted as 1000 votes. So the regeneration kept the pair, against 253 call sites that name the shell body of ours (0x00428a60). The native twins 0x007e4e40 and 0x00829d90 stayed unpaired.
- **Fix 1, in data_rows:** a 100% body vote now lapses when another body of ours has more call-site votes for that original than this body has. A 100% body pair proves two bodies are identical, not which twin is which.
- **Fix 2, the match: rule:** a stale `match:` IDA name at the shell copy (0x00405b50, spin_mutex) no longer beats a position row when another position row gives that symbol to an identical twin.
- `OGRE_VOTES=hex,hex` prints the votes for those addresses (ours or original) to stderr, for debugging.
- I added both rules to the docstring. `--selftest` passes.
- **Effect** (same link, same evidence, fully regenerated):

  | | Old generator | New generator |
  |---|---|---|
  | PLAYER | 10,831 (P13/P14) | 10,909 |
  | STUDIO | 10,304 (S13/S14) | 10,373 |

  That is **+78 PLAYER and +69 STUDIO**. It recovered the 13 native `~shared_ptr` callers and the `_Destroy_range` callers that the stand-in had cost (closeState, printCallStack, the TGenericSlotWrapper pair, ContentProviderJob, TimerService::onHeartbeat, and others).

**Regeneration rounds.** Each round pools the two latest runs of the current link (build/sweep16/rounds.sh). Figures are entries at effective 100%:
- **PLAYER**, old generator: 10,701 → 10,790 → 10,815 → 10,827 → 10,830 → 10,831 → 10,831.
- **PLAYER**, new generator: Q 10,909 → 10,909.
- **PLAYER**, after apply: PA1–PA4 10,909.
- **STUDIO**, old generator: 10,168 → 10,260 → … → 10,304 (S11–S14).
- **STUDIO**, new generator: T 10,320 → 10,373 → 10,373.
- **STUDIO**, after apply: SA1–SA4 10,373.

**Sweep.** I ran `app_sweep apply Q4.json T6.json`. In the swept scope:

| | FUNCTION | FOLDED | STUB |
|---|---|---|---|
| PLAYER | 1,341 | 49 | 558 |
| STUDIO | 1,294 | 49 | 560 |

Against HEAD, in Client/App, Base and Network (build/sweep16/markers.py HEAD):

| | New FUNCTION | New STUB | Promoted | Demoted |
|---|---|---|---|---|
| PLAYER | 21 | 9 | 35 | 4 |
| STUDIO | 21 | 7 | 31 | 5 |

- **PLAYER demotions:**
  - ScriptContext::setTimeout 0x0067eee0
  - httpGetPostWinHttp 0x00826990
  - InsertService::insert 0x007d6a20
  - ArchiveBinder::processID 0x006fb300
- **STUDIO demotions:**
  - setTimeout 0x007e4db0
  - RunService::start 0x0079af80
  - ArchiveBinder::processID 0x00749890
  - TaskScheduler::disableThreads 0x00a6f8f0
  - Job::computeStandardSleepTime 0x00a70ab0
- All nine are among the losses below. stubcheck finds no STUB marker outside Rendering on an address at effective 100%.

## Final figures (sweep15 in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| **Entries at effective 100%** | **10,909** (10,729) | **10,373** (10,192) |
| Non-library entries | 8,332 (8,164) | 7,903 (7,750) |
| **Functions at 100%** | **8,161** (7,983) | **7,825** (7,650) |
| **Non-library functions at 100%** | **5,651** (5,485) | **5,402** (5,255) |
| Functions at 100%: owner side / Rendering (minus g3d) / third party | 5,639 / 2 / 2,520 (5,474 / 2 / 2,507) | 5,391 / 3 / 2,431 (5,242 / 3 / 2,405) |
| Vftables at 100%: owner / Rendering / third party | 2,666 / 1 / 12 (2,665 / 1 / 11) | 2,490 / 1 / 8 (2,485 / 1 / 7) |
| Vftables at 100% | 2,679 / 3,512 (2,677 / 3,493) | 2,499 / 3,449 (2,493 / 3,430) |
| reccmp Implemented, with libraries | 9,812 / 40,550 (9,680) | 9,489 / 42,383 (9,350) |
| reccmp Implemented, `--nolib` | 7,040 / 37,739 (6,925 / 37,756) | 6,864 / 39,635 (6,761 / 39,672) |
| App markers: FUNCTION + FOLDED + STUB | 1,192 + 27 + 518 (1,146 + 27 + 535) | 1,151 + 27 + 522 (1,107 + 27 + 539) |
| Base markers | 92 + 0 + 19 (86 + 0 + 24) | 86 + 0 + 20 (83 + 0 + 22) |
| Network markers | 54 + 1 + 9 (unchanged) | 54 + 1 + 9 (unchanged) |

**Agents' private figures, against the integrated tree:**

| Agent | PLAYER | STUDIO |
|---|---|---|
| shell-base | 10,815 | 10,268 |
| world-5d-6 | 10,819 | 10,227 |
| script-5e-5 | 10,825 | 10,302 |
| **Integrated** | **10,909** | **10,373** |

**Rows** (PLAYER, tools/row_coverage.py on PA1 = P-final; per-function CSVs in build/sweep16/row-*.csv)

| Row | At 100% / total | Share | sweep15 |
|---|---|---|---|
| 5a | 138/155 | 89.0% | 138 |
| 5b | 731/1,927 | 37.9% | 702 |
| 5c | 89/152 | 58.6% | 88 |
| 5d | 465/1,554 | 29.9% | 421 |
| 5e | 487/1,335 | 36.5% | 479 |
| 5f | 597/3,280 | 18.2% | 584 |
| 5g (row_coverage_5g.py) | 323/609 | 53.0% | 324 |
| 5h | 527/1,359 | 38.8% | 525 |
| 5k | 697/3,493 | 20.0% | 664 |

- **5e:** now 487, against script-5e-5's 475. The generator fix recovered its `~shared_ptr` twin losses.
- **5g:** lost two and gained one:
  - Lost `httpGetPostWinHttp` 0x00826990 (an immediate coincidence) and the static `RBX::callback` 0x0086f850 (no longer paired). Both are covered under Losses.
  - Gained AsyncHttpQueue::asyncRequest 0x00870430 (99.6 → 100).

**Near misses** (app_sweep report, build/sweep16/app-sweep.csv):

| Target | Short | inlining-size | ebp-frame | other | unpaired | unpaired-callee | x87-vs-sse | assert-file-line | stack-cookie | data-ref | At 90% or more | Pairing-only |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| PLAYER | 558 | 169 | 163 | 129 | 49 | 20 | 20 | 5 | 2 | 1 | 62 | 21 |
| STUDIO | 560 | 168 | 161 | 133 | 38 | 31 | 20 | 5 | 3 | 1 | 81 | 32 |

Closest (PLAYER):

| Function | Address | Score | Pairing-only? |
|---|---|---|---|
| TaskScheduler::remove | 0x009b52b0 | 99.49 | yes |
| XmlNameValuePair::toString | 0x00614a30 | 99.42 | yes |
| ScriptContext::setTimeout | 0x0067eee0 | 99.41 | yes |
| TextXmlWriter::serializeNode | 0x00628a60 | 99.39 | no |
| httpGetPostWinHttp | 0x00826990 | 99.32 | no (immediates) |
| TaskScheduler::setThreadCount | 0x009b7ec0 | 99.26 | yes |
| TaskScheduler::getJobsInfo | 0x009b75f0 | 99.21 | yes |
| World::createAutoJoints | 0x0074aa80 | 99.14 | yes |
| GuiService::openBrowserWindow | 0x0069a540 | 99.08 | yes |
| TaskScheduler::getJobsByName | 0x009b78f0 | 98.99 | yes |
| DebrisService::addItem | 0x00845000 | 98.98 | yes |
| TaskScheduler ctor | 0x009b55e0 | 98.88 | no |
| World::destroyAutoJoints | 0x00749340 | 98.84 | no |

- ~DataModel, the DataModel ctor, ~TaskScheduler, ~Job, RunService::start (PLAYER), ServiceProvider::onChildAdded and HeartbeatTask have left this list: they are now at 100%.

## Losses (against sweep15's P-final / S-final, summ.py, each read with --verbose)

Lists are in build/sweep16/loss-Q4.txt and build-studio/sweep16/loss-T6.txt. P-final = PA4 and S-final = SA4 have the same sets.

**PLAYER: 15 losses.**

- **8 no-.reloc immediate coincidences.** None has a code change:
  - Lua: luaV_execute 0x0098ad00, symbexec 0x008a8330, constructor 0x0098f9a0, patchtestreg 0x009a6800, removevalues 0x009a6880, luaK_setreturns 0x009a6c30.
  - RakNet: ReliabilityLayer::IsOlderOrderedPacket 0x005c8930.
  - httpGetPostWinHttp 0x00826990.
  - Each diff line is `test/and reg, <OFFSETn>`, or a character immediate (`"R"`, `"P"`) read as a string address. Our link layout now puts a named entity at the immediate's value. Example: Lua's `test edi, 0x7fc000` hits our `??_R1A@?0A@EA@?$NonFactoryProduct<PartInstance,sFormFactorPart>` base class descriptor at 0x007fc000.
  - CLAUDE.md Gotchas expects this churn. These come back as the layout moves.
- **6 pairing-only, one twin callee each.** The original keeps a shell and a native copy of each of these callees, and our link pairs the other copy:

  | Function | Address | Callee paired to the other twin |
  |---|---|---|
  | `~vector<shared_ptr<Instance>>` | 0x00535280 | `_Destroy_range<Instance>` / `<GuiBase>` |
  | ScriptContext::setTimeout | 0x0067eee0 | `shared_from<DataModel>` / `<GuiItem>` |
  | ArchiveBinder::processID | 0x006fb300 | `map<string, shared_ptr<Script>>` / `map<string, InstanceHandle>` |
  | InsertService::insert | 0x007d6a20 | `~vector<shared_ptr<thread>>` / `~vector<shared_ptr<Instance>>` |
  | `unchecked_copy<shared_ptr<Instance>*>` | 0x008b3170 | `_Copy_opt` |
  | `callable<signal<void(shared_ptr<Instance>)>::slot>` | 0x0093d4a0 | `function1<…>::operator()` |

  These callees are mixed-ownership symbols the generator fix cannot separate: our body serves symbols that the 2013 shell owns and symbols it does not.
- **1 unpaired static: `RBX::callback` 0x0086f850 (AsyncHttpQueue.cpp:311).** sweep15 paired it with a position row, voted by AsyncHttpQueue::processRequests 0x0086f9d0, which pushes its address at 0x0086ffda. processRequests is at 88.6% and no longer aligned, so no vote reaches it. The oracle row cannot pair it either, because two statics in our PDB are named `RBX::callback`: this one and WinInet.cpp's.

**STUDIO: 9 losses, all pairing-only.**
- ArchiveBinder::processID 0x00749890, setTimeout 0x007e4db0, `callable<…>` 0x008f7c80 and `unchecked_copy` 0x00987a70: the same callees as in PLAYER.
- RunService::start 0x0079af80: `shared_from<DataModel>` / `<RunService>`.
- `vector<shared_ptr<Instance>>::push_back` 0x00987d60: `_Uninit_fill_n` and `_Insert_n` paired to the `shared_ptr<thread>` twins.
- TaskScheduler::disableThreads 0x00a6f8f0: `push_back<Instance>` / `<Coordinator>`. This is the stand-in's Coordinator entry, which is PLAYER-only.
- Job::computeStandardSleepTime 0x00a70ab0: `~exception_ptr` / `~shared_ptr<Players>`. This is the STUDIO side of the same twin pair.
- `RBX::callback` 0x009bc9e0: as in PLAYER.

## Checks

| Check | PLAYER (sweep15) | STUDIO (sweep15) |
|---|---|---|
| decomplint: function_out_of_order | 307 (305) | 313 (312) |
| decomplint: duplicate_offset | 12 (12) | 4 (4) |
| reccmp-vtable: traceback | none | none |
| reccmp-vtable: "Problem with adjuster thunk" lines | 11 (11) | 13 (13) |
| reccmp-datacmp: traceback | none | none |
| reccmp-datacmp: variables | 2,619 (2,588) | 2,478 (2,447) |
| reccmp-datacmp: issues | 55 (57) | 58 (60) |
| check_folded: annotated addresses | 2,687 (2,649) | 2,528 (2,499) |
| check_folded: folding problems | 0 | 0 |
| check_folded: "Debug data out of sync" lines | 19 (19) | 19 (19) |

- **decomplint** exits 1 under `--warnfail`, as in every sweep. Descents changed in these files:

  | File | Change | Cause |
  |---|---|---|
  | type.cpp | +1 | script-5e-5's 2013 assert layout; the plan reports it pinned |
  | SpanningEdge.cpp | +1 | world-5d-6's 2013 assert layout; pinned |
  | Joint.cpp | +1 (PLAYER only) | world-5d-6's 2013 assert layout; pinned |
  | World.cpp | +2 | world-5d-6's 2013 assert layout; pinned |
  | LuaInstanceBridge.cpp | −2 | script-5e-5's reorder |
  | TaskScheduler.cpp | −1 | my reorder |
- **reccmp-datacmp:** issues fell by 2 per target. `FFlag::FreeFallOptimization` and `FFlag::ImpulseSolverForContact` now default to true (shell-base).
- **reccmp_addr_padding:** 1,669 files, 0 badly padded.
- **check_2016_copies:** silent.

## Saved

- Final JSONs:
  - build/sweep16/P-final.json (= PA4)
  - build-studio/sweep16/S-final.json (= SA4)
- Alongside them:
  - P/S-nolib.json and .txt, -vtable.txt, -datacmp.txt, -folded.txt
  - build/sweep16/{PLAYER,STUDIO}-decomplint.txt
  - app-sweep.csv, row-*.csv, loss-*.txt
- Scored exe, pdb, map and pdbview: build/sweep16/scored/ and build-studio/sweep16/scored/.
- Every round's JSONs and gen logs: build*/sweep16/.
- Generator debug logs: gen-dbg*.log.
- Pre-reorder TaskScheduler.cpp: build/sweep16/TaskScheduler.cpp.bak.
- Helper scripts: the sweep15 set with paths moved to sweep16, in build/sweep16/.

## Files changed by me

- tools/ida/gen_reccmp_csv.py: the twin vote rule, the match: swap rule, `OGRE_VOTES` and the docstring (+37 lines).
- reccmp/: regenerated:
  - player-libdata.csv, player-symbols.csv
  - studio-libdata.csv, studio-symbols.csv
  - About +590 / −135 lines.
- Client/Base/rbx/TaskScheduler.cpp: reorder_markers apply (one function moved).
- Client/_generated/{player,studio}/unresolved.txt, through gen_link_stubs.
- Marker words in Client/App, Base and Network, through app_sweep set/apply.

## Unresolved

- **Mixed-ownership twin callees** (the 6 PLAYER and 7 STUDIO pairing-only losses above). Our single body serves symbols that the 2013 shell owns and symbols it does not. The real shell classes, or more stand-in entries (`shared_from<DataModel>`, `map<string, shared_ptr<Script>>`, `_Copy_opt`, `function1<…shared_ptr<Instance>>::operator()`), would separate them. Each entry needs an A/B measurement, as shell-base did.
- **`RBX::callback`** (PLAYER 0x0086f850, STUDIO 0x009bc9e0) is unpaired. Possible fixes:
  - raise AsyncHttpQueue::processRequests 0x0086f9d0 (88.6%) until it aligns;
  - mark the static (`// FUNCTION:` once it scores);
  - teach the generator to pair a static by its oracle signature when two procs share the name.
- **no-.reloc coincidences:** 6 Lua functions, IsOlderOrderedPacket and httpGetPostWinHttp. These are layout luck and should not be chased.
- **The agents' other items, carried over unchanged:**
  - shell-base:
    - the per-.obj signal operator() copies (raiseHeartbeat);
    - the TaskScheduler ctor's empty-temporary stores and `function0::assign_to<bind_t<TaskScheduler>>`;
    - the Thread.obj push_back chain;
    - the real shell classes.
  - world-5d-6:
    - Rendering's IAdornable float, which Client/Rendering owns and destroyAutoJoints needs;
    - the setMeInParent register tail;
    - the SleepStage::traverse helpers;
    - the unwritten World and SleepStage step bodies;
    - the STUDIO allocator and contact ctor rows;
    - the Contact(p0,p1) and PolyContact bases;
    - JointInstance.h for the datamodel owner to review.
  - script-5e-5:
    - the type_holder layout (Base);
    - `Name::lookup(const char* const&)`;
    - `Type::tag` as a `const Name&` (XmlSerializer.cpp:537);
    - startScript's try/catch;
    - callCallback's placement;
    - reflection_property.cpp's assert lines.
- **For CLAUDE.md (owner's call):**
  - The generator rule: a 100% body vote yields to call-site votes for twins.
  - Wait loops: a `pgrep -f` wait loop matches its own bash command line, so use a pid or a file, not a pattern.
