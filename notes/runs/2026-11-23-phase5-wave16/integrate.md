# integrate (workflow phase5-wave16), 2026-11-26

This integrates the three phase5-wave16 Work agents: app-16, shell-5k-7 and network-5f-7. All three finished. Their uncommitted work builds from clean in both targets with 0 failures, so all of it stays and nothing was reverted.

On top of their work I made three changes:
- **CMakeLists.txt:** linked the shell agent's four private sources.
- **The generator:** three name fixes, and the wave-15 entries dropped.
- **Marker words:** a sweep.

| | sweep22 | Lost | Gained | **sweep23** |
|---|---|---|---|---|
| PLAYER | 12,496 | 6 | 107 | **12,597** (+101) |
| STUDIO | 11,780 | 14 | 114 | **11,880** (+100) |

Both targets ended above every agent's private figure:

| Agent's private figure | PLAYER | STUDIO |
|---|---|---|
| app-16 | 12,590 | 11,871 |
| network-5f-7 | 12,588 | 11,883* |

\* network-5f-7's STUDIO figure (11,883) is above mine (11,880). That figure comes from a link in which the 13 no-.reloc coincidences listed below happened not to fire.

Every loss was read with `--verbose`. **None comes from a code regression.** The losses are a removed invented function, shell-twin folding, no-.reloc coincidences, and one TU move (each one is under "Losses explained" below).

Housekeeping:
- Every heavy command ran through tools/capped.sh, with one build at a time and no cap hits.
- No commits, no IDA database opened, and nothing under Client/Rendering/ edited.
- The shared reccmp/ was written only by the generator.

## What I did

1. **Saved the agents' tree** to build/sweep23/: pre.diff, pre-status.txt and pre-untracked.tgz.
2. **CMakeLists.txt** (shell-5k-7's list):
   - `Client/Win/placeholder_creators.cpp` goes into `OGRE_CLIENT_COMMON_SOURCES` after `placeholder.cpp`, so both shells get it.
   - `Client/WindowsClient/main.cpp`, `Application.cpp` and `FunctionMarshaller.cpp` go into `player_client` after `View.cpp`.
3. **`app_sweep set FUNCTION`** flipped 570 PLAYER and 566 STUDIO markers. Then I ran clean builds (reconfigure, `--target clean`, build): PLAYER build/ took 710 steps and STUDIO build-studio/ took about 690, both with 0 failures.
4. **`gen_link_stubs.py`, then a relink** (build/sweep23/pipe2.sh).
   - **PLAYER:** 25 unresolved (sweep22: 30), 23 stubs, unknown 11 (10).
   - **STUDIO:** 23 unresolved (27), 20 stubs, unknown 10.
   - **The four `ClassRegistrar<…>::registrar` statics are no longer unresolved.** network-5f-7 defined them.
   - **New in PLAYER:**
     - unresolved data `CRenderSettings::minGameWindowSize` (Rendering).
     - unknown `FrameRateManager::ResumeAutoAdjustment` (Rendering, 0x009c7ac0, which has no name in names.csv).
     - Both come from the shell's new View/VideoControl code; shell-5k-7 listed both for the Rendering contributor.
5. **Regeneration rounds**, scoring twice and regenerating from the pair until a round gains nothing (entries at effective 100%):
   - PLAYER 12,490 → 12,555 → 12,586 → 12,593 → 12,594 → 12,594 (P1–P12).
   - STUDIO 11,767 → 11,831 → 11,868 → 11,874 → 11,875 → 11,875 (S1–S12).
6. **Generator fixes** (tools/ida/gen_reccmp_csv.py, my file; `--selftest` passes), then rounds PB1–4 and SB1–4: PLAYER 12,597, STUDIO 11,880.
   - **MATCH_FIX PLAYER 0x006aaf50 / STUDIO 0x00809d30 → `FactoryProduct<FriendService>` scalar deleting dtor.** app-16 showed that it stores only FactoryProduct's vftables, so the match: name `??_GFriendService` was stale.
     - In both links our base D0 is folded with DescribedCreatable's.
     - FriendService's own D0, 0x006b2b30 / 0x00811830 (app-16's SYNTHETIC marker), and its dtor 0x006b2970 now pair at 100.
   - **MATCH_FIX PLAYER 0x006aaf40 / STUDIO 0x00809d20 → the base's `{24}` adjustor thunk `??_E?$FactoryProduct<FriendService>…OBI`.**
     - It sits just before the base D0. FriendService's own thunks are 0x006b2950 / 0x006b2960.
     - With the old name, it took FriendService's thunk. That cost the thunk itself and vftable 0x00b394b0 / 0x00c13c50 (both 0.5).
     - Both are 100 now.
   - **New MATCH_DROP for STUDIO 0x0042b260 / 0x0042e370:** the /GL shell's `bind` / `list1<weak_ptr<DataModel>>` (names from the wave-15 IDA sync).
     - Our STUDIO shell has no GameVerbs.cpp, so it has no shell copy of these. Their names fell on the engine body, which is folded with DebrisService's `weak_ptr<Instance>` binds; they took it at 0.70 / 0.75.
     - That left the engine originals 0x0098a310 / 0x0098a120 unpaired (they were 100 at sweep22).
     - Both are 100 again. Drop each entry once the STUDIO shell instantiates the symbol.
   - **Dropped:**
     - the wave-15 MATCH_FIX entries 0x006ee210 and 0x0041c8b0;
     - all of MATCH_SUB (0x0073da60, 0x0073db90, 0x0074c040, 0x00844de0, 0x00844f60), together with its code.
     - ida/exports/PLAYER-names.csv now carries those names.
     - **Proof:** I regenerated PLAYER from P11+P12 with and without the entries, and the CSVs came out byte-identical (build/sweep23/csv-with/).
