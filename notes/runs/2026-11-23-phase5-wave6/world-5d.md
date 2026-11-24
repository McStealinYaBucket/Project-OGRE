# world-5d (workflow phase5-wave6), 2026-11-23

Phase 5d, third pass: stage layouts, Primitive, Kernel. Own directories only (Client/App/v8world, v8kernel,
include/V8World, include/V8Kernel, Util/SpanningNode.h, Util/Profiling.h + util/Profiling.cpp, the new
include/Voxel/CellChangeListener.h), private builds in build-wld/ and build-wld-studio/, private reccmp project in
build-wld/proj (CSVs regenerated there from pooled runs of the scored link; the shared reccmp/ was not written).
Every heavy command ran through tools/capped.sh, one build at a time; nothing hit the cap.
Scored copies: build-wld/w6e/ (exe, pdb, map; R1/R2 shared-ish runs, R3 private regenerated CSVs),
build-wld-studio/w6b/ (S1/S2 shared CSVs, S3 private regenerated CSVs).

## Row 5d (PLAYER, tools/row_coverage.py --row 5d)

| | functions at effective 100% |
|---|---|
| before (build/sweep12/P-final.json) | 293 / 1,528 (19.2%) |
| after (build-wld/w6e/R3.json, private regenerated CSVs) | **309 / 1,528 (20.2%)** |

Per-function lists: world-5d-row-before.csv / world-5d-row-after.csv here. Gained 17, lost 1 (pairing only, below):
World::loadProfilers 0x00747f20, onPrimitiveExtentsChanged 0x007458a0, onAssemblyExtentsChanged 0x00747620,
removeFromBreakable 0x00747e40, Primitive::~Primitive 0x00755540, EdgeList::EdgeList 0x007563a0,
Allocator<Body>::operator new 0x007569c0, Kernel::Kernel 0x00831590, ~Kernel 0x0082f670, Kernel::removeBody 0x00831690,
KernelData::KernelData 0x008310b0, IndexArray<Contact> element ctor 0x008e6e50, SleepStage::SleepStage 0x008e7f50,
~SleepStage 0x008e7160, SleepStage::onEdgeAdded 0x008e7e00, Body D0 0x008ee6c0, HumanoidStage::HumanoidStage 0x0099b050.
Outside the row: Profiling::Profiler::Profiler 0x007451d0, CodeProfiler::CodeProfiler 0x00745230, the shared deleting
dtor 0x00745250 and both vftables, SpanningNode D0 0x00753900, SpatialHash ctor 0x008eef80 (effective).
The loss: `singleton_pool<Body,240>::free` 0x00756970 (100 → 96%): its get_pool call 0x007568a0 no longer pairs because
our pool is now `singleton_pool<Body,236>` (the correct size, see Layouts); regenerated shared CSVs should pair it.

Whole image, PLAYER: sweep12 9,721 (shared CSVs) → 9,867 (private regenerated CSVs, same method as wave5's 9,627).
STUDIO: sweep12 9,479 → 9,532 (private regenerated CSVs, S3). Every lost 100% was read with --verbose; the ones in my
area are pairing-only (list under "What blocks"); the rest are other agents' concurrent edits (Settings/GlobalAdvanced
Settings items, ServiceProvider vftables, Lua/RobloxExtraSpace, isNullClassName include spellings).

STUDIO markers set from S3: SleepStage ctor 0x00a06180 / dtor 0x00a053a0 / onEdgeAdded 0x00a06030, World loadProfilers
0x00888870 / onPrimitiveExtentsChanged 0x008866e0 / onAssemblyExtentsChanged 0x00887f70 / removeFromBreakable
0x00888790, EdgeList ctor 0x008956c0, Kernel ctor 0x009a3bf0 / dtor 0x009a1c20 / removeBody 0x009a3cf0, HumanoidStage
ctor 0x00a5abf0, ~Primitive 0x00894860, Profiler/CodeProfiler ctors 0x007d7a30/0x007d7a90, KernelData ctor 0x009a3710.
World::World STUDIO stays 99.1% (signal ctor 0x00887780 pairing, as in wave5).

## Layouts proven

