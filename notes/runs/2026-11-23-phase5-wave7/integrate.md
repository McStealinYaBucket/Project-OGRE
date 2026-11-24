# integrate (workflow phase5-wave7), 2026-11-23/24

I integrated the three phase5-wave7 Work agents (util-target, world-5d-4, datamodel-5b-5). Then I rebuilt both targets from clean, regenerated the CSVs until a round gained nothing, swept the markers and reordered one file. Both targets are stable at the end:

- The last regeneration round, run on the final link, gained nothing.
- PLAYER held 10,076 across seven scorings: P11/P12, PSW, P13 and P14–P16.
- STUDIO held 9,803 across seven scorings: S9/S10, SSW, S11 and S12–S14.

**Against sweep13, 1 entry is lost per target, and it is explained below:** the `TypedStatsItem<int>` vftable that util-target already reported.

Every heavy command ran through tools/capped.sh, one build at a time, and nothing hit the cap. There are no commits. I opened no IDA database and edited nothing under Client/Rendering/.

## What I did

**Leftovers.** All three agents finished. All of their uncommitted work compiles in both targets from clean, so I kept it all and reverted nothing. That includes the untracked Client/App/v8datamodel/FactoryRegistration.cpp, the CMakeLists.txt per-target define, and the stand-in instantiations at the end of Network/Player.cpp.

**Clean rebuilds.** For each target I configured, ran `--target clean`, then did a full build:
- PLAYER in build/: 679 steps, 0 failures.
- STUDIO in build-studio/ with `--target studio`: 674 steps, 0 failures.

**Link stubs.** I deleted each exe, ran gen_link_stubs per target, then relinked.
- Each target: 32 unresolved, 11 stub addresses (was 10): unknown 17, data 10, unparsed 4, sig 1.
- The new stub is `RootInstance::publicInsertRaw`, PLAYER 0x007e33f0 / STUDIO 0x00789030, called by datamodel-5b-5's `InsertService::insert`.
- Client/_generated/{player,studio}/stubs_00.cpp: +8 lines each.

**Regeneration rounds.** Each round pools the two latest runs of the current link (build/sweep14/rounds.sh). Figures are entries at effective 100%:
- PLAYER: 9,934 with the stale CSVs (P0), then 9,936 → 10,025 → 10,058 → 10,074 → 10,076 → 10,076.
- STUDIO: 9,679 → 9,756 → 9,790 → 9,803 → 9,803.
- After the sweep and the reorder, one more round on the final link gained nothing in either target (P14/P15 → P16, S12/S13 → S14).

Pairing counts:

| | By signature | Vftables by RTTI name |
|---|---|---|
| PLAYER | 1,339 | 3,296 of 7,333 |
| STUDIO | 1,381 | 3,244 of 7,707 |

**Sweep.**
- `app_sweep set FUNCTION` flipped 585 PLAYER and 599 STUDIO markers.
- I rebuilt both and scored PSW and SSW: 10,076 and 9,803, the same as before the flip. Then I ran `apply`.
- Swept scope:

  | | FUNCTION | FOLDED | STUB |
  |---|---|---|---|
  | PLAYER | 1,247 | 49 | 592 |
  | STUDIO | 1,206 | 49 | 591 |

- Against HEAD, in Client/App, Base and Network (build/sweep14/markers.py HEAD):
  - Promoted from STUB: 28 PLAYER, 26 STUDIO.
  - **Demoted: 0.**
  - New markers from the agents: PLAYER 53 FUNCTION + 19 STUB, STUDIO 53 + 20.
- stubcheck: no STUB marker outside Rendering sits on an address at effective 100%.
- datamodel-5b-5 did not score STUDIO. The sweep promoted its STUDIO markers wherever they measured 100% and left the rest as STUB.

**Reorder.**
- `reorder_markers plan` found three fixable files:
  - type.cpp and LuaInstanceBridge.cpp: their reorders break the compile, as in every earlier wave, so I left them.
  - **Client/App/v8world/Contact.cpp: applied.** `Contact::onPrimitiveContactParametersChanged` 0x00851ae0 now comes before `BlockBlockContact::getConnector` 0x00851f40. Descents went from 3 to 2 in each target, the same as HEAD.
