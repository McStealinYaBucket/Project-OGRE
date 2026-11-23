# world-5d (workflow phase5-wave5), 2026-11-23

Phase 5d, second pass: the 2013 World layout and physics. Own directories only (Client/App/v8world, v8kernel,
include/V8World, include/V8Kernel), private builds in build-wld/ and build-wld-studio/, private reccmp project in
build-wld/proj (CSVs regenerated there from two pooled runs of the same link; the shared reccmp/ was not written).
Every heavy command ran through tools/capped.sh; one build at a time; nothing hit the cap.

## Row 5d (PLAYER, tools/row_coverage.py --row 5d)

| | functions at effective 100% |
|---|---|
| before (build/sweep11/P-final.json) | 252 / 1,527 (16.5%) |
| after, shared CSVs (build-wld/sc6/P1.json) | 279 / 1,527 (18.3%) |
| after, private regenerated CSVs (build-wld/sc7/P1.json) | **289 / 1,527 (18.9%)** |

The denominator is 1,527 rather than 1,504 because new markers widened the World.cpp range (it now starts at
EThrottle 0x00745590) and the Constants.cpp range. Per-function lists: row-5d-before.csv, row-5d-after.csv here.
Gained in row 5d: 37, lost 0. One of them (Body::getPV_Spin_Lock 0x007554e0) came from another agent's marker flip
in v8kernel/Body.cpp (see Notes), not from me.

Whole-image PLAYER, same link, shared CSVs: sweep11 9,405 → sc6 9,401. The difference is the known six Lua
functions (symbexec, luaV_execute, constructor, patchtestreg, removevalues, luaK_setreturns): `and eax, 0x7fc000`
read as a Complete Object Locator after a relink (checked with --verbose on patchtestreg 0x009a6800). The
other changes were other agents' concurrent edits (the whole tree was shared). With private regenerated CSVs the
same link scores 9,627. No function of mine lost a 100% in any run (sc1→sc7 compared with a script each time).

## Layouts proven (100% ctor and dtor, or the stores that size them)

- **World, 0xb8** (`include/V8World/World.h`, rewritten from the 2013 bytes). Ctor 0x0074ae30 and dtor 0x00749ae0
  both 100%, so the layout is proven:
  - 0x00 autoJoinSignal, 0x04 autoDestroySignal (both `rbx::signal<void(Joint*)>`, one ctor 0x00746cc0),
    0x08 primitiveCollideSignal, 0x0c `rbx::signal<void(Primitive*)> m_unk0x0c` (no name evidence).
  - 0x10 frmThrottle (int 1), 0x14 EThrottle (0xc), 0x20 `G3D::Array<TouchInfo>` (POD element: dtor 0x007b4980),
    0x30 inStepCode, 0x34 inJointNotification, 0x38 worldStepId (reset 0x0070d390 zeroes it).
  - 0x3c `IndexArray<Primitive, &Primitive::worldIndexFunc>` primitives, 0x4c groundPrimitive, 0x50 movingPrimitives,
    0x60 breakableJoints, 0x6c numJoints, 0x70 numContacts, 0x74 numLinkCalls.
  - 0x78 tempPrimitives, 0x88 tempJoints, 0x98..0xa8 five `boost::scoped_ptr<Profiling::CodeProfiler>` (the dtor's EH
    states make them members), 0xac ContactManager*, 0xb0 SendPhysics*, 0xb4 CleanStage* (raw, deleted in the body).
  - The Mac-model member map the header used to carry is gone; all World.cpp bodies use the 2013 names.
- **EThrottle, 0xc**: requestedSkip/usedSkip/throttleIndex; its ctor is inline (World ctor carries it, no out-of-line
  copy). `FFlag::ConsistentThrottlingIncrements` (0x00d30efc) and `EThrottle::globalDebugEThrottle` (0x00df72a8) gate it.
- **ContactManager, 0x6c**: base `Voxel::CellChangeListener` (1 slot, no virtual dtor), ConcurrencyValidator 0x04,
  spatialHash 0x28, myMegaClusterPrim 0x2c, world 0x30, two unordered_sets 0x34/0x50 with a byte at 0x4c,
  `scoped_ptr<CodeProfiler>` profiler 0x68. Proven by the World ctor's `new 0x6c` at 100% and the dtor
  0x007bb5e0 at 100%. Its ctor 0x007b63a0 stays STUB (77%: ContactManagerSpatialHash is 0xa4, 2013 news 0xdc; the
  unordered_set prime-list data rows do not pair).
- **Primitive, 0x170**: the "bring-up" members (geometryType, partSize, coordinateFrame, pv, bodyUpdateSerial and two
  Mac stand-ins, 0x98 bytes) are removed. getGeometryType/getCollideType/getSize are the 2013 out-of-line functions
  over the Geometry (+0x104), getPV/setCoordinateFrame/getBodyUpdateSerial go through the Body. The ctor creates the
  geometry (`newGeometry`, as 2013 0x00755970). Proven by the World ctor's `new 0x170` at 100%.
