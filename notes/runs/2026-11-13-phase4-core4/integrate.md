# integrate (workflow phase4-core4, wf_cf7a5075-c0f)

INTEGRATE, REGENERATE, SWEEP, ORDER: all three steps are done. Against build/sweep5 and build-studio/sweep5, no function or vftable lost its 100% except 7 PLAYER library functions. Each of those I read with --verbose, and each is a no-.reloc coincidence. Every heavy command ran through tools/capped.sh at the default 8G cap; none hit it. I ran one build at a time.

**What I did**
- Rebuilt both targets from clean.
  - PLAYER: link stubs unchanged.
  - STUDIO: gen_link_stubs added 2 stubs, registerNewLightAPI 0x007d7380 and Limits::Counter::add 0x00a6afd0, then relinked.
- Regenerated the CSVs and rescored until nothing more was gained, pooling only runs of the same link: PLAYER P1-P6 and PF1-PF4, STUDIO S2-S7.
- Swept: `app_sweep set FUNCTION`, scored, `apply PF4 S7`.
- Reordered (step 2), rebuilt both, then regenerated again from runs of the new link only: PLAYER PA1-PA4, STUDIO SA1-SA3.
  - The last two rounds of each are identical: 0 lost, 0 gained.
  - Re-applying the sweep to the final JSONs changes no file.

**Changes to tools/ida/gen_reccmp_csv.py (selftest extended and passing)**
1. **true_size.** function-size rows now extend over an in-function jump table, and over a tail IDA cut at a call it treated as noreturn (16-aligned start, no int3, ends in ret). 264 PLAYER and 284 STUDIO sizes changed, e.g. 0x00758320 245→264 and Bridge<T>::on_newindex 0x00670170 40→48. The extra 8 bytes are the tail IDA cut off; reccmp then scores all 14 per target at 100%.
2. **Float sizes.** A float row whose __real@ name our linker ICF-folded into a named constant now takes its size from the map's __real@ alias. Fixes 00be6a48/00be6ab0 and STUDIO 00d2e9c4/00d2eab0.
3. **Library rows over a jump table.** Such a row is now read up to the first differing instruction, and its own table never becomes a nameless code row. This pairs zlib's `order` table 00bbff50, so inflate 0x009e22e0 is at 100%, as are gzip_header::process 0x006125e0 and codecvt_error_cat::message 0x009c0e40.
4. **Unpairable STUB markers.** A STUB marker that reccmp reports as 'Debug data out of sync' no longer blocks a position row at its address. Type.h:82 Type::Type<void> 0x0079fc70 is paired again, so getSingleton<void> 0x0079ff00 is back at 100%.
5. **Lesson, now in the docstring.** Pooling --json runs from an earlier link pairs stale recomp addresses. The first post-rebuild regeneration did this and briefly lost 9, e.g. VerbContainer D0 0x006f9250 got Assembly::onLowersChanged. Regenerating from runs of the current build fixed it.

**Results** (final JSONs are build/sweep6/P-final.json and build-studio/sweep6/S-final.json)

| | PLAYER | STUDIO |
|---|---|---|
| Implemented with libraries | 4,348 / 40,468 (was 4,116) | 4,222 / 42,330 (was 3,989) |
| --nolib | 2,223 / 38,286 (was 2,008 / 38,303) | 2,203 / 40,148 (was 1,988 / 40,162) |
| Entries at effective 100% | 3,646 (was 3,387) | 3,527 (was 3,263) |
| Non-library at effective 100% | 1,724 (was 1,488) | 1,661 (was 1,427) |
| Functions at 100% | 3,378 | 3,292 |
| App markers (Client/App) | 410 FUNCTION + 24 FOLDED + 741 STUB | 403 + 24 + 726 |
| Vftables at 100% | 200 / 412 compared (was 192 / 371) | 187 / 359 (was 178 / 322) |
| Vftable mismatches, structural / pairing-only | 34 / 178 | 15 / 157 |

- **Where the gains came from.** The fresh build with the old CSVs scored 3,500 (PLAYER) and 3,382 (STUDIO). Regeneration and the generator fixes added 146 and 145.
- **Gained against the baseline.** PLAYER 266 (236 non-library), STUDIO 264 (232).
  - New PLAYER vftables at 100%: FWFinal<FWInstance>, <FWPartInstance> and <FWPVInstance>, FWPartInstance, FactoryProduct<SoundService>, Assembly{for IPipelined}, G3D ReferenceCountedObject and MemoryManager.
