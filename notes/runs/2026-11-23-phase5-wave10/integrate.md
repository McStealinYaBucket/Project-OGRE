# integrate (workflow phase5-wave10), 2026-11-24

I integrated the three phase5-wave10 Work agents: reflect-5e-6 (script-5e-6.md), shell-5k and network-5f. All three finished, and their uncommitted work compiles in both targets from clean, so nothing was reverted. I kept shell-5k's one-line CMakeLists.txt change (`Client/ClientBase/ReflectionMetadata.cpp` in `OGRE_CLIENT_COMMON_SOURCES`); it belongs in a small separate commit.

I rebuilt both targets from clean, ran gen_link_stubs, regenerated the CSVs until a round gained nothing, swept the markers and ran every check. Both targets are stable:
- PLAYER held **11,435** across P11–P14 and PA1–PA4.
- STUDIO held **10,908** across S11–S14 and SA1–SA4.

| | sweep16 | Lost | Gained | sweep17 |
|---|---|---|---|---|
| PLAYER | 10,909 | 9 | 535 | **11,435** (+526) |
| STUDIO | 10,373 | 15 | 550 | **10,908** (+535) |

Every loss was read with `--verbose`. None needs a code fix: 6 per target are one no-.reloc string coincidence, and the rest are twin pairing or a wrong IDA name (see Losses).

- Every heavy command ran through tools/capped.sh, one build at a time, and nothing hit the cap.
- No commits, no IDA database opened, nothing under Client/Rendering/ edited. The 2016-copy check is silent.

## What I did

**Clean rebuilds**
- `app_sweep set FUNCTION` first (544 PLAYER and 550 STUDIO markers flipped), so every scoring run saw every in-scope marker.
- PLAYER: build/ clean, 682 steps, 0 failures.
- STUDIO: build-studio/ clean, `--target studio`, 676 steps, 0 failures. Both caches keep `OGRE_BOOST_PO_GL=ON`.

**Link stubs.** I deleted each exe, ran `gen_link_stubs.py --build <dir> <T>` per target and relinked.
- Each target has 31 unresolved symbols (was 30). The new one is `RBX::operator<<(std::ostream&, const Name&)`, which ReflectionMetadata.cpp declares locally. gen_link_stubs reports it as `claimed`: a CSV row already owns PLAYER 0x006131f0, so it gets no stub and stays with /FORCE:UNRESOLVED until App defines it.
- Stubs are unchanged: 11 addresses each. Only `Client/_generated/{player,studio}/unresolved.txt` changed.

**Regeneration rounds** (build/sweep17/rounds.sh: score twice, regenerate from both, until a round gains nothing). Entries at effective 100%:
- PLAYER: 10,991 → 11,294 → 11,371 → 11,391 → 11,425 → 11,435 → 11,435.
- STUDIO: 10,433 → 10,760 → 10,841 → 10,862 → 10,897 → 10,908 → 10,908.
- After apply: PA1–PA4 11,435 and SA1–SA4 10,908, 0 lost and 0 gained against P14/S14.

**Sweep.** I ran `app_sweep apply P14.json S14.json`. In the swept scope it wrote FUNCTION 1,377 / STUB 550 / FOLDED 49 (PLAYER) and 1,321 / 556 / 49 (STUDIO). Against HEAD, in Client/App, Base and Network (build/sweep17/markers.py HEAD):

| | New FUNCTION | New STUB | Promoted | Demoted |
|---|---|---|---|---|
| PLAYER | 26 | 2 | 15 | 5 |
| STUDIO | 21 | 2 | 11 | 5 |

- PLAYER demotions: DataModel ctor 0x0064ec50, StarterGuiService ctor 0x0070c150, ContactManager ctor 0x007b63a0, LibraryService ctor 0x0088ec90, LegacyContentTable ctor 0x008f08a0. All five are the string coincidence below.
- STUDIO demotions: StarterGuiService 0x00798860, ContactManager 0x008d2550, LibraryService 0x009eb7b0, LegacyContentTable 0x009add30 (string coincidence), and ServiceProvider::onChildAdded 0x0076f7b0 (twin pairing).
- stubcheck: no STUB marker outside Rendering sits on an address at effective 100%. No FUNCTION marker anywhere outside Rendering (shell files included) is below 100%.
- The shell markers were already right: ClientBase, Win, WindowsClient, RobloxStudio and ClientShared hold PLAYER 37 FUNCTION + 7 STUB and STUDIO 23 + 4, all consistent with the final runs.

