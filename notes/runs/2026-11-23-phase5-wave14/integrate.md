# integrate (workflow phase5-wave14), 2026-11-25

I integrated the three phase5-wave14 Work agents: app-14, shell-5k-5 and network-5f-5. All three finished. Their uncommitted work builds from clean in both targets with 0 failures, so I kept all of it and reverted nothing.

Two changes to CMakeLists.txt went in with it:
- `Client/WindowsClient/View.cpp` is now in `player_client`, after GameVerbs.cpp. shell-5k-5 had linked it the same way in its private build.
- `/D_MBCS` is on App, Base and Network only. app-14 measured it at +1/−0. I confirmed the gain on the integrated build (below).

| | sweep20 | Lost | Gained | **sweep21** |
|---|---|---|---|---|
| PLAYER | 11,997 | 3 | 206 | **12,200** (+203) |
| STUDIO | 11,378 | 2 | 171 | **11,547** (+169) |

Both targets ended above every agent's private figure:

| Agent's private figure | PLAYER | STUDIO |
|---|---|---|
| app-14 | 12,155 | 11,520 |
| shell-5k-5 | 12,154 | 11,502 |
| network-5f-5 | 12,158 | 11,500 |

I read all five losses with `--verbose`. None of them is a code regression in an owner file. Each comes from a change made this wave and is a pairing or COMDAT effect (see "Losses").

Housekeeping:
- Every heavy command ran through tools/capped.sh, one build at a time, and nothing hit the cap.
- No commits and no IDA database opened.
- Nothing under Client/Rendering/ was edited.
- The shared reccmp/ was written only by the generator.

## What I did

1. **Saved the agents' tree** (build/sweep21/pre.diff, pre-status.txt, pre-untracked.tgz).
   - Edited CMakeLists.txt (View.cpp, `/D_MBCS`).
   - `app_sweep set FUNCTION` flipped 558 PLAYER and 561 STUDIO markers.
2. **Clean builds.** PLAYER build/ took 701 steps and STUDIO build-studio/ (`--target studio`) took 688, both with 0 failures.
3. **`gen_link_stubs.py --build <dir> <T>`, then a relink.**
   - PLAYER: 27 unresolved (32 at sweep20), 20 stub addresses (23), 13 unknown (10).
   - STUDIO: 23 unresolved (32), 20 stubs (22), 10 unknown (10).
   - Dropped because now defined:
     - PLAYER: `View::SetFullscreen` 0x00467880, `StringConverter<ProtectedString>::convertToValue` 0x0066f460 and `StringConverter<ContentId>::convertToValue` 0x008d3540.
     - STUDIO: 0x00780050 and 0x00846e30.
   - The three GameBasicSettings registrars are no longer unresolved: app-14 defined them.
   - New unresolved in PLAYER:
     - FLog::RobloxWndInit (data), which shell-5k-5 asked the App owner for.
     - Three unknowns with no ida/exports address: `UserInput::PostUserInputMessage`, `CRenderSettings::getResolutionPreset` and `VideoControl::startRecording(SoundService*)`.
4. **Regeneration rounds** (build/sweep21/rounds.sh: score twice, then regenerate from the pair, until a round gains nothing). Entries at effective 100%:
   - PLAYER: 11,924 → 12,110 → 12,159 → 12,182 → 12,192 → 12,200 → 12,200 (P1–P14).
   - STUDIO: 11,332 → 11,479 → 11,513 → 11,531 → 11,539 → 11,547 → 11,547 (S1–S14).