- **SleepStage, 0x14c** (ctor 0x008e7f50, dtor 0x008e7160, both 100%; HumanoidStage ctor news 0x14c). The 2013 layout is
  the 2016 member order: eight std::vector scratch lists 0x10-0x8c (toDeep ... toSleepingJoint: the ctor zeroes three
  dwords per 0x10 and its EH states 2-9 precede the first set), numContactsInStage 0x90, numContactsInKernel 0x94,
  throttling 0x98, debugReentrant 0x99, longStepId 0x9c (-1), six AssemblySets 0xa0-0xdc, steppingContacts[2] 0xe8,
  touchingContacts[2] 0x108 (`eh vector constructor iterator` with the 0x008e6e50 element ctor), steppingJoints 0x128,
  recursivePassId 0x134, externalRecursiveWake 0x138, four scoped_ptr<CodeProfiler> 0x13c-0x148 ("Collision",
  "Joint Sleep", "Wake", "Sleep"). The wave5 note "no toSleepingJoint vector" was wrong: all eight vectors are there.
  Dtor asserts laid on SleepStage.cpp:64-77 (the file's 2013 path is Client\App\V8World\SleepStage.cpp).
- **Profiling::Profiler / CodeProfiler, 0xc040** (util/Profiling.cpp, the 2013 assert path): vptr, bucketTimeSpan 0x08
  (= profilingWindow 0x00d5cec0 (3.0) * 0.003), currentBucket 0x10, boost::array<Bucket,0x800> 0x18 (Bucket = two
  doubles and an int, 0x18), lastSampleTime 0xc018, name 0xc020; one-slot vftables 0x00b526d4/0x00b52790 share the D0.
- **IStage is 0xc** (vptr, upstream, downstream); **IWorldStage adds world at 0xc**. Proof: Kernel's ctor 0x00831590 stores
  [+4]/[+8] before its vptr and its own +0xc (maxBodies, 0) after it. Kernel: maxBodies 0x0c, ..., data 0x24,
  scoped_ptr profilers 0x28 ("Kernel Bodies") / 0x2c ("Kernel Connectors"); ctor and dtor 100%.
- **Body is 0xec, not 0xf0**: Primitive's ctor pushes 0xec to Allocator<Body>::operator new, the delete adds 0xec back,
  and the pool's get_pool 0x007568a0 stores requested size 0xec. (The "singleton_pool<Body,240>" name was our own build's.)
- **Primitive 0x158-0x170 is 2016's ComputeProp<float, Primitive>** {val 0x158, dirty 0x15c, object 0x160, getFunc 0x168}:
  the member-function pointer is 8-aligned (+0x164 is padding) and the ctor takes it as an argument, so &computeJointK is
  loaded before the dirty/object stores. Modeled as Primitive::JointKProp (Util/ComputeProp.h is empty and not mine).
  The C2 also default-constructs fuzzyExtents (Extents() = maxFinite/-maxFinite, no temporaries), initializes
  networkOwner from NetworkOwner::Unassigned(), leaves networkIsSleeping/specificGravity/val unset, and logs
  "Primitive %p created" before the surface loop. ~Primitive (100%) deletes geometry and body before nulling both.
- **SpatialHash<Primitive,Contact,ContactManager,4>, 0xdc** (ctor 0x008eef80 effective 100%): ConcurrencyValidator 0x00,
  maxNodes 0x24, treeNodeCount 0x28, world 0x2c, manager 0x30, four bucket vectors 0x34, nodeCount 0x74, int80 0x78,
  G3D::Array<Primitive*> 0x7c, listener std::vector 0x8c, RBX::object_pool<TreeNode, roblox_allocator> 0x9c and
  object_pool<SpatialNode> 0xbc (0x20 each: boost::object_pool 0x1c + the CallDestructor byte; requested sizes 0x30/0x20).
- **MultiJoint, 0xcc** (RotateJoint::surfaceTypeToJoint 0x00974d90 news 0xcc for RotateJoint, 0xd8 for RotateP/V):
  `// SIZE 0xcc` + DECOMP_SIZE_ASSERT.
- **SpanningNode::~SpanningNode is inline** (only its D0 0x00753900 is out of line, SYNTHETIC marker).
- **Voxel::CellChangeListener** moved to include/Voxel/CellChangeListener.h (2016 path; no 2013 __FILE__ names it).

## Assert-line layout

- Kernel.cpp: ctor, dtor :41, insertBody :77, removeBody :88/:90/:92 (2013 expressions), insertPoint :98,
  insertConnector :104, removePoint :110, removeConnector :116, deletePoint :172. newPointLocal :146, step :185 and the
  stepWorld/stepWorldThrottled asserts (:303-:393) are not there yet (different bodies).
- World.cpp, reordered into 2013 order after the dtor: ticklePrimitive :362, onPrimitiveExtentsChanged :425/:426,
  onAssemblyExtentsChanged :432/:433, insertJoint :705/:732, removeFromBreakable :746, destroyContact :824,
  insertPrimitive :856-:861/:869, removePrimitive :875-:877/:888-:893, createAutoJoints :999 (2013 writes
  RBXASSERT(!inStepCode) there, not assertNotInStep). Still off: :371-:404 (EngineChanging..PreventCollide are STUBs
  without bodies), :781/:806 (joint helpers), :910-:948 destroyAutoJoints, :1034/:1048.
- SleepStage.cpp, reordered: canThrottle :290, stepJoints :365/:372, changeContactState :678, changeJointState
  :743/:751/:756, changeAssemblyState :768/:772/:776/:777/:784/:786 (2013 writes the HUMANOID_STAGE branch first),
  onEdgeAdded :932-:934, onEdgeRemoving :957/:979-:981. stateToSet :824 is not reachable: our changeAssemblyState tail is
  ~18 lines longer than 2013's.

## Patterns proven (rule, tell, function)