- **MultiJoint**: numConnector 0x94, an unidentified 0x20 block, maxConnector 0xb8, `connector[4]` 0xbc (dtor
  0x009741c0 at 100%, assert text "connector[0] == NULL", MutilJoint.cpp:41-42).
- **BasicSpatialHashPrimitive** now lives in its 2013 header `include/V8World/BasicSpatialHashPrimitive.h` (it was an
  empty file; SpatialHashPrimitive.h is deleted): dtor assert at :45 then level = -2, getSpatialNodeLevel assert at :55.

## Functions added or raised (PLAYER, effective 100%, markers FUNCTION)

- World.cpp: World::World 0x0074ae30, ~World 0x00749ae0, getKernel 0x00745780, onAssemblyInSimluationStage 0x00746270,
  onJointPrimitiveNulling/Set 0x00745a00/0x00745a10, removeJoint 0x00747eb0, getSendPhysics 0x008bb760 (folded with
  PartInstance::getPartPrimitive), EThrottle::increaseLoad 0x00745590, computeThrottle 0x00745600,
  getEnvironmentSpeed 0x00745720 (new bodies), World::reset 0x0070d390 (World.h:166).
- Templates in World.obj (TEMPLATE namerefs in World.h): the three signal ctors 0x00746cc0/0x00746d20/0x00746d80, the
  two signal dtors 0x00749000/0x00749150, `G3D::Array<shared_ptr<JointInstance>>::~Array` 0x007465a0.
- v8kernel/Constants.cpp: uiStepsPerSec 0x0084d380, uiDt 0x0084d450 (`1.0f / uiStepsPerSec()`, fild/fld1/fdivrp).
- Primitive.cpp: getGeometryType 0x00753cb0, getCollideType 0x00753d20, getSize 0x00753d80 (new, asserts at their
  2013 lines 588/596), computeJointK 0x00753da0.
- SimulateStage::putFirstMovingRootInSendPhysics 0x00998920 and removeFromSendPhysics 0x009987f0 (World layout),
  HumanoidStage::toHumanoid 0x0099ad70 / fromHumanoid 0x0099af50, ContactStage::onEdgeAdded 0x009aa880 (effective),
  JointStage::visitAddedPrimitive 0x009982a0, Joint::setPrimitive 0x00820d30, Kernel::insertConnector 0x00832370 /
  removeConnector 0x008323d0, MultiJoint::~MultiJoint 0x009741c0, ContactManager::~ContactManager 0x007bb5e0
  (100% with the private CSVs; marker left STUB until the shared CSVs carry it).
- New STUB markers with real addresses: CodeProfiler ctor 0x00745230, World::loadProfilers 0x00747f20 (first five
  push_backs written), ContactManager::terrainCellChanged 0x007bb6c0, CleanStage ctor 0x008e62d0, Primitive dtor
  0x00755540 (92.5%), BasicSpatialHashPrimitive::getSpatialNodeLevel 0x00501790 (91.7%).
- GLOBAL markers: FFlag::ConsistentThrottlingIncrements, EThrottle::globalDebugEThrottle, EThrottle::throttleSetting
  0x00b52860, FLog::PrimitiveLifetime 0x00de3f55 (all with STUDIO addresses).

## STUDIO (build-wld-studio/sc1/S1.json, shared CSVs, one run)

- Whole image 9,163 (sweep11) → 9,159. The 49 losses are all outside my directories and match the PLAYER run-1
  losses from other agents' concurrent edits (Type::getSingleton<T> at 91.3%, the LuaSettings factory rows, the
  `{for AbstractFactoryProduct}` vftables at 0.5). Gained 45, among them mine: World::~World 0x0088a430, reset
  0x0082a960, EThrottle increaseLoad/computeThrottle/getEnvironmentSpeed 0x008863d0/0x00886440/0x00886560,
  getKernel 0x008865c0, onJointPrimitiveNulling/Set, onAssemblyInSimluationStage 0x00887090, removeJoint 0x00888800,
  Primitive getGeometryType/getCollideType/getSize/computeJointK 0x00893060-0x00893150, Joint::setPrimitive
  0x009143e0, Kernel insert/removeConnector 0x009a49d0/0x009a4a30, uiDt 0x009a5140, MultiJoint dtor 0x009bf3a0,
  JointStage::visitAddedPrimitive 0x00a58600, ContactStage::onEdgeAdded 0x00a618d0. Their STUDIO markers are FUNCTION
  (Kernel::insertConnector has no STUDIO line: adding one would move its :104 assert).
- World::World is 99.1% in STUDIO: the `rbx::signals::signal<void(Joint*)>` ctor at 0x00887780 does not pair (the
  STUDIO names export calls it `??1strstreambuf@std@@UAE@XZ_45`, a folded body); STUDIO ctor marker stays STUB.

## Patterns proven (rule, tell, function)