7. **Sweep.**
   - **Run:** `app_sweep apply PB4.json SB4.json`, then stability rounds PC1–4 / SC1–4. The result was 12,597 / 11,880 in every run, which is stable.
   - **No rebuild after apply:** apply rewrites only marker words, so no line or byte of code changes.
   - **Against HEAD** (build/sweep23/markers.py HEAD): PLAYER 8 promoted, 13 new FUNCTION, 7 new STUB; STUDIO 5 promoted, 18 new FUNCTION, 7 new STUB.
   - **STUDIO demotions (3), all no-.reloc coincidences:**
     - Math.cpp isDenormal 0x00898bc0;
     - isNanInfDenormVector3 0x00898cc0;
     - WinHttp.cpp httpGetPostWinHttp 0x0091cd80.
     - These follow the FUNCTION-only-at-100% rule. A later link may flip them back.
   - **Shell markers** (shellcheck.py): PLAYER 95 FUNCTION + 23 STUB, STUDIO 58 + 8. No FUNCTION is below 100 and no STUB is at 100. stubcheck finds no STUB outside Rendering at effective 100%.
8. **Reorder: not needed.**
   - `reorder_markers plan` (reorder-plan-pre.txt) differs from sweep22's in one line: Replicator.cpp P 1 / S 1 descents, "not fixable (pinned by assert lines)". That is network-5f-7's MTU descent at line 3416, which the 2013 asserts at 3418 and 3424 pin.
   - decomplint: function_out_of_order 314 → 315 (PLAYER) and 318 → 319 (STUDIO). The only new finding is Replicator.cpp:3416. Players.cpp's existing descent moved from line 280 to 289 with the agents' edits.

## Losses explained (against build/sweep22/P-final.json and build-studio/sweep22/S-final.json)

**PLAYER (6):**

