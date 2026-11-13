# integrate (workflow phase5-wave2, wf_cba1073c-3f1)

INTEGRATE, REGENERATE, SWEEP, ORDER: all three steps are done, and both targets are stable at the end. Every heavy command ran through tools/capped.sh. I ran one build at a time and nothing hit the memory cap.

**Before building, I made three code fixes and one generator fix:**
- **Generator crash.** gen_reccmp_csv.py failed without --json (UnboundLocalError on `collections`). I moved the import to module level; the selftest passes.
- **ProfanityFilter.cpp.** ContainsProfanityWorker now has STUB markers at PLAYER 0x00668e90 / STUDIO 0x008a8420 and a STUB() body, and sits before ContainsProfanity in address order. ContainsProfanity no longer folds into the engine's only `xor al,al; ret` (0x00927cc0).
- **Instance.cpp:1143 / Instance.h:221.** The 2016 override Instance::onGuidChanged is gone. The lines are replaced by comments, so the line count is unchanged and no assert moves.

**Clean rebuilds and link stubs.**
- PLAYER rebuilt from clean in 8m40s, STUDIO in about 9 minutes.
- gen_link_stubs ran end to end for both targets. No new stubs (still 5 addresses each). Both unresolved.txt files gained FLog::GuiTargetLifetime.

**Regeneration rounds** (each pools the two latest runs of the current link):
- PLAYER went 6,557 → 7,234 → 7,579 → 7,699 → 7,737 → 7,753 → 7,759 → 7,759.
- STUDIO went 6,025 → 7,086 → 7,485 → 7,613 → 7,655 → 7,675 → 7,681 → 7,681.
- Every round's two scorings agreed exactly.
- Then the sweep (set FUNCTION, score, apply), and another round with no change.

**Further fixes, each measured on its own:**
- **(a) Arbiter::getNumPlayers removed** from Client/Base/include/rbx/TaskScheduler.h:60 (line replaced by a comment). It is a 2016 virtual: the 2013 Arbiter vftables have 6 slots (ExclusiveArbiter 0x00bb8af0), and DataModelArbiter 0x00b48950 has its deleting dtor at slot 6 and getNumPlayers at slot 7.
  - DataModelArbiter's vftable reached 1.0.
  - The swapped rows that datamodel-5b reported (0x0063e220 = getNumPlayers, 0x0064de30 = the ??_E thunk) are now right.
- **(b) MATCH_FIX in the generator** for the stale match: name at PLAYER 0x008b2ea0 / STUDIO 0x00961180. It is GuiMain's {28} thunk: it jumps to the next function and fills slot 0 of both GuiMain vftables. The GuiMain vftables are back at 100%.
- **(c) program_options split** into its own CMake library, boost_po, linked right after player_client in PLAYER (STUDIO keeps it last). PLAYER +15, 0 lost: 0x00474630, 0x004746e0 and 0x004748d0 are back, plus the bad_function_call, bad_lexical_cast and boost::any bodies.
- **(d) Tried and reverted:** compiling boost_po with the shell's /GL flags (the 2013 configuration) cost 154 PLAYER entries (197 lost, 43 gained). Its LTCG bodies fold with the shell's trivial Instance virtuals, and canClientCreate 0x00402730 went absent. A rebuild after the revert reproduced 7,784 exactly.

**Reorder.** reorder_markers applied to Math.cpp (PLAYER descents 17 → 12) and IndexedMesh.cpp (3 → 2); assert lines are unchanged. Both targets rebuilt, scored identically, and a confirming regeneration round gained 0. Everything else is pinned by assert lines, crosses runs, or has studio-only chunks. type.cpp and LuaInstanceBridge.cpp were left alone, as last time.