- After that I rebuilt both and scored P13 and S11. No 100% was lost (10,076 / 9,803 held).
- New descents left pinned by their 2013 assert lines (plan: "pinned by assert lines"). All of them come from this wave's line layouts. decomplint per-file counts against sweep13, PLAYER/STUDIO:

  | File | sweep13 | Now |
  |---|---|---|
  | Profiling.cpp | 0/0 | 4/4 |
  | Kernel.cpp | 6/4 | 8/6 |
  | Assembly.cpp | 2/2 | 3/3 |
  | EdgeBuffer.cpp | 2/2 | 4/4 |
  | Joint.cpp | 7/7 | 8/8 |
  | MovingAssemblyStage.cpp | 4/4 | 5/5 |
  | Primitive.cpp | 15/13 | 16/15 |
  | World.cpp | 7/7 | 8/8 |

## Final figures (sweep13 in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| Entries at effective 100% | **10,076** (9,893) | **9,803** (9,637) |
| Non-library entries | 7,593 (7,410) | 7,359 (7,194) |
| **Functions at 100%** | **7,605** (7,439) | **7,368** (7,220) |
| **Non-library functions at 100%** | **5,189** (5,023) | **4,971** (4,824) |
| Functions at 100%: owner side / Rendering (minus g3d) / third party | 5,178 / 2 / 2,425 (5,010 / 2 / 2,427) | 4,959 / 3 / 2,406 (4,813 / 3 / 2,404) |
| Vftables at 100%: owner / Rendering / third party | 2,389 / 1 / 12 (2,372 / 1 / 12) | 2,377 / 1 / 8 (2,359 / 1 / 8) |
| Vftables at 100% | 2,402 / 3,375 (2,385 / 3,358) | 2,386 / 3,312 (2,368 / 3,287) |
| reccmp Implemented, with libraries | 9,355 / 40,550 (9,222) | 9,104 / 42,383 (8,974) |
| reccmp Implemented, `--nolib` | 6,677 / 37,832 (6,544) | 6,513 / 39,670 (6,384) |
| App markers: FUNCTION + FOLDED + STUB | 1,100 + 27 + 551 | 1,066 + 27 + 551 |
| Base markers | 90 + 0 + 20 (88 + 0 + 22) | 83 + 0 + 22 (81 + 0 + 24) |
| Network markers | 54 + 1 + 9 (53 + 1 + 8) | 54 + 1 + 9 (53 + 1 + 8) |
| Sweep scope: markers at 100% / short | 1,296 / 592 | 1,255 / 591 |

- Gained against sweep13: 184 PLAYER and 167 STUDIO entries. Lost: 1 in each target (see Losses).
- The integrated tree is above each agent's own private-CSV figure:

  | Agent | PLAYER | STUDIO |
  |---|---|---|
  | util-target | 10,064 | 9,796 |
  | world-5d-4 | 10,019 | 9,750 |
  | datamodel-5b-5 | 10,012 | not scored |

- App/Base/Network marker counts come from build/sweep14/markers.py, the same method as the sweep13 report.

**Rows** (PLAYER). The sweep13 column is sweep13's P-final scored on the current ranges:

| Row | At 100% / total | Share | sweep13 |
|---|---|---|---|
| 5a | 138/155 | 89.0% | 138 |
| 5b | 649/1,710 | 38.0% | 628 |
| 5c | 87/152 | 57.2% | 87 |
| 5d | 367/1,551 | 23.7% | 314 |
| 5e | 481/1,335 | 36.0% | 480 |
| 5f | 544/3,280 | 16.6% | 538 |
| 5g (row_coverage_5g.py) | 319/603 | 52.9% | 311 |
| 5h | 520/1,359 | 38.3% | 512 |
| 5k | 546/3,493 | 15.6% | 545 |

- 5b's denominator grew from 1,660 to 1,710, because the new StarterGuiService ctor marker widens that file's range (datamodel-5b-5).
- 5d's denominator grew from 1,528 to 1,551.
- Per-function CSVs: build/sweep14/row-*.csv.

**Near misses** (app_sweep report, build/sweep14/app-sweep.csv):

