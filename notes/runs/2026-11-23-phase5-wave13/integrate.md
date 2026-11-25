# integrate (workflow phase5-wave13), 2026-11-24/25

I integrated the three phase5-wave13 Work agents: app-13, shell-5k-4 and network-5f-4. All three finished. Their uncommitted work builds from clean in both targets with 0 failures, so I reverted nothing and kept everything. None of them changed CMakeLists.txt, and neither did I.

I rebuilt both targets from clean, ran gen_link_stubs and relinked. Then I regenerated the CSVs until a round gained nothing, swept the markers, reordered one file, rebuilt and confirmed nothing moved. Three generator fixes went in along the way (below).

| | sweep19 | Lost | Gained | **sweep20** |
|---|---|---|---|---|
| PLAYER | 11,797 | **0** | 200 | **11,997** (+200) |
| STUDIO | 11,197 | **0** | 181 | **11,378** (+181) |

Both targets are above every agent's private figure. app-13 had 11,996 / 11,373, network-5f-4 had 11,990 / 11,365, and shell-5k-4 had 11,762 / 11,165 on its own base.

Nothing was lost against build/sweep19/P-final.json or build-studio/sweep19/S-final.json in either target, so there was no loss to read with `--verbose`. The losses the agents reported are all back at their sweep19 scores:
- **genericConvert<ProtectedString> / <ContentId>** (PLAYER 0x0066f7f0 / 0x00701c40) are back at 94.78% effective. gen_link_stubs now stubs their callees 0x0066f460 / 0x008d3540, so the calls pair again.
- **Guid::compare** (0x006f9500) is at 100%.
- **`~intrusive_ptr<thread_data_base>`** is at 100%.

Housekeeping:
- Every heavy command ran through tools/capped.sh, one build at a time, and nothing hit the cap.
- No commits, no IDA database opened, nothing under Client/Rendering/ edited, and the shared reccmp/ was written only by the generator.

## What I did

1. **`app_sweep set FUNCTION`.** This flipped 559 PLAYER and 561 STUDIO markers.
2. **Clean builds.** PLAYER build/ took 697 steps and STUDIO build-studio/ (`--target studio`) took 685, both with 0 failures.
3. **Link stubs.** `gen_link_stubs.py --build <dir> <T>`, then a relink.

   | | Unresolved (sweep19) | Stub addresses (sweep19) | Unknown (sweep19) |
   |---|---|---|---|
   | PLAYER | 32 (36) | 23 (17) | 10 (21) |
   | STUDIO | 32 (35) | 22 (16) | 10 (21) |

   - New stubs include the two `StringConverter<>::convertToValue` callees app-13 named.
   - The 10 unknown are the same in both targets: CSHA1 ×6, RakNet TCPInterface::AllocatePacket / DeallocatePacket, and PacketizedTCP::Send / SendList.
