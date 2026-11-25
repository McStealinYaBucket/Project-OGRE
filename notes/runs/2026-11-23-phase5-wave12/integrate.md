# integrate (workflow phase5-wave12), 2026-11-24

I integrated the three phase5-wave12 Work agents: app-12 (app-followups.md), shell-5k-3 and network-5f-3. All three finished, and their uncommitted work builds in both targets from clean with 0 failures, so nothing was reverted. That includes the ConcurrentRakPeer.cpp the shell agent had to leave out of its private links. I kept shell-5k-3's CMakeLists.txt edit, which adds DumpErrorUploader.cpp, VersionInfo.cpp, VideoControl.cpp and VistaTools.cpp to `OGRE_CLIENT_COMMON_SOURCES`. It belongs in a small separate commit.

I rebuilt both targets from clean, ran gen_link_stubs and regenerated the CSVs until a round gained nothing. Then I swept the markers, fixed the generator for the 0x0044db50 twin and ran every check. Both targets are stable:
- PLAYER held **11,797** across P7–P11 and PA1–PA5.
- STUDIO held **11,197** across S7–S10 and SA1–SA5.

| | sweep18 | Lost | Gained | sweep19 |
|---|---|---|---|---|
| PLAYER | 11,673 | 2 | 126 | **11,797** (+124) |
| STUDIO | 11,071 | 1 | 127 | **11,197** (+126) |

I read every loss with `--verbose` (see Losses). None was caused by integration. The `~intrusive_ptr<thread_data_base>` loss in each target needs a shell source edit outside my files. The 0x0044db50 entry was a false `library` 100% that now pairs correctly.

- Every heavy command ran through tools/capped.sh. Only one build ran at a time; scoring overlapped with the other target's build. Nothing hit the cap.
- No commits, no IDA database opened, nothing under Client/Rendering/ edited. The 2016-copy check is silent.

## What I did

**Clean rebuilds**
- `app_sweep set FUNCTION` first (563 markers flipped per target).
- PLAYER: build/ clean, 697 steps, 0 failures.
- STUDIO: build-studio/ clean, `--target studio`, 685 steps, 0 failures.

**Link stubs.** For each target I ran `gen_link_stubs.py --build <dir> <T>`, which deletes the exe itself, then relinked.
- PLAYER: 36 unresolved (sweep18: 49). Stubs went from 13 to 17 addresses.
- STUDIO: 35 unresolved (sweep18: 47). Stubs went from 13 to 16 addresses.
- The drop in unresolved symbols is the agents' new code: CrashReporter, VersionInfo, VistaTools, DumpErrorUploader and FileSystem.
- 21 unresolved per target are still `unknown`, because ida/exports names nothing for them (see Unresolved).

**Generator fix: the std::exception row at 0x0044db50** (tools/ida/gen_reccmp_csv.py, my file).

Cause:
- 0x0044db50 is the `error_info_injector<rbx::bad_placement_any_cast>` {12} adjustor thunk, slot 0 of RTTI vftable 0x00b11c68. names.csv says so with an `rtti:` row.
- Adjustor thunks are kept out of the function list, so the `decorated` pass (match:/lib:/rtti:/str: names) never saw it.
- A position row had paired it with our byte-identical `<std::exception>` thunk (mapped_file.obj, now 0x00709e30). Once reccmp scored that pairing at 100%, three things kept it alive:
  - the 1000-vote body pair;
  - the sticky libdata row;
  - the pass order (the `--json` pass result wins over the aligned-refs pass, which had 3 votes to 1 for the shell thunk).
- Debugging this with `OGRE_VOTES=0044db50` showed `00706580: 0044db50=1001` against `00413ed0: 0044db50=1` in the first pass.

Fix, in two lines:
1. `decorated` also takes adjustor thunks (`a in kept or a in thunks`).
2. An adjustor thunk's `rtti:` name counts as `proven`, like a `match:` name. The existing `beaten` rule then drops the contradicting position row, and the `rtti:` row pairs our shell thunk.

Effect:
- On the sweep18 link, the dry run changed exactly one row per CSV: the libdata std row disappeared and the symbols row appeared. Selftest passes.
- PLAYER: 0x0044db50 now pairs `??_E?$error_info_injector@Vbad_placement_any_cast@rbx@@@…WM@AEPAXI@Z` (libdata row, untyped).
- The `<bad_placement_any_cast>` scalar deleting destructor 0x0044d6c0 was 87.5% against the std twin and is now **100%**. The thunk's jump target had been voting for the std D0.
- Vftable 0x00b11c68 was 0.0 and is now **100%**.
- STUDIO 0x00410f30 gets the same row.

