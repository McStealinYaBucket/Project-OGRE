# integrate (workflow phase5-wave6), 2026-11-23

I integrated the three phase5-wave6 Work agents (settings-serialization, world-5d-3, script-5e-4), rebuilt both targets from clean, regenerated the CSVs, swept the markers and reordered one file. Both targets are stable at the end: two scorings of the final link agree exactly, and the last regeneration round gained nothing (PLAYER 9,893 three rounds running; STUDIO 9,637 two rounds running).

**Lost against sweep12: 4 entries per target, all explained below.** Two are twin pairing between identical `_Allocate` copies (the trade gains about 9 callers). Two are a COMDAT-selection effect that settings-serialization already reported. Every heavy command ran through tools/capped.sh, one build at a time, and nothing hit the cap. No commits, no IDA databases opened, nothing under Client/Rendering/ edited. (The machine was busy with an unrelated game for the first hour, which slowed the builds.)

## What I did

**Leftovers.** All three agents finished. All of their uncommitted work compiles in both targets from clean, so I kept all of it and reverted nothing. That includes the untracked Client/App/include/Voxel/CellChangeListener.h, CMakeLists.txt (the lapi.c/lstate.c include override), staging Lua patch 10 and Client/Network/Client.cpp.

**Clean rebuilds.**
- PLAYER: configured build/, ran `--target clean`, then a full build. 678 steps, 0 failures.
- STUDIO: the same in build-studio with `--target studio`. 673 steps, 0 failures.

**Link stubs.** I deleted each exe, ran gen_link_stubs per target, then relinked.
- PLAYER: 31 unresolved, 10 stub addresses (was 12): unknown 17, data 10, unparsed 4.
- STUDIO: the same counts. The 2 RobloxExtraSpace stubs (0x00924c30, 0x00926390) were dropped as now defined, which clears script-5e-4's STUDIO item. Client/_generated: −34 lines across 4 files.

**Regeneration rounds.** Each round pools the two latest runs of the current link (build/sweep13/rounds.sh). Figures are entries at effective 100%, and the first number in each target is the stale-CSV score.
- PLAYER: 9,708 → 9,816 → 9,863 → 9,863 → 9,883 → 9,890 → 9,891 → 9,892 → 9,893 → 9,893.
- STUDIO: 9,467 → 9,572 → 9,610 → 9,628 → 9,634 → 9,635 → 9,636 → 9,637 → 9,637.
- Pairing counts:

  | | By signature | Vftables by RTTI name |
  |---|---|---|
  | PLAYER | 1,334 | 3,271 of 7,333 |
  | STUDIO | 1,374 | 3,219 of 7,707 |

**Sweep.**
- `app_sweep set FUNCTION` flipped 594 PLAYER and 603 STUDIO markers. I rebuilt both, scored PSW and SSW (9,893 and 9,637, unchanged), then ran `apply`.
- Swept scope: PLAYER 1,167 FUNCTION + 49 FOLDED + 601 STUB; STUDIO 1,128 + 49 + 597.
- Against HEAD (Client/App, Base and Network): 19 PLAYER and 18 STUDIO markers promoted from STUB, **0 demoted**. New markers from the agents: PLAYER 17 FUNCTION + 6 STUB, STUDIO 16 + 5.
- stubcheck: no STUB marker outside Rendering sits on an address at effective 100%.
- The STUDIO flips script-5e-4 left to the sweep (the luaconf.h RobloxExtraSpace markers and the ThreadRef/ScriptContext twins) were applied wherever they measured 100%.

**Reorder.**
- `reorder_markers plan` found three fixable files:
  - type.cpp and LuaInstanceBridge.cpp: their reorders break the compile, as in every earlier wave, so I left them.
  - **Client/App/script/ThreadRef.cpp: applied.** The new `WeakThreadRef(const WeakThreadRef&)` 0x00871b90 moved ahead of createNewNode 0x00872340. Descents 12 → 11 in each target; the rest are pinned by assert lines.
- After that I rebuilt both targets (391 steps each) and scored each twice. No 100% was lost (9,893 / 9,637 held), and decomplint's ThreadRef.cpp count is back at sweep12's.
- New descents left pinned by their 2013 assert lines (plan: "pinned by assert lines"), all from this wave's line layouts:
  - ScriptContext.cpp: 22 → 24 (P) / 23 (S).
  - Kernel.cpp: 3 → 6 (P) / 4 (S).
  - Primitive.cpp: +1 in each target.
  - World.cpp went 8 → 7.