5. **Sweep.** `app_sweep apply P14.json S14.json` set the in-scope markers (app_sweep counts PLAYER 1,479 FUNCTION, 51 FOLDED, 559 STUB; STUDIO 1,418, 49, 561).
   - Then a rebuild (415 / about 400 steps, 0 failures) and rounds PA1–PA4 / SA1–SA4: 12,200 / 11,547, with 0 lost and 0 gained against P14 / S14. It is stable.
   - No demotions against HEAD (build/sweep21/markers.py HEAD): 30 new FUNCTION, 9 promoted and 9 new STUB per target.
   - Against the agents' own marker words, apply changed only these. Each is at effective 100% in its target:
     - `StringConverter<ContentId>::convertToValue` 0x008d3540 / 0x00846e30 (UDim.cpp), now that its link stub is gone;
     - `RunService` 0x0072da30 / 0x0079b6b0;
     - `SendPhysics` 0x008e5fe0, 0x008e60b0 / 0x00a042e0, 0x00a043b0;
     - `World` 0x007492d0 and `World::createAutoJoints` 0x0074aa80 / 0x00889c20, 0x0088b3d0. createAutoJoints was a 99.14 "pairing only" near miss at sweep20.
   - **Shell marker (by hand, a marker word only):** View.cpp:210 `View::SetFullscreen` 0x00467880 went from STUB to FUNCTION. It is at 100% now that its stub is gone. shellcheck (build/sweep21/shellcheck.py) found it as the only mismatch.
   - After the flip, shell markers are PLAYER 90 FUNCTION + 20 STUB and STUDIO 58 + 8, with no mismatch. stubcheck finds no STUB outside Rendering at effective 100%.
6. **Reorder.** Not needed.
   - `reorder_markers plan` (build/sweep21/reorder-plan-pre.txt) finds no fixable file.
   - The only difference from sweep20's plan is ProtectedString.cpp, which was fixed last wave.
   - decomplint's findings are identical to sweep20's, compared as a multiset of (file, kind): 313 function_out_of_order + 12 duplicate_offset in PLAYER, and 317 + 4 in STUDIO.
   - The new files (GameBasicSettings.cpp, GuidRegistryService.cpp, View.cpp, UserInputBase.cpp) add no descents.

### /D_MBCS, confirmed on the integrated build (kept)

| Function | Address | sweep20 | sweep21 |
|---|---|---|---|
| `ATL::CUrl::CUrl` | 0x00401930 | 0.947 | **1.0** |
| `Http::isMoneySite` | 0x00623bb0 | 0.851 | 0.858 |

- With `/D_MBCS`, isMoneySite calls `_mbsnbcpy_s`. Its 4-byte frame difference is still there.
- None of the five losses involves MBCS (each is explained below).
- **Scope:** I left `/D_MBCS` out of OGRE_APP_FLAGS, because G3D, RbxOgre, GfxAdapters and the stubs library inherit that set and were not measured. Instead it is on `ogre_library(App …)`, OGRE_BASE_FLAGS and OGRE_NETWORK_FLAGS. The prior-tree App sources join the App target, so they get it too.
- I did not run a separate no-MBCS build on the integrated tree. The A/B evidence is app-14's M1-vs-W3 pair plus the two functions above.

## Losses against sweep20 (all read with --verbose)

**1. PLAYER 0x00707830 / STUDIO 0x007d3620, `EventDescImpl<1,Lighting,void(bool)>::connectGeneric` (98.96%): pairing between twins.**
- In 2013, Lighting's and GameBasicSettings' `EventDescBase<…,void(bool)>::connect` are one folded body at 0x006440c0. That address sits in the bool-signal group near `sBoolValue`.
- app-14's new GameBasicSettings EventDescs make GBS's two connectGenerics vote for that address, so the generator now gives 0x006440c0 to our **GBS** connect. That pairing scores 40% (0x005c02f0). Lighting's call then counts as a mismatch.
- **The real cause is in App code.** Our GBS connect does a checked cast at Event.h:239 (`classDescriptor()` + `ClassDescriptor::isA`, then `lea eax,[esi-0x1c]`). The 2013 body does the plain static adjustment (`add eax,-0x1c`), as Lighting's does.
- Once GBS's cast takes the same path as Lighting's, the two bodies fold again and both connectGenerics should come back.

**2. PLAYER vftables 0x00b44908 / 0x00b451d0 (SoundService, 100 → 50): shell/engine twin.** shell-5k-5 predicted this, and it nets +17 by their count.
- Slot 1 of each vftable is 2013's folded `className`/`getClassName` body 0x006ee210.
- Our shell (`/GL`) now has its own `FactoryProduct<SoundService>::className` copy (0x004089f0), because the shell's `create<SoundService>` instantiates it. The original's shell create 0x0043c7d0 calls the engine's 0x006ee210, so its pair goes to the shell copy, and create<SoundService> is at 100% for it.
- The engine's `getClassName` (0x0053f900), which the vftables point to, is left unpaired.
- **Fix:** keep the shell from emitting an out-of-line `className<SoundService>`.