- **Pairing-only near misses from the work reports, now at 100% in PLAYER** (STUDIO twins likewise):
  - the 14 on_newindex wrappers per target
  - ~SimBody 0x00976a00
  - Math: radiansToQuadrant, intersectRayPlane, intersectRayConvexPolygon, matrixTiltQuadrant
  - Face: fuzzyContainsInExtrusion, cornersAligned
  - Extents::separatedByMoreThan
  - normalIdToU and the other jump-table functions
  - png_sig_cmp
  - G3D: ReferenceCountedObject ctor and zeroWeakPointers, ReferenceCountedPointer<MemoryManager>::setPointer, MemoryManager::create
  - FWFinal<FWInstance> D0 0x0061f220, Frustum ctor 0x009ee010, IndexedMesh ctor 0x008f9050
- **Phase 5a row coverage** (tools/row_coverage.py on P-final): 133/154 = 86.4%, up from 53/154. Math 81/81, Extents 10/10, Face 10/10, NormalId 11/11, IndexedMesh 11/15, Region3 4/5, InterpolatedCFrame 2/18, Frustum 1/1.
- **The 7 lost (all PLAYER library code).** IsOlderOrderedPacket 0x005c8930, symbexec 0x008a8330, luaV_execute 0x0098ad00, constructor 0x0098f9a0, patchtestreg 0x009a6800, removevalues 0x009a6880, luaK_setreturns 0x009a6c30. In each, an immediate equals one of our ??_R0 RTTI addresses: luaK_setreturns `and edx,<OFFSET1>`, IsOlderOrderedPacket `lea edx,[eax-<OFFSET2>]`. No code differs. STUDIO lost 0. RecvFromLoop and LocklessUint32_t::Decrement are back in both targets.
- **Near-miss shapes** (app_sweep report on the finals; scope App/Base/Network/Log/Rendering):
  - PLAYER, 770 short: inlining-size 255, other 211, ebp-frame 203, unpaired 63, x87-vs-sse 25, unpaired-callee 7, assert-file-line 3, stack-cookie 2, data-ref 1. 46 are at 90% or more; 8 are pairing-only.
  - STUDIO, 754 short: inlining-size 252, other 212, ebp-frame 201, unpaired 51, x87-vs-sse 25, unpaired-callee 6, assert-file-line 3, stack-cookie 3, data-ref 1. 46 at 90% or more; 7 pairing-only.
  - Near-miss CSV: /tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/i6/app-sweep-final.csv

**Step 2, ordering**
- LuaAtomicClasses.cpp had duplicated marker pairs left by a work agent: both FUNCTION and STUB for on_components 0x00886d90 and on_toEulerAnglesXYZ 0x00886e20, which the sweep turned into 4 FOLDED. I removed the duplicates; no __LINE__ user follows them. A scan finds no other duplicate marker in Client/.
- **LuaAtomicClasses.cpp:** loose reorder, PLAYER descents 22→0 and STUDIO 27→2. The compile error the last integrate hit (C2065) is fixed by forward-declaring the two statics on a line that was blank, so no line moves.
- **LuaSignal.cpp:** 4→0 in both targets. g_allowSignalConnect's definition moved onto blank line 126, and its old line 301 became a comment, so the line count is unchanged.
- Both rebuilt, rescored and regenerated with 0 lost. type.cpp (C2908) and LuaInstanceBridge (a private member) were left alone.
- decomplint function_out_of_order: PLAYER 254→244, STUDIO 267→254.

**Checks**
- decomplint: PLAYER 244 out-of-order + 20 duplicate_offset + 3 unicode_decode_error; STUDIO 254 + 12 + 3. The non-order warnings are the same as before.
- reccmp-vtable: both run with no traceback (exit status 1, with the same 8 'Problem with adjuster thunk' boost vtordisp lines as in sweep5).
- datacmp: PLAYER 743 variables, 24 issues; STUDIO 713, 26; no traceback.
- check_folded: PLAYER 1,193 annotated addresses, STUDIO 1,174, 0 problems.
- reccmp_addr_padding: 1,654 files, 0 bad markers.
- gen_reccmp_csv selftest: ok.