4. **Generator fixes** (tools/ida/gen_reccmp_csv.py, my file; selftest passes):
   - **`__rtc_*` sentinels excluded.** The position row `00c7f9f4,global,,___rtc_tzz` was a wrong pairing.
     - In the original, 0x00c7f9f0 holds FuncInfo magic 0x19930522, so 0x00c7f9f4 is that FuncInfo's nstate (2). That is the "original holds 2" datacmp issue.
     - The CRT's `.rtc$IAA/IZZ/TAA/TZZ` sentinels are zero-size, so a reference past one lands on whatever data follows it in our image.
     - A new `RTC = ("___rtc_",)` filter applies to the libdata reference pass, aligned_refs and sticky rows.
     - PLAYER datacmp issues went from 56 to 55, and the row is gone.
   - **`__sehtable$` scope tables** (network-5f-4's request).
     - aligned_refs now recognises `push -1; push <table>; push _except_handler3`. Both sides must have unnamed tables whose first record starts with enclosing level -1.
     - The original table address becomes a nameless 1-byte global row, as for `__ehhandler$` stubs.
     - This emits PLAYER 0x00c80b58, 0x00c80b98 and 0x00cd2798.
     - **CrashReporter::WatcherThreadFunc (PLAYER 0x0051fe50 / STUDIO 0x005d76a0) and ProcessExceptionStatic (0x005202d0 / 0x005d7b20) are now 100% in both targets.** The sweep promoted them to FUNCTION.
   - **Sticky anonymous rows (a bug).**
     - `sticky()` did `taken.add(sym or name)`. The first nameless libdata row kept from the previous file therefore put `""` into `taken`, and every later nameless row was dropped.
     - So a nameless row (`__ehhandler$`, `__sehtable$`) survived only while its function still had the diff that voted for it. It vanished in the next round, and the function fell back.
     - This is why PLAYER oscillated 11,997 / 11,994 / 11,997 (P15–P19) on the two seh functions and RBX::SetThreadName 0x009b1aa0.
     - The fix: an empty key is never checked against `taken`. That kept 16 more rows on the same input (164 → 180 kept). After the fix, P19–P22 held 11,997.
5. **Regeneration rounds** (build/sweep20/rounds.sh: score twice, regenerate from the pair). Entries at effective 100%:
   - PLAYER: 11,707 → 11,895 → 11,962 → 11,984 → 11,985 → 11,989 → 11,994 → 11,997 → (11,994, the sticky bug) → 11,997 → 11,997 (P1–P22).
   - STUDIO: 11,113 → 11,290 → 11,352 → 11,370 → 11,369 → 11,378 → 11,378 (S1–S14).
6. **Sweep.** `app_sweep apply P22.json S14.json`, then PA1–PA4 and SA1–SA4: 11,997 / 11,378, with 0 lost and 0 gained against P22 / S14.
   - No demotions against HEAD (build/sweep20/markers.py HEAD).
   - PLAYER: 12 promoted, 16 new FUNCTION, 1 new STUB. STUDIO: 9 promoted, 18 new FUNCTION, 1 new STUB.
   - Against the agents' own marker words, apply changed only these:
     - **Promoted:** WatcherThreadFunc and ProcessExceptionStatic in both targets (the seh rows).
     - **Promoted, prime_list coincidence:** the five prime_list constructors, LibraryService 0x0088ec90, LegacyContentTable 0x008f08a0, DataModel 0x0064ec50 (PLAYER only), StarterGuiService 0x0070c150 and ContactManager 0x007b63a0. network-5f-4 measured this: with `unordered_set<string>` in Players, Players.obj supplies prime_list, and the `value+end` address now lands where it should. This is the no-.reloc layout coincidence in CLAUDE.md, so expect these to flip back if the data after the prime_list COMDAT moves.
     - Every other marker word is as the agents left it.
   - **Shell markers** (build/sweep20/shellcheck.py): PLAYER 83 FUNCTION + 12 STUB, STUDIO 58 + 8. No FUNCTION is below 100% and no STUB is at 100%, so nothing needed flipping. stubcheck finds no STUB outside Rendering at effective 100%.
7. **Reorder.** `reorder_markers plan` (build/sweep20/reorder-plan.txt) found one fixable file, Client/App/util/ProtectedString.cpp.
   - app-13's getSalt 0x0066ef10 sat after the copy ctor 0x0066f170. `apply` swapped the two chunks, taking the file from 2 descents to 1 in each target.
   - The remaining descent is the cross-run Type::getSingleton<ProtectedString> 0x0066f200 at the top of the file.
   - The file has no asserts. Rebuilding then recompiled 411 PLAYER and 402 STUDIO steps (the marker words touch headers).
   - PB1–PB4 held 11,997 and SB1–SB4 held 11,378, with 0 lost against PA4 / SA4.
   - The other new descents agree with the agents' reports and are pinned by asserts:

     | File | Descents (both targets) | Function |
     |---|---|---|
     | TaskScheduler.Thread.cpp | 4 → 5 | printJobs at the end |
     | Network/Client.cpp | 1 → 2 | disconnect, which holds assert line 82 |
     | Network/Streaming.cpp | 0 → 1 | the IdSerializer functions after the operators |

## Final figures (sweep19 in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| **Entries at effective 100%** | **11,997** (11,797) | **11,378** (11,197) |
| Non-library entries | 9,322 (9,141) | 8,815 (8,654) |
| **Functions at 100%** | **9,069** (8,893) | **8,658** (8,498) |
| **Non-library functions at 100%** | **6,461** (6,304) | **6,142** (6,002) |
| Functions at 100%: owner / Rendering (minus g3d) / third party | 6,451 / 2 / 2,616 (6,294 / 2 / 2,597) | 6,131 / 3 / 2,524 (5,988 / 3 / 2,507) |
| Vftables at 100%: owner / Rendering / third party | 2,846 / 1 / 11 (2,820 / 1 / 13) | 2,662 / 1 / 7 (2,641 / 1 / 7) |
| Vftables at 100% / paired | 2,858 / 3,688 (2,834 / 3,662) | 2,670 / 3,624 (2,649 / 3,598) |
| reccmp Implemented, with libraries | 10,564 / 40,550 (10,390 / 40,550) | 10,196 / 42,384 (10,040 / 42,384) |
| reccmp Implemented, `--nolib` | 7,722 / 37,669 (7,563 / 37,684) | 7,494 / 39,565 (7,358 / 39,580) |
| App markers: FUNCTION + FOLDED + STUB | 1,230 + 28 + 507 (1,217 + 28 + 513) | 1,184 + 27 + 513 (1,172 + 27 + 518) |
| Base markers | 93 + 0 + 19 (92 + 0 + 19) | 87 + 0 + 20 (86 + 0 + 20) |
| Network markers | 117 + 2 + 21 (103 + 2 + 26) | 108 + 1 + 19 (94 + 1 + 22) |
| Shell markers (FUNCTION + STUB) | 83 + 12 (82 + 9) | 58 + 8 (57 + 8) |

- The owner / third-party split comes from the .obj in the link map (build/sweep20/split.py).
- PLAYER's third-party vftable count went from 13 to 11 with no vftable lost. Two vftables now come from an owner .obj, most likely Players.obj or Streaming.obj instantiating them first.

**Rows** (PLAYER, tools/row_coverage.py on P-final; per-function CSVs in build/sweep20/row-*.csv; the ranges are unchanged since sweep19):

| Row | At 100% / total | Share | sweep19 |
|---|---|---|---|
| 5a | 139/155 | 89.7% | 139 |
| 5b | 1,108/2,979 | 37.2% | 1,095 |
| 5c | 105/158 | 66.5% | 105 |
| 5d | 471/1,554 | 30.3% | 465 |
| 5e | 503/1,335 | 37.7% | 502 |
| 5f | 717/3,281 | 21.9% | 697 |
| 5g (row_coverage_5g.py) | 331/612 | 54.1% | 328 |
| 5h | 534/1,359 | 39.3% | 533 |
| 5k | 997/3,493 | 28.5% | 969 |

- 5f's 717 equals network-5f-4's figure.
- 5k gained 28, against shell-5k-4's +12 on its own base. The rest are most likely app-13's GameSettings/FastLog pieces and the two seh functions' shell callers, which only come together in one link. I did not check them one by one.

**Near misses** (app_sweep report, build/sweep20/app-sweep.csv; markers not at 100%):

| Target | Short | inlining-size | ebp-frame | other | unpaired | unpaired-callee | x87-vs-sse | assert-file-line | data-ref | stack-cookie | At 90% or more | Pairing-only |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| PLAYER | 559 | 169 | 174 | 123 | 48 | 18 | 20 | 4 | 3 | 0 | 59 | 21 |
| STUDIO | 561 | 165 | 170 | 128 | 37 | 33 | 20 | 4 | 3 | 1 | 83 | 36 |

The closest PLAYER near misses (the prime_list constructors have left this list):

| Function | Address | Score | Shape |
|---|---|---|---|
| TaskScheduler::remove | 0x009b52b0 | 99.49 | pairing only |
| ScriptContext::setTimeout | 0x0067eee0 | 99.41 | pairing only |
| TextXmlWriter::serializeNode | 0x00628a60 | 99.39 | other |
| TaskScheduler::setThreadCount | 0x009b7ec0 | 99.26 | pairing only |
| ObjectBridge::newInstance (new, app-13) | 0x0087af00 | 99.21 | pairing only, the getPtr wrapper 0x00877530 |
| TaskScheduler::getJobsInfo | 0x009b75f0 | 99.21 | pairing only |
| World::createAutoJoints | 0x0074aa80 | 99.14 | pairing only |
| GuiService::openBrowserWindow | 0x0069a540 | 99.08 | pairing only |
| TaskScheduler::getJobsByName | 0x009b78f0 | 98.99 | pairing only |
| DebrisService::addItem | 0x00845000 | 98.98 | pairing only |
| InsertService::insert | 0x007d6a20 | 98.90 | pairing only |
| TaskScheduler ctor | 0x009b55e0 | 98.88 | other |
| World::destroyAutoJoints | 0x00749340 | 98.84 | other |
| DataModel::onlyJobsLeftForThisArbiterAreGenericJobs | 0x0063a120 | 98.68 | other |
| Peer::~Peer | 0x005b8bf0 | 98.68 | pairing only |

shell-5k-4 asked me to recheck some shell functions on the integrated build:
- At 100% in both targets: GetAppVersion (PLAYER 0x0043eb80), the RobloxCrashReporter ctor, WriteCrashDump and ~MainLogManager.
- PLAYER getCrashEventName 0x0043ee20 is at 100%.
- STUDIO getCrashEventName 0x0041c970 is at 63.9%. Its marker is STUB, and it was not at 100% in sweep19 either.

## Checks

| Check | PLAYER (sweep19) | STUDIO (sweep19) |
|---|---|---|
| decomplint: function_out_of_order | 313 (310) | 317 (314) |
| decomplint: duplicate_offset | 12 (12) | 4 (4) |
| reccmp-vtable: traceback | none | none |
| reccmp-vtable: "Problem with adjuster thunk" lines | 13 (13) | 15 (15) |
| reccmp-datacmp: traceback | none | none |
| reccmp-datacmp: variables | 3,013 (2,958) | 2,919 (2,858) |
| reccmp-datacmp: issues | **55** (56) | 58 (58) |
| check_folded: annotated addresses | 2,879 (2,849) | 2,672 (2,645) |
| check_folded: folding problems | 0 | 0 |
| check_folded: "Debug data out of sync" lines | 19 (19) | 19 (19) |
| reccmp_addr_padding | 1,690 files, 0 bad | |
| check_2016_copies | silent | |

- decomplint exits 1 under `--warnfail`, as in every sweep. The +3 per target are the three assert-pinned descents listed under Reorder.
- PLAYER datacmp lost one issue: `___rtc_tzz`, now that the row is gone.

## Saved

- **Final JSONs:**
  - build/sweep20/P-final.json (= PB4)
  - build-studio/sweep20/S-final.json (= SB4)
- **Scored exe, pdb, map and pdbview:** build/sweep20/scored/ and build-studio/sweep20/scored/.
- **Alongside the finals:**
  - every round's JSON and .txt (P1–P22, PA1–4, PB1–4; S1–S14, SA1–4, SB1–4), gen logs and rounds-*.log
  - P/S-nolib.json and .txt, -vtable.txt, -datacmp.txt, -folded.txt, and {PLAYER,STUDIO}-decomplint.txt (in build/sweep20)
  - app-sweep.csv, row-*.csv, reorder-plan.txt, stubs.log
  - pre.diff / pre-status.txt / pre-untracked.tgz: the agents' tree before my changes
- **Helper scripts:** the sweep19 set with paths moved to sweep20, in build/sweep20/.
- **build/sweep20/root:** a private generator root (symlinks plus its own reccmp/), used to debug the seh and sticky fixes without touching the shared CSVs.

## Files changed by me

- **tools/ida/gen_reccmp_csv.py:**
  - the `RTC` filter in three places;
  - `SEH_PUSH` and the `"seh"` node in aligned_refs / data_rows;
  - the empty-key fix in `sticky()`.
- **reccmp/:** regenerated player-* and studio-*.
- **Client/_generated/{player,studio}/stubs_00.cpp and unresolved.txt**, through gen_link_stubs.
- **Marker words**, through app_sweep. Only the words listed above differ from the agents' tree: the 2 seh functions and the 5 prime_list ctors, in both targets where marked.
- **Client/App/util/ProtectedString.cpp:** reorder_markers swapped the getSalt and copy-ctor chunks. No body changed.
- No change to CMakeLists.txt, tools/app_sweep.py or tools/reorder_markers.py.

## Unresolved

- **App (app-13): the GameBasicSettings registrars are unresolved data in both links**, kept only by `/FORCE:UNRESOLVED` (Client/_generated/*/unresolved.txt):
  - `ClassRegistrar<GameBasicSettings>::registrar`
  - `EnumRegistrar<GameBasicSettings::ControlMode>::registrar`
  - `EnumRegistrar<GameBasicSettings::RenderQualitySetting>::registrar`

  GameBasicSettings.h declares them, but no TU defines them. Define them in GameSettings.cpp.
- **IDA owner: names for gen_link_stubs.**
  - 10 unknown per target, with no ida/exports row: CSHA1 ctor/dtor/Reset/Update/Final/GetHash, RakNet TCPInterface::AllocatePacket / DeallocatePacket, and PacketizedTCP::Send / SendList.
  - Also app-13's list: checkLookup 0x00614db0 (STUDIO 0x006db970 / 0x00703620), checkLookups 0x00615b20, printJobs 0x009b6730 / 0x00a6ec30, the GameBasicSettings setters 0x006f6200..0x006f6270, getFullScreen 0x006f61f0, the GameSettings setters 0x006f4810 / 0x006f4840, and the Game ctor helper 0x00653790.
  - Types to import: GameBasicSettings 0x90 and GlobalBasicSettings 0xb4.
- **prime_list constructors.** LibraryService, LegacyContentTable, DataModel (PLAYER), StarterGuiService and ContactManager are FUNCTION now, through layout luck: a no-.reloc `value+end` coincidence. A later change to what follows the prime_list COMDAT can flip them back to 99.9%. If one flips, read it with `--verbose` before blaming the change.
- **The two new seh FUNCTIONs depend on nameless libdata rows** that are now sticky. If a regeneration ever runs from a JSON in which these functions are already at 100% and with a reccmp/ that lacks the rows (a fresh checkout before this commit), they would drop to 99.3 / 97.9 for one round and come back on the next.
- **Carried from the agents** (owner edits outside this task):
  - **Network and App, in one commit:** move the four FFlag stand-ins (ImmediateCrash, HangUploadEnabled, DebugLocalRccServerConnection, CrispFilteringEnabled) from Network into FastLogSettings.cpp. app-13.md has the ready block.
  - **CMakeLists flags owner:** measure `/D_MBCS` for App, Base and Network (isMoneySite's `_mbsnbcpy_s`).
  - **Base owner:**
    - a per-type `call_once` static for `signal_with_args<1,…>::operator()`, which blocks View::SetFullscreen;
    - an inline `RBX::isFinite`, for RunningAverage<double>::sample.
  - **App:**
    - GameBasicSettings::setFullScreen must fire the signal before it stores;
    - DataModel::TakeScreenshotTask takes `weak_ptr<DataModel>`;
    - a UserInputBase header;
    - FriendService playerAdded/playerRemoving;
    - ScriptContext::openState lacks 2013's debug-API block;
    - ContentProvider 0x00664260 and the Variant converts;
    - isMoneySite's 4-byte frame difference;
    - the getPtr wrapper 0x00877530, which holds newInstance at 99.2%.
  - **Network:**
    - Server ctor/dtor: Server.h's 0xb00/0xb0c should be `std::set<std::string>`, and the multi_index at 0xb18 blocks them;
    - the onServiceProvider bodies;
    - Network::GuidRegistryService;
    - the Replicator ctor/dtor;
    - Players.h's 0x180 member type.
  - **Rendering contributor:**
    - ViewBase, for the RecordToggleVerb ctor 0x00439000;
    - CRenderSettings graphicsMode as a direct member.