**3. STUDIO 0x0073cfc0, `EnumDesc<GameSettings::UploadSetting>::convertToValue` (86.4%): COMDAT choice.**
- 2013 STUDIO inlines `EnumDesc<UploadSetting>::singleton()` (call_once + doGetSingleton). Ours calls it out of line.
- Both of our links take this COMDAT from GameBasicSettings.cpp.obj, the new file, which globs ahead of GameSettings.cpp.obj.
- PLAYER's 2013 copy is the out-of-line form, so PLAYER stays at 100%. STUDIO's 2013 copy evidently came from a TU that inlined it.
- **Fix:** an App owner change, either the STUDIO link order of these two objects or the GBS TU's use of the enum. app-14 listed this already.

**Losses the agents reported that are back:**
- The six engine `bind<…weak_ptr<DataModel>…>` / function1 rows (0x0073da60, 0x0073db90, 0x008445f0, 0x008447e0, 0x00844de0, 0x00844f60) are 100% in PLAYER after regeneration.
- TypedStatsItem update and AsyncHttpQueue are not lost against sweep20 either.
- **genericConvert<ProtectedString> / <ContentId>** (0x0066f7f0 / 0x00701c40) stay at their sweep20 score, 94.78% effective, so they are not lost.
- `StringConverter<ProtectedString>::convertToValue` 0x0066f460 is now paired but not at 100%, so its marker stays STUB. `<ContentId>` 0x008d3540 is FUNCTION.

## Final figures (sweep20 in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| **Entries at effective 100%** | **12,200** (11,997) | **11,547** (11,378) |
| Non-library entries | 9,500 (9,322) | 8,957 (8,815) |
| **Functions at 100%** | **9,228** (9,069) | **8,792** (8,658) |
| **Non-library functions at 100%** | **6,595** (6,461) | **6,249** (6,142) |
| Functions at 100%: owner / Rendering (minus g3d) / third party | 6,586 / 2 / 2,640 (6,451 / 2 / 2,616) | 6,238 / 3 / 2,551 (6,131 / 3 / 2,524) |
| Vftables at 100%: owner / Rendering / third party | 2,888 / 1 / 13 (2,846 / 1 / 11) | 2,696 / 1 / 8 (2,662 / 1 / 7) |
| Vftables at 100% / paired | 2,902 / 3,736 (2,858 / 3,688) | 2,705 / 3,666 (2,670 / 3,624) |
| reccmp Implemented, with libraries | 10,721 / 40,550 (10,564) | 10,333 / 42,384 (10,196) |
| reccmp Implemented, `--nolib` | 7,856 / 37,646 (7,722 / 37,669) | 7,612 / 39,546 (7,494 / 39,565) |
| App markers: FUNCTION + FOLDED + STUB | 1,257 + 28 + 506 (1,230 + 28 + 507) | 1,211 + 27 + 512 (1,184 + 27 + 513) |
| Base markers | 93 + 0 + 19 (93 + 0 + 19) | 87 + 0 + 20 (87 + 0 + 20) |
| Network markers | 126 + 2 + 22 (117 + 2 + 21) | 117 + 1 + 20 (108 + 1 + 19) |
| Shell markers (FUNCTION + STUB) | 90 + 20 (83 + 12) | 58 + 8 (58 + 8) |

The owner / third-party split comes from the .obj in the link map (build/sweep21/split.py).

**Rows** (PLAYER, tools/row_coverage.py on P-final; per-function CSVs in build/sweep21/row-*.csv):

| Row | At 100% / total | Share | sweep20 |
|---|---|---|---|
| 5a | 139/155 | 89.7% | 139 |
| 5b | 1,109/2,979 | 37.2% | 1,108 |
| 5c | 105/158 | 66.5% | 105 |
| 5d | 475/1,554 | 30.6% | 471 |
| 5e | 504/1,335 | 37.8% | 503 |
| 5f | 748/3,294 | 22.7% | 717/3,281 |
| 5g (row_coverage_5g.py) | 331/612 | 54.1% | 331 |
| 5h | 534/1,359 | 39.3% | 534 |
| 5k | 1,037/3,493 | 29.7% | 997 |

