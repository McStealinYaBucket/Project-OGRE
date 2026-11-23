# integrate (workflow phase5-wave4), 2026-11-23

I integrated the three phase5-wave4 Work agents (xml-5c, script-5e-2, datamodel-5b-4) and applied script-5e-2's generator patch. Then I regenerated the CSVs, ran the sweep and reordered markers. Both targets are stable at the end: in the last rounds two scorings of the same link agree exactly, and a regeneration round gains nothing.

Every heavy command ran through tools/capped.sh, one build at a time. Nothing hit the cap. No commits.

## What I did

**Leftovers.** All three agents finished. All of their uncommitted work compiles in both targets from clean, so I kept all of it and reverted nothing. The deleted PhysicsJob/HeartbeatTask .cpp/.h files drop out of the globbed source lists on reconfigure, so CMakeLists.txt needed no edit.

**Generator patch.** I applied `script-5e-norm_sig.patch` to tools/ida/gen_reccmp_csv.py. It adds three `norm_sig` regexes for bool, enum and reference non-type template arguments, and `--selftest` passes. Most of this wave's gain beyond the agents' own measurements comes from it (see the totals).

**Clean rebuilds.**
- build/.ninja_log was corrupt, so I deleted it. After that, builds in build/ are incremental again: the post-sweep rebuild recompiled only 391 of 677 steps, and the stub relink was 3 steps.
- PLAYER: configure, `--target clean`, full build (677 steps, 0 errors).
- STUDIO: the same in build-studio with `--target studio`.

**Link stubs.** I deleted each exe and ran gen_link_stubs per target. Stubs went from 10 to 12 addresses per target:
- Added: `DataModel::physicsStep(float,double,double,int)` (PLAYER 0x00642850), which datamodel-5b-4 declared.
- Added: `G3D::Log::common()` (PLAYER 0x009d5c60), which xml-5c's loadInstances calls.
- No longer unresolved: `TextXmlParser::parse` and `XmlParser::XmlParser(streambuf*)`, which xml-5c defined.
- Newly unresolved, with no stub: `G3D::Log::println`. The Rendering fork has no G3D/Log.h.

**Regeneration rounds.** Each round pools the two latest runs of the current link.
- PLAYER: 9,068 → 9,181 → 9,399 → 9,404 → 9,404.
- STUDIO: 8,858 → 8,975 → 9,159 → 9,162 → 9,162.
- The agents measured 9,068–9,211 in their own builds. The jump from round 2 to round 3 is the norm_sig pairs taking effect in the libdata and symbols rows.

**Sweep.**
- `app_sweep set FUNCTION`, then scored both targets (PSW/SSW), then `apply`. After the rebuild I ran `apply` again on the final scores.
- Result, all swept dirs: PLAYER 1,077 FUNCTION + 49 FOLDED + 649 STUB; STUDIO 1,049 + 49 + 638.
- Demoted FUNCTION → STUB:
  - SendPhysics::onMovingAssemblyRootAdded/Removing in both targets (PLAYER 0x008e5fe0 / 0x008e60b0). This is pairing churn (see the losses below).
  - Http::httpGetPostWinHttp PLAYER 0x00826990. This is a no-.reloc coincidence; STUDIO 0x0091cd80 went the other way, to FUNCTION.
- Promoted to FUNCTION: Math::isDenormal and isNanInfDenormVector3 for STUDIO.

**Two marker fixes in SerializerV2.h**, comment lines only, with line count unchanged. Both entries were at 100% in sweep10 and the xml agent's runs, and went *absent* once the regenerated CSVs let the markers take over:
- The `SYNTHETIC` nameref for the MemoryBinder deleting destructor said `??_GMemoryBinder@@UAEPAXI@Z`, but our PDB names it `??_EMemoryBinder@@UAEPAXI@Z`. I fixed the nameref, and PLAYER 0x006fc970 / STUDIO 0x0074af00 are back at 100%.
- `TEMPLATE: PLAYER 0x006fa600` named `map<string,InstanceHandle>::_Max`. Our link folds that body into the `map<string,shared_ptr<Script>>` copy, so the name is not in our PDB and the marker could not pair. It also blocked the CSV row that paired it in sweep10. I replaced it with a two-line comment saying why there is no marker. 0x006fa600 is back at 100%.

