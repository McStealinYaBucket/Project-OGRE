# integrate (workflow phase5-wave18), 2026-11-26

This integrates the three phase5-wave18 Work agents: app-18, shell-5k-9 and network-5f-9. All three finished, and their uncommitted work builds from clean in both targets with 0 failures. All of it stays; nothing was reverted.

| | sweep24 | Lost | Gained | **sweep25** |
|---|---|---|---|---|
| PLAYER | 12,766 | 2 | 180 | **12,944** (+178) |
| STUDIO | 12,014 | 6 | 147 | **12,155** (+141) |

Both targets end above every agent's private figure (app-18 12,815 / 12,052; shell-5k-9 12,778 PLAYER; network-5f-9 12,918 / 12,116). The 8 losses are not code: 2 are a linker-fold change (PLAYER) and 6 are no-.reloc coincidences (STUDIO). Details are under "Losses explained".

Housekeeping: every heavy command ran through tools/capped.sh, one build at a time, and none hit the cap. No commits, no IDA database opened, nothing under Client/Rendering/ edited. Only the generator wrote the shared reccmp/.

## What I did

1. **Saved the agents' tree** to build/sweep25/ (pre.diff, pre-status.txt, pre-untracked.tgz).
2. **CMakeLists.txt:** added `Client/WindowsClient/UserInput.cpp` and `Client/WindowsClient/Teleporter.cpp` to `player_client`, after FunctionMarshaller.cpp, where shell-5k-9 linked them privately. Game.cpp and the rest come in through the App and Network globs.
3. **`app_sweep set FUNCTION`** (595 PLAYER, 593 STUDIO), then clean builds (reconfigure, `--target clean`, delete the exe PDB, build). PLAYER took 714 steps and STUDIO built cleanly; both had 0 failures.
4. **Exe PDBs deleted before every link** (clean build, gen_link_stubs relink, post-sweep rebuild), as app-18 asked. link.exe otherwise keeps type records from objects that are no longer linked.
5. **reccmp guard:** I scored through app-18's wrapper (copied to build/sweep25/rc.py; it catches only the zero-size-element assert). **It never fired**: no run printed `ZERO-SIZE-ELEMENT`. With a fresh PDB, the `CRegParser` typemap crash does not reproduce, so the guard is only a safety net.
6. **`gen_link_stubs.py` and relink:**
   - PLAYER: 29 unresolved, 23 stubs, unknown 13.
   - STUDIO: 23 unresolved, 20 stubs, unknown 10.
   - **`ClassRegistrar<ControllerService>::registrar` is no longer unresolved** in either target.
   - **The duplicate `ogre_stub_0045f1b0` is gone** (stubs_00.cpp lost 8 lines).
   - New PLAYER unresolved names:
     - `SleepingJob::wake`, `Network::initWithPlayerSecurity` and `ViewBase::FUN_00487d50` (unknown).
     - `DirectInput8Create@20` and `IID_IDirectInput8A` (unparsed; no dinput8 or dxguid lib).
7. **Generator (tools/ida/gen_reccmp_csv.py; `--selftest` passes):**
   - **MATCH_DROP emptied.** The STUDIO shell placeholder now instantiates `bind<weak_ptr<DataModel>>`. In the final run, 0x0042b260 and 0x0042e370 are at 100, and their engine twins 0x0098a310 and 0x0098a120 also stay at 100.
   - **Both wave-17 MATCH_FIX entries dropped** (PLAYER 0x005bd080 and 0x005baa50). Proof: regenerating PLAYER from P11+P12 with and without them gives byte-identical reccmp/ CSVs (build/sweep25/csv-with/, gen_with.py.bak). Both addresses are at 100 in the final run.
   - The comment and docstring are updated. Only the GuiMain `{28}` thunk entries remain in MATCH_FIX.
8. **Regeneration rounds** (score twice, regenerate from the pair, until a round gains nothing). Entries at effective 100%:
   - PLAYER 12,788 → 12,833 → 12,906 → 12,940 → 12,944 → 12,944 (P1–P12).
   - STUDIO 12,022 → 12,054 → 12,120 → 12,150 → 12,155 → 12,155 (S1–S12).
9. **Sweep:** `app_sweep apply P12 S12`. Against HEAD (markers.py):
   - PLAYER: 9 promoted, 15 new FUNCTION, 12 new STUB.
   - STUDIO: 6 promoted, 12 new FUNCTION, 11 new STUB.
   - No demotion.