**Final figures, PLAYER / STUDIO** (sweep7 baseline in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| Entries at effective 100% | 7,784 (5,972) | 7,687 (5,848) |
| Non-library entries | 5,374 (3,790) | 5,329 (3,744) |
| Functions at 100% | 5,664 (4,621) | 5,544 (4,474) |
| Non-library functions at 100% | 3,321 (2,506) | 3,233 (2,417) |
| reccmp Implemented, with libraries | 7,383 / 40,534 (6,133) | 7,200 / 42,377 (5,923) |
| reccmp Implemented, --nolib | 4,760 / 37,866 (3,739) | 4,673 / 39,719 (3,656) |
| Vftables at 100% | 2,051 / 3,110 (1,282 / 2,886) | 2,094 / 3,039 (1,325 / 2,815) |
| App markers | 633 FUNCTION + 31 FOLDED + 686 STUB (510 + 24 + 686) | 612 + 31 + 684 |
| Sweep scope (App, Base, Network, Log, Rendering): at 100% / short | 703 / 713 | 680 / 710 |

**Instance vftables and the Phase 4 exit criterion: met in both targets.** {for Debugable}, {for GuidItem}, {for Described} and {for AbstractFactoryProduct} are all at 1.0: PLAYER 0x00b2a6ec, 0x00b2a6e0, 0x00b2a6c4, 0x00b2a6d4; STUDIO 0x00bed4ac, 0x00bed4a0, 0x00bed484, 0x00bed494.
- Also at 100%: GuidItem<Instance>::onGuidChanged 0x0040b950, the shell copies 0x004027c0 / 0x00402730 / 0x00402740, Point's deleting dtor 0x00978890 / 0x00a420c0, and the FunctionDescriptor ctor 0x0071a650 / 0x007c3700.
- Now 100% but at 0, 0.25–0.34 or unpaired in the baseline: the RunService, Debris, Teams and Lighting functions datamodel-5b reported, the TextLabel / ImageLabel / ScreenGui() / GuiLayerCollector ctors, Scale9Rect2D, recursiveRender2d, and httpGetPostWinInet.

**Row coverage** (PLAYER, current ranges; baseline on the same ranges in brackets)
- 5a: 131/154 (133)
- 5b: 447/1,604 (249)
- 5e (Client/App/script/*.cpp): 254/848 (246)
- 5g: 299/601 (179)
- 5h: 120/1,038 (43)

**Near misses** (app_sweep report)
- PLAYER, 713 short: inlining-size 234, ebp-frame 198, other 192, unpaired 49, x87-vs-sse 23, unpaired-callee 9, assert-file-line 3, data-ref 3, stack-cookie 2. 48 are at 90% or more; 12 are pairing-only.
- STUDIO, 710 short: inlining-size 230, ebp-frame 198, other 198, unpaired 38, x87-vs-sse 23, unpaired-callee 14, assert-file-line 3, data-ref 3, stack-cookie 3. 60 at 90% or more; 17 pairing-only.
- Closest: LegacyContentTable ctor 0x008f08a0 99.97, ~DataModel 0x0064d850 99.76, asyncRequest 0x00870430 99.59, ~TaskScheduler 0x009b4e00 99.52, TaskScheduler::remove 0x009b52b0 99.49, httpGetPostWinHttp 0x00826990 99.32.

**Every 100% lost against sweep7** (PLAYER 37 lost against 1,849 gained; STUDIO 30 against 1,869). I read each one with --verbose.
1. **The client-shell stand-in** (Client/ClientBase/placeholder.cpp), kept on purpose. Net it is clearly positive (about 140 Instance-derived vftables plus 0x004027c0 / 0x00402730 / 0x00402740), but it costs:
   - 3 functions per target, all thread_resource_error clone_impl / error_info_injector bodies: PLAYER 0x0040ded0, 0x0040e5c0, 0x0040e610; STUDIO 0x0040d7a0, 0x0040da00, 0x0040db10. The original keeps these in the shell (LTCG); our LTCG copy calls ~clone_impl where the original inlines it. They were at 100% before only because a native copy happened to match.
   - 21 PLAYER and 14 STUDIO vftables (logic_error, length_error, out_of_range, lock_error, the gregorian errors, the program_options errors, and error_info_injector<logic_error> 0x00b26db4 / 0x00b26dd8). Their `what` slot is the shell's 0x0041bc60, where the original folds runtime_error::what with logic_error::what; ours keeps logic_error::what native. sp_counted_base 0x00aff920 is at 0.8 for the same reason (its slot is the shell's deleting dtor 0x00435970).
2. **No .reloc immediates read as addresses** (`and 0x7fc000` / `0x7fffff`), 12 per target: Math::isDenormal, isNanInfDenormVector3, symbexec, luaV_execute, constructor, patchtestreg, removevalues, luaK_setreturns, luaK_setoneret, luaK_nil, exp2reg, luaK_posfix.
3. **PLAYER Instance::raisePropertyChanged 0x0061c750 (0.982).** The original calls the shell's copy of signal_with_args<1, void(PropertyDescriptor const*)>::operator() at 0x00456d20. Ours is ICF-folded with the lua_State instantiation and pairs with 0x0067b250 instead.
4. **STUDIO 0x0040e640.** The original's copy is the shell's /GS one; ours comes from WinHttp.cpp, built without /GS. This cannot be fixed in STUDIO by link order.

**Checks**
- decomplint: PLAYER 242 function_out_of_order and 13 duplicate_offset; STUDIO 253 and 5. Both pre-existing. There are no byname_function_in_cpp or unicode errors any more (the baseline had 52 and 3).
- reccmp-vtable: no traceback; 11 / 13 "Problem with adjuster thunk" lines, as in the baseline.
- reccmp-datacmp: PLAYER 1,496 variables, 46 issues; STUDIO 1,452, 47; no traceback.
- check_folded: PLAYER 1,711 annotated addresses, 0 problems; STUDIO 1,648, 0.
- reccmp_addr_padding: 1,664 files, 0 bad.
- check_2016_copies: silent.
- gen_reccmp_csv selftest: ok.

**Saved.** build/sweep8/P-final.json and build-studio/sweep8/S-final.json (with .txt logs and the --nolib, vtable and datacmp outputs alongside). The exe, pdb, map and pdbview they scored are in scratchpad/scored-i8/P and scratchpad/scored-i8/S.

CLAUDE.md also shows a 13-line diff; I did not make it.

## Files changed

- tools/ida/gen_reccmp_csv.py
- CMakeLists.txt
- Client/App/Gui/ProfanityFilter.cpp
- Client/App/v8tree/Instance.cpp
- Client/App/include/V8Tree/Instance.h
- Client/Base/include/rbx/TaskScheduler.h
- Client/App/util/Math.cpp
- Client/App/util/IndexedMesh.cpp
- Client/_generated/player/unresolved.txt
- Client/_generated/studio/unresolved.txt
- reccmp/player-boost.csv
- reccmp/player-function-size.csv
- reccmp/player-libdata.csv
- reccmp/player-symbols.csv
- reccmp/studio-boost.csv
- reccmp/studio-crt.csv
- reccmp/studio-function-size.csv
- reccmp/studio-g3d.csv
- reccmp/studio-libdata.csv
- reccmp/studio-libzip.csv
- reccmp/studio-lua.csv
- reccmp/studio-raknet.csv
- reccmp/studio-stl.csv
- reccmp/studio-symbols.csv
- reccmp/studio-zlib.csv
- build/sweep8/P-final.json
- build-studio/sweep8/S-final.json
- build/sweep8/P-nolib.json
- build-studio/sweep8/S-nolib.json

## Unresolved

- Shell stand-in (Client/ClientBase/placeholder.cpp plus the CMakeLists foreach) kept on net gain. It costs 3 functions per target (PLAYER 0x0040ded0/0x0040e5c0/0x0040e610, STUDIO 0x0040d7a0/0x0040da00/0x0040db10) and 21 PLAYER / 14 STUDIO exception vftables, whose what slot is the shell's 0x0041bc60 where the original folds runtime_error::what with logic_error::what. The real fix is the 2013 shell sources (ClientBase/ReflectionMetadata.cpp, the Stats items).
- PLAYER Instance::raisePropertyChanged 0x0061c750 (0.982): the original calls the shell's copy of signal_with_args<1,void(PropertyDescriptor const*)>::operator() at 0x00456d20. Ours is folded with the lua_State instantiation and pairs with 0x0067b250. It needs the shell code that instantiates it.
- STUDIO 0x0040e640 and the error_info_injector<logic_error> vftables 0xbdec84/0xbdeca8: STUDIO's original copy is the shell's /GS one; ours comes from WinHttp.cpp, built without /GS. Link order cannot fix it in STUDIO.
- Compiling PLAYER's program_options with the shell's /GL flags (the 2013 configuration) cost 154 entries (197 lost, 43 gained; its LTCG bodies fold with the shell's trivial Instance virtuals), so boost_po stays native and only its link position moved.
- FLog::GuiTargetLifetime is still unresolved data (both unresolved.txt files). It needs LOGVARIABLE(GuiTargetLifetime, 0) in v8datamodel/FastLogSettings.cpp at PLAYER 0x00de3f47; the STUDIO address has not been found.
- Still below 100% after regeneration: convertToValue 0x00636ab0 (0.818; needs a factoryregistration.cpp with the getSingleton<T> specializations at a per-target link position, plus a symbol row for placement_any<Region3>::operator=<Enum> 0x00633680); the EnumDesc vftables for InputObject 0xb5c5a4/0xb5c5bc, Joint, SurfaceConstraint, Axis and SurfaceType (0.6, same cause); DataModel ctor 0x0064ec50 0.973 and arbiterName 0x0064de40 0.815 (Instance::fw out of line); HttpQueueStatsItem ctor 0x0086f270 0.872; ContentProviderJob ctor 0x008efdc0 0.866 (tbb spin_mutex).
- No-.reloc coincidences remain: 12 Lua and Math functions per target are at 95.8–99.9% only because an `and 0x7fc000`/`0x7fffff` immediate is read as an address. httpGetPostWinHttp 0x00826990 is in the same state.
- decomplint: 242 PLAYER / 253 STUDIO function_out_of_order and 13 / 5 duplicate_offset remain, all pre-existing. They are pinned by assert lines, cross runs or are studio-only. type.cpp (C2908 compile error) and LuaInstanceBridge.cpp (needs a private member) were not reordered.
- CLAUDE.md has an uncommitted 13-line diff that this task did not make.
