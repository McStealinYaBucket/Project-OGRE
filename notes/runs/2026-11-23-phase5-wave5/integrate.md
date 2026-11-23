# integrate (workflow phase5-wave5), 2026-11-23

I integrated the three phase5-wave5 Work agents (base-shared, world-5d-2, script-5e-3), regenerated the CSVs, ran the sweep and reordered markers. Both targets are stable at the end: in the last round, two scorings of the same link agree exactly, and a regeneration round gains nothing. **Nothing was lost against sweep11 in either target.**

Every heavy command ran through tools/capped.sh, one build at a time. Nothing hit the cap. No commits, no IDA databases opened, nothing under Client/Rendering/ edited.

## What I did

**Leftovers.** All three agents finished. All of their uncommitted work compiles in both targets from clean, so I kept all of it and reverted nothing:
- 3rdparty/tbb-2.1 (untracked)
- the SpatialHashPrimitive.h deletion
- the luaconf.h change and its staging patch
- CMakeLists.txt (the TBB include root, and the Lua fork override removed)

**Clean rebuilds.**
- PLAYER: configured build/, ran `--target clean`, then a full build. 677 steps, 0 errors.
- STUDIO: the same in build-studio with `--target studio`. 672 steps, 0 errors.

**Link stubs.** I deleted each exe, ran gen_link_stubs per target, then relinked. The result is unchanged from sweep11:
- 12 stub addresses per target.
- 32 unresolved: 18 unknown, 10 data, 4 unparsed.
- Client/_generated did not change.

**Regeneration rounds.** Each round pools the two latest runs of the current link. The figures are entries at effective 100%; the first number in each target is the stale-CSV score.
- PLAYER: 9,408 → 9,630 → 9,702 → 9,713 → 9,719 → 9,721 → 9,721.
- STUDIO: 9,159 → 9,399 → 9,463 → 9,473 → 9,478 → 9,479.
- The first PLAYER run on the stale CSVs had Instance::fw 0x006beaa0 absent. base-shared dropped that marker as an ICF fold, and the regenerated rows pair it again.
- Pairing counts: PLAYER 1,330 by signature, 3,236 of 7,333 vftables by RTTI name. STUDIO 1,368 and 3,183 of 7,707.

**Sweep.**
- `app_sweep set FUNCTION` flipped 617 PLAYER and 614 STUDIO markers.
- Then I scored PSW and SSW (counts unchanged), ran `apply`, and ran `apply` again on the final scores (no change).
- Result, all swept dirs: PLAYER 1,131 FUNCTION + 49 FOLDED + 618 STUB; STUDIO 1,094 + 49 + 616.
- Against HEAD: 70 markers promoted STUB → FUNCTION, **0 demoted**.
- stubcheck: no STUB marker outside Rendering sits on an address at effective 100%.
- The flips the agents asked other owners for happened in the sweep: WeakThreadRef(lua_State*), the STUDIO ThreadRef functions and the World.obj templates.

**Reorder.**
- I ran `reorder_markers apply` on Client/App/script/ScriptContext.cpp: descents 25 → 22 in both targets. The remaining 22 are pinned by assert lines.
- After that I rebuilt both targets (388 steps each) and rescored. No 100% was lost; 9,721 / 9,479 held.
- Not reordered:
  - type.cpp and LuaInstanceBridge.cpp: their default plan breaks the compile, as in earlier waves, and `--strict` finds nothing fixable.
  - World.cpp (8), Primitive.cpp (14/12) and LuaLibrary.cpp (7): pinned by their 2013 assert lines.
  - ContactManager.cpp (2): the descents cross runs.

## Final figures (sweep11 baseline in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| Entries at effective 100% | **9,721** (9,405) | **9,479** (9,163) |
| Non-library entries | 7,242 (6,932) | 7,040 (6,731) |
| **Functions at 100%** | **7,323** (7,017) | **7,118** (6,812) |
| **Non-library functions at 100%** | **4,911** (4,611) | **4,726** (4,427) |
| Functions at 100%: owner side / Rendering (minus g3d) / third party | 4,897 / 2 / 2,424 | 4,716 / 3 / 2,399 |
| Vftables at 100%: owner / Rendering / third party | 2,317 / 1 / 11 | 2,304 / 1 / 7 |
| Vftables at 100% | 2,329 / 3,320 (2,319 / 3,307) | 2,312 / 3,249 (2,302 / 3,236) |
| reccmp Implemented, with libraries | 9,112 / 40,548 (8,871) | 8,870 / 42,382 (8,637) |
| reccmp Implemented, `--nolib` | 6,438 / 37,834 (6,201) | 6,283 / 39,670 (6,055) |
| App markers: FUNCTION + FOLDED + STUB | 988 + 39 + 580 (938 + 39 + 607) | 958 + 39 + 577 (918 + 39 + 594) |
| Base markers: FUNCTION + FOLDED + STUB | 88 + 0 + 22 (84 + 0 + 26) | 81 + 0 + 24 (76 + 0 + 29) |
| Network markers: FUNCTION + FOLDED + STUB | 52 + 2 + 9 | 52 + 2 + 9 |
| Sweep scope: markers at 100% / short | 1,180 / 618 | 1,143 / 616 |