## Final figures (sweep12 in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| Entries at effective 100% | **9,893** (9,721) | **9,637** (9,479) |
| Non-library entries | 7,410 (7,242) | 7,194 (7,040) |
| **Functions at 100%** | **7,439** (7,323) | **7,220** (7,118) |
| **Non-library functions at 100%** | **5,023** (4,911) | **4,824** (4,726) |
| Functions at 100%: owner side / Rendering (minus g3d) / third party | 5,010 / 2 / 2,427 | 4,813 / 3 / 2,404 |
| Vftables at 100%: owner / Rendering / third party | 2,372 / 1 / 12 | 2,359 / 1 / 8 |
| Vftables at 100% | 2,385 / 3,358 (2,329 / 3,320) | 2,368 / 3,287 (2,312 / 3,249) |
| reccmp Implemented, with libraries | 9,222 / 40,548 (9,112) | 8,974 / 42,382 (8,870) |
| reccmp Implemented, `--nolib` | 6,544 / 37,830 (6,438) | 6,384 / 39,668 (6,283) |
| App markers: FUNCTION + FOLDED + STUB | 1,023 + 27 + 559 (HEAD 988 + 27 + 575) | 991 + 27 + 556 (958 + 27 + 574) |
| Base markers | 88 + 0 + 22 (88 + 0 + 22) | 81 + 0 + 24 (81 + 0 + 24) |
| Network markers | 53 + 1 + 8 (52 + 1 + 9) | 53 + 1 + 8 (52 + 1 + 9) |
| Sweep scope: markers at 100% / short | 1,216 / 601 | 1,177 / 597 |

- Gained against sweep12: 176 PLAYER and 162 STUDIO entries. Lost: 4 in each target (see Losses).
- Compared with the agents' own private-CSV figures, the integrated tree is above or level with each:

  | Agent | PLAYER | STUDIO |
  |---|---|---|
  | settings-serialization | 9,892 | 9,623 |
  | world-5d-3 | 9,867 | 9,532 |
  | script-5e-4 | 9,879 | not measured |

- **Counting method changed for the marker rows.** The App/Base/Network marker counts come from build/sweep13/markers.py, which counts `git ls-files` under each directory. The HEAD figures in brackets use the same method. They differ from the method of the sweep12 report (which gave App 988 + 39 + 580), so compare only within this table.
- The owner / Rendering / third-party split goes by the `.obj` the link map puts at each recompiled address (split.py). Library rows count as third party.

**Rows** (PLAYER; sweep12 measured on the same current ranges in brackets):

| Row | At 100% / total | Share | sweep12 |
|---|---|---|---|
| 5a | 138/155 | 89.0% | 138 |
| 5b | 614/1,660 | 37.0% | 612 |
| 5c | 87/152 | 57.2% | 86 |
| 5d | 314/1,528 | 20.5% | 293 |
| 5e (`--row 5e`, ScriptContext.obj from 0x0066fa60) | 480/1,335 | 36.0% | 454 |
| 5f | 538/3,280 | 16.4% | 532 |
| 5g (row_coverage_5g.py) | 311/601 | 51.7% | 309 |
| 5h | 512/1,359 | 37.7% | 511 |
| 5k | 545/3,493 | 15.6% | 546 |

- 5k's −1 is the `_Allocate` twin trade (0x00429b60 / 0x00429c80 lost, CRenderSettingsItem's item ctor 0x00456ee0 gained).
- 5e now uses script-5e-4's new row definition, so it is not comparable with the 310/848 in the sweep12 report.
- Per-function CSVs: build/sweep13/row-*.csv.

