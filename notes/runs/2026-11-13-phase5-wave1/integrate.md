# integrate (workflow phase5-wave1, wf_322cba92-6a4)

INTEGRATE, REGENERATE, SWEEP, ORDER: all three steps are done. Every heavy command ran through tools/capped.sh at 8G, and none hit the cap. I ran one build at a time.

**What I did**
- Clean-rebuilt both targets: PLAYER 8m17s, STUDIO 8m04s.
- Ran gen_link_stubs. The stubs did not change; unresolved.txt lost the 12 FLog channels that are now defined.
- Ran 11 regeneration rounds. Each round regenerated the CSVs from the two latest runs of the current link, then scored each target twice. The two runs of every round agreed, with 0 differences.
- Swept: `set FUNCTION`, score, `apply`, twice.
- Reordered, rebuilt both, and regenerated again on the new link.
- The rebuild changed nothing: the old CSVs on the new link scored 5,934 / 5,812 effective entries, identical to before the rebuild.
- The scores are still rising slowly: the last two rounds gained +7 per target and lost 0.
- Final JSONs (= runs P26 / S26): build/sweep7/P-final.json and build-studio/sweep7/S-final.json. The exe, pdb, map and pdbview they scored are in /tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/scored-i7/P and /tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/scored-i7/S.

