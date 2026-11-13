# core-vtables (workflow phase4-core2, wf_fef33d15-1aa)

PHASE 4 vftable pass. Every heavy command (builds, reccmp, the scripts) ran through tools/capped.sh with the default 8G cap. None hit the cap, and only one build ran at a time.

What the earlier killed attempt left behind: its edits to Edge.h, Joint.h, RigidJoint.h, IWorldStage.h, PartInstance.h and MD5Hasher were measured in build-vt and compared with build/sweep2/P-final3.json. No function at 100% dropped. Gains: Joint::isGroundJoint 0x00745c40, MovingAssemblyStage::onEdgeRemoving 0x008eb4d0 (96.55% -> 100%), IWorldStage::getMetric 0x008e4760 (100%), and the vftables MD5HasherImpl 0x00b30e84 and JointStage 0x00bb2b00 (now 100%). The edits are kept. AssemblyStage 0x00b94b28 was already 100% in PLAYER (6 slots). In STUDIO it reached 100% only after I added the `STUB: STUDIO 0x009cf910` getMetric marker (it read 0.833 without it).

Final counts (build-vt / build-vt-studio):
- **PLAYER:** 160 of 330 vftables do not match (baseline 167 of 329; 166 after the earlier attempt's edits). 170 of 330 vftable rows are at 100%. Implemented 3,840 / 40,468 (was 3,828). Non-library functions at 100% (excluding vftable rows): 1,211 (baseline 1,188).
- **STUDIO:** 117 of 268 do not match (first run of this session: 119; no earlier STUDIO baseline was saved). 151 of 268 rows at 100%. Implemented 3,703 / 42,329. Non-library at 100%: 1,158 (baseline 1,137).

Some of these gains come from the other agent's DataModel and ScriptContext edits, which landed in my build directories at the same time (for example DataModel::loadGame and ScriptContext::setRobloxPlace).

New tool: tools/vtable_structural.py splits reccmp-vtable mismatches into structural ones (a slot missing, extra or misplaced) and pairing-only ones. Pairing-only means identical-bytes twins such as `ret 4` at 0x004027c0 vs 0x00575d00, unpaired scalar deleting destructors, or `_purecall` import thunks. Final split: PLAYER 32 structural / 128 pairing-only; STUDIO 11 / 106.

Per class, PLAYER vftable score before -> after:
- **PartInstance 0x00b4c784:** 0.271 -> 0.725.
  - Added hitTestImpl (+0x78, as a PVInstance pure virtual), setLinearVelocity / setRotationalVelocity (+0xc4/+0xc8), getSurface (+0xd4, declaration only), partNeeds3dAdorn (+0xfc), and onAncestorChanged (+0x4c, stub). PVInstance::readProperty (+0x6c) is a new PVInstance override.
  - Order changes: set before get for TranslationUi/RotationUi; resize moved after getMinimumResizeIncrement.
  - Made non-virtual: setPartSizeUi, setPartSizeUnjoined, setColor3, setAlphaModifier, setDragging, setCoordinateFrameRoot.
  - PVInstance now declares hitTestImpl, computeExtentsWorld and legacyTraverseState. ModelInstance got STUB overrides at 0x006d2060 / 0x006d2120 / 0x006d3bc0 (vftable 0x00b3f6bc, 33 slots).
  - PartInstance::resize (0x00720810, STUDIO 0x007bc260) is back to 100% in both targets: FUNCTION.
  - The only structural slot left is +0x0d4 getSurface, which links to 0 until a link stub exists. The secondary bases (IHasPrimaryPart, IAdornable, CameraSubject, Selectable) are not modelled.
- **BaseScript / CoreScript / StarterScript family:** 0.606 -> 0.667 on 5 vftables. extraErrorReporting is +0x7c, before requestHash (+0x80), per 0x00b339a4 and iOS.
- **BaseThreadPool::PoolData 0x00b744d8:** 0.667 -> 1.0, both targets. Removed the 2016-only shouldSchedule and kMaxScheduleSize. Evidence: PriorityThreadPool::schedule 0x0082cd60 has no size check and no return value, and poolData is at +0x2c. Mac Http::init calls ThreadPool(8, 3), where 3 is NoAction.
- **StatsItem (2013 RBX::Stats::Item) 0x00b01ba4:** 0.567 -> 0.621.
  - Added the askAddChild override (+0x3c, PLAYER 0x0048e4e0, STUDIO 0x00443f80). Its code is identical to the original; it scores 96.67% only because the classDescriptor callee is unpaired (we name the class StatsItem, 2013 names it Stats::Item).
  - getValue is no longer virtual (the extra +0x74 slot is gone).
- **SoundService 0x00b451e4:** 0.571 -> 0.607. Added the onServiceProvider override stub (+0x20, 0x006f4600 / STUDIO 0x00809650).
- **Joint, AnchorJoint, FreeJoint, MultiJoint (SpanningEdge vftables):** 0.5 -> 0.833. Markers added for otherNode 0x00820f80, isHeavierThan 0x00820ef0 and getNode 0x0081fd10. otherConstNode now has the same body as otherNode, so the two fold, as they do in the original.
- **Joint Edge vftables (0x00b7252c etc.):**
  - The `STUB 0x008210d0` marker was on the static getJointType(const Edge*). It now marks the virtual getJointType.
  - The 2013 asserts pin getJointType to Joint.h line 106, align to line 111 and resetLink to line 183 (same in both targets). Joint.h was laid out so those lines match: getJointType 17.1% -> 100%, align -> 100%, resetLink 78.9% -> 100% in both targets (all FUNCTION).
  - Joint Edge 0.231 -> 0.421.
- **MultiJoint 0x00bae10c:** 0.083 -> 0.474. Added putInKernel (0x007b8eb0, folded with Contact::putInKernel; STUDIO 0x009bf480), removeFromKernel and isBroken overrides in the previously empty MutilJoint.cpp. No structural slots left.
- **EventSource processRemoteEvent:** the Windows body is not empty: it calls descriptor.fireEvent(this, args). Now 100% in both targets (0x0066edf0 / STUDIO 0x007412c0, FUNCTION). EventSource and every Described<Instance> secondary vftable went 0.333 -> 0.667, and 7 of them reached 100%.
- **STUDIO AssemblyStage 0x00c67d48 and JointStage 0x00c74540:** 1.0 after the STUDIO getMetric marker. getMetric is FUNCTION in both targets.

Losses: the only functions that lost 100% are protReader (PLAYER 0x00875f10 / STUDIO 0x00925c00) and luaL_loadprotected (0x00876140 / 0x00925e30), now 99.0% and 98.97% in both targets. `--verbose` shows the only difference is one call target, an identical-twin pairing. The original calls 0x0066ef40 (`mov eax,[ecx+0x1c]; ret`, CSV name ProgramMemoryCheckerImpl::getLastCompletedHash). In the 17:03 build ours was a separate deque<ChatLine>::size; it is now /OPT:ICF-folded into a group with IndexedMesh::getUpper, ProtectedString::getOriginalHash and the WaitingThread / timestamped_safe_queue size() functions. I edited none of those files, so this is the twin churn CLAUDE.md describes (Gotchas). I did not revert anything for it.

Checks:
- check_folded PLAYER: 0 problems.
- reccmp_addr_padding: one badly padded marker, in DataModel.h:462, which belongs to the other agent.
- decomplint: my new markers are in ascending order inside the blocks I touched (I reordered Joint.cpp getNode and isHeavierThan, and moved PVInstance::readProperty). The out-of-order errors that remain in Joint.cpp and PartInstance.cpp were already there.

## Structured fields

```json
{
 "files": [
  "tools/vtable_structural.py",
  "Client/App/include/V8DataModel/PartInstance.h",
  "Client/App/v8datamodel/PartInstance.cpp",
  "Client/App/include/V8DataModel/PVInstance.h",
  "Client/App/v8datamodel/PVInstance.cpp",
  "Client/App/include/V8DataModel/ModelInstance.h",
  "Client/App/v8datamodel/ModelInstance.cpp",
  "Client/App/include/V8DataModel/BaseScript.h",
  "Client/App/include/V8DataModel/StatsItem.h",
  "Client/App/include/Util/SoundService.h",
  "Client/App/util/SoundService.cpp",
  "Client/App/include/Util/ThreadPool.h",
  "Client/App/util/ThreadPool.cpp",
  "Client/App/util/Http.cpp",
  "Client/App/include/V8World/Joint.h",
  "Client/App/v8world/Joint.cpp",
  "Client/App/include/V8World/MultiJoint.h",
  "Client/App/v8world/MutilJoint.cpp",
  "Client/App/include/V8World/IWorldStage.h",
  "Client/App/reflection/Event.cpp",
  "Client/App/v8world/MovingAssemblyStage.cpp",
  "Client/App/include/V8World/Edge.h",
  "Client/App/include/V8World/RigidJoint.h",
  "Client/App/util/MD5Hasher.cpp",
  "Client/App/include/Util/MD5Hasher.h",
  "build-vt/vt8.txt",
  "build-vt/m8.json",
  "build-vt-studio/svt3.txt",
  "build-vt-studio/s3.json"
 ],
 "unresolved": [
  "PartInstance::getSurface (+0xd4; PLAYER 0x0071c220, STUDIO 0x007b7c80) is declared but not defined, because no 2013 Surface class exists yet. Its slot links to 0 until the Integrate agent reruns tools/gen_link_stubs.py.",
  "PartInstance and ModelInstance do not model the 2013 secondary bases IHasPrimaryPart@96, IAdornable@116, CameraSubject@140, Selectable@160 and virtual IHasLocation. getLocation, getRenderLocation, getRenderSize, onCameraNear, onGuidChanged and getPrimaryPart therefore stay as extra primary slots after +0xfc, and the class layout (and every member offset after +96) still needs that restructure.",
  "protReader and luaL_loadprotected (PLAYER 0x00875f10/0x00876140, STUDIO 0x00925c00/0x00925e30) dropped from 100% to 99.0%. The cause is ICF twin pairing of their call to 0x0066ef40 (getLastCompletedHash vs deque::size) after the fold group changed; none of my files are involved. The Integrate agent should check this when regenerating CSVs (a symbol row, or a marker on the real ProgramMemoryCheckerImpl::getLastCompletedHash).",
  "128 PLAYER / 106 STUDIO mismatching vftables are pairing-only (identical-byte twins such as 0x004027c0/0x00575d00, 0x00927cc0/0x00402730 and 0x00528720/0x009d55f0; unpaired scalar deleting destructors such as EdgeBuffer 0x008ea600 vs AssemblyStage 0x008ea3c0; EnumDesc<T>::convertToString folded at 0x00772590; sp_counted_impl destructors). Fixing them needs CSV/pairing work by the Integrate agent, not header changes.",
  "DataModel vftable 0x00b2ffec (34 slots vs our 29) belongs to the other agent and was not touched.",
  "Stats::Item::askAddChild 0x0048e4e0 has identical code but scores 96.67% because our class is named StatsItem, not RBX::Stats::Item, so its classDescriptor callee does not pair. The class rename is out of scope here.",
  "The new STUB bodies (ModelInstance hitTestImpl/computeExtentsWorld/legacyTraverseState, PartInstance setLinearVelocity/setRotationalVelocity/hitTestImpl/onAncestorChanged, PVInstance::readProperty, SoundService::onServiceProvider, MultiJoint putInKernel/removeFromKernel/isBroken) are placeholders and still need to be written.",
  "No STUDIO vftable baseline from before this session was saved, so the STUDIO before-count (119) is from the first run in this session, after the edits."
 ]
}
```