**Near misses** (app_sweep report, build/sweep13/app-sweep.csv):
- PLAYER, 601 short: inlining-size 184, ebp-frame 169, other 141, unpaired 48, unpaired-callee 29, x87-vs-sse 21, data-ref 4, assert-file-line 3, stack-cookie 2. 77 are at 90% or more; 33 are pairing-only.
- STUDIO, 597 short: inlining-size 181, ebp-frame 167, other 144, unpaired 37, unpaired-callee 36, x87-vs-sse 21, data-ref 4, assert-file-line 3, stack-cookie 4. 90 are at 90% or more; 40 are pairing-only.
- Closest (PLAYER):

  | Function | Address | Score |
  |---|---|---|
  | LegacyContentTable ctor | 0x008f08a0 | 99.97 |
  | ~DataModel | 0x0064d850 | 99.76 |
  | AsyncHttpQueue::asyncRequest | 0x00870430 | 99.59 |
  | ~TaskScheduler | 0x009b4e00 | 99.52 |
  | TaskScheduler::remove | 0x009b52b0 | 99.49 |
  | TextXmlWriter::serializeNode | 0x00628a60 | 99.39 |
  | RunService::start | 0x0072d300 | 99.37 |
  | MemoryBinder::processID | 0x006fc500 | 99.33 |
  | ~Job | 0x009ba1f0 | 99.28 |
  | Job::computeStandardSleepTime | 0x009b9a90 | 99.21 |
  | ServiceProvider::onChildAdded | 0x006a9770 | 99.19 |
  | World::createAutoJoints | 0x0074aa80 | 99.14 |

## Losses (address by address against sweep12's P-final / S-final, summ.py)

Each was read with `--verbose`.