**Reorder.**
- `reorder_markers apply` on Client/App/v8world/Primitive.cpp: PLAYER descents 14 → 13, STUDIO 12 → 11. I rebuilt both targets afterwards and no 100% was lost (the two PLAYER losses after that rebuild are no-.reloc coincidences; see below).
- Not reordered:
  - The wave's new files: SerializerV2.cpp (2), XmlSerializer.cpp (1), ScriptContext.cpp (26) and ThreadRef.cpp (11) are pinned by assert lines. LuaLibrary.cpp and ScriptEvent.cpp have descents that cross runs.
  - type.cpp: its plan breaks the compile with C2908, as before.
  - LuaInstanceBridge.cpp: still needs a private member.

## Final figures (sweep10 baseline in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| Entries at effective 100% | **9,405** (9,029) | **9,163** (8,826) |
| Non-library entries | 6,932 (6,561) | 6,731 (6,400) |
| **Functions at 100%** | **7,017** (6,665) | **6,812** (6,494) |
| **Non-library functions at 100%** | **4,611** (4,264) | **4,427** (4,115) |
| Functions at 100%: owner side / Rendering (minus g3d) / third party | 4,598 / 2 / 2,417 | 4,416 / 3 / 2,393 |
| Vftables at 100%: owner / Rendering / third party | 2,307 / 1 / 11 | 2,294 / 1 / 7 |
| Vftables at 100% | 2,319 / 3,307 (2,295 / 3,288) | 2,302 / 3,236 (2,283 / 3,217) |
| reccmp Implemented, with libraries | 8,871 / 40,548 (8,371) | 8,637 / 42,382 (8,132) |
| reccmp Implemented, `--nolib` | 6,201 / 37,838 | 6,055 / 39,674 |
| App markers: FUNCTION + FOLDED + STUB | 938 + 39 + 607 | 918 + 39 + 594 |
| Base markers: FUNCTION + FOLDED + STUB | 84 + 0 + 26 | 76 + 0 + 29 |
| Network markers: FUNCTION + FOLDED + STUB | 52 + 2 + 9 | 52 + 2 + 9 |
| Sweep scope: markers at 100% / short | 1,126 / 649 | 1,098 / 638 |

The owner / Rendering / third-party split goes by the `.obj` the link map puts at each recompiled address (build/sweep11/split.py). Library-flagged rows count as third party.

**Rows** (PLAYER, current ranges; sweep10 on the same ranges in brackets):