| Target | Short | inlining-size | ebp-frame | other | unpaired | unpaired-callee | x87-vs-sse | data-ref | assert-file-line | stack-cookie | At 90% or more | Pairing-only |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| PLAYER | 592 | 171 | 172 | 136 | 49 | 32 | 20 | 6 | 4 | 2 | 80 | 38 |
| STUDIO | 591 | 171 | 170 | 138 | 38 | 40 | 20 | 6 | 4 | 4 | 94 | 46 |

Closest (PLAYER):

| Function | Address | Score | Cause |
|---|---|---|---|
| LegacyContentTable ctor | 0x008f08a0 | 99.97 | data-ref, the prime_list end pointer 0x00bdb080 |
| ~DataModel | 0x0064d850 | 99.76 | |
| AsyncHttpQueue::asyncRequest | 0x00870430 | 99.59 | |
| ~TaskScheduler | 0x009b4e00 | 99.52 | |
| TaskScheduler::remove | 0x009b52b0 | 99.49 | `ret` twins |
| HeartbeatTask::stepDataModelJob | 0x0072dc30 | 99.45 | new, pairing-only |
| TextXmlWriter::serializeNode | 0x00628a60 | 99.39 | |
| RunService::start | 0x0072d300 | 99.37 | |
| MemoryBinder::processID | 0x006fc500 | 99.33 | |
| ~Job | 0x009ba1f0 | 99.28 | |
| Job::computeStandardSleepTime | 0x009b9a90 | 99.21 | |
| ServiceProvider::onChildAdded | 0x006a9770 | 99.19 | |
| World::createAutoJoints | 0x0074aa80 | 99.14 | |
| DataModel ctor | 0x0064ec50 | 99.10 | |

- Every function in that list except serializeNode is pairing-only.
- Most are the boost_po twin-copy cause datamodel-5b-5 described: program_options is built native here, so its folded template copies pair with the client shell's copies instead of the engine's.

## Losses (address by address against sweep13's P-final / S-final, summ.py)

**`RBX::Stats::TypedStatsItem<int>::vftable{for Debugable}`, PLAYER 0x00b01ce4 / STUDIO 0x00bd614c (96.55%).**
- `--verbose` shows a single diff, at slot 0x70. The original points to `TypedStatsItem<int>::update()` 0x0048e7a0, an override with no body in our tree. Ours points to `Stats::Item::update`.
- It was at 100% in sweep13 only because the old `StatsItem` name paired `Item::update` with the wrong address.
- util-target reported this as its one loss. Fixing it needs the 2013 `TypedStatsItem<T>`, which keeps only a `boost::function` at +0x88 and overrides `update()` (0x0048e7a0 for int, 0x0048e7d0 for bool).
- It is a code change, not a pairing one, so I kept the rename. The rename is right, and it is worth +15 or so entries elsewhere.

**No other entry lost 100% in either target.**
- With the stale CSVs (P0), 51 entries were missing. Most were the renamed `StatsItem` → `Stats::Item` templates and vftables, and all of them except the vftable above came back by the second regeneration round.
- The no-.reloc coincidences the agents saw (six Lua functions, IsOlderOrderedPacket 0x005c8930, httpGetPostWinHttp 0x00826990) are all at 100% in this link.
- `~ContactManager` 0x007bb5e0 is at 100% in this link. It flips between twins, as world-5d-4 noted.

## Checks

| Check | PLAYER (sweep13) | STUDIO (sweep13) |
|---|---|---|
| decomplint: function_out_of_order | 300 (287) | 307 (293) |
| decomplint: duplicate_offset | 12 (12) | 4 (4) |
| reccmp-vtable: traceback | none | none |
| reccmp-vtable: "Problem with adjuster thunk" lines | 11 (11) | 13 (13) |
| reccmp-datacmp: traceback | none | none |
| reccmp-datacmp: variables | 2,502 (2,472) | 2,371 (2,339) |
| reccmp-datacmp: issues | 55 (55) | 58 (58) |
| check_folded: annotated addresses | 2,579 (2,501) | 2,430 (2,351) |
| check_folded: folding problems | 0 | 0 |
| check_folded: "Debug data out of sync" lines | 19 (19) | 19 (19) |