10. **Shell markers** (outside app_sweep's scope; shellcheck on P12): `View::HandleWindowsMessage` 0x00466510 and `UserInput::PostUserInputMessage` 0x0045f1b0 are both at 100 now that the stub is gone. I flipped them to FUNCTION, a marker word only.
    - PostUserInputMessage pairs at 100 even though `SleepingJob::wake` is still unresolved.
    - No shell FUNCTION is below 100, and no STUB in any directory except Rendering is at 100 (shellcheck and stubcheck on the final runs).
11. **Reorder:** `reorder_markers plan` differs from sweep24's post-sweep plan in four files, and none of them can be fixed by moving functions:

    | File | Descents (PLAYER / STUDIO) | Status |
    |---|---|---|
    | SecurityContext.cpp | 1 / 2 | New. Impersonator markers at shell-copy addresses; isInRole's assert is pinned at line 33 (app-18). |
    | DataModel.cpp | 18 / 17 | +1 PLAYER, from the `~Implementation` address (app-18); assert-pinned. |
    | Replicator.cpp | 2 / 2 | +1, the readItem/onPropertyChanged STUBs below `~Replicator` (network-5f-9); assert-pinned at 348/350. |
    | GuiBuilder.cpp | 2 / 2 | Unchanged count; the plan now says "not fixable". |

    Nothing was applied.
12. **Rebuild and stability rounds:** delete the PDBs, rebuild both targets (428 steps for PLAYER, because the marker words in App headers changed), then PC1–4 and SC1–4. **PLAYER scored 12,944 and STUDIO 12,155 in every run**, so the result is stable.
    - `app_sweep apply PC4 SC4` changes nothing (the `git diff` md5 is identical before and after).
    - There are no no-.reloc demotions to re-sweep: all 6 STUDIO coincidences are Lua library rows, which carry no marker.

## Losses explained (against build/sweep24/P-final.json and build-studio/sweep24/S-final.json)

- **PLAYER `TypedPropertyDescriptor<ContentId>` vftable 0x00b3388c and `PropDescriptor<BaseScript, ContentId>` vftable 0x00b33904 (both 92.86).**
  - `--verbose` shows a single slot differs, slot 0x18 `copyValue`.
  - 2013 has one body, 0x00669fd0, for both `copyValue<ContentId>` and `copyValue<SoundId>`. In sweep24 our linker also folded the two (one body at 0x0054f2b0, with a shared `__ehhandler$` in the EnumProperty.cpp group).
  - Now our two bodies are byte-identical except for their `__ehhandler$` stub. The ContentId handler folds into the UDim.cpp handler group (0x00745098) and the SoundId handler into the Sound.cpp group (0x00744478). Because the handlers differ, the bodies no longer fold: ContentId's copy is at 0x0057bd40 and SoundId's at 0x00554850.
  - 0x00669fd0 pairs with the SoundId body (still at 100). The ContentId vftables lose that slot, and the SoundId vftable 0x00b44c2c rises from 78.57 to 92.86.
  - This is linker ICF grouping of EH handlers, not code. A fix would have to make the two handlers' FuncInfo fold again.
- **STUDIO Lua library rows** (symbexec 0x00926cc0, luaV_execute 0x00a30a90, constructor 0x00a35730, patchtestreg 0x00a5d800, removevalues 0x00a5d880, luaK_setreturns 0x00a5dc30; 97.8–99.9).
  - `--verbose` on all six: a Lua mask immediate (`and/test reg, <imm>`) is read as the string `"Points        : "`, which our STUDIO link now places at that address.
  - This is the known no-.reloc coincidence; app-18 and network-5f-9 reported the same six.

**Not lost after all:** the 4 STUDIO `list<connection>::_Buynode` entries that app-18 and network-5f-9 saw lost on their private links (0x006d13d0 and its three callers) are at 100 in the integrated link. 0x006d13d0 is at 1.0, and the shell copy 0x0049de90 is not paired.

## Final figures (sweep24 in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| **Entries at effective 100%** | **12,944** (12,766) | **12,155** (12,014) |
| Non-library entries | 10,181 (10,017) | 9,558 (9,412) |
| **Functions at 100%** | **9,856** (9,701) | **9,296** (9,176) |
| **Non-library functions at 100%** | **7,160** (7,019) | **6,746** (6,621) |
| Functions at 100%: owner / Rendering (minus g3d) / third party | 7,149 / 2 / 2,705 (7,003 / 2 / 2,696) | 6,736 / 3 / 2,557 (6,611 / 3 / 2,562) |
| Vftables at 100%: owner / Rendering / third party | 3,007 / 1 / 10 (2,983 / 1 / 11) | 2,800 / 1 / 8 (2,779 / 1 / 8) |
| Vftables at 100% / paired | 3,018 / 3,857 (2,995 / 3,826) | 2,809 / 3,781 (2,788 / 3,754) |
| reccmp Implemented, with libraries | 11,359 / 40,551 (11,155 / 40,551) | 10,890 / 42,384 (10,720 / 42,384) |
| reccmp Implemented, `--nolib` | 8,429 / 37,584 (8,240 / 37,598) | 8,154 / 39,536 (7,985 / 39,535) |
| App markers: FUNCTION + FOLDED + STUB | 1,289 + 29 + 517 (1,272 + 29 + 520) | 1,239 + 28 + 523 (1,226 + 28 + 526) |
| Base markers | 93 + 0 + 19 (93 + 0 + 19) | 87 + 0 + 20 (87 + 0 + 20) |
| Network markers | 182 + 3 + 54 (175 + 3 + 50) | 167 + 2 + 47 (162 + 2 + 41) |
| Shell markers (FUNCTION + STUB) | 106 + 35 (98 + 28) | 58 + 8 (58 + 8) |

The owner and third-party split comes from the .obj that the link map puts at each recompiled address (split.py).

**Rows** (PLAYER, tools/row_coverage.py on the final link; per-function CSVs are in build/sweep25/row-*.csv):

| Row | At 100% / total | Share | sweep24 |
|---|---|---|---|
| 5a | 139/155 | 89.7% | 139/155 |
| 5b | 1,288/3,305 | 39.0% | 1,243/3,305 |
| 5c | 105/158 | 66.5% | 105/158 |
| 5d | 475/1,554 | 30.6% | 475/1,554 |
| 5e | 564/1,337 | 42.2% | 515/1,335 |
| 5f | 917/3,327 | 27.6% | 906/3,327 |
| 5g (row_coverage_5g.py) | 334/613 | 54.5% | 332/612 |
| 5h | 549/1,359 | 40.4% | 548/1,359 |
| 5k | 1,119/3,493 | 32.0% | 1,088/3,493 |

**Near misses** (app_sweep report, build/sweep25/app-sweep.csv; markers not at 100%):

| Target | Not at 100% | ebp-frame | inlining-size | other | unpaired | unpaired-callee | x87-vs-sse | assert-file-line | data-ref | regalloc-sched | stack-cookie | At 90% or more |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| PLAYER | 602 | 198 | 186 | 129 | 48 | 12 | 21 | 4 | 3 | 1 | 0 | 55 |
| STUDIO | 599 | 189 | 178 | 131 | 37 | 34 | 21 | 4 | 3 | 1 | 1 | 84 |

**Closest PLAYER misses:**

| Function | Address | Score |
|---|---|---|
| TaskScheduler::remove | 0x009b52b0 | 99.49 |
| ScriptContext::setTimeout | 0x0067eee0 | 99.41 |
| TextXmlWriter::serializeNode | 0x00628a60 | 99.39 |
| Players::onChildAdded (new; waits on the Impersonator pairing) | 0x0056ff40 | 99.28 |
| TaskScheduler::setThreadCount | 0x009b7ec0 | 99.26 |
| LegacyLock::Implementation ctor (new; assert line 1054) | 0x00641e60 | 99.25 |
| World::destroyAutoJoints | 0x00749340 | 99.23 |
| getJobsInfo | 0x009b75f0 | 99.21 |
| GuiService::openBrowserWindow | 0x0069a540 | 99.08 |

**Closest STUDIO misses:**

| Function | Address | Score |
|---|---|---|
| DataModel::getJobsExtendedStats | 0x00768d70 | 99.75 |
| ScriptContext::ypcall | 0x007ed020 | 99.73 |
| Game::Game (operator= twin) | 0x007cddd0 | 99.65 |
| TaskScheduler::remove | 0x00a6d0a0 | 99.49 |

## Checks

| Check | PLAYER (sweep24) | STUDIO (sweep24) |
|---|---|---|
| decomplint: function_out_of_order | 317 (314) | 321 (318) |
| decomplint: duplicate_offset | 12 (12) | 4 (4) |
| reccmp-vtable: traceback | none | none |
| reccmp-vtable: "Problem with adjuster thunk" lines | 13 (13) | 15 (15) |
| reccmp-datacmp: traceback | none | none |
| reccmp-datacmp: variables | 3,197 (3,162) | 3,068 (3,031) |
| reccmp-datacmp: issues | 74 (59) | 78 (61) |
| check_folded: annotated addresses | 3,106 (3,058) | 2,832 (2,804) |
| check_folded: folding problems | 0 | 0 |
| check_folded: "Debug data out of sync" lines | 19 (19) | 19 (19) |
| reccmp_addr_padding | 1,708 files, 0 bad | |
| check_2016_copies | silent | |

- **decomplint**, per file (ooo.py):
  - PLAYER: SecurityContext.cpp 0 → 1, DataModel.cpp 17 → 18, Replicator.cpp 1 → 2.
  - STUDIO: SecurityContext.cpp 0 → 2, Replicator.cpp 1 → 2.
  - The agents explain all of these (step 11). As in every sweep, decomplint exits 1 under `--warnfail`.
- **check_folded** shows network-5f-9's PLAYER problem at 0x00656230 gone (0 problems).
- **datacmp +15 PLAYER / +17 STUDIO issues.** All of them are newly paired data, not code:
  - **13 `RBX::Lua::*Bridge::classLibrary` arrays plus one unnamed `classLibrary` per target.** openState now reaches registerClassLibrary. The `.func` slots are the usual function pointers that reccmp cannot see without `.reloc`. Some rows also pair the wrong array: orig 0x00b81704 holds `{"new", nullptr}`, but it is paired with our three-entry `Vector3Bridge::classLibrary`. Name these from the 2013 data (see Unresolved).
  - **`FFlag::DebugCrashEnabled`** (both targets): the 2013 `.data` byte is 1, ours is 0 (`bool DebugCrashEnabled = false;` in util/FastFlags.cpp). It is newly referenced by ScriptContext.
  - **STUDIO only:** `RBX::sOverlayDataModel` and one more boost::function `stored_vtable`.

## Saved

- **Final JSONs:** build/sweep25/P-final.json (= PC4) and build-studio/sweep25/S-final.json (= SC4).
- **Scored exe, pdb, map and pdbview.tsv:** build/sweep25/scored/ and build-studio/sweep25/scored/.
- **Per-round output:** every round's JSON and .txt (P1–12, PC1–4; S1–12, SC1–4), the gen logs, and rounds-*.log.
- **MATCH_FIX drop proof:** csv-with/ and gen_with.py.bak.
- **Check outputs:** P/S-nolib, -vtable, -datacmp, -dc-issues, -folded, {PLAYER,STUDIO}-decomplint.txt, ooo-*.txt, app-sweep.csv, row-*.csv, reorder-plan-pre.txt, stubs.log, and the build logs.
- **Scripts:**
  - The sweep24 set retargeted to sweep25, plus rc.py (the reccmp guard) and rebuild.sh (delete PDBs, rebuild, stability rounds).
  - loss.py and ooo.py (per-file decomplint diff).
  - checks.sh and score.sh now go through rc.py.

## Files changed by me

- CMakeLists.txt: UserInput.cpp and Teleporter.cpp added to `player_client`.
- tools/ida/gen_reccmp_csv.py: MATCH_DROP emptied, the two `_internal_accept_owner` MATCH_FIX entries removed, comments and docstring updated.
- reccmp/: regenerated player-* and studio-* (libdata, symbols, stl).
- Client/_generated/{player,studio}/: unresolved.txt and stubs, through gen_link_stubs.
- Marker words through app_sweep (no line moved).
- Two shell marker words set to FUNCTION: View.cpp:130 (0x00466510) and UserInput.cpp:115 (0x0045f1b0).
- No change to tools/app_sweep.py or tools/reorder_markers.py, and no function moved.

## Unresolved

- **App:**
  - `FFlag::DebugCrashEnabled` starts true in 2013 (`.data` byte 1 at PLAYER 0x00d3ae00); util/FastFlags.cpp has `false`.
  - The `Lua::*Bridge::classLibrary` data rows pair the wrong arrays (orig 0x00b81704 is a one-entry `{"new"}` library), so the arrays need names from the 2013 `.data`.
  - The items app-18 listed: setupDataModel, shutdownGameDataModel, openState 0.983, the Implementation ctor assert line, STUDIO ~Game and Game::Game, and the Impersonator shell copies (which also unblock Players::onChildAdded 0x0056ff40 at 99.28).
- **Linker fold:** `TypedPropertyDescriptor<ContentId>` and `<SoundId>::copyValue` no longer fold, because their `__ehhandler$` stubs land in different ICF groups. This costs PLAYER vftables 0x00b3388c and 0x00b33904. The fix is to find what changed the handler groups (a new unwind funclet twin from this wave's code).
- **Network:** declare `RBX::Network::initWithPlayerSecurity()` (unresolved, STUB 0x00526e10); the rest is network-5f-9's list.
- **IDA owner:**
  - PLAYER 0x0072b220 `SleepingJob::wake`, so gen_link_stubs can bind it (it is unresolved now).
  - The names in app-18.md and shell-5k-9.md.
- **Shell:** `DirectInput8Create` and `IID_IDirectInput8A` are unresolved (no dinput8.lib or dxguid.lib in the toolchain); the Application ctor depends on `rbx::atomic<int>()` staying out of line (Base).
- **Rendering contributor:** `ViewBase::FUN_00487d50` (RbxOgre 0x00487d50) and `CRenderSettings::minGameWindowSize` are unresolved.
- **No-.reloc churn:** the six STUDIO Lua rows will come back when the `"Points        : "` string moves.
- **gen_link_stubs unknowns:** PLAYER 13 (CSHA1 ×6, TCPInterface/PacketizedTCP ×4, wake, initWithPlayerSecurity, FUN_00487d50), STUDIO 10.