- **A member-function pointer is 8-aligned under the MI model** even though it is two dwords: a struct {float; bool;
  T*; float (C::*)()} is 0x18 with padding before the pointer. Tell: an unexplained unset dword before a stored
  member-pointer pair. Primitive ctor 0x00756a40 (+0x164 never stored); the struct grew Primitive to 0x178 until val
  moved to 0x158.
- **An ICF-folded deleting destructor pairs by its marker only when no CSV row claims it with another decorated
  name**: Profiler/CodeProfiler D0 0x00745250 keeps `??_EProfiler` as the last PUBLICS; the shared CSV row's
  `??_GCodeProfiler` symbol blocked the SYNTHETIC marker until the private CSVs dropped the marker-owned row.
- **New template instantiations move twin pairings**: adding vector<Contact*> made `??$_Allocate@PAVContact` the last
  PUBLICS of the folded _Allocate body, so the shared libdata row for 0x00429c80 (`_Allocate@PAVChatOption`) stopped
  pairing. Fix by regeneration, or by a TEMPLATE nameref with the decorated name (used for the two IndexArray
  fastRemove copies 0x00746f40 / 0x008e6cf0, which lost their libdata rows the same way).
- **VC9 drops the EH state store before a call it knows cannot throw** (inline body visible): the original Primitive ctor
  has no state-1 store before ExtentsInt32() 0x004e5df0; ours, with the ctor defined in util/ExtentsInt32.cpp, stores
  it. Consistent with 2013 defining ExtentsInt32() inline in its header (not yet tested: other owner).

## What blocks the rest

- **Primitive ctor 0x00756a40, 98.9%**: (1) ExtentsInt32() should be inline in Util/ExtentsInt32.h (state-1 store);
  (2) ConcurrencyValidator's ctor store order: the original stores `writing` before the EH this-save / string ctor
  (also in the ContactManager ctor 0x007b63a0 and the SpatialHash ctor); Util/ConcurrencyValidator.h is not mine;
  (3) NetworkOwner::Unassigned 0x00559100 (Network) does not pair with our Primitive.obj copy (51%).
- **ContactManager ctor 0x007b63a0, 80.4%**: ConcurrencyValidator order above, a 4-byte smaller frame (the original keeps
  separate locals for the two unordered_set bucket counts), and the prime_list end address (0x00bdb080) pairing with a
  string in the CSVs.
- **StepJointsStage ctor 0x008ef410 (41.7%)**: 2013 has a boost::intrusive list (size 0x10, root 0x14/0x18) of edges
  hooked at edge+0x24/+0x28, then scoped_ptr profilingJointUpdate 0x1c (now a scoped_ptr here, so loadProfilers matches).
  Needs the Edge hook members.
- **~SpatialHash 0x007b95d0 (71%)** and the .inl bodies are still the Mac reconstruction (they now allocate from the pools).
- Pairing-only losses that a shared-CSV regeneration should restore: `_Allocate` twin 0x00429c80 and
  array_constructor::construct 0x00429b60 (STUDIO 0x004791e0/0x004865c0), singleton_pool<Body,240> get_pool/free
  0x007568a0/0x00756970 (STUDIO 0x00895c40), the libdata rows for `G3D::Array<SpatialHashListener*>::resize` 0x0082f180
  and `_Move_backward_opt<SpatialNode**>` 0x008e6c10 (those instantiations are gone; the twins pair elsewhere in the
  private CSVs).

## Files changed (mine unless noted)

include/V8World: SleepStage.h (rewritten), World.h, IWorldStage.h, IPipelined.h, ContactManager.h, StepJointsStage.h,
MultiJoint.h, Primitive.h, SpatialHashMultiRes.h/.inl. include/V8Kernel: IStage.h, Kernel.h, KernelData.h, Body.h.
v8world: SleepStage.cpp, World.cpp, HumanoidStage.cpp (marker), StepJointsStage.cpp, EdgeStage.cpp, Primitive.cpp,
MutilJoint.cpp. v8kernel: Kernel.cpp, Body.cpp (marker + blank lines), Constants.cpp (CodeProfiler stub removed).
Util/Profiling.h, util/Profiling.cpp, Util/SpanningNode.h, include/Voxel/CellChangeListener.h (new).
**util/SpanningNode.cpp** (Util owner's file): its out-of-line ~SpanningNode was removed, since the dtor is now inline in
the header (a duplicate definition otherwise).

## Notes for the other owners

- Util/ExtentsInt32.h: make ExtentsInt32() inline (see Patterns); Util/ConcurrencyValidator.h: ctor store order.
- Util/ComputeProp.h is empty: the 2013 ComputeProp<Type,O> is {val, dirty, object, getFunc}; Primitive.h has a nested
  stand-in (Primitive::JointKProp) that could become ComputeProp<float, Primitive>.
- Client/Network: NetworkOwner::Unassigned() is out of line at PLAYER 0x00559100 / STUDIO 0x005b7bc0.
- Integrator: regenerate the shared reccmp/ CSVs (new markers; the pairing-only items above).