- **5f:** network-5f-5 reported 749. Row 5f's range grew by 13 functions (the GuidRegistryService and Server markers widen it).
- **5k:** +40, against shell-5k-5's own 1,029. SetFullscreen and the regenerated pairings account for the rest.
- **5b:** +1 only. The GameSettings, GameBasicSettings and GlobalBasicSettings files are mostly outside 5b's file list.

**Near misses** (app_sweep report, build/sweep21/app-sweep.csv; markers not at 100%):

| Target | Short | ebp-frame | inlining-size | other | unpaired | unpaired-callee | x87-vs-sse | assert-file-line | data-ref | stack-cookie | At 90% or more | Pairing-only |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| PLAYER | 559 | 178 | 169 | 125 | 48 | 12 | 20 | 4 | 3 | 0 | 53 | 15 |
| STUDIO | 561 | 174 | 165 | 130 | 37 | 27 | 20 | 4 | 3 | 1 | 77 | 30 |

The closest PLAYER near misses (newInstance and createAutoJoints have left this list):

| Function | Address | Score | Shape |
|---|---|---|---|
| TaskScheduler::remove | 0x009b52b0 | 99.49 | pairing only |
| ScriptContext::setTimeout | 0x0067eee0 | 99.41 | pairing only |
| TextXmlWriter::serializeNode | 0x00628a60 | 99.39 | other |
| TaskScheduler::setThreadCount | 0x009b7ec0 | 99.26 | pairing only |
| World::destroyAutoJoints | 0x00749340 | 99.23 | other |
| TaskScheduler::getJobsInfo | 0x009b75f0 | 99.21 | pairing only |
| GuiService::openBrowserWindow | 0x0069a540 | 99.08 | pairing only |
| TaskScheduler::getJobsByName | 0x009b78f0 | 98.99 | pairing only |
| DebrisService::addItem | 0x00845000 | 98.98 | pairing only |
| InsertService::insert | 0x007d6a20 | 98.90 | pairing only |
| TaskScheduler ctor | 0x009b55e0 | 98.88 | other |
| DataModel::onlyJobsLeftForThisArbiterAreGenericJobs | 0x0063a120 | 98.68 | other |
| Peer::~Peer | 0x005b8bf0 | 98.68 | pairing only (the `ret` twin 0x00528720) |
| TaskScheduler::setJobsExtendedStatsWindow | 0x009b7760 | 98.55 | pairing only |
| Joint::Joint | 0x00821410 | 98.47 | pairing only |

## Checks

| Check | PLAYER (sweep20) | STUDIO (sweep20) |
|---|---|---|
| decomplint: function_out_of_order | 313 (313) | 317 (317) |
| decomplint: duplicate_offset | 12 (12) | 4 (4) |
| reccmp-vtable: traceback | none | none |
| reccmp-vtable: "Problem with adjuster thunk" lines | 13 (13) | 15 (15) |
| reccmp-datacmp: traceback | none | none |
| reccmp-datacmp: variables | 3,061 (3,013) | 2,948 (2,919) |
| reccmp-datacmp: issues | 55 (55) | 58 (58) |
| check_folded: annotated addresses | 2,928 (2,879) | 2,706 (2,672) |
| check_folded: folding problems | 0 | 0 |
| check_folded: "Debug data out of sync" lines | 19 (19) | 19 (19) |
| reccmp_addr_padding | 1,695 files, 0 bad | |
| check_2016_copies | silent | |

decomplint exits 1 under `--warnfail`, as in every sweep, with findings identical to sweep20's.

## Saved

- **Final JSONs:**
  - build/sweep21/P-final.json (= PA4)
  - build-studio/sweep21/S-final.json (= SA4)
- **Scored exe, pdb, map and pdbview:** build/sweep21/scored/ and build-studio/sweep21/scored/.
- **Alongside the finals:**
  - every round's JSON and .txt (P1–P14, PA1–4; S1–S14, SA1–4), gen logs and rounds-*.log;
  - P/S-nolib.json and .txt, -vtable.txt, -datacmp.txt and -folded.txt;
  - {PLAYER,STUDIO}-decomplint.txt (+ .norm), app-sweep.csv, row-*.csv, reorder-plan-pre.txt and stubs.log;
  - build logs (build1, build2, build-PA/SA).