- **0x00419510 `boost::bad_any_cast::what`** (library row, now absent). app-16 removed the invented pointer-returning const `any_cast`, which was our only source of this shell copy. It returns with the real shell exception classes or program_options `variable_value::as<T>`.
- **0x005b1970, 0x005b8e10, 0x00633950, 0x0072bc60 and 0x0072bcd0: five `sp_pointer_construct<Job…>` copies (97.67).** All of them call `_internal_accept_owner`.
  - **In 2013** that callee is one fold, 0x005bd080. Its EH handler 0x00a174d8 unwinds through the **shell** copy of the folded `~shared_ptr` at 0x00405b50 (as does 0x006678b0, StandardOut's).
  - **In our link** the shell's View.cpp (new this wave) now instantiates `~shared_ptr<TaskScheduler::Job>` (/GL copy 0x0041c790). So the Job `_internal_accept_owner` copies get that unwind target and fold with StandardOut's at 0x00619fe0, which is paired 1001 with 0x006678b0.
  - Registry's, Thread's, FWBase's and YieldFunctionStateObject's copies still unwind through the native `~shared_ptr` (0x0069fd40). They form a second fold at 0x005b9b40, which takes 0x005bd080 (0.98).
  - **So the Job side now matches 2013's EH shape, and the Registry side is the incomplete one.** The cause is the same as wave 9's shell-twin rule. Fix (shell owner, not mine): instantiate `~shared_ptr<GuidItem<Instance>::Registry>` (and the other three types) in the shell stand-in (`Client/Win/placeholder.cpp`). The two groups should then refold into one.
  - network-5f-7 and shell-5k-7 saw the same five losses.

**STUDIO (14):**

- **13 no-.reloc coincidences.** In each one the original has an immediate that equals one of our data addresses (in the FactoryProduct<Snap> vftable area): `and eax,0x7fffff` and `lea edx,[eax-…]`. Confirmed with --verbose for isDenormal, luaK_nil, IsOlderOrderedPacket and httpGetPostWinHttp:
  - RakNet `IsOlderOrderedPacket`;
  - Math `isDenormal` and `isNanInfDenormVector3`;
  - `httpGetPostWinHttp`;
  - Lua `symbexec`, `luaV_execute`, `constructor`, `patchtestreg`, `removevalues`, `luaK_setreturns`, `luaK_setoneret`, `luaK_nil` and `luaK_posfix`.
  - shell-5k-7 saw the same set.
- **0x00734740 `Described<FriendService>::classDescriptor` (0.86).** app-16's FriendService work moved our copy into FriendService.cpp. The 2013 copy reads `sFriendService` from data and comes from an earlier TU, most likely the 2013 Network Players.cpp. It returns once Players.cpp includes FriendService.h (see Unresolved).

## Final figures (sweep22 in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| **Entries at effective 100%** | **12,597** (12,496) | **11,880** (11,780) |
| Non-library entries | 9,888 (9,792) | 9,300 (9,200) |
| **Functions at 100%** | **9,545** (9,472) | **9,051** (8,979) |
| **Non-library functions at 100%** | **6,903** (6,835) | **6,518** (6,446) |
| Functions at 100%: owner / Rendering (minus g3d) / third party | 6,893 / 2 / 2,650 (6,823 / 2 / 2,647) | 6,508 / 3 / 2,540 (6,435 / 3 / 2,541) |
| Vftables at 100%: owner / Rendering / third party | 2,968 / 1 / 13 (2,941 / 1 / 12) | 2,771 / 1 / 7 (2,742 / 1 / 8) |
| Vftables at 100% / paired | 2,982 / 3,806 (2,954 / 3,793) | 2,779 / 3,737 (2,751 / 3,723) |
| reccmp Implemented, with libraries | 10,998 / 40,550 (10,926) | 10,592 / 42,384 (10,509) |
| reccmp Implemented, `--nolib` | 8,121 / 37,634 (8,055 / 37,640) | 7,869 / 39,546 (7,797 / 39,556) |
| App markers: FUNCTION + FOLDED + STUB | 1,267 + 28 + 511 (1,261 + 28 + 510) | 1,217 + 27 + 521 (1,214 + 27 + 517) |
| Base markers | 93 + 0 + 19 (93 + 0 + 19) | 87 + 0 + 20 (87 + 0 + 20) |
| Network markers | 154 + 2 + 35 (139 + 2 + 37) | 141 + 1 + 25 (124 + 1 + 24) |
| Shell markers (FUNCTION + STUB) | 95 + 23 (93 + 17) | 58 + 8 (58 + 8) |

The owner / third-party split comes from the .obj in the link map (build/sweep23/split.py).

**Rows** (PLAYER, tools/row_coverage.py on P-final; per-function CSVs in build/sweep23/row-*.csv):

| Row | At 100% / total | Share | sweep22 |
|---|---|---|---|
| 5a | 139/155 | 89.7% | 139 |
| 5b | 1,130/2,979 | 37.9% | 1,126 |
| 5c | 105/158 | 66.5% | 105 |
| 5d | 475/1,554 | 30.6% | 475 |
| 5e | 515/1,335 | 38.6% | 513 |
| 5f | 858/3,319 | 25.9% | 827/3,306 |
| 5g (row_coverage_5g.py) | 332/612 | 54.2% | 332 |
| 5h | 545/1,359 | 40.1% | 544 |
| 5k | 1,054/3,493 | 30.2% | 1,048 |

- **5f:** network-5f-7 reported 859 on its private link. The integrated figure is 858, short by one of the five sp_pointer_construct copies counted in 5f.
- **5k:** +6, from RegisterWindowClass, View::resetScheduler and the FriendService Creator copies.

**Near misses** (app_sweep report, build/sweep23/app-sweep.csv; markers not at 100%):

| Target | Not at 100% | ebp-frame | inlining-size | other | unpaired | unpaired-callee | x87-vs-sse | assert-file-line | data-ref | stack-cookie | At 90% or more | Pairing-only |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| PLAYER | 577 | 185 | 180 | 126 | 48 | 11 | 20 | 4 | 3 | 0 | 52 | 14 |
| STUDIO | 575 | 176 | 172 | 133 | 37 | 28 | 20 | 4 | 4 | 1 | 82 | 32 |

The closest PLAYER near misses are unchanged from sweep22: TaskScheduler::remove 0x009b52b0 99.49 (pairing only), ScriptContext::setTimeout 0x0067eee0 99.41, TextXmlWriter::serializeNode 0x00628a60 99.39, TaskScheduler::setThreadCount 0x009b7ec0 99.26, World::destroyAutoJoints 0x00749340 99.23, TaskScheduler::getJobsInfo 0x009b75f0 99.21, GuiService::openBrowserWindow 0x0069a540 99.08, TaskScheduler::getJobsByName 0x009b78f0 98.99, InsertService::insert 0x007d6a20 98.90 and the TaskScheduler ctor 0x009b55e0 98.88.

New shell near misses (STUB):
- View::HandleWindowsMessage 0x00466510: 95.4.
- Application::OnGetMinMaxInfo 0x00403fa0: 94.9.
- WndProc 0x004426a0: 75.0.
- FunctionMarshaller::Submit 0x00437500: 75.3.
- static_getCreator<FriendService> 0x0052c200: 90.5.

## Checks

| Check | PLAYER (sweep22) | STUDIO (sweep22) |
|---|---|---|
| decomplint: function_out_of_order | 315 (314) | 319 (318) |
| decomplint: duplicate_offset | 12 (12) | 4 (4) |
| reccmp-vtable: traceback | none | none |
| reccmp-vtable: "Problem with adjuster thunk" lines | 13 (13) | 15 (15) |
| reccmp-datacmp: traceback | none | none |
| reccmp-datacmp: variables | 3,141 (3,124) | 3,014 (2,991) |
| reccmp-datacmp: issues | 57 (57) | 59 (59) |
| check_folded: annotated addresses | 2,997 (2,967) | 2,752 (2,725) |
| check_folded: folding problems | 0 | 0 |
| check_folded: "Debug data out of sync" lines | 19 (19) | 19 (19) |
| reccmp_addr_padding | 1,702 files, 0 bad | |
| check_2016_copies | silent | |

decomplint exits 1 under `--warnfail`, as in every sweep. The one new finding per module is Replicator.cpp:3416, which the asserts pin.

## Saved

- **Final JSONs:** build/sweep23/P-final.json (= PC4) and build-studio/sweep23/S-final.json (= SC4).
- **Scored exe, pdb, map and pdbview:** build/sweep23/scored/ and build-studio/sweep23/scored/.
- **Alongside them:**
  - every round's JSON and .txt (P1–12, PB1–4, PC1–4; S1–12, SB1–4, SC1–4), gen logs and rounds-*.log;
  - gen-votes.log (OGRE_VOTES=5bd080,5b9b40,619fe0);
  - csv-with/ (the drop proof);
  - P/S-nolib.json and .txt, -vtable.txt, -datacmp.txt and -folded.txt;
  - {PLAYER,STUDIO}-decomplint.txt, app-sweep.csv, row-*.csv, reorder-plan-pre.txt and stubs.log;
  - the build logs.
- **Helper scripts:** the sweep22 set with paths moved to sweep23, plus clean-builds.sh, rounds2.sh and rounds3.sh.

## Files changed by me

- **CMakeLists.txt:** the four shell sources (above).
- **tools/ida/gen_reccmp_csv.py:**
  - MATCH_FIX for PLAYER 0x006aaf50 / 0x006aaf40 and STUDIO 0x00809d30 / 0x00809d20;
  - the new MATCH_DROP (STUDIO 0x0042b260 / 0x0042e370);
  - the wave-15 MATCH_FIX entries and all of MATCH_SUB removed;
  - the docstring updated.
- **reccmp/:** regenerated player-* and studio-* (boost, libdata, symbols).
- **Client/_generated/{player,studio}/:** unresolved.txt (and the stubs) through gen_link_stubs.
- **Marker words** through app_sweep. No body changed and no line moved.
- No change to tools/app_sweep.py or tools/reorder_markers.py.

## Unresolved

- **IDA owner:**
  - Rename PLAYER 0x006aaf50 → `FactoryProduct<FriendService>` scalar deleting dtor, and 0x006aaf40 → its `{24}` adjustor thunk. In STUDIO the pair is 0x00809d30 / 0x00809d20.
  - Name FriendService's D0 0x006b2b30 (STUDIO 0x00811830), its thunks 0x006b2950 / 0x006b2960 and its dtor 0x006b2970 (STUDIO 0x00811670).
  - Drop the matching MATCH_FIX entries once names.csv carries the names.
  - Push app-16's new names: InvalidDescendentDetector, the FriendService ctor and handlers, and `Variant::cast_unk0x0058f560`.
  - Name `FrameRateManager::ResumeAutoAdjustment` 0x009c7ac0, so that gen_link_stubs can bind it.
- **Shell:**
  - Instantiate `~shared_ptr<GuidItem<Instance>::Registry>`, `<TaskScheduler::Thread>`, `<FWBase>` and `<YieldFunctionStateObject>` in the shell stand-in. This should refold the two `_internal_accept_owner` groups and restore the five sp_pointer_construct copies. Measure it A/B.
  - Also from shell-5k-7:
    - static_getCreator<FriendService> 0x0052c200 (90.5);
    - the boost::function ctor inlining (Submit, stopAction, the ScreenshotVerb ctor);
    - WinMain / Application::Initialize;
    - the rest of View.cpp.
  - A STUDIO shell instantiation of `bind<weak_ptr<DataModel>>` would retire MATCH_DROP.
- **Network:**
  - Players.cpp can now `#include "V8DataModel/FriendService.h"`, because placeholder_creators.cpp is linked. That should restore STUDIO 0x00734740 classDescriptor and unblock onChildAdded / onChildRemoving.
  - Server::onServiceProvider (88.99), the ClusterReplicationData ctor, InstancePacketCache (which waits on App's Util/Http.h `_Buynode` TEMPLATE namerefs), and STUDIO rawPeer (DataModel's per-target layout).
- **App** (from network-5f-7 and app-16):
  - move the three packet-cache `ClassRegistrar` statics into FactoryRegistration.cpp, and `FLog::NetworkCache` into ScriptContext.cpp;
  - add NetworkSettings printSplitMessages / dataSendPriority;
  - add `Instance::unlockParent`;
  - write FriendService::playerAdded, the Game ctor and openState.
- **Rendering contributor:** GfxBase ViewBase.h / FrameRateManager.h and `CRenderSettings::minGameWindowSize` (now unresolved data in PLAYER).
- **No-.reloc churn:** the three STUDIO demotions (isDenormal, isNanInfDenormVector3, httpGetPostWinHttp) are layout luck. Re-sweep when the coincidence moves.
- **gen_link_stubs unknowns:**
  - PLAYER 11: the CSHA1 ×6 and TCPInterface / PacketizedTCP ×4, as before, plus ResumeAutoAdjustment.
  - STUDIO 10.