**Reorder.** `reorder_markers plan` over every .cpp finds no fixable file: 75 files have descents, and each is pinned by asserts or has descents that cross runs. The touched files are ScriptContext.cpp (24/23, pinned), type.cpp (3, pinned), LuaInstanceBridge.cpp (2, pinned), XmlSerializer.cpp (1, pinned), Players.cpp (1, crosses runs) and Name.cpp (1, crosses runs). Player.cpp and ReflectionMetadata.cpp have none. So nothing was moved and no rebuild was needed for step 2. decomplint totals are unchanged (below).

## Final figures (sweep16 in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| **Entries at effective 100%** | **11,435** (10,909) | **10,908** (10,373) |
| Non-library entries | 8,841 (8,332) | 8,426 (7,903) |
| **Functions at 100%** | **8,579** (8,161) | **8,242** (7,825) |
| **Non-library functions at 100%** | **6,052** (5,651) | **5,807** (5,402) |
| Functions at 100%: owner / Rendering (minus g3d) / third party | 6,041 / 3 / 2,535 (5,639 / 2 / 2,520) | 5,796 / 3 / 2,443 (5,391 / 3 / 2,431) |
| Vftables at 100%: owner / Rendering / third party | 2,773 / 1 / 12 (2,666 / 1 / 12) | 2,607 / 1 / 8 (2,490 / 1 / 8) |
| Vftables at 100% / paired | 2,786 / 3,619 (2,679 / 3,512) | 2,616 / 3,568 (2,499 / 3,449) |
| reccmp Implemented, with libraries | 10,140 / 40,550 (9,812) | 9,804 / 42,383 (9,489) |
| reccmp Implemented, `--nolib` | 7,363 / 37,734 (7,040 / 37,739) | 7,173 / 39,631 (6,864 / 39,635) |
| App markers: FUNCTION + FOLDED + STUB | 1,203 + 27 + 508 (1,192 + 27 + 518) | 1,158 + 27 + 516 (1,151 + 27 + 522) |
| Base markers | 92 + 0 + 19 (unchanged) | 86 + 0 + 20 (unchanged) |
| Network markers | 79 + 1 + 11 (54 + 1 + 9) | 74 + 1 + 11 (54 + 1 + 9) |

**Agents' private figures, against the integrated tree:**

| Agent | PLAYER | STUDIO |
|---|---|---|
| reflect-5e-6 | 11,425 | 10,758 |
| network-5f | 11,386 | 10,760 |
| shell-5k (own delta, built on a different tree state) | +153 / −3 | +144 / −5 |
| **Integrated** | **11,435** | **10,908** |

**Rows** (PLAYER, tools/row_coverage.py on P-final; per-function CSVs in build/sweep17/row-*.csv). The "sweep16" column is sweep16's P-final measured with today's ranges, so both columns cover the same functions. Row 5b's range grew from 1,927 to 2,452 functions since wave 9's report; on the old count sweep16 had 731.

| Row | At 100% / total | Share | sweep16, same ranges |
|---|---|---|---|
| 5a | 139/155 | 89.7% | 138 |
| 5b | 945/2,452 | 38.5% | 902 |
| 5c | 103/152 | 67.8% | 89 |
| 5d | 464/1,554 | 29.9% | 465 |
| 5e | 502/1,335 | 37.6% | 487 |
| 5f | 652/3,280 | 19.9% | 597 |
| 5g (row_coverage_5g.py) | 322/609 | 52.9% | 323 |
| 5h | 532/1,359 | 39.1% | 527 |
| 5k | 810/3,493 | 23.2% | 697 |