1. **`std::_Allocate` twin, PLAYER 0x00429c80 / STUDIO 0x004791e0**, and its caller **boost `array_constructor<ptr_bucket>::construct`, PLAYER 0x00429b60 / STUDIO 0x004865c0** (96.4%: the only diff is the unpaired call).
   - The original keeps two identical 4-byte `_Allocate` bodies, 0x00429c80 and 0x005cee00. Our linker folds them, and now also `_Allocate<Contact*>` (world-5d-3's new instantiation), into one body at 0x0048a180. reccmp pairs one original per body, so only one twin can pair.
   - The regenerated libdata now pairs 0x005cee00 (`_Allocate<Contact*>` row). Its original callers include about 10 deque/vector helpers that are now at 100% and were at 99.3–99.6% in sweep12: vector<CallbackDescriptor*> 0x0058deb0, the timestamped_safe_queue deque 0x006341a0, vector<ChatMode> 0x0065ab00, deque<WaitingThread> 0x006758e0, deque<shared_ptr<ChatLine>> 0x00844a70, deque<AsyncRetryTask> 0x0086c670, vector<Instance*> 0x008bc330, deque<ContentProviderTask> 0x008ef7b0, among others (build/sweep13/callers.py).
   - Pairing 0x00429c80 instead would win back only array_constructor, so I kept the generator's choice.
2. **`Described<GlobalAdvancedSettings>::classDescriptor` PLAYER 0x005268b0 / STUDIO 0x00607940** and **`Described<LuaSettings>::classDescriptor` PLAYER 0x00788570 / STUDIO 0x00735cc0** (86.2%).
   - The original reads the class name through the extern (`mov eax,[RBX::sGlobalAdvancedSettings]`). Our kept COMDAT pushes the literal "GlobalSettings" directly.
   - The kept copy now comes from GlobalAdvancedSettings.cpp.obj, where the string is defined. In sweep12 it came from Network's Client.cpp.obj, through the inline settings ctors that settings-serialization moved out of line.
   - In 2013 the copy sits in the Network registration block (0x00525da0–0x005264a0: Described<Peer/Replicator/…/NetworkSettings/Settings>), so a Network TU that sees the class name only as an extern must instantiate it. This is a code change, not a pairing issue (see Unresolved).

No other entry lost 100% in either target. Some functions flip on no-.reloc coincidences: symbexec, luaV_execute, patchtestreg, IsOlderOrderedPacket 0x005c8930 and httpGetPostWinHttp 0x00826990. In this link they are all at 100% in both targets.

## Checks

| Check | PLAYER (sweep12) | STUDIO (sweep12) |
|---|---|---|
| decomplint: function_out_of_order | 287 (282) | 293 (291) |
| decomplint: duplicate_offset | 12 (12) | 4 (4) |
| reccmp-vtable: traceback | none | none |
| reccmp-vtable: "Problem with adjuster thunk" lines | 11 (11) | 13 (13) |
| reccmp-datacmp: traceback | none | none |
| reccmp-datacmp: variables | 2,472 (2,449) | 2,339 (2,316) |
| reccmp-datacmp: issues | 55 (56) | 58 (59) |
| check_folded: annotated addresses | 2,501 (2,462) | 2,351 (2,317) |
| check_folded: folding problems | 0 | 0 |

- **decomplint** exits 1 under `--warnfail`, as in every sweep. The descents added in this wave are the assert-pinned ones listed under Reorder.
- **check_folded** reports 19 "Debug data out of sync" lines, the same count as sweep12.
- **reccmp_addr_padding:** 1,665 files, 0 bad.
- **check_2016_copies:** silent.
- **Datacmp issue changes.**
  - Fixed: `RBX::sGeometryService` ("Geometry") and `RBX::Stats::sStatsService` ("Stats"), by settings-serialization.
  - New: one more `stored_vtable` row (boost::function manager/invoker pointers into `.text`). The regenerated libdata now pairs it, and reccmp reads its slots as "Unknown pointer" because the target has no .reloc. This is a pairing limit, not a code difference.

## Saved

- Final JSONs: build/sweep13/P-final.json (= P22) and build-studio/sweep13/S-final.json (= S20).
- Alongside them:
  - P/S-nolib.json, -vtable.txt, -datacmp.txt, -folded.txt
  - build/sweep13/PLAYER/STUDIO-decomplint.txt
  - app-sweep.csv and row-*.csv
- Scored exe, pdb, map and pdbview: build/sweep13/scored/ and build-studio/sweep13/scored/.
- Every round's JSONs and gen logs: build*/sweep13/.
- reccmp/ before regeneration: build/sweep13/reccmp-before/.
- Pre-reorder ThreadRef.cpp: build/sweep13/ThreadRef.cpp.bak.
- Helper scripts in build/sweep13/:
  - score.sh, gen.sh, round.sh, v.sh
  - rounds.sh (new: loops rounds until one gains nothing)
  - summ.py, split.py, stubcheck.py
  - markers.py (new: marker counts against a git revision)
  - callers.py (new: the original's rel32 call sites of an address, with their scores in two runs)

## Files changed

- reccmp/: regenerated, +706 / −207 lines.
  - player-libdata.csv, player-symbols.csv
  - studio-libdata.csv, studio-symbols.csv
  - studio-crt.csv and studio-stl.csv: one row each no longer resolves (`SchedulerBase::GetAnonymousScheduleGroup`, `~strstreambuf_29`); neither was at 100%.
- Client/_generated/{player,studio}/stubs_00.cpp and unresolved.txt: gen_link_stubs output.
- Marker words in Client/App, Base and Network, made by the sweep.
- Client/App/script/ThreadRef.cpp: reordered (one whole marked definition moved).
- No changes to tools/ or CMakeLists.txt by me.

## Unresolved

- **classDescriptor COMDAT selection** (settings or Network owner): `Described<GlobalAdvancedSettings>` and `Described<LuaSettings>::classDescriptor` (PLAYER 0x005268b0 / 0x00788570, STUDIO 0x00607940 / 0x00735cc0) need a copy instantiated in a Network TU that sees only the extern class-name strings. The 2013 copy sits in the Network registration block next to Described<NetworkSettings>/<Settings>. That TU must link before App's GlobalAdvancedSettings.cpp/LuaSettings.cpp.
- **`_Allocate` twins** 0x00429c80 / 0x005cee00 (STUDIO 0x004791e0 and its twin): only one can pair while our link folds them. The current choice is the better one; no action unless the original's two copies can be told apart.
- **Per-target compile define** (from settings-serialization): DataModel.obj's `util/object.h` spelling is PLAYER-only. It needs `/DOGRE_TARGET_PLAYER`-style defines from cmake/msvc9-wine.cmake, which I did not add in this pass.
- **type.cpp and LuaInstanceBridge.cpp** reorders still break the compile.
- **Assert-pinned descents** added this wave: ScriptContext.cpp +2/+1, Kernel.cpp +3/+1, Primitive.cpp +1/+1. They need the bodies rewritten to their 2013 shape before they can move.
- **From the agents' lists, still open:**
  - Settings and serialization: NetworkSettings ctor 47.4%, GlobalAdvancedSettings::singleton 38.1%, serializeNode 99.4%, ContentProvider::readContent.
  - Util owner: inline ExtentsInt32(); ConcurrencyValidator store order; ComputeProp.
  - Network: NetworkOwner::Unassigned pairing.
  - World: StepJointsStage's intrusive list.
  - Script: closeStates 96.3%; FFlag::AllowYieldInHooks belongs to ThreadRef.obj.
  - IDA owner: the GlobalAdvancedSettings::Item and `__TBB_LockByte` renames.
  - Rendering contributor: GfxBase's RenderSettings registration.