- **Helper scripts** in build/sweep21/: the sweep20 set with paths moved to sweep21, plus `pipeline.sh` (stubs, relink, rounds) and `rebuild.sh` (rebuild both targets, then PA/SA rounds).

## Files changed by me

- **CMakeLists.txt:**
  - View.cpp in player_client;
  - `/D_MBCS` on App (the `ogre_library(App …)` line), OGRE_BASE_FLAGS and OGRE_NETWORK_FLAGS, with the deciding functions in a comment.
- **reccmp/:** regenerated player-* and studio-* (libdata, symbols, studio-stl).
- **Client/_generated/{player,studio}/stubs_00.cpp and unresolved.txt**, through gen_link_stubs.
- **Marker words:** through app_sweep (the promotions listed above), plus View.cpp:210 by hand. No body changed and no line moved.
- No change to tools/ida/gen_reccmp_csv.py, tools/app_sweep.py or tools/reorder_markers.py.

## Unresolved

- **App, GameBasicSettings:**
  - The EventDescBase<GameBasicSettings,void(bool)>::connect cast at Event.h:239 should take Lighting's static-adjust path, not the `isA` checked path. That brings back Lighting's connectGeneric (PLAYER 0x00707830 / STUDIO 0x007d3620) and pairs GBS's connect with 0x006440c0.
  - STUDIO `EnumDesc<UploadSetting>::convertToValue` 0x0073cfc0: our link takes the COMDAT from GameBasicSettings.cpp.obj; 2013 STUDIO's copy inlines singleton().
- **Shell:** the shell's out-of-line `FactoryProduct<SoundService>::className` (0x004089f0) costs the SoundService vftables 0x00b44908 / 0x00b451d0. 2013's shell create 0x0043c7d0 calls the engine's 0x006ee210.
- **gen_link_stubs unknowns** (IDA owner: give them names with addresses):
  - PLAYER 13: the CSHA1 ×6, TCPInterface / PacketizedTCP ×4, `UserInput::PostUserInputMessage`, `CRenderSettings::getResolutionPreset` and `VideoControl::startRecording(SoundService*)`.
  - STUDIO 10: the CSHA1 ×6 and TCPInterface / PacketizedTCP ×4.
- **Unresolved data:**
  - FLog::RobloxWndInit (App: FastLogSettings.cpp, .data 0x00d30edd, default 1).
  - `CRenderSettings::aaSamples` is now referenced as **public** (`?aaSamples@CRenderSettings@RBX@@2W4…`). That comes from the shell's stand-in in Client/ClientBase/RenderSettingsItem.h:159. The Rendering header declares it protected. It is unresolved either way.
- **/D_MBCS:** measured only for App, Base and Network. Whether Rendering (RbxOgre, GfxAdapters) or g3d also want it is the Rendering contributor's call. A separate no-MBCS A/B on the integrated tree was not run.
- **Carried from the agents** (owner edits outside this task), unchanged from their reports:
  - **App:**
    - NetworkSettings private members (+0x110, +0xf0, +0x79/+0x7a/+0xdc);
    - PhysicsService::numSenders;
    - FFlag::UseClusterCache;
    - DebugSettings::videoMemory / gfxcard;
    - the ConstProperty assert line in Property.h (255);
    - Selection as a real Service, then the GlobalAdvancedSettings ctor 0x00658580;
    - the Game ctor 0x00655e70;
    - openState (0.583);
    - DataModel's unnamed functions;
    - an include-spelling fix so Players.cpp can include FriendService.h.
  - **Network:** the Server::onServiceProvider packet-cache classes and NetworkOwnerJob, the Replicator ctor/dtor, and Players::onChildRemoving.
  - **IDA owner:**
    - app-14's renames (inFullScreen 0x006f61f0 and the rest);
    - shell-5k-5's note that rows 0x008445f0… are DebrisService's `weak_ptr<Instance>` chain. Those rows are 100% this round under their current names, so nothing is lost while the names stay as they are.
  - **Rendering contributor:** FrameRateManager::ResumeAutoAdjustment and ViewBase's frame-data callback.
- **prime_list constructors:** still FUNCTION, through no-.reloc layout luck. Nothing flipped this wave.
