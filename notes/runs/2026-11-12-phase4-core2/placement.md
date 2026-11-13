# placement (workflow phase4-core2, wf_fef33d15-1aa)

I moved 32 PLAYER functions (33 STUDIO) to the files their 2013 assert `__FILE__` strings name. Both targets were rebuilt, swept and rescored, and **no function at 100% was lost in either target.** Every heavy step ran through tools/capped.sh at the default 8G cap and none hit it. Builds ran one at a time with no -j, and no memory problems came up.

**Step 1: the list.** I listed every function whose assert file name differs between the original and our build (P17/S14). There are 37 rows in PLAYER and 38 in STUDIO; the earlier figure of 101 counted assert pairs, not functions.

**Step 2: what moved** (bodies unchanged; old copies are in the scratchpad's mv/backup folder):
- **SimJobTracker → v8world/SimJob.cpp.** All 5 functions plus containedBy, placed in PLAYER/STUDIO address order: tracking 0x00822270, getSimJob 0x008222e0, SimJob ctor/dtor, stopTracking 0x008226d0, setSimJob 0x008227b0, transferTrackers 0x008228c0. SimJobTracker.cpp is deleted (2013 has no such file).
- **~MultiJoint (PLAYER 0x009741c0, STUDIO 0x009bf3a0)** moved out of line from MultiJoint.h into v8world/MutilJoint.cpp. It sits after putInKernel so PLAYER's order ascends.
- **EdgeList class → V8World/Primitive.h** (~EdgeList, getEdge, getOther stay inline). getNext 0x007546b0, removeEdge 0x00756040 and insertEdge 0x007563f0 are out of line in v8world/Primitive.cpp, at positions where they ascend in both targets. EdgeList.h is deleted.
- **IPipelined::inStage(type) 0x006d51c0** (new marker), **inStage(stage) 0x008e6230, inOrDownstreamOfStage 0x008e6670 and downstreamOfStage 0x008e3880** are now inline in IPipelined.h, in 2013 line order.
- **Inline in their 2013 headers:**
  - IStage::getKernel 0x008e4710 → IStage.h (was IWorldStage.cpp)
  - Contact::removeFromKernel 0x007b8ec0 → Contact.h
  - GeoPair::match 0x00850480 → Pair.h (its assert now lands on Pair.h:133, the 2013 line)
  - SendPhysics::nextSimJob 0x005e3bd0 → SendPhysics.h
  - World::reset 0x0070d390 → World.h
  - Body::getMeInParent 0x0071ea50 (new marker), updatePV 0x0071eab0 and accumulateForceAtBranchCofm 0x00920bc0 → Body.h
- **KernelData.cpp → KernelData.h.** All 11 members are now inline, and KernelData.cpp is deleted (no KernelData.cpp string exists in 2013). New STUB markers: insertBody 0x00831a60, removeBody 0x00831250, addLeafBody 0x008306e0, addLeafBodies 0x00831180, addConnector 0x00831ba0, with STUDIO addresses alongside.
- **MemoryStreams.h → v8xml/SerializerBinary.cpp.** I added a STUDIO marker for MemoryOutputStream::grow 0x007416c0; it has no PLAYER address. MemoryStreams.h is deleted.
- **TextXmlParser.cpp + TextXmlWriter.cpp → v8xml/XmlSerializer.cpp.**
  - The 13 marked functions (PLAYER 0x00627c50–0x00629630) are in address order, which also ascends in STUDIO.
  - The dormant 2016 copy of XmlSerializer.cpp was replaced (the read-only `2013 55% App/` folder still has it).
  - I removed its exclusion from CMakeLists.txt; that was the only CMake change.

**Scores.**

| | Before (P17 / S14) | After (P3 / S3) |
|---|---|---|
| PLAYER functions at effective 100% | 3,145 (1,280 non-library) | 3,145 (1,280) |
| STUDIO functions at effective 100% | 3,032 (1,229) | 3,032 (1,229) |
| PLAYER Implemented, with libraries | 3,930 / 40,468 | 3,923 / 40,468 |
| STUDIO Implemented, with libraries | 3,799 / 42,329 | 3,791 / 42,329 |
| PLAYER --nolib | 1,841 / 38,322, accuracy 83.90%, progress 4.03% | 1,834 / 38,322, 84.00%, 4.02% |
| STUDIO --nolib | 1,820 / 40,182, 82.75%, 3.75% | 1,812 / 40,182, 82.85%, 3.74% |
| App markers PLAYER | 205 FUNCTION + 23 FOLDED | unchanged |
| App markers STUDIO | 197 FUNCTION + 23 FOLDED | unchanged |
| Vtables at 100% | PLAYER 174 / 339, STUDIO 163 / 283 | unchanged |

- **Why Implemented fell by 7–8:** reccmp's "Implemented" counts compared functions and skips STUB markers. The functions I newly marked STUB (5 KernelData, inStage(type), getMeInParent, grow) are now skipped. Nothing that matched before was lost.
- **One mid-run drop, fixed:** GlueJoint's scalar deleting destructor (PLAYER 0x009745f0, STUDIO 0x009c1540) briefly fell to 90.9% because a stale ~MultiJoint marker stayed behind in MultiJoint.h as a duplicate. I replaced it with a comment of the same line count, and both are back at 100%.
- **Moved functions, before → after:** PLAYER 28 up, 3 down, 18 unchanged; STUDIO 29 up, 3 down, 6 unchanged. For example, Contact::removeFromKernel went from 72.4% to 86.2% and EdgeList::getNext from 69.2% to 81.5%.
- **The three that went down:**
  - World::reset, 55.3% → 51.1%. The original also zeroes `[esi+0x38]`, which our body does not.
  - Body::getMeInParent, 50.0% → 47.2%.
  - KernelData::removeBody, 72.3% → 67.0%. Its asserts now carry KernelData.h, but at our line numbers, not 2013's.

After the moves, PLAYER's wrong-file list is down from 37 rows to 5 (see the unresolved list).

**Checks:**
- **decomplint:** PLAYER 328 function_out_of_order (was 338) + 23 duplicate_offset; STUDIO 334 (was 342) + 15. None of the remaining errors is in a moved block.
- **reccmp-vtable:** same 8 / 10 adjuster-thunk messages as before.
- **reccmp-datacmp:** completes with no traceback. PLAYER 655 variables, 24 issues; STUDIO 623 variables, 26 issues (unchanged).
- **check_folded:** PLAYER 1,022 annotated addresses, STUDIO 1,004; 0 folding problems.
- **reccmp_addr_padding:** 1,648 files, 0 badly padded markers. It needs the venv python; plain python3 fails to import reccmp.
- **Link:** 0 LNK2019/LNK2001 in any link log.

I did not regenerate the reccmp/*.csv files: no symbol names changed, and the scores show no loss without it. The databases and gen_reccmp_csv.py were not touched.

## Files changed

- Client/App/v8world/SimJob.cpp
- Client/App/v8world/MutilJoint.cpp
- Client/App/include/V8World/MultiJoint.h
- Client/App/include/V8World/Primitive.h
- Client/App/v8world/Primitive.cpp
- Client/App/include/V8World/IPipelined.h
- Client/App/v8world/IPipelined.cpp
- Client/App/include/V8Kernel/IStage.h
- Client/App/v8world/IWorldStage.cpp
- Client/App/include/V8World/Contact.h
- Client/App/v8world/Contact.cpp
- Client/App/include/V8Kernel/Pair.h
- Client/App/v8kernel/Pair.cpp
- Client/App/include/V8World/SendPhysics.h
- Client/App/v8world/SendPhysics.cpp
- Client/App/include/V8World/World.h
- Client/App/v8world/World.cpp
- Client/App/include/V8Kernel/Body.h
- Client/App/v8kernel/Body.cpp
- Client/App/include/V8Kernel/KernelData.h
- Client/App/v8xml/SerializerBinary.cpp
- Client/App/v8xml/XmlSerializer.cpp
- CMakeLists.txt
- build/sweep4/P3.json
- build/sweep4/P3.txt
- build/sweep4/PLAYER-nolib.txt
- build-studio/sweep4/S3.json
- build-studio/sweep4/S3.txt
- build-studio/sweep4/STUDIO-nolib.txt
- build/sweep4/lint-PLAYER.txt
- build/sweep4/lint-STUDIO.txt
- build/sweep4/vtable-PLAYER.txt
- build-studio/sweep4/vtable-STUDIO.txt
- build/sweep4/datacmp-PLAYER.txt
- build-studio/sweep4/datacmp-STUDIO.txt

## Unresolved

- Not moved: getComponent (PLAYER 0x0071b080, STUDIO 0x007b6bc0, 90.9%). 2013 puts it in PartInstance.cpp:2166, together with the TypedPropertyDescriptor<CoordinateFrame> specializations. getDataSize 0x0071b0f0, getSingleton 0x00720850, readValue 0x007208c0 and writeValue 0x00723bf0 all sit inside PartInstance.cpp's PLAYER range and are already at 100%. Moving the block means placing explicit specializations before every use inside PartInstance.cpp, and the block's full extent (convert, StringConverter) is not proven.
- Not moved: MergeBinder::announceIDREF (PLAYER 0x00447460, STUDIO 0x00435250, 89.7%). 2013 has MergeBinder in v8xml/SerializerV2.h, which is reached through a relative include from another v8xml header (hence the lowercase path). Moving it out of Reference.h would change which TUs see it and would still not reproduce that path spelling.
- Not moves, because the 2013 bodies call inline helpers our tree does not have: World::insertJoint/removeJoint (World.h:142 '!inStepCode'), and GuiBuilder::updateSummaryStats (only line and message differences in utilities.h and GuiBuilder.cpp).
- Body::accumulateForceAtBranchCofm is now inline in Body.h, as the 2013 assert says, but our short body gets inlined at every call site. reccmp reports 'Failed to find function symbol Body.h:98' and the function drops out of measurement; the original keeps an out-of-line copy at 0x00920bc0 / 0x00948dc0, so its 2013 body was probably larger.
- The moved functions still differ from 2013 in assert text and line numbers. I did not change bodies, which was outside this task. Examples: EdgeList 'i >= 0 && i < edges.size()' vs 2013 'i < size()'; ~EdgeList has one combined assert where 2013 has two; GeoPair 'pairType' vs '_pairType'; IPipelined 'stage != NULL' vs 'iStage'; Body/KernelData use our own spellings. World::reset is missing the original's store to [this+0x38].
- reccmp/*.csv were not regenerated: no symbol names changed and no score was lost without it. The 5 new KernelData STUB markers (and grow, inStage(type), getMeInParent) take those functions out of reccmp's Implemented count, which is the 7 / 8 drop in that figure. That follows the project's STUB convention; nothing was lost.