- **decomplint** exits 1 under `--warnfail`, as in every sweep. Every added descent is one of the assert-pinned ones in the Reorder table.
- **reccmp_addr_padding:** 1,666 files, 0 bad.
- **check_2016_copies:** silent.

## Saved

- Final JSONs: build/sweep14/P-final.json (= P16) and build-studio/sweep14/S-final.json (= S14).
- Alongside them:
  - P/S-nolib.json, -vtable.txt, -datacmp.txt, -folded.txt
  - build/sweep14/PLAYER/STUDIO-decomplint.txt
  - app-sweep.csv and row-*.csv
- Scored exe, pdb, map and pdbview: build/sweep14/scored/ and build-studio/sweep14/scored/.
- Every round's JSONs and gen logs: build*/sweep14/.
- reccmp/ before regeneration: build/sweep14/reccmp-before/.
- Pre-reorder Contact.cpp: build/sweep14/Contact.cpp.bak.
- Helper scripts in build/sweep14/:
  - The sweep13 set with paths updated: score, gen, round, rounds, v, summ, split, stubcheck, markers, callers.
  - checks.sh (new): runs `--nolib`, vtable, datacmp and check_folded for one target.

## Files changed by me

- reccmp/: regenerated, +654 / −168 lines.
  - player-libdata.csv, player-symbols.csv
  - studio-libdata.csv, studio-symbols.csv
  - studio-stl.csv: one row dropped, `??1strstreambuf@std@@UAE@XZ_5` at 0x0060af40. That address now pairs as a folded rbx::signals body, which is at 100% in both sweeps.
- Client/_generated/{player,studio}/stubs_00.cpp: gen_link_stubs output (publicInsertRaw).
- Marker words in Client/App, Base and Network, set by the sweep.
- Client/App/v8world/Contact.cpp: reordered. One whole marked definition moved; no line holding an assert moved.
- No changes to tools/ or CMakeLists.txt by me.

## Unresolved

- **TypedStatsItem<T>** (Stats owner): write the 2013 class, with a `boost::function` at +0x88 and the `update()` overrides at PLAYER 0x0048e7a0 / 0x0048e7d0. That wins back vftable 0x00b01ce4 / 0x00bd614c.
- **boost_po twin copies** (owner side, build layout): program_options is `/GL` in 2013, so the engine keeps its own copies of the shared templates. Our native boost_po folds them into the shell's copies. It is the main cause of the 99%+ near misses listed above.
- **prime_list end pointer 0x00bdb080** (reccmp pairing): it costs the LegacyContentTable, DataModel, StarterGuiService, ContactManager and LibraryService ctors.
- **type.cpp and LuaInstanceBridge.cpp** reorders still break the compile.
- **Assert-pinned descents** added this wave: Profiling.cpp 4/4, Kernel.cpp +2/+2, EdgeBuffer.cpp +2/+2, and +1 each in Assembly.cpp, Joint.cpp, MovingAssemblyStage.cpp, Primitive.cpp and World.cpp (Primitive.cpp +2 in STUDIO). They need the bodies rewritten to their 2013 shape before they can move.
- **Network/Player.cpp stand-in instantiations** (util-target): not 2013 code and carry no marker. Delete each line once its real caller exists.
- **From the agents' lists, still open:**
  - world-5d-4: the KernelData IndexArray accessors, Joint ctors via `Primitive::isGeometryOrthogonal`, `destroyAutoJoints`, `jointsStepUi`, the `Guid::copyDataFrom` assign, `Time::Interval` return, ContactManager.cpp line layout, and the SpanningEdge.h review (Util owner).
  - datamodel-5b-5: the IAdornable 0x18 layout (Rendering contributor), `Heartbeat::expirationTime` as Time, the `getRobloxLocked` return, `Creator::create` at base +0x18, `ServiceProvider::create` calling setAndLockParent, the Base STL per-.obj copies, PhysicsInstructions, and moving StarterGuiService into PlayerGui.obj.
  - util-target: the RbxOgre copy of `~ConcurrencyValidator` (Rendering contributor), a volatile `rbx::atomic` (Base owner), and GlobalBasicSettings' getter 0x006589b0.