- **A STUB callee defined in the caller's .cpp changes the caller, even `__declspec(noinline)`.** VC9 sees that the
  stub cannot throw and does not touch its arguments: the new-expression EH states around `new T(...)` vanish and a
  local `std::vector` passed by reference is optimized out (no zeroing, no `delete`). Tell: missing
  `mov byte [esp+N], state` pairs around `new` + ctor call, missing vector init/free. Fix: put the stub in another
  .obj (CodeProfiler ctor moved to v8kernel/Constants.cpp) or give it the real first statements (loadProfilers pushes
  into the vector). World::World 0x0074ae30 went 88.0% → 99.6% → 100%.
- **Member-pointer IndexArray keys are out-of-line COMDATs in the user's .obj.** `IndexArray<Primitive,
  &Primitive::worldIndexFunc>` emits `Primitive::worldIndexFunc` (`lea eax,[ecx+0xe0]; ret`) at 0x00745580 in
  World.obj; the function must exist in Primitive.h with that name.
- **Operand order of an inline sum decides the loads in another context.** `getNumEdges()` is `joints.size() +
  contacts.size()` (World dtor 0x00749b25 loads +0xc4 first); GroundStage/TreeStage onPrimitiveAdded/Removing stay
  100% with the swap.
- **`p == member ? a : b` emits `cmp reg, [mem]`;** `member == p` emits `mov eax,[mem]; cmp eax, reg`.
  JointStage::visitAddedPrimitive 0x009982a0 uses `j->otherPrimitive(p)`.
- **Branch order follows the written condition.** IPipelined::findWorld is `if (!currentStage) return NULL;` then
  `!inKernel() ? currentStage : currentStage->getUpstream()` (null and not-in-kernel arms first):
  Joint::setPrimitive 0x00820d30 88.3% → 100%.
- **Primitive::getJoint(Primitive*, Primitive*, int) is static** (ContactStage::onEdgeAdded 0x009aa93c pushes p0 and
  cleans 0xc): the Mac signature counts no `this`.
- **The 2013 FFlag macro registers only the flag** (initializer 0x00a658c0: four pushes, no PlaceFilter string), so
  World.cpp spells the flag out with GLOBAL markers instead of FASTFLAGVARIABLE.

## What blocks the rest

- **Primitive ctor 0x00756a40 (63%) and dtor (92.5%):** the dtor inlines `SpanningNode::~SpanningNode` (vptr store +
  IndexedMesh dtor) where ours calls it out of line; SpanningNode is in Util/ (another owner). The ctor needs the
  ExtentsInt32/EdgeList/guid/Body sequence checked; next pass.
- **SleepStage is 0x140, 2013 0x14c** (HumanoidStage ctor 0x0099b050 news 0x14c, 98%): 2013 has touchingContacts at
  0x108, steppingJoints (set) 0x128, an int 0x134, a bool 0x138, four CodeProfiler pointers 0x13c..0x148 and no
  toSleepingJoint vector. Its ctor 0x008e7f50 should be rebuilt from the bytes with that layout.
- **ContactManagerSpatialHash 0xa4 vs 2013 0xdc**, which keeps the ContactManager ctor at 77%.
- **SendPhysics::onMovingAssemblyRootAdded/Removing (98.4/95.5%) are pairing-only:** their
  `signal_with_args<1, void(Primitive*)>::operator()` call pairs with our folded `void(lua_State*)` twin.
- **The signal ctor/dtor bodies pair with the private CSVs only** (safe_static_init_mutex etc. unpaired with the shared
  ones); they will pair once the shared CSVs are regenerated.
- **Kernel.cpp** asserts are mostly not on their 2013 lines (2013: 41, 88-116, 146, 172, 185...); only
  insertConnector/removeConnector were laid out.
- World.cpp bodies after the dtor are still the Mac reconstruction; their asserts (2013 lines 362-1034) are not laid
  out yet, so most stay STUB (onPrimitiveExtentsChanged, insertPrimitive, removePrimitive, createAutoJoints, ...).
- decomplint function_out_of_order in World.cpp (184, 209, 245), Primitive.cpp (74, 96, 142) and ContactManager.cpp
  (121, 183): the ctor/dtor and the 588/596 accessors are pinned by their 2013 assert lines; the CodeProfiler stub
  sits in Constants.cpp only for pairing.

## Notes for the other owners

- `Profiling::CodeProfiler` (and its out-of-line ctor at PLAYER 0x00745230 / STUDIO 0x007d7a90) belong in
  Util/Profiling.h and a util/Profiling.cpp; they are declared in V8World/ContactManager.h and stubbed in
  v8kernel/Constants.cpp until the Util owner moves them. `Voxel::CellChangeListener` is declared in the same header
  for the same reason (it belongs with the voxel headers).
- `SpanningNode::~SpanningNode` should be inline in Util/SpanningNode.h for the Primitive dtor 0x00755540.
- Another agent flipped v8kernel/Body.cpp getPV_Spin_Lock to FUNCTION during this wave (my directory); it measures
  100%, so I kept it.
- The shared reccmp/ CSVs should be regenerated by the integrator: the new markers above need it, and
  ContactManager::~ContactManager plus the World.obj signal templates reach 100% only then.