**Regeneration rounds** (build/sweep19/rounds.sh; each round scores twice and regenerates from the pair). Entries at effective 100%:
- PLAYER: 11,530 → 11,763 → 11,793 → 11,797 → 11,797 (P1–P10), P11 11,797.
- STUDIO: 10,940 → 11,152 → 11,194 → 11,197 → 11,197 (S1–S10).
- After `apply`: PA1–PA4 11,797 and SA1–SA4 11,197, with 0 lost and 0 gained against P11/S10.
- After the shell marker flips (below): PA5 11,797 and SA5 11,197, again 0 lost and 0 gained.

**Sweep.** I ran `app_sweep apply P11.json S10.json`. In scope it holds:

| | FUNCTION | STUB | FOLDED |
|---|---|---|---|
| PLAYER | 1,415 | 570 | 51 |
| STUDIO | 1,355 | 569 | 49 |

There are no demotions against HEAD (build/sweep19/markers.py HEAD).
- PLAYER: 2 promoted, 24 new FUNCTION, 19 new STUB, 1 new FOLDED.
- STUDIO: 1 promoted, 24 new FUNCTION, 15 new STUB.

Against the agents' own marker words, apply changed only `Players::askAddChild`:
- PLAYER 0x0070a240 became `FOLDED`, because PlayerGui.cpp:176 marks the same address.
- STUDIO 0x00796960 lost its `FOLDED`, because STUDIO has only the one marker at that address.

check_folded reports 0 folding problems in both targets.