- 5e 502 matches reflect-5e-6's own figure. 5f 652 is above network-5f's 645, and 5k 810 is well above shell-5k's 731: the agents' changes help each other's rows (the type_holder and Variant fix reaches shell and Network templates).
- 5c +14 and 5b +43 come mostly from reflect-5e-6 (Variant copy/destroy, `Type::tag`, `Name::lookup`).
- 5d −1 is the ContactManager ctor 0x007b63a0. 5g lost the LRUCache ctor 0x00663980 and the LegacyContentTable ctor 0x008f08a0 and regained httpGetPostWinHttp 0x00826990 (wave 9's immediate coincidence has gone away). All three losses are the string coincidence below.

**Near misses** (app_sweep report, build/sweep17/app-sweep.csv):

| Target | Short | inlining-size | ebp-frame | other | unpaired | unpaired-callee | x87-vs-sse | assert-file-line | data-ref | stack-cookie | At 90% or more | Pairing-only |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| PLAYER | 550 | 166 | 165 | 126 | 49 | 17 | 20 | 4 | 3 | 0 | 60 | 20 |
| STUDIO | 556 | 165 | 163 | 130 | 38 | 32 | 20 | 4 | 3 | 1 | 83 | 35 |

Closest (PLAYER):

| Function | Address | Score | Pairing-only? |
|---|---|---|---|
| LegacyContentTable ctor | 0x008f08a0 | 99.97 | no (string coincidence) |
| TaskScheduler::remove | 0x009b52b0 | 99.49 | yes |
| ScriptContext::setTimeout | 0x0067eee0 | 99.41 | yes |
| DataModel ctor | 0x0064ec50 | 99.40 | no (string coincidence) |
| TextXmlWriter::serializeNode | 0x00628a60 | 99.39 | no |
| TaskScheduler::setThreadCount | 0x009b7ec0 | 99.26 | yes |
| TaskScheduler::getJobsInfo | 0x009b75f0 | 99.21 | yes |
| World::createAutoJoints | 0x0074aa80 | 99.14 | yes |
| GuiService::openBrowserWindow | 0x0069a540 | 99.08 | yes |
| TaskScheduler::getJobsByName | 0x009b78f0 | 98.99 | yes |
| DebrisService::addItem | 0x00845000 | 98.98 | yes |
| InsertService::insert | 0x007d6a20 | 98.90 | yes |
| TaskScheduler ctor | 0x009b55e0 | 98.88 | no |
| World::destroyAutoJoints | 0x00749340 | 98.84 | no |

## Losses (against sweep16's P-final / S-final, summ.py, each read with --verbose)

Lists: build/sweep17/loss-P14.txt and build-studio/sweep17/loss-S14.txt. P-final = PA4 and S-final = SA4 have the same sets.

**The string coincidence (PLAYER 7, STUDIO 7).** PLAYER: boost unordered table 0x005c4490, DataModel ctor 0x0064ec50, LRUCache 0x00663980, StarterGuiService ctor 0x0070c150, ContactManager ctor 0x007b63a0, LibraryService ctor 0x0088ec90, LegacyContentTable ctor 0x008f08a0. STUDIO: LRUCache 0x0078df90, StarterGuiService 0x00798860, ContactManager 0x008d2550, the boost unordered helpers 0x009ad9c0 (and the two `lib` rows 0x009ad8c0 / 0x009adac0, see below), LegacyContentTable 0x009add30, LibraryService 0x009eb7b0.
- Every diff is the same two lines, `push`/`cmp` of `prime_list_template<unsigned>::value+end`.
- Our COMDAT for `prime_list_template<unsigned>::value` comes from reflection_property.cpp.obj, in both sweep16 and sweep17 (PLAYER map 0x0079eb28, size 0xa0). The address right after it is the obj's first assert string, `std::find(allEnums().begin(), allEnums().end(), this) != allEnums().end() file: …`.
- In sweep16 that string had the wrong line, so it existed only in our binary and reccmp left it unnamed. reflect-5e-6 put reflection_property.cpp's asserts on their 2013 lines, so the string now also exists in the original. reccmp pairs it by content and names our `value+end` address after the string.
- So neither "the 1.0f constant" (script-5e-6) nor "the Players unordered_set" (network-5f) is the cause in the final tree. It is the right assert string sitting next to a COMDAT, with no .reloc. To fix it, another obj that links before reflection_property.cpp would have to supply `prime_list_template`, or the string would have to move. Neither is a code fix, so I kept the change: reflect-5e-6's work gains far more than these 7.

**Wrong IDA names (PLAYER 2).**
- `error_info_injector<…>::vftable{for boost::exception}` 0x00b11c68 (now 0.0). RTTI says this vftable belongs to `error_info_injector<rbx::bad_placement_any_cast>`, so the 0x00b11c5c–0x00b11c94 block pairs correctly now. sweep16's 100% was a wrong pairing with our `<std::exception>` twin. Its one slot, 0x0044db50, is named `error_info_injector<std::exception>` D0 thunk in ida/exports (a `library` row from lib-resolved), so it pairs with our native iostreams thunk 0x00702ab0 instead of the shell's bad_placement thunk 0x00413620. The same mistake names 0x0044d400 `throw_exception<std::exception>` (reflect-5e-6's unresolved item), which keeps 6 `any_cast<T&>` at 97.5.
- `boost::bad_any_cast::vftable` 0x00bda1b0 (now 50%). The original folded the D0s of bad_any_cast and bad_placement_any_cast into one shell body, 0x0044d280. Our link keeps two (shell 0x00411210, native 0x004aab00), so only one of the two vftables can pair slot 0. The pairing moved to bad_placement_any_cast, whose vftable 0x00b11c5c gained. Net zero.

**Twin pairing (STUDIO 8).**
- `shared_from<RunService>` against `shared_from<DataModel>`: Instance::onDescendantAdded 0x006f11e0, RefPropDescriptor<BillboardGui,Instance>::getVariant 0x0075eaf0, ServiceProvider::onChildAdded 0x0076f7b0, YieldFunctionStateObject ctor 0x009deec0. This is the same pair wave 9 listed under RunService::start, now resolved the other way round.
- `~pair<InstanceHandle const, InstanceHandle>` against the original's folded name `ScriptStats::StatCollection::~StatCollection`: map<InstanceHandle,InstanceHandle> erase 0x007490e0, _Erase 0x007493b0, operator[] 0x0074a610.
- `~pair<string const, string>` against the original's folded name `G3D::GImage::Error::~Error`: boost unordered node_constructor 0x009ad8c0 and table_impl 0x009adac0 (lib rows). They sit next to the prime_list coincidence block but fail on the destructor twin, not the string.

## Checks

| Check | PLAYER (sweep16) | STUDIO (sweep16) |
|---|---|---|
| decomplint: function_out_of_order | 307 (307) | 313 (313) |
| decomplint: duplicate_offset | 12 (12) | 4 (4) |
| reccmp-vtable: traceback | none | none |
| reccmp-vtable: "Problem with adjuster thunk" lines | 13 (11) | 15 (13) |
| reccmp-datacmp: traceback | none | none |
| reccmp-datacmp: variables | 2,881 (2,619) | 2,782 (2,478) |
| reccmp-datacmp: issues | 55 (55) | 58 (58) |
| check_folded: annotated addresses | 2,734 (2,687) | 2,553 (2,528) |
| check_folded: folding problems | 0 | 0 |
| check_folded: "Debug data out of sync" lines | 19 (19) | 19 (19) |
| reccmp_addr_padding | 1,671 files, 0 bad | |
| check_2016_copies | silent | |

- decomplint exits 1 under `--warnfail`, as in every sweep. The totals are unchanged, and the only per-line changes are line shifts: Players.cpp now reports line 221 (the EnumDesc ctor after isNetworkClient, which crosses runs), and ScriptContext.cpp's descents moved from 2709/2717 to 2715/2723.
- The two new adjuster-thunk lines are `clone_impl<error_info_injector<rbx::bad_placement_any_cast>>::clone`/`rethrow` `vtordisp{-4,0}` (0x0044db00 / 0x0044db40). That class is newly paired, and its vtordisp thunks have the same limit as the other eight clone_impl lines.

## Saved

- Final JSONs: build/sweep17/P-final.json (= PA4) and build-studio/sweep17/S-final.json (= SA4).
- Scored exe, pdb, map and pdbview: build/sweep17/scored/ and build-studio/sweep17/scored/.
- Alongside: every round's JSON and .txt, gen logs, rounds-*.log, loss-P14.txt / loss-S14.txt, P/S-nolib.json and .txt, -vtable.txt, -datacmp.txt, -folded.txt, {PLAYER,STUDIO}-decomplint.txt, app-sweep.csv, row-*.csv, markers-before.txt / markers-after.txt, stubs.log.
- Helper scripts: the sweep16 set with paths moved to sweep17, plus summ.py, stubcheck.py and v.sh from sweep15, in build/sweep17/.

## Files changed by me

- reccmp/: regenerated player-boost.csv, player-libdata.csv, player-symbols.csv, studio-libdata.csv, studio-symbols.csv.
- Client/_generated/{player,studio}/unresolved.txt, through gen_link_stubs.
- Marker words in Client/App, Base and Network, through app_sweep set/apply. Files touched only by that, beyond the agents' files: LuaEnum.cpp, LuaLibrary.cpp, LegacyContentTable.cpp, WinHttp.cpp, DataModel.cpp, PlayerGui.cpp, RunService.cpp, Service.cpp, ContactManager.cpp, SerializerV2.cpp, XmlElement.cpp.
- CMakeLists.txt: kept shell-5k's source-list line (no change of my own).
- No change to tools/ida/gen_reccmp_csv.py, tools/app_sweep.py or tools/reorder_markers.py this wave.

## Unresolved

- **Wrong IDA names at the shell's any_cast block (for the IDA owner).** PLAYER 0x0044d400 is `throw_exception<rbx::bad_placement_any_cast>`, not `<std::exception>`. 0x0044db50 is the `error_info_injector<rbx::bad_placement_any_cast>` D0 thunk, not `<std::exception>`'s. Both come from lib-resolved `identical` body matches against boost-iostreams, so the `match:`/`library` rows need dropping there (tools/lib/lib_consistency.py). Fixing them should give the 0x00b11c68 vftable and about 6 `any_cast<T&>` (97.5%) their 100%. Check STUDIO for the same pair.
- **prime_list string coincidence** (7 per target, above): a layout problem, not code. It goes away if an obj linked before reflection_property.cpp supplies `prime_list_template<unsigned>::value`, for example once Players' `unordered_set<std::string>` at 0xb8 is real (network-5f), or if its first string moves. Measure any such change with a full regeneration.
- **STUDIO twin pairs** `shared_from<RunService>`/`<DataModel>`, `~pair<InstanceHandle…>` and `~pair<string,string>`: the same mixed-ownership twin problem wave 9 listed. They resolve one way or the other on each regeneration.
- **`RBX::operator<<(std::ostream&, const Name&)`** (PLAYER 0x006131f0) is unresolved and claimed by a CSV row, so it gets no stub. App's Util/Name.h/Name.cpp should declare and define it (shell-5k).
- **Carried from the agents, unchanged** (each needs an App/Base/Network owner edit, outside this task's files):
  - reflect-5e-6: `Descriptor::name` as `const Name&`; include spellings in ProtectedString.cpp, ContentId.cpp and EnumProperty.cpp; `Limits::Counter::canAdd` returning int (newInstance 0x0087af00); EnumProperty.cpp's `Variant::convert<T>` on the rbx::any_cast forms; startScript (48.7%) and callCallback; LuaArguments.h `withVariantValue`; folded `typed_holder<T>::destruct_func` twins (about 40 `placement_any::operator=<T>` at 97.4).
  - shell-5k: Util/Object.h's five `&create` address-takes (about 60 shell `Creator::create`); the 2013 Permissions enum in security/SecurityContext.h; `Name::toString()` returning `const std::string&`; the remaining shell classes; FrameRateManagerStatsItem (PLAYER copy is in RbxOgre, Client/Rendering).
  - network-5f: PhysicsInstructions can call the new Player setters; LOGVARIABLE(Network, 1) in ScriptContext.cpp; `getTranslationUi` returning `const Vector3&`; Players as DescribedNonCreatable and Player's RUNTIME functionality (each with a CSV regeneration); Player's missing vftable overrides; Server : Peer at 0xb78 and the Replicator family; setTeamColor/setNeutral at 95.45% (boost::function safe-bool `ret` twin).