Copies of the exe, pdb, map and pdbview they scored are in /tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/scored-i6/P and /tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/scored-i6/S.

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
- Client/_generated/studio/stubs_00.cpp
- Client/_generated/studio/unresolved.txt
- Client/_generated/player/unresolved.txt
- Client/App/script/LuaAtomicClasses.cpp
- Client/App/script/LuaSignal.cpp
- Client/App/include/Gui/UnifiedWidget.h
- Client/App/include/lua/luabridge.h
- Client/App/include/Util/FeatherweightDict.h
- Client/App/include/V8World/Primitive.h
- Client/App/util/AsyncHttpQueue.cpp
- Client/App/util/Extents.cpp
- Client/App/util/Face.cpp
- Client/App/util/IndexedMesh.cpp
- Client/App/util/IndexedTree.cpp
- Client/App/util/Math.cpp
- Client/App/util/NormalId.cpp
- Client/App/v8datamodel/GuiBuilder.cpp
- Client/App/v8kernel/SimBody.cpp
- Client/App/v8tree/Instance.cpp
- Client/App/v8world/SimulateStage.cpp
- Client/Base/rbx/TaskScheduler.cpp
- Client/Base/rbx/Time.cpp
- build/sweep6/P-final.json
- build/sweep6/P-final.txt
- build/sweep6/P-final-nolib.txt
- build/sweep6/vtable-PLAYER.txt
- build/sweep6/datacmp-PLAYER.txt
- build/sweep6/lint-PLAYER.txt
- build-studio/sweep6/S-final.json
- build-studio/sweep6/S-final.txt
- build-studio/sweep6/S-final-nolib.txt
- build-studio/sweep6/vtable-STUDIO.txt
- build-studio/sweep6/datacmp-STUDIO.txt
- build-studio/sweep6/lint-STUDIO.txt

## Unresolved

- The 7 PLAYER library losses are no-.reloc coincidences, not code: an immediate in the original equals one of our ??_R0 RTTI data addresses. They are IsOlderOrderedPacket 0x005c8930, symbexec 0x008a8330, luaV_execute 0x0098ad00, constructor 0x0098f9a0, patchtestreg 0x009a6800, removevalues 0x009a6880 and luaK_setreturns 0x009a6c30. They will come back whenever our data layout moves.
- FWInstance D0, PLAYER 0x0061c320: our linker ICF-folds ??_GFWInstance with ??_GFWPVInstance at 0x00535920, and the PDB keeps the FWPVInstance name, so the SYNTHETIC nameref `RBX::FWInstance::`scalar deleting destructor'` never pairs. This is what keeps the FWInstance vftable 0x00b2a234 at 0%. The fix is in the source, e.g. a body difference or a FWPVInstance marker, once the original FWPVInstance D0 address is known.
- ProfanityFilter::ContainsProfanity, PLAYER 0x006690f0 / STUDIO 0x008a8680: the stub body compiles to `xor al,al; ret` and our ICF folds it into the shared trivial body. That body is named RakNet::PluginInterface2::UsesReliabilityLayer, and the marker pairs it with the 252-byte original, which is why the core-close report saw the shared xor-al body pair wrongly. A STUB(0x006690f0) in the body (not mine) would unfold it and free the twin for vftable slot pairing.
- Twin bodies (the original keeps two copies, ours one) still block 100% in these pairing-only near misses: ~DataModel 0x0064d850 99.76% (our ~exception_ptr pairs the client-shell copy 0x00405b50; the engine copy is 0x007e4e40), ServiceProvider::onChildAdded 0x006a9770, ContentId::convertToLegacyContent 0x0074ecf0, AsyncHttpQueue::isUrlBad 0x0086e540, LibraryService::ContentReadyHelper 0x0088edf0 and STUDIO registerDevelopmentLibrary 0x009e9800. CSV rows cannot pair one recompiled body with two originals.
- DataModel::startCoreScripts 0x006400f0 (92.86%): its callee loadCoreScripts 0x0063fbb0 is VMProtect-touched (notes/vmprotect-player.csv), so function-size drops it and the call cannot pair in PLAYER. Adding it would need a marker line in DataModel.cpp, which shifts its asserts, or a generator rule that adds `stub` rows for VMP functions.
- StatsItem::formatValueIy 0x0073b480 / STUDIO 0x007c3b20 (95.83%): the source passes "%I64u"; 2013 passes "%llu". This is a source edit in StatsItem.cpp.
- Security::Context::current 0x0053ac40 (89.66%): 2013 calls boost.dll get_tss_data as `call [imp]`. It needs BOOST_THREAD_DYN_LINK or BOOST_ALL_DYN_LINK in CMakeLists.txt; that is a flag change, not a source-list change, so I left it.
- Not reordered: reflection/type.cpp (loose order gives C2908) and LuaInstanceBridge.cpp (4→3; YieldFunctionStateObject::execute is private). Files pinned by asserts (43) and cross-run descents remain, as in sweep5.
- Vftables, pairing-only: PLAYER 178 and STUDIO 157 remain. Most are unpaired adjustor-thunk slots (the generator still filters `sub ecx,N; jmp` thunks out of function-size, e.g. Instance's 0x007bc400 / 0x00958d00) and twin trivial bodies (0x004027c0 vs 0x00575d00, 0x00402730 vs 0x00927cc0). Structural: PLAYER 34, STUDIO 15.
- The math-5a JSONs (build-math/runs/*) were not pooled: they are from another link, and pooling runs across links pairs stale recompiled addresses. Their position evidence was rebuilt from this build's own runs instead.
