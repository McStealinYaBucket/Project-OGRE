# near-miss-2 (workflow phase4-core4, wf_cf7a5075-c0f)

Near-miss pass on App markers I own. Measured in my own build dirs: build-nm (PLAYER) and build-nm-studio (STUDIO). I compared against the baseline JSONs build/sweep5/P-final.json and build-studio/sweep5/S-final.json. Final JSONs are in /tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/nm2/ as P3.json and S3.json.

**Totals.** These include other agents' concurrent work: the Math and Time gains come from the math and Base agents.
- PLAYER: effective 100% went from 3,387 to 3,493 (non-library 1,488 to 1,603). Implemented is 4,101 / 40,468.
- STUDIO: effective 100% went from 3,263 to 3,379 (non-library 1,427 to 1,545). Implemented is 3,973 / 42,330.
- Vftables at 100% are unchanged: PLAYER 192 / 371, STUDIO 178 / 322.
- App markers now: PLAYER 296 FUNCTION + 25 FOLDED + 856 STUB; STUDIO 289 + 25 + 841. These counts include other agents' edits.
- Checks: check_folded reports 0 folding problems in both targets; reccmp_addr_padding reports 0 bad markers.

**Lost 100% entries, all read with the diff:**
- PLAYER: 9 RakNet and Lua functions (IsOlderOrderedPacket, RecvFromLoop, LocklessUint32_t::Decrement, symbexec, luaV_execute, constructor, patchtestreg, removevalues, luaK_setreturns). Each is a no-.reloc coincidence: an immediate such as Lua's mask now equals one of our RTTI data addresses, or a name-pairing change. No code in those functions differs.
- STUDIO: only RecvFromLoop and Decrement, same cause.
- TreeStage::destroyMechanism and destroyAssembly dipped in one intermediate run because IndexedMesh::setUpper (the math agent's file) was briefly unpaired. Both are back at 100% in the final runs.

**Raised to effective 100% and flipped to FUNCTION.** PLAYER address, baseline score in brackets; the STUDIO twin moved the same way in every case unless noted.
- EdgeList: getEdge 0x00754560 [87.72], getOther 0x0071b6f0 [87.72], getNext 0x007546b0 [89.23], insertEdge 0x007563f0 [96.72].
- YieldFunctionStateObject::onRaiseException 0x0087b2f0 [98.29].
- TextService: registerTypesetter 0x0073a860 [91.67], getTypesetter 0x0073a7e0 [87.8].
- IWorldStage::onEdgeAdded 0x00997450 and onEdgeRemoving 0x009974c0 [91.18 each]; IStage::getKernel 0x008e4710 [88.46].
- IndexArray<IndexedTree>::operator[] 0x0082e750 [95.45].
- JointStage::moveEdgeToDownstream 0x009979b0 and removeEdgeFromDownstream 0x00997a30 [85.71 each].
- EdgeBuffer::pushKinematicOk 0x00998fb0 [93.48], EdgeBuffer::onEdgeAdded 0x00999270 [81.82].
- StatsItem::formatValueId 0x0073b3f0 [63.64], formatValueIf 0x0073b420 [85.71], formatValueIi 0x0073b450 [81.82].
- MovingStage: new assertNotInPipeline2 0x0099a490 (STUDIO 0x00a5a750) [unpaired]; onMechanismAdded 0x0099a510 [85.94]; onMechanismRemoving 0x0099a5b0 [98.21].
- GroundStage: addGroundJoint 0x008eaaa0 [93.48], removeGroundJoint 0x008eabd0 [87.5], onKernelJointAdded 0x008ea7a0 and onKernelJointRemoving 0x008ea890 [73.13 each].
- TreeStage: removeFromPipeline 0x008ec030 [96.61], dirtyMechanism 0x008ec0f0 [91.89], cleanMechanism 0x008ebc40 [90.7].
- SpatialFilter::changePhase 0x008e47b0 [90.99], onFixedAssemblyRootAdded 0x008e4c80 [80.65].
- StepJointsStage::onEdgeRemoving 0x008ef210 [90.67].
- Lua bridges: Bridge<UDim2>::on_index 0x00887be0 [93.51], Bridge<CoordinateFrame>::on_index 0x008871a0 [91.2], Vector2int16Bridge::on_div 0x008850b0 [98.69].
- New markers that pair at 100%:
  - CoordinateFrameBridge::on_inverse 0x008865b0, on_toWorldSpace 0x00886620, on_toObjectSpace 0x00886710 (STUDIO = PLAYER + 0xaa000).
  - Vector3Bridge::pushVector3 0x00877190 (STUDIO 0x0092cf30).
  - UDimBridge::pushUDim 0x00883080 (STUDIO 0x0092d080).
- WeakThreadRef::~WeakThreadRef 0x00871c40 [85.71]; Bridge<EventInstance>::on_index 0x0087ee40 [96.98].
- GuiItem::onDescendantRemoving 0x00768d00 [91.3], GuiItem::getSize 0x006c8c40 [66.67], UnifiedWidget::onLoseFocus 0x006c8a10 [50.0], TextDisplay::render2d 0x00768cc0 [80.95].
- STUDIO only: MemoryOutputStream::grow 0x007416c0 [85.71].

**Improved but still short:**
- LibraryService::ContentReadyHelper: 86.05 to 98.88.
- SimulateStage::putFirstMovingRootInSendPhysics: 91.04 to 98.51.
- AsyncHttpQueue::isUrlBad: 98.46 to 99.23 (PLAYER).
- GroundStage::rebuildFreeGround: 93.06 to 97.22.
- JointStage::~JointStage: 87.68 to 92.03.
- EdgeBuffer::pushEdgeIfOk: 89.66 to 92.61.
- BlockBlockContact::findGeoPairConnector: 90.91 to 93.94.
- Bridge<CoordinateFrame>::registerClass: 85.47 to 86.32.

**Causes, by category:**
1. **Assert 2013 line, text or file.** I laid files out with tools/assert_lines.py and blank padding, and rewrote assert expressions to the 2013 strings.
   - EdgeList in Primitive.h: `i < size()` at lines 46–60.
   - Primitive.cpp: getNext/insertEdge/removeEdge moved to lines 226–251, removeEdge's local renamed `removeIndex`.
   - LuaInstanceBridge.cpp: the hand-written `/Applications/...` assert literals at :948/:967 became RBXASSERT.
   - TextService.cpp: lines 65/128/134.
   - IWorldStage.cpp, IStage.h:69, IndexArray.h:103/113, JointStage.cpp:26–46, EdgeBuffer.cpp (all 7), GroundStage.cpp (17 of 18; `Joint::ANCHOR_JOINT` spellings).
   - TreeStage.cpp: functions moved into 2013 order at :327/336/385–394/422–472.
   - SpatialFilter.cpp: :30–58, :279/280, :295/296. SimulateStage.cpp: all 8 asserts.
   - SerializerBinary grow: `size > capacity` at :122.
2. **Layout or type.**
   - Joint's JointType enum moved inside class Joint; the 2013 strings say `Joint::GLUE_JOINT` etc. Outside users are qualified, and no CSV row uses the old mangling.
   - GuiItem gains the 2013 secondary base GuiTarget at +0x60 (RTTI plus the C2 at 0x0076a4f0), so focus is at +0x64; GuiTarget::process is defined in GUI.cpp.
   - SpatialFilter's center is a Region2::WeightedPoint (C2 at 0x008e4e20 writes 0.0 at +0x24).
   - StatsItem formatValueId/If/Ii/Iy/Ib take const T&; the oracle signature is formatValue<T>(T const&).
   - ContentReadyHelper takes its two strings by value (iOS signature).
   - EdgeList::getNext parameter order is (const Primitive*, Edge*) (iOS).
3. **Source shape.**
   - MovingStage: its own assertNotInPipeline2 at :35, and the grounded branch carries the child loop.
   - StepJointsStage: `!Joint::isKinematicJoint(edge)`.
   - requirePermission(RobloxPlace, "connectLast").
   - WeakThreadRef dtor: the extra liveThreadRef.reset() is removed.
   - Vector2int16 on_div: the scalar is cast to `(G3D::int16)`.
   - UDim2 and CoordinateFrame on_index call pushUDim/pushVector3, and lookVector uses `-column(2)`.
   - grow: stores happen before delete[].
4. **Unpaired callees.** Markers added for WeakThreadRef::reset (0x00871920 / STUDIO 0x00827da0), pushVector3, pushUDim, and 9 CoordinateFrameBridge closures.

**Build fixes and dropped attempts:**
- Added `#include <boost/thread/tss.hpp>` to reflection/_headercheck.cpp. Another agent's 00:46 edit made SecurityContext.h's inline Context::current need it, and that TU stopped the App build.
- Tried and reverted: the luaconf.h `unsigned int : 0` before yieldCaptured had no effect, since VC9 still packs it at +0x18.
- Tried and reverted: findGeoPairConnector with cached locals scored 54% (the 2013 code re-reads members).

**What the first attempt's leftovers achieved:** they are in the baseline. Its edits to NormalId.cpp, SpanningEdge, Name, GUI and similar files left no near-miss in my list, and I built on top of them.

**Pairing-only items for the Integrate agent:**
- The 14 per-target Bridge<T>::on_newindex(lua_State*) wrappers (PLAYER 0x00670170–0x00672190; STUDIO 0x007d8440–0x007da0f0). function-size.csv truncates them at the call: IDA treated the thrower as noreturn. The real size is 0x30, e.g. 0x00670170..0x006701a0, ending `add esp,0x24; pop edi; xor eax,eax; pop esi; ret`.
- ServiceProvider::onChildAdded 0x006a9770 / STUDIO 0x0076f7b0: shared_ptr<Instance>::operator= is unpaired at the call.
- ContentId::convertToLegacyContent 0x0074ecf0 / 0x00779cd0: twin body pairs with a boost::_bi::list3 name.
- SimBody::~SimBody 0x00976a00 / 0x009c4330: the float 0.0 data reference.
- AsyncHttpQueue::isUrlBad: unique_lock<mutex> vs <recursive_mutex> lock name.
- LibraryService::registerDevelopmentLibrary STUDIO 0x009e9800.
- ContentReadyHelper 0x0088edf0: weak_ptr<ScriptContext>::lock is named after its weak_ptr<PartInstance> twin.
- Bridge<CoordinateFrame>::registerClass and its siblings will still need protect_metatable to pair once it moves (below).

## Files changed

- Client/App/include/V8World/Primitive.h
- Client/App/v8world/Primitive.cpp
- Client/App/script/LuaInstanceBridge.cpp
- Client/App/v8datamodel/TextService.cpp
- Client/App/v8world/IWorldStage.cpp
- Client/App/include/V8Kernel/IStage.h
- Client/App/include/Util/IndexArray.h
- Client/App/include/Util/IndexedTree.h
- Client/App/v8world/JointStage.cpp
- Client/App/v8world/EdgeBuffer.cpp
- Client/App/v8world/Contact.cpp
- Client/App/include/V8DataModel/StatsItem.h
- Client/App/v8datamodel/StatsItem.cpp
- Client/App/v8world/MovingStage.cpp
- Client/App/v8world/SimulateStage.cpp
- Client/App/v8world/GroundStage.cpp
- Client/App/include/V8World/Joint.h
- Client/App/v8world/Joint.cpp
- Client/App/v8world/ContactStage.cpp
- Client/App/v8world/Assembly.cpp
- Client/App/v8world/World.cpp
- Client/App/v8world/TreeStage.cpp
- Client/App/include/V8World/SpatialFilter.h
- Client/App/v8world/SpatialFilter.cpp
- Client/App/v8world/StepJointsStage.cpp
- Client/App/script/LuaAtomicClasses.cpp
- Client/App/include/script/LuaAtomicClasses.h
- Client/App/script/ThreadRef.cpp
- Client/App/script/LuaSignal.cpp
- Client/App/script/LuaLibrary.cpp
- Client/App/include/script/LuaLibrary.h
- Client/App/v8xml/SerializerBinary.cpp
- Client/App/include/V8DataModel/Item.h
- Client/App/Gui/GUI.cpp
- Client/App/reflection/_headercheck.cpp

## Unresolved

- Integrate (function-size.csv): the 14 Bridge<T>::on_newindex(lua_State*) wrappers per target (PLAYER 0x00670170,0x00671610,0x006717f0,0x00671850,0x00671910,0x00671b50,0x00671bb0,0x00671c10,0x00671c70,0x00671d30,0x00671d90,0x00671df0,0x00672040,0x00672190; STUDIO 0x007d8440..0x007da0f0) are truncated at the call by IDA's noreturn analysis. Their real size is 0x30. Correcting the size would put them at 100% (87.18 now).
- ScriptContext owner: protect_metatable (PLAYER 0x0066fad0) lived in ScriptContext.cpp in 2013, between `illegal` 0x0066fa60 and openScriptLibrary 0x0066fb00. Ours is in LuaAtomicClasses.cpp, so VC9 inlines it into 6 registerClass bodies (0x00888460, 0x008885e0, 0x008807f0, 0x008890a0, 0x00888160, 0x00888160-family; 85-86%) and into LuaEnum's. Moving it needs an edit to ScriptContext.cpp, which I don't own.
- DataModel owner: GcJob::stepDataModelJob 0x00679e60 and WaitingScriptsJob::stepDataModelJob 0x0068a4e0 need DataModel::get(Instance*) inline (an __RTDynamicCast of Instance to DataModel in 2013); ours makes an out-of-line call. WaitingScriptsJob ctor 0x00679f90 reads LuaSettings+0x84 in 2013; ours reads +0xac, a LuaSettings layout issue.
- Guid.h Registry::unregister 0x0058dd90 (92.81): in 2013 Guid::Data's scope is pointer-like (`data.scope!=NULL` assert, Guid.h:95 in lookupByGuid 0x0055e030), so Data has no getNullName ctor. The asserts belong at Guid.h 89/144/150/167/172. This is a Guid redesign touching Instance; not done.
- EnumDescriptor ctor 0x006fa1a0 (88.46): 2013 passes a temporary copy of the key (`allEnumsNameLookup()[&name]`, as in 2016), so Descriptor::name was a `const Name&` member. Ours stores `const Name*`. That change ripples into others' files; not done.
- Layout-blocked, not attempted:
- Contact::removeFromKernel 0x007b8ec0: deleteAllConnectors sits at vtbl+0x18 in 2013, ours at +0x20; assert 'getKernel()' at Contact.h:57.
- World::removeFromBreakable 0x00747e40: member at +0x60 vs +0x38.
- CoreGuiService::createRobloxScreenGui 0x00708d80: robloxGui at +0x78 vs +0x64.
- SleepStage::onEdgeAdded 0x008e7e00: offsets plus asserts at 932-934.
- MultiJoint dtor 0x009741c0: +0xbc vs +0x9c; 'connector[0] == NULL' at MutilJoint.cpp:41.
- Body::refreshCofm 0x008edf30.
- KernelData::insertBody 0x00831a60: Body caches root at +0x28.
- JointStage visitAddedPrimitive/removeJointFromHere and TreeStage::onSpanningEdgeRemoved.
- GroundStage::rebuildFreeGround 0x008ead20: the Primitive anchored/dragging bytes are at +0x114/+0x115.
- SimulateStage::putFirstMovingRootInSendPhysics 0x00998920 (98.51): Assembly::getFilterPhase is the folded getter 0x008bb760 reading +0xb0, ours +0x70.
- YieldFunctionStateObject::execute 0x0087c580 (97.01): 2013 sizeof(RobloxExtraSpace)=0x2c with yieldCaptured as a byte bitfield at +0x1c. VC9 packs our `bool : 1` into the +0x18 unit even after `unsigned int : 0`, so another spelling is needed (luaconf.h, untested). The second diff ([eax+4] vs [eax]) is also unexplained.
- BlockBlockContact::findGeoPairConnector 0x00854510 (93.94): the 2013 assert 'myData && contactParams' names members that are re-read after the assert (+0x2c, +0x28). Ours are accessors over solverData/heapData (heapData is private to Contact). Renaming the members is needed.
- SpatialHashStatic::safeExtents 0x004cd9a0 (96.77): the 2013 copy came from a RbxOgre TU (include root spelled `Client\App\Include`). No RbxOgre file of ours instantiates it.
- MergeBinder::announceIDREF 0x00447460: 2013 has it in include/v8xml/SerializerV2.h:76 (lowercase includer path), not Reference.h. That means moving a function between files; not done.
- Vector3int16Bridge::on_div 0x00884b40 (91.97): the local stack slots are 2 bytes off; cause not found.
- Joint.h now has the JointType enum inside class Joint. Any other agent's file that names ANCHOR_JOINT etc. unqualified outside Joint scope must use Joint::. All current users in Client/ were updated.
- GuiItem now derives GuiTarget (+0x60). GuiObject/Frame also carry a GuiTarget vftable in 2013 (RTTI), but that hierarchy was not touched.