- Gained against sweep11: 316 in each target. Lost: 0 in each.
- The agents' own private-CSV figures were: base-shared 9,712 / 9,460, world-5d-2 9,627 (PLAYER), script-5e-3 9,604 / 9,394. The integrated tree is above each.
- The owner / Rendering / third-party split goes by the `.obj` the link map puts at each recompiled address (build/sweep12/split.py). Library-flagged rows count as third party.

**Rows** (PLAYER, current ranges; sweep11 on the same ranges in brackets):

| Row | At 100% / total | Share | sweep11 |
|---|---|---|---|
| 5a | 138/155 | 89.0% | 138 |
| 5b | 612/1,660 | 36.9% | 574 |
| 5c | 86/152 | 56.6% | 86 |
| 5d | 292/1,527 | 19.1% | 252 |
| 5e (Client/App/script/*.cpp) | 310/848 | 36.6% | 297 |
| 5f | 532/3,259 | 16.3% | 498 |
| 5g (row_coverage_5g.py) | 309/601 | 51.4% | 301 |
| 5h | 511/1,359 | 37.6% | 496 |
| 5k | 546/3,493 | 15.6% | 511 |

- 5b and 5d include the new totals widened by this wave's markers (5b 1,653 → 1,660; 5d 1,504 → 1,527).
- 5e understates the gain. The row tool takes ScriptContext.obj from 0x00682220, but it starts at 0x0066fa60. As script-5e-3 reported, getGlobalState, closeState, setThreadIdentity and spawn are therefore not counted.
- Per-function CSVs: build/sweep12/row-*.csv.

**Near misses** (app_sweep report, build/sweep12/app-sweep.csv):
- PLAYER, 618 short:
  - By shape: inlining-size 190, ebp-frame 171, other 150, unpaired 48, unpaired-callee 28, x87-vs-sse 21, data-ref 4, assert-file-line 4, stack-cookie 2.
  - 76 are at 90% or more; 32 are pairing-only.
- STUDIO, 616 short:
  - By shape: inlining-size 190, ebp-frame 169, other 153, unpaired 37, unpaired-callee 34, x87-vs-sse 21, data-ref 4, assert-file-line 4, stack-cookie 4.
  - 88 are at 90% or more; 38 are pairing-only.
- Closest (PLAYER):

  | Function | Address | Score |
  |---|---|---|
  | LegacyContentTable ctor | 0x008f08a0 | 99.97 |
  | ~DataModel | 0x0064d850 | 99.76 |
  | AsyncHttpQueue::asyncRequest | 0x00870430 | 99.59 |
  | ~TaskScheduler | 0x009b4e00 | 99.52 |
  | TaskScheduler::remove | 0x009b52b0 | 99.49 |
  | RunService::start | 0x0072d300 | 99.37 |
  | MemoryBinder::processID | 0x006fc500 | 99.33 |
  | ~Job | 0x009ba1f0 | 99.28 |
  | Job::computeStandardSleepTime | 0x009b9a90 | 99.21 |
  | ServiceProvider::onChildAdded | 0x006a9770 | 99.19 |

  STUDIO World::World 0x0088b780 is at 99.12, with its signal ctor twin unpaired.

## Losses

**None in either target**, compared address by address with build/sweep11/P-final.json and build-studio/sweep11/S-final.json (summ.py).

Some functions flip on no-.reloc coincidences. In this link they all happen to be at 100% in both targets:
- symbexec, luaV_execute and patchtestreg, which base-shared and world-5d-2 had seen flip.
- IsOlderOrderedPacket and httpGetPostWinHttp.

Expect them to flip again on a later relink.

These are still short of 100%, as in sweep11; none was ever at 100%, so none is a loss:
- SendPhysics::onMovingAssemblyRootAdded/Removing 0x008e5fe0 / 0x008e60b0 (98.4 / 95.5%): pairing churn from our folded `signal_with_args` twin.

## Checks

| Check | PLAYER (sweep11) | STUDIO (sweep11) |
|---|---|---|
| decomplint: function_out_of_order | 282 (276) | 291 (285) |
| decomplint: duplicate_offset | 12 (12) | 4 (4) |
| reccmp-vtable: traceback | none | none |
| reccmp-vtable: "Problem with adjuster thunk" lines | 11 (11) | 13 (13) |
| reccmp-datacmp: traceback | none | none |
| reccmp-datacmp: variables | 2,449 (2,252) | 2,316 (2,136) |
| reccmp-datacmp: issues | 56 (55) | 59 (58) |
| check_folded: annotated addresses | 2,462 (2,429) | 2,317 (2,284) |
| check_folded: folding problems | 0 | 0 |

- **decomplint** exits 1 under `--warnfail`, as in every sweep. The +6 per target, by file:
  - LuaLibrary.cpp 1 → 7: assert-pinned, from script-5e-3's line layout.
  - ScriptContext.cpp 26 → 22: my reorder.
  - Primitive.cpp +1 and World.cpp 5 → 8: assert-pinned, per world-5d-2.
- **check_folded** reports 19 "Debug data out of sync" lines, the same set as sweep11; Name.h:35 is now Name.h:36.
- **reccmp_addr_padding:** 1,664 files, 0 bad.
- **check_2016_copies:** silent.
- **Datacmp issue changes.**
  - Fixed: sGlobalAdvancedSettings and sGlobalBasicSettings (base-shared's "GlobalSettings" / "UserSettings").
  - New issues are real source differences that the regenerated libdata now pairs:
    - `RBX::sGeometryService` (PLAYER 0xb79530) is "Geometry" in 2013; ours is "GeometryService". Source: v8datamodel/GeometryService.cpp:9.
    - `RBX::Stats::sStatsService` (0xb509d8) is "Stats" in 2013; ours is "StatsService". Source: v8datamodel/StatsService.cpp:10.
    - `FLog::ScriptContextRemove` (0xde3f7a): the LOGVARIABLE data pairing script-5e-3 reported.

## Saved

- Final JSONs: build/sweep12/P-final.json (= P21) and build-studio/sweep12/S-final.json (= S18).
- Alongside them:
  - P/S-nolib.json, -vtable.txt, -datacmp.txt, -folded.txt
  - build/sweep12/PLAYER/STUDIO-decomplint.txt
  - app-sweep.csv and row-*.csv
- Scored exe, pdb, map and pdbview: build/sweep12/scored/ and build-studio/sweep12/scored/.
- Every round's JSONs and gen logs: build*/sweep12/.
- reccmp/ before regeneration: build/sweep12/reccmp-before/.
- Helper scripts: build*/sweep12/{score,gen,round,v}.sh, summ.py, split.py, stubcheck.py.
- Pre-reorder ScriptContext.cpp: build/sweep12/ScriptContext.cpp.bak.

## Files changed

- reccmp/player-{libdata,raknet,symbols}.csv and reccmp/studio-{libdata,raknet,symbols}.csv, regenerated: +1,286 / −165 lines.
- Marker words in 55 files under Client/App, Base and Network, made by the sweep.
- Client/App/script/ScriptContext.cpp: reordered (whole marked definitions only).
- No changes to tools/, CMakeLists.txt or Client/_generated.

## Unresolved

- **2013 class-name strings** (v8datamodel owner): sGeometryService = "Geometry" (GeometryService.cpp:9) and Stats::sStatsService = "Stats" (StatsService.cpp:10). Check them against the RTTI and the iOS 10-29 strings first.
- **tools/row_coverage.py**: row 5e's ScriptContext.obj range should start at 0x0066fa60, not 0x00682220. Four ScriptContext matches are not counted.
- **Pairing churn from our linker's folds** (unchanged from sweep11):
  - `signal_with_args<1,void(Primitive*)>`: SendPhysics::onMovingAssemblyRootAdded/Removing.
  - `shared_from<T>`: RunService::start 0x0072d300.
  - STUDIO World::World 0x0088b780: its signal ctor twin 0x00887780 does not pair.
- **No-.reloc coincidences** to recheck on every relink: symbexec, luaV_execute, patchtestreg and three more Lua functions; IsOlderOrderedPacket 0x005c8930; httpGetPostWinHttp 0x00826990.
- **type.cpp and LuaInstanceBridge.cpp** reorders still break the compile; `--strict` finds nothing fixable.
- From the agents' lists, still open:
  - **ScriptContext:** `++timedoutCount` through RbxInterlockedIncrement (onCheckTimeout 98.2%); `GlobalAdvancedSettingsItem<LuaSettings>::singleton` 0x00654600 is not modelled; ypcall frame.
  - **The other settings items** still derive GlobalAdvancedSettings: Debug, Game, Network, TaskScheduler, Render, Physics and FastLog.
  - **Serialization:** serializeNode 0x00628a60 and SerializerV2::load 0x006fd070 need the new ContentId and readContent signatures.
  - **DataModel.obj:** the Object.h include spelling, for isNullClassName<sContentFilter> 0x00632290 at 95.2%.
  - **Layouts:** SleepStage 0x14c, ContactManagerSpatialHash 0xdc, Primitive ctor/dtor, and SpanningNode::~SpanningNode inline.
  - **Placement:** Profiling::CodeProfiler and Voxel::CellChangeListener belong in Util and the voxel headers.
  - **Kernel.cpp and World.cpp:** assert lines.
  - **Lua TUs:** the typed RobloxExtraSpace, so the lstate.obj ctor and dtor can match.
  - **IDA owner:** the FactoryProduct<LuaSettings> → GlobalAdvancedSettings::Item renames (PLAYER 0x0085db70/0x0085dc20, STUDIO 0x009481a0/0x00948250).