**Shell markers** (outside app_sweep's scope; build/sweep19/shellcheck.py).
- 20 STUB markers sat on addresses at effective 100% in the final runs. I flipped them to FUNCTION:
  - Win/LogManager.cpp, PLAYER: getCrashEventName 0x0043ee20, getLogFileName 0x0043efb0, getFastLogFileName 0x0043f0f0, MakeLogFileName 0x0043f230, ThreadLogManager::getLogFileName 0x0043f380, RobloxCrashReporter ctor 0x0043f570, WriteCrashDump 0x0043f730, CreateFakeCrashDump 0x0043f8c0, purecallHandler 0x0043f9b0, handleDebugAssert 0x00440850, handleFailure 0x00440920.
  - Win/LogManager.cpp, STUDIO: the eight twins of these at 0x0041cb00–0x0041e2c0.
  - ClientBase/ReflectionMetadata.cpp:66: the ReflectionMetadata singleton 0x00447970.
- These are the ATL-helper callers that app-12's FileSystem/Http work released. CreateFakeCrashDump and the RobloxCrashReporter ctor were released by network-5f-3's CrashReporter.
- No line count changed. PA5/SA5 confirm 0 lost.
- The shell now holds PLAYER 82 FUNCTION + 9 STUB and STUDIO 57 + 8 (sweep18: 50 + 27 and 23 + 4).
- No FUNCTION marker is below 100%, and no STUB marker is at 100%.

**stubcheck:** no STUB marker outside Rendering sits on an address at effective 100%.

**Reorder.** `reorder_markers plan` finds no fixable file (build/sweep19/reorder-plan.txt): every file with descents is pinned by asserts, has a STUDIO-only chunk, or has descents that cross runs.
- The only new descent per decomplint is in Client/App/util/ContentProvider.cpp (PLAYER, 6 → 7): app-12's 0x0065ff50 at the end of the file, pinned by assert lines.
- Http.cpp's STUDIO urlEncode/trustCheckBrowser order (app-12) does not change STUDIO's per-file count. The two targets order these functions differently, so one target must keep the descent.
- Nothing was moved, so step 2 needed no rebuild.

## Final figures (sweep18 in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| **Entries at effective 100%** | **11,797** (11,673) | **11,197** (11,071) |
| Non-library entries | 9,141 (9,045) | 8,654 (8,545) |
| **Functions at 100%** | **8,893** (8,782) | **8,498** (8,382) |
| **Non-library functions at 100%** | **6,304** (6,221) | **6,002** (5,903) |
| Functions at 100%: owner / Rendering (minus g3d) / third party | 6,294 / 2 / 2,597 (6,209 / 2 / 2,571) | 5,988 / 3 / 2,507 (5,891 / 3 / 2,488) |
| Vftables at 100%: owner / Rendering / third party | 2,820 / 1 / 13 (2,809 / 1 / 11) | 2,641 / 1 / 7 (2,632 / 1 / 6) |
| Vftables at 100% / paired | 2,834 / 3,662 (2,821 / 3,656) | 2,649 / 3,598 (2,639 / 3,594) |
| reccmp Implemented, with libraries | 10,390 / 40,550 (10,298 / 40,551) | 10,040 / 42,384 (9,931 / 42,384) |
| reccmp Implemented, `--nolib` | 7,563 / 37,684 (7,495 / 37,709) | 7,358 / 39,580 (7,264 / 39,595) |
| App markers: FUNCTION + FOLDED + STUB | 1,217 + 28 + 513 (1,211 + 27 + 506) | 1,172 + 27 + 518 (1,165 + 27 + 514) |
| Base markers | 92 + 0 + 19 (unchanged) | 86 + 0 + 20 (unchanged) |
| Network markers | 103 + 2 + 26 (84 + 1 + 16) | 94 + 1 + 22 (76 + 1 + 12) |
| Shell markers (FUNCTION + STUB) | 82 + 9 (50 + 27) | 57 + 8 (23 + 4) |

The owner/third-party split comes from the .obj in the link map (build/sweep19/split.py).

**Agents' private figures, against the integrated tree:**

| Agent | PLAYER | STUDIO |
|---|---|---|
| app-12 | 11,789 | 11,194 |
| shell-5k-3 (link E) | 11,708 | 11,158 |
| network-5f-3 | 11,788 | 11,184 |
| **Integrated** | **11,797** | **11,197** |

**Rows** (PLAYER, tools/row_coverage.py on P-final; per-function CSVs in build/sweep19/row-*.csv). The "sweep18, same ranges" column is sweep18's P-final measured with today's ranges, so both columns cover the same functions. Row 5b's range grew from 2,916 to 2,979 with app-12's FileSystem.cpp and voxel/Grid.cpp.

| Row | At 100% / total | Share | sweep18, same ranges |
|---|---|---|---|
| 5a | 139/155 | 89.7% | 139 |
| 5b | 1,095/2,979 | 36.8% | 1,079 |
| 5c | 105/158 | 66.5% | 105 |
| 5d | 465/1,554 | 29.9% | 465 |
| 5e | 502/1,335 | 37.6% | 502 |
| 5f | 697/3,281 | 21.2% | 678 |
| 5g (row_coverage_5g.py) | 328/612 | 53.6% | 322 |
| 5h | 533/1,359 | 39.2% | 533 |
| 5k | 969/3,493 | 27.7% | 923 |

- 5f 697 equals network-5f-3's figure.
- 5k 969 is above shell-5k-3's 949. The other 20 are most likely app-12's ATL helpers and the LogManager callers that only come together in one link. I did not check them one by one.

**Near misses** (app_sweep report, build/sweep19/app-sweep.csv; markers not at 100%):

| Target | Short | inlining-size | ebp-frame | other | unpaired | unpaired-callee | x87-vs-sse | assert-file-line | data-ref | stack-cookie | At 90% or more | Pairing-only |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| PLAYER | 570 | 174 | 174 | 130 | 48 | 17 | 20 | 4 | 3 | 0 | 65 | 20 |
| STUDIO | 569 | 168 | 170 | 134 | 37 | 32 | 20 | 4 | 3 | 1 | 88 | 35 |

The closest PLAYER near misses are the same as in sweep18. Two are new: `DataModel::onlyJobsLeftForThisArbiterAreGenericJobs` 0x0063a120 (98.68, other) and `Peer::~Peer` 0x005b8bf0 (98.68, pairing only).

| Function | Address | Score | Cause |
|---|---|---|---|
| LegacyContentTable ctor | 0x008f08a0 | 99.97 | prime_list string coincidence |
| TaskScheduler::remove | 0x009b52b0 | 99.49 | pairing only |
| ScriptContext::setTimeout | 0x0067eee0 | 99.41 | pairing only |
| DataModel ctor | 0x0064ec50 | 99.40 | prime_list string coincidence |
| TextXmlWriter::serializeNode | 0x00628a60 | 99.39 | |
| TaskScheduler::setThreadCount | 0x009b7ec0 | 99.26 | pairing only |
| TaskScheduler::getJobsInfo | 0x009b75f0 | 99.21 | pairing only |
| World::createAutoJoints | 0x0074aa80 | 99.14 | pairing only |
| GuiService::openBrowserWindow | 0x0069a540 | 99.08 | pairing only |
| TaskScheduler::getJobsByName | 0x009b78f0 | 98.99 | pairing only |
| DebrisService::addItem | 0x00845000 | 98.98 | pairing only |
| InsertService::insert | 0x007d6a20 | 98.90 | pairing only |
| TaskScheduler ctor | 0x009b55e0 | 98.88 | |
| World::destroyAutoJoints | 0x00749340 | 98.84 | |

## Losses (against sweep18's P-final / S-final, summ.py, each read with --verbose)

PLAYER lost 2:
- **`~intrusive_ptr<thread_data_base>` 0x004069d0 (0.057).** STUDIO lost the same one at 0x004256d0 (0.057), its only loss.
  - `--verbose` shows an unoptimized body (`push ebp; mov ebp,esp; …`). The link map puts our only copy in CrashReporter.cpp.obj, which is `#pragma optimize("", off)`.
  - In 2013 the client shell supplies this COMDAT. Our shell does not instantiate it, so no optimized copy exists.
  - The fix is a stand-in instantiation in `Client/Win/placeholder.cpp`, which is shell source outside my files. network-5f-3 and app-12 both reported the same cause.
- **`??_E error_info_injector<…>` {12} thunk 0x0044db50.**
  - In sweep18 this was a false `library` entry at 100%, our `<std::exception>` twin.
  - The generator fix above now pairs it with the right twin through an untyped libdata row. Adjustor thunks are kept out of the function list, so the entry leaves the JSON instead of scoring.
  - The fix raised the D0 0x0044d6c0 and vftable 0x00b11c68, so the net change is +1.

The agents' other losses are back at 100% in the integrated link: `IsOlderOrderedPacket` 0x005c8930 and `httpGetPostWinHttp` 0x00826990, the no-.reloc `0x7fffff` coincidences app-12 saw.

## Checks

| Check | PLAYER (sweep18) | STUDIO (sweep18) |
|---|---|---|
| decomplint: function_out_of_order | 310 (309) | 314 (314) |
| decomplint: duplicate_offset | 12 (12) | 4 (4) |
| reccmp-vtable: traceback | none | none |
| reccmp-vtable: "Problem with adjuster thunk" lines | 13 (13) | 15 (15) |
| reccmp-datacmp: traceback | none | none |
| reccmp-datacmp: variables | 2,958 (2,937) | 2,858 (2,819) |
| reccmp-datacmp: issues | 56 (56) | 58 (59) |
| check_folded: annotated addresses | 2,849 (2,785) | 2,645 (2,563) |
| check_folded: folding problems | 0 | 0 |
| check_folded: "Debug data out of sync" lines | 19 (19) | 19 (19) |
| reccmp_addr_padding | 1,689 files, 0 bad | |
| check_2016_copies | silent | |

- decomplint exits 1 under `--warnfail`, as in every sweep. The one new descent is ContentProvider.cpp's (PLAYER), listed above.
- **datacmp:** `FLog::Network` is gone in both targets (app-12 set it to 6).
- PLAYER has one new issue: `___rtc_tzz` at 0x00c7f9f4. The original holds 2 there and ours holds a null.
  - This is a new position row (`00c7f9f4,global,,___rtc_tzz`) that a CRT `_RTC_*` reference voted for. The original's real `.rtc$TZZ` sentinel is probably elsewhere.
  - It is a data row only, and no function score depends on it. I left it; see Unresolved.

## Saved

- Final JSONs:
  - build/sweep19/P-final.json (= PA5)
  - build-studio/sweep19/S-final.json (= SA5)
- Scored exe, pdb, map and pdbview: build/sweep19/scored/ and build-studio/sweep19/scored/.
- Alongside the finals:
  - every round's JSON and .txt, gen logs, rounds-*.log
  - P/S-nolib.json and .txt, -vtable.txt, -datacmp.txt, -folded.txt, {PLAYER,STUDIO}-decomplint.txt
  - app-sweep.csv, row-*.csv, reorder-plan.txt, markers-before.txt / markers-after.txt, shell-flip.txt, stubs.log
  - pre.diff / post.diff / pre-status.txt / pre-untracked.tgz (the agents' tree before my changes)
- Helper scripts: the sweep18 set with paths moved to sweep19, in build/sweep19/.
- build/sweep19/root is a private generator root (symlinks plus its own reccmp/). I used it for the dry run of the generator fix on the sweep18 link.

## Files changed by me

- tools/ida/gen_reccmp_csv.py: adjustor thunks enter `decorated`, and an adjustor thunk's `rtti:` name counts as `proven`. These are the two edits around the comments dated 2026-11-24.
- reccmp/: regenerated player-* and studio-*.
- Client/_generated/{player,studio}/stubs_00.cpp and unresolved.txt, through gen_link_stubs.
- Marker words:
  - Client/Win/LogManager.cpp: 19 STUB → FUNCTION.
  - Client/ClientBase/ReflectionMetadata.cpp:66: STUB → FUNCTION.
  - Client/Network/Players.cpp:254/255: FOLDED word, through app_sweep.
  - Every other marker word app_sweep wrote is the same as the agents left it.
- CMakeLists.txt: shell-5k-3's source-list edit kept, with no change of my own.
- No change to tools/app_sweep.py or tools/reorder_markers.py.

## Unresolved

- **Shell owner: instantiate `boost::intrusive_ptr<boost::detail::thread_data_base>::~intrusive_ptr` in Client/Win/placeholder.cpp.** No marker is needed. This brings back PLAYER 0x004069d0 and STUDIO 0x004256d0, whose only copy is now CrashReporter.obj's unoptimized one. Measure it A/B.
- **`___rtc_tzz` position row at PLAYER 0x00c7f9f4** (datacmp: the original holds 2 there). Read which `_RTC_*` function votes for it, and whether the original's sentinel sits elsewhere. If the row is wrong, add a generator exclusion for the `__rtc_*` section markers.
- **IDA owner: names for gen_link_stubs.** 21 unresolved callees per target are still `unknown` because ida/exports/*-names.csv has no row for them (stubs.log).
  - Wave 11 listed them: G3D::Log common/getCommonLogFilename, the G3D hook setters, View::SetFullscreen and CSHA1.
  - Add app-12's list too: getBaseCacheDirectory 0x006ea260, getCacheDirectory 0x006eab90, VistaAPIs::~VistaAPIs 0x006ea0e0, the CStringT helpers 0x006ea0f0/0x00623230/0x00623640/0x00623840, Http::isMoneySite/trustCheck/trustCheckBrowser, MemberDescriptorContainer::compare 0x00614cd0, and `GlobalAdvancedSettingsItem<CRenderSettingsItem>::singleton` 0x00407230 / createInstance 0x00409020.
- **Lib-resolved:** 0x0044d400 is still a `conflict` row named `throw_exception<std::exception>`. It now pairs correctly through libdata (84.0, the shell LTCG frame). The IDA owner can drop the conflict row.
- **Carried from the agents** (owner edits outside this task):
  - **app-12**
    - GlobalAdvancedSettingsItem<CRenderSettingsItem> ctor 0x00456ee0 (84.3) needs the shell's `singleton()` callers and an out-of-line createInstance.
    - CStringT(const wchar_t*) 0x00440d20 (59.2).
    - isMoneySite 85.1 and trustCheckBrowser 94.1.
    - ContentProvider FUN_00664260 61.9, and FUN_0065ff50, which is byte-identical but loses to the `push 10000000` no-.reloc coincidence.
    - MemberDescriptorContainer::compare needs a TEMPLATE marker in reflection/member.h, with that header laid out for it first.
    - Variant converts, ObjectBridge::newInstance 67.5, and the unnamed groups.
  - **shell-5k-3**
    - utf8_encode/utf8_decode (Base) and FLog::Verbs/CrashReporterInit.
    - GameSettings +0x90 member order (VideoControl ctor).
    - Game/GameBasicSettings, which block RecordToggleVerb, ScreenshotVerb and View::SetFullscreen.
    - ReflectionMetadata writeProperty/writeEvent inlining.
    - Ogre::IFrameDataCallback for the Rendering contributor.
  - **network-5f-3**
    - FFlag stand-ins to FastLogSettings.cpp.
    - FriendService playerAdded/playerRemoving.
    - TaskScheduler::printJobs 0x009b6730, which blocks WatcherThreadFunc.
    - Players ctor/dtor signal types.
    - Player::prop_SuperSafeChatReplicate.
    - The onServiceProvider bodies.
    - Player::setCharacter: keep its opaque call.
    - The Replicator members past 0x10a8.