**Changes to tools/ida/gen_reccmp_csv.py** (on top of phase4-exit's RTTI vftables, adjustor thunks and twin yield; selftest ok, docstring updated)
1. **LIB_FIX.** The library rows PLAYER 0x004365f0 and STUDIO 0x0041b220 are the `unique_lock<recursive_mutex>` constructor.
   - Evidence: the body calls 0x004366c0 / 0x0041b2f0, which is ios: `unique_lock<recursive_mutex>::lock` (GetCurrentThreadId plus a recursion count).
   - The position pass now names that lock correctly. AsyncHttpQueue::isUrlBad 0x0086e540 went from 0.992 to 1.0.
2. **Fallback to the previous symbol.** When a library row's matcher instantiation has left our link, the row falls back to the symbol it carried last time.
   - Cause: BOOST_THREAD_DYN_LINK removed `_Allocate`/`_Xlen<pair<condition_variable*,mutex*>>`. Row 0x004f3010 then lost our body to the client-shell twin 0x004222d0.
   - Raw call-site count: 0x004222d0 has 3, 0x004f3010 has 17.
   - That lost 9 PLAYER entries (STUDIO likewise), now restored: 0x004f3010, 0x0084cc70, circular_buffer::allocate, and the vector `_Insert_n` / ctor functions.
3. **Library rows keep their symbol.** A position row may no longer give a library row's symbol to another original.
4. **Unresolvable library rows.** A library row that cannot be resolved no longer keeps position votes off its address. 0x008db820 is paired by position again.

**Results** (baseline sweep6 in brackets)

| | PLAYER | STUDIO |
|---|---|---|
| With libraries | 6,133 / 40,527 (4,348 / 40,468) | 5,923 / 42,371 (4,222 / 42,330) |
| --nolib | 3,739 / 38,073 (2,223 / 38,286) | 3,656 / 39,950 (2,203 / 40,148) |
| Entries at effective 100% | 5,972 (3,646) | 5,848 (3,527) |
| Non-library entries | 3,790 (1,724) | 3,744 (1,661) |
| Functions at 100% | 4,621 (3,378) | 4,474 (3,292) |
| Non-library functions at 100% | 2,506 (1,522) | 2,417 (1,472) |
| App markers (Client/App) | 510 FUNCTION + 24 FOLDED + 686 STUB (410 + 24 + 741) | 495 + 24 + 678 (403 + 24 + 726) |
| Sweep scope (App, Base, Network, Log, Rendering) | 537 FUNCTION / 29 FOLDED / 714 STUB | 520 / 29 / 705 |
| Vftables at 100% | 1,282 of 2,886 compared (200 / 412) | 1,325 of 2,815 (187 / 359) |
| Vftables mismatched: structural / pairing-only | 1,604: 658 / 946 | 1,490: 684 / 806 |

The structural / pairing-only split uses my classifier: structural means a slot count differs or a slot is unmatched. On sweep6 the same classifier gives PLAYER 47/165 and STUDIO 42/130.

**Instance vftables and the Phase 4 exit criterion**
- {for Described}: PLAYER 0x00b2a6c4 and STUDIO 0x00bed484 are 1.0.
- {for AbstractFactoryProduct}: 0x00b2a6d4 / 0x00bed494 are 1.0.
- {for Debugable}: 0x00b2a6ec / 0x00bed4ac are 0.714.
- {for GuidItem}: 0x00b2a6e0 / 0x00bed4a0 are 0.
- Also at 1.0: FWInstance 0x00b2a234, FWPVInstance 0x00b4bce8, AbstractFactoryProduct<Instance> 0x00b2a2f0 and FWFinal<FWInstance>. Object 0x00b29f08 is 0.5.
- Instance's two constructors are at effective 100% in both targets: 0x00620da0 0.988 and 0x00620ed0 0.990 (STUDIO 0x006f55e0 / 0x006f5710). ~Instance (0x0061d510 / 0x006f1f40) and FWPVInstance D0 0x0061c320 are 1.0.
- **The exit criterion is not met:** Debugable and GuidItem are held by the twin trivial bodies (see Unresolved).

**Pairing fixes the reports asked for, now at 100% in PLAYER**
- isUrlBad 0x0086e540
- The 5 EnumDesc ctors 0x0063d6a0, 0x0063d7b0, 0x0063d970, 0x0063da80 and 0x00722680 (were about 0.4)
- Bridge registerClass 0x00880470 and 0x00888160
- LuaProfiler::hookCall 0x006774e0
- EnumDescriptor on_index 0x0087d310
- Team ctor 0x006aa970, setScore 0x006aa0b0, askSetParent 0x006aa8b0
- Teams ctor 0x007a4f10, ~Teams 0x007a44f0, askAddChild 0x007a4610
- setPlaceVersion 0x0062d910
- Players::isNetworkClient 0x0056b8c0
- Security::Context::current 0x0053ac40 (STUDIO 0x005e9cb0 too)
- ContentId::convertToLegacyContent 0x0074ecf0

Still short: DataModel ctor 0x0064ec50 0.973, TaskScheduler ctor 0x009b55e0 0.972, HttpQueueStatsItem ctor 0x0086f270 0.872, ContentProviderJob ctor 0x008efdc0 0.866.

**Row coverage (PLAYER, sweep6 → now)**
- 5a: 133/154 = 86.4%, unchanged.
- 5b: 129 → 200 of 1,372 (9.4% → 14.6%).
- 5e (Client/App/script/*.cpp): 152 → 246 of 848 (17.9% → 29.0%).
- 5g: 67 → 177 of 475 (14.1% → 37.3%).

**Every 100% lost against sweep6** (PLAYER 7, STUDIO 6; I read the diffs with --verbose)
- **Rows that cannot resolve after BOOST_THREAD_DYN_LINK.** Their symbols are tss_data_node / pair<condvar,mutex> instantiations that our link no longer has, and no caller votes for their address.
  - PLAYER: map dtor 0x00493590, _Tree 0x0085ede0 and 0x0085f680, vector _Buy 0x008b3120.
  - STUDIO: 0x005b88e0, 0x0066ee90, 0x00a15430, 0x00987a10.
- **Twin yields by design** (phase4-exit rule, vftable slot votes):
  - PLAYER 0x00469e30 sp_counted_impl_p::dispose goes to 0x005bcf50 (12 slots vs 6).
  - PLAYER 0x00475080 RakNet::ReliabilityLayer::IsSendThrottled goes to 0x0070c430 (2,698 slots vs 2).
  - STUDIO 0x00481790 goes to 0x00625210 (12 vs 4).
- **Point scalar deleting destructor** PLAYER 0x00978890 / STUDIO 0x00a420c0, now 0.967.
  - This is a real code difference. Our body stores KernelIndex's vftable; the original stores Point's.
  - It was only at 100% before because the wrong vftable was paired by position. The RTTI pairing exposed it, as phase4-exit predicted.
- **Transient (not a baseline loss).** BasicPartInstance D0 0x00794cb0 was at 100% in one round only (P10).
  - Its callee ~FactoryProduct<BasicPartInstance> 0x00794120 (0.76) had a position row from aligned references, which later regenerations no longer emit. I did not trace why.

**Step 2, ordering**
- reorder_markers applied to four files; line counts unchanged and no assert line moved:
  - AsyncHttpQueue.cpp: PLAYER and STUDIO descents 8 → 4
  - ContentProvider.cpp: PLAYER 8 → 7
  - DataModel.cpp: 17 → 16
  - ScriptEvent.cpp: 2 → 1
- Both targets rebuilt with 0 lost.
- Left alone:
  - type.cpp: the reorder gives the C2908 compile error.
  - LuaInstanceBridge.cpp: the reorder needs a private member.
  - The rest are pinned by assert lines or have descents that cross runs.

**Near-miss shapes** (app_sweep report on the finals)
- PLAYER, 714 short: inlining-size 226, ebp-frame 197, other 190, unpaired 60, x87-vs-sse 25, unpaired-callee 8, assert-file-line 3, data-ref 3, stack-cookie 2. 42 are at 90% or more; 11 are pairing-only.
  - Top: LegacyContentTable ctor 0x008f08a0 99.97 (a no-.reloc prime_list immediate), ~DataModel 0x0064d850 99.76, asyncRequest 0x00870430 99.59, ~TaskScheduler 0x009b4e00 99.52, TaskScheduler::remove 0x009b52b0 99.49, ServiceProvider::onChildAdded 0x006a9770 99.19.
- STUDIO, 705 short: inlining-size 222, ebp-frame 197, other 196, unpaired 48, x87-vs-sse 25, unpaired-callee 8, assert-file-line 3, data-ref 3, stack-cookie 3. 49 at 90% or more; 11 pairing-only.
- Full list: /tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/i7/app-sweep-final.csv

**Checks**
- decomplint:
  - PLAYER: 244 function_out_of_order, 20 duplicate_offset, 52 byname_function_in_cpp, 3 unicode_decode_error.
  - STUDIO: 255, 12, 52, 3.
  - The 52 byname warnings are new: content-5g's TEMPLATE markers in .cpp files (AsyncHttpQueue.cpp 19, ContentProvider.cpp 25, ContentProviderJob.cpp 8).
- reccmp-vtable: no traceback in either target. Exit status 1, with 11 (PLAYER) and 13 (STUDIO) "Problem with adjuster thunk" lines.
- reccmp-datacmp: PLAYER 1,109 variables, 44 issues; STUDIO 1,070, 46; no traceback.
- check_folded: PLAYER 1,268 annotated addresses, 0 problems; STUDIO 1,246, 0.
- reccmp_addr_padding: 1,657 files, 0 bad markers.
- gen_reccmp_csv selftest: ok.

## Files changed

- tools/ida/gen_reccmp_csv.py
- reccmp/player-function-size.csv
- reccmp/player-symbols.csv
- reccmp/player-libdata.csv
- reccmp/player-imports.csv
- reccmp/player-crt.csv
- reccmp/player-stl.csv
- reccmp/player-atl.csv
- reccmp/player-boost.csv
- reccmp/player-g3d.csv
- reccmp/player-lua.csv
- reccmp/player-raknet.csv
- reccmp/player-zlib.csv
- reccmp/player-libzip.csv
- reccmp/studio-function-size.csv
- reccmp/studio-symbols.csv
- reccmp/studio-libdata.csv
- reccmp/studio-crt.csv
- reccmp/studio-stl.csv
- reccmp/studio-atl.csv
- reccmp/studio-boost.csv
- reccmp/studio-g3d.csv
- reccmp/studio-lua.csv
- reccmp/studio-raknet.csv
- reccmp/studio-zlib.csv
- reccmp/studio-libzip.csv
- Client/_generated/player/unresolved.txt
- Client/_generated/studio/unresolved.txt
- Client/App/util/AsyncHttpQueue.cpp
- Client/App/util/ContentProvider.cpp
- Client/App/v8datamodel/DataModel.cpp
- Client/App/script/ScriptEvent.cpp
- Client/App/include/reflection/EnumConverter.h
- Client/App/include/reflection/Reflection.h
- Client/App/include/security/SecurityContext.h
- Client/App/include/Util/Object.h
- Client/App/script/LuaAtomicClasses.cpp
- Client/App/script/LuaEnum.cpp
- Client/App/script/ScriptContext.cpp
- Client/App/util/ContentProviderJob.cpp
- Client/App/util/LegacyContentTable.cpp
- Client/App/v8datamodel/PartInstance.cpp
- Client/App/v8datamodel/Team.cpp
- Client/App/v8datamodel/Teams.cpp
- Client/Network/Players.cpp
- build/sweep7/P-final.json
- build/sweep7/P-final.txt
- build/sweep7/P-final-nolib.txt
- build/sweep7/vtable-PLAYER.txt
- build/sweep7/datacmp-PLAYER.txt
- build/sweep7/lint-PLAYER.txt
- build-studio/sweep7/S-final.json
- build-studio/sweep7/S-final.txt
- build-studio/sweep7/S-final-nolib.txt
- build-studio/sweep7/vtable-STUDIO.txt
- build-studio/sweep7/datacmp-STUDIO.txt
- build-studio/sweep7/lint-STUDIO.txt

## Unresolved

- Instance {for Debugable} (0.714) and {for GuidItem} (0) are still held by the twin trivial bodies, so the Phase 4 exit criterion is not met. I did not release the markers that hold them, because releasing them lowers a function now at 100% (0x00575d00 FOLDED, IAdornable.h:114-129, IndexedTree.h:24-33, Item.h:76, SpanningTree.h:70; STUDIO 0x007aa330), in exchange for the twin. This needs a decision. Measured .rdata references to each copy: ret 4: 0x004027c0 3,620 vs 0x00575d00 930. xor al,al: 0x00402730 781 vs 0x00927cc0 1,799. ret 8: 0x00402740 499 vs 0x008c8450 233. ret: 0x0040b950 590 vs 0x00528720 331. reccmp also drops the duplicate STUB markers at 0x00927cc0, 0x00528720, 0x009d55f0, 0x0096bde0, 0x008c8450 and 0x00850a50 ('Dropped duplicate address').
- Regeneration has not fully converged: the last two rounds gained +7 per target and lost 0. Another round (tools: /tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/i7/round.sh 25 26 27 28) would add a few more entries.
- These library rows stay unpaired: the tss_data_node map/_Tree rows (PLAYER 0x00493590, 0x0085ede0, 0x0085f680; STUDIO 0x005b88e0, 0x0066ee90, 0x00a15430) and vector _Buy (0x008b3120 / 0x00987a10). Their matcher instantiations left our link with BOOST_THREAD_DYN_LINK, and no position vote names their address. Re-exporting ida/exports/<T>-names.csv and lib-resolved with our current names (an IDA sync) would let them pair again.
- Point scalar deleting destructor (PLAYER 0x00978890, STUDIO 0x00a420c0, 0.967): a real code difference. Our body stores KernelIndex's vftable where the original stores Point's. The Point class layout or its destructor needs fixing in the source.
- BasicPartInstance D0 0x00794cb0 (0.909): its callee ~FactoryProduct<BasicPartInstance> 0x00794120 (0.76) lost the position row it had from aligned references in one round. I did not trace why later regenerations drop it.
- tools/gen_link_stubs.py: when the exe is already up to date, `ninja -d keeprsp` does not relink, so no studio.rsp exists, link.exe fails (LNK1104) and the tool writes an empty unresolved.txt ('0 unresolved'). Workaround I used: delete the exe first. The tool should force the relink itself.
- decomplint reports 52 new byname_function_in_cpp warnings: content-5g's TEMPLATE markers in AsyncHttpQueue.cpp (19), ContentProvider.cpp (25) and ContentProviderJob.cpp (8). They fail --warnfail. Moving the namerefs to headers would fix them, but would change those files' line layout.
- Still unpaired after regeneration: rbx::placement_any::operator=<Enum> 0x00633680, so EnumDesc convertToValue 0x00636ab0 stays at 0.818. Also the near misses that need source changes: DataModel ctor 0x0064ec50 (0.973, Instance::fw() out of line), TaskScheduler ctor 0x009b55e0 (0.972), HttpQueueStatsItem ctor 0x0086f270 (0.872), ContentProviderJob ctor 0x008efdc0 (0.866, needs tbb::spin_mutex), LegacyContentTable ctor 0x008f08a0 (99.97, a no-.reloc immediate).
- Not reordered: reflection/type.cpp (the reorder gives C2908) and LuaInstanceBridge.cpp (it needs a private member). function_out_of_order remains: PLAYER 244, STUDIO 255.