| Row | At 100% / total | Share | sweep10 |
|---|---|---|---|
| 5a | 138/155 | 89.0% | 138 |
| 5b | 569/1,653 | 34.4% | 527 |
| 5c | 86/152 | 56.6% | 32 |
| 5d | 251/1,504 | 16.7% | 252 |
| 5e (Client/App/script/*.cpp) | 297/848 | 35.0% | 255 |
| 5f | 498/3,259 | 15.3% | 489 |
| 5g (row_coverage_5g.py) | 301/601 | 50.1% | 300 |
| 5h | 496/1,359 | 36.5% | 489 |
| 5k | 511/3,493 | 14.6% | 510 |

5d lost 1: SendPhysics, see the losses. Per-function CSVs are in build/sweep11/row-*.csv.

**Near misses** (app_sweep report, build/sweep11/app-sweep.csv):
- PLAYER, 649 short:
  - By shape: inlining-size 198, ebp-frame 178, other 163, unpaired 50, unpaired-callee 30, x87-vs-sse 22, data-ref 4, assert-file-line 2, stack-cookie 2.
  - 76 are at 90% or more; 34 are pairing-only.
- STUDIO, 638 short:
  - By shape: inlining-size 196, ebp-frame 176, other 165, unpaired 38, unpaired-callee 33, x87-vs-sse 22, data-ref 3, stack-cookie 3, assert-file-line 2.
  - 84 are at 90% or more; 36 are pairing-only.
- Closest (PLAYER): LegacyContentTable ctor 0x008f08a0 99.97, httpGetPostWinHttp 0x00826990 99.89 (coincidence), ~DataModel 0x0064d850 99.76, asyncRequest 0x00870430 99.59, ~TaskScheduler 0x009b4e00 99.52, TaskScheduler::remove 0x009b52b0 99.49, RunService::start 0x0072d300 99.37, MemoryBinder::processID 0x006fc500 99.33.

## Every 100% lost against sweep10 (read with --verbose)

**PLAYER: 6 lost, 382 gained.**
- **Pairing only: our folded template body pairs with another original twin.**
  - SendPhysics::onMovingAssemblyRootAdded 0x008e5fe0 (0.984) and onMovingAssemblyRootRemoving 0x008e60b0 (0.955). The call to `signal_with_args<1,void(Primitive*)>::operator()` folds into the lua_State copy in our link. datamodel-5b-4 reported the same.
  - sBoolValue `EventDescImpl::connectGeneric` 0x0079d8e0 (0.990). Its `connect` callee pairs with Lighting's new `EventDescBase<Lighting,void(bool),…>` twin, which our linker folds with it.
  - `_Tree<string,shared_ptr<Script>>` 0x004bc580 (0.996): one callee is unpaired (the twin `_Max`/`_Buynode` resolution moved).
- **No-.reloc coincidences** after the rebuild:
  - RakNet `ReliabilityLayer::IsOlderOrderedPacket` 0x005c8930 (library, 0.95): the immediate in `lea edx,[eax-imm]` now equals the `FactoryProduct<SelectionBox>` Complete Object Locator.
  - `Http::httpGetPostWinHttp` 0x00826990 (0.999): `and edx, imm`, the same COL.
  - Both flip with every relink.
- MemoryBinder D0 0x006fc970 and `_Tree::_Max` 0x006fa600 were lost too, and I fixed both (above).

**STUDIO: 9 lost, 346 gained.**
- The same three pairing-churn rows as PLAYER: SendPhysics 0x00a042e0 / 0x00a043b0, connectGeneric 0x008c17d0.
- **Library twins between the /GS client shell and App:**
  - `pair<string,string>::~pair` 0x00403d80 (0.58), `char_separator` copy ctor 0x0048c660 (0.70) and token_iterator 0x0048c540 (0.58). The library row for the shell's `/GS` copy now pairs with our native App copy, which has no cookie.
  - `G3D::GImage::Error::~Error` 0x00528680 is absent (folded in our link), so `basic_gzip_decompressor::~basic_gzip_decompressor` 0x00687690 (0.975) calls an unpaired twin.
- The MemoryBinder D0 0x0074af00 was fixed, as in PLAYER.

## Checks

| Check | PLAYER (sweep10) | STUDIO (sweep10) |
|---|---|---|
| decomplint: function_out_of_order | 276 (276) | 285 (284) |
| decomplint: duplicate_offset | 12 (12) | 4 (4) |
| reccmp-vtable: traceback | none | none |
| reccmp-vtable: "Problem with adjuster thunk" lines | 11 | 13 |
| reccmp-datacmp: traceback | none | none |
| reccmp-datacmp: variables | 2,252 (1,721) | 2,136 (1,669) |
| reccmp-datacmp: issues | 55 (51) | 58 (53) |
| check_folded: annotated addresses | 2,429 | 2,284 |
| check_folded: folding problems | 0 | 0 |

- decomplint exits 1 under `--warnfail`, as in every sweep. STUDIO's +1 is from the wave's assert-pinned files.
- The same 19 "Debug data out of sync near Reflection.h:130" lines as sweep10.
- reccmp_addr_padding: 1,665 files, 0 bad.
- check_2016_copies: silent.

**The new datacmp issues are real source differences that the regenerated libdata now pairs:**
- In 2013, `RBX::sGlobalAdvancedSettings` points to "GlobalSettings" (PLAYER 0xb31084) and `sGlobalBasicSettings` to "UserSettings". Ours spell "GlobalAdvancedSettings" and "GlobalBasicSettings".
- `sSettings` differs as well.
- `sHopper` pairs with a "GuiItem" string. The generator log shows a signature-vs-position conflict between `Described<GuiItem>` and `Described<Hopper>` (STUDIO 0x00432210), so this one may be a pairing question rather than a source one.
- Also new: two `??_R0` WARN rows (`G`, `_J`).

## Saved

- Final JSONs: build/sweep11/P-final.json and build-studio/sweep11/S-final.json.
- Alongside them: P/S-nolib.json, -vtable.txt, -datacmp.txt, -folded.txt, PLAYER/STUDIO-decomplint.txt, app-sweep.csv and row-*.csv.
- Scored exe, pdb, map and pdbview: build/sweep11/scored/ and build-studio/sweep11/scored/.
- Every round's JSONs and gen logs: build*/sweep11/.
- Helper scripts: build/sweep11/{score,gen,round,v}.sh, summ.py, split.py.

## Files changed

- tools/ida/gen_reccmp_csv.py: the norm_sig patch.
- reccmp/player-{boost,libdata,symbols}.csv and reccmp/studio-{boost,libdata,symbols}.csv, regenerated.
- Client/_generated/{player,studio}/stubs_00.cpp and unresolved.txt.
- Client/App/include/v8xml/SerializerV2.h: two marker comment fixes.
- Client/App/v8world/Primitive.cpp: reordered.
- Marker words in SendPhysics.cpp, WinHttp.cpp and Math.cpp. The sweep's other flips landed in the wave's own files.

## Unresolved

- **Pairing churn from our linker's folds** (no file owner). Each fix would keep separate copies in our link, as the original does:
  - `signal_with_args<1,void(Primitive*)>` folds with the lua_State copy, which costs SendPhysics::onMovingAssemblyRootAdded/Removing their 100% in both targets.
  - Lighting's `EventDescBase<…,void(bool)>::connect` folds with sBoolValue's, which costs connectGeneric 0x0079d8e0 / 0x008c17d0.
  - `shared_from<T>` folds into `shared_from<GuiItem>`, which keeps RunService::start 0x0072d300 at 99.4%.
- **Shell vs App library twins (STUDIO):** pair<string,string> dtor, char_separator and token_iterator 0x00403d80 / 0x0048c540 / 0x0048c660 need the shell's own `/GS` instantiations, the same mechanism as placeholder.cpp.
- **2013 class-name strings** (datacmp): sGlobalAdvancedSettings = "GlobalSettings", sGlobalBasicSettings = "UserSettings" (v8datamodel owner; check against the RTTI and the iOS 10-29 strings first). Check sHopper / Described<GuiItem> vs Described<Hopper> pairing too.
- **G3D::Log::println** has no stub or oracle address. G3D/Log.h belongs to the Rendering contributor (xml-5c's request).
- From the agents' lists, unchanged: Name::declare inline in Util/Name.h; TaskScheduler::Job::Stats layout (Base); the fork Lua include override on ScriptContext.cpp, ThreadRef.cpp and LuaInstanceBridge.cpp; spin_mutex as tbb 2.1; rbx::atomic out-of-line calls; LuaSettings offsets; Instance::fw() out of line; the World 0xb8 layout; ContentId and ContentProvider signatures; InstanceHandle ctors out of line.
- The ida/exports names for 0x0067fcd0 still carry the NonFactoryProduct spelling (next IDA sync).
- type.cpp and LuaInstanceBridge.cpp reorders still break the compile.
