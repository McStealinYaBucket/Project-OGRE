# world-5d (workflow phase5-wave7), 2026-11-23

Phase 5d, fourth pass: stages, spatial hash, assert layout. Own directories only (Client/App/v8world, v8kernel,
include/V8World, include/V8Kernel, Util/Profiling.h + util/Profiling.cpp), builds in build-wld/ and build-wld-studio/,
private reccmp project build-wld/proj (CSVs regenerated there from 2 pooled runs of the scored link; the shared reccmp/
was never written). Every heavy command ran through tools/capped.sh, one build of mine at a time; nothing hit the cap.

The interrupted earlier run of this wave had left uncommitted edits in my files (StepJointsStage/MovingAssemblyStage
intrusive lists, Joint hooks, Profiling Mark, SleepStage/World/Kernel bodies, AssemblyHistory, SpatialHash .inl). I
reviewed and built them: they compiled after one fix (World.cpp `destroyJ->links(p)` needed `using Edge::links;` in
Joint), but they had shifted assert lines in Joint.h (getJointType 106 / align 111 / resetLink 183), Primitive.h
(EdgeList 46-60), World.h (reset 166) and Primitive.cpp; all re-laid. One of their FUNCTION markers was wrong
(Profiling::Mark::~Mark 0x00745150, 69%: now STUB). Everything else was kept.

Scored copies: build-wld/w7e/ (PLAYER exe/pdb/map + R1.json, private CSVs), build-wld/w7d/ (R1/R2 the pooled runs
the private CSVs were generated from, R3-R5), build-wld-studio/w7b/ (STUDIO, S1.json private CSVs), build-wld-studio/w7/
(S1/S2 pooled, S3-S5).

## Row 5d (PLAYER, tools/row_coverage.py --row 5d)

| | functions at effective 100% |
|---|---|
| before (build/sweep13/P-final.json) | 314 / 1,551 (20.2%) |
| after (build-wld/w7e/R1.json, private regenerated CSVs) | **367 / 1,551 (23.7%)** |

Per-function lists: world-5d-row-before-sweep13.csv / world-5d-row-after.csv here (world-5d-row-before.csv is the
interrupted run's file, kept). Whole image against sweep13: PLAYER 9,893 -> **10,019** (+139 / -13), STUDIO 9,637 ->
**9,750** (+123 / -10). Every loss was read with --verbose; none is caused by my files:
- PLAYER/STUDIO StatsItem / StatsService Name::doDeclare / getClassName / classDescriptor rows (another agent's
  StatsItem.cpp edits in the tree), and 6 Lua functions (symbexec, luaV_execute, constructor, patchtestreg,
  removevalues, luaK_setreturns): a no-.reloc coincidence, e.g. luaK_setreturns' `and edx, 0x...` mask now equals the
  address of a `sp_counted_impl_pd<ForceField>` RTTI record in our image.

Raised to 100% this wave (PLAYER; STUDIO the same unless noted; "line fix" = restored after the interrupted run's
edits had moved its assert line):
StepJointsStage ctor 0x008ef410 / dtor 0x008ef340 / removeJoint 0x008ef110 / onEdgeAdded 0x008ef1a0 / jointsStepWorld
0x008ef270; MovingAssemblyStage ctor 0x008eb610 / dtor 0x008eb520 / removeJoint 0x008eb3f0 / onEdgeAdded 0x008eb250;
~SpatialHash 0x007b95d0 (STUDIO 0x008d5f70), SpatialHash::TreeNode::~TreeNode 0x007b36f0 (0x008cf770), the two
object_pool destructors 0x007b7450 (0x008d4400) and 0x00502c50 (0x00592e90); Primitive ctor 0x00756a40,
getSizeMultiplier 0x00753ac0, getNextContact 0x00754890, getGeometryType 0x00753cb0 / getCollideType 0x00753d20 (line
fix); World getSleepStage 0x00745d50, ticklePrimitive 0x00745dd0, onPrimitiveEngineChanging/Changed 0x00745e50/0x00745fc0,
onPrimitiveFixedChanging/Changed 0x00746040/0x00746110, onPrimitivePreventCollideChanged 0x007461e0,
onPrimitiveContactParametersChanged 0x00745820, destroyContact 0x00745a80, destroyAutoJointsToWorld 0x007498b0,
createAutoJointsToWorld 0x0074abf0, createAutoJointsToPrimitives 0x0074ad10, reset 0x0070d390 (line fix); EdgeList
getEdge/getOther/~EdgeList (line fix); Assembly::reset 0x007b15b0, visitPrimitives<EdgeBuffer bind> 0x009993f0;
AssemblyHistory::wakeUp 0x0095d500; AssemblyStage onEngineChanging/Changed 0x008ea4b0/0x008ea510; GroundStage
onPrimitiveFixedChanging 0x008eaf70, rebuildOthers 0x008eb0b0, onPrimitiveFixedChanged 0x008eb0f0; EdgeBuffer
afterAssemblyAdded 0x00999460, beforeAssemblyRemoving 0x00999480, assemblyPrimitiveRemoved 0x009990f0; SleepStage
stateToSet 0x008e6860, onAssemblyAdded 0x008e6e60, changeAssemblyState 0x008e7980 (58% -> 100%), onAssemblyRemoving
0x008e7c90; SendPhysics ctor 0x008e61b0; Kernel insertBody 0x008322d0, deletePoint 0x00830ab0, newPointLocal 0x00831820;
Point::setLocalPos 0x00978a10; SimBody::clearVelocity 0x00976b80; Body::resetForceAccumulators 0x008e6910 /
resetImpulseAccumulators 0x008e6970; Joint::isSpringJoint 0x0071b680, isKernelJoint 0x00745c70, isAutoJoint 0x00745cb0
(header statics, out of line in 2013); Profiling Bucket rates 0x00745020/30/50, CodeProfiler::log 0x00745070.
ContactManager::~ContactManager 0x007bb5e0 is 100% again (twin pairing of an unordered `delete_nodes`, see below).

## Layouts proven

- **Joint: SpanningEdge has no data members.** Joint ctor 0x008215c0: Edge 0x00-0x1c, SpanningEdge vptr 0x1c,
  `list_base_hook<tag<StepJointsStage>>` 0x20, `list_base_hook<tag<MovingAssemblyStage>>` 0x28 (safe-mode hooks,
  zeroed), jointOwner 0x30, jointCoord0 0x34, jointCoord1 0x64. The Mac model's four hook pointers inside SpanningEdge
  are gone (Util/SpanningEdge.h edit, see "Outside my directories").
- **StepJointsStage 0x20**: IWorldStage 0x00-0x10, constant-size boost::intrusive list over Joint's StepJointsStageHook
  {size 0x10, root 0x14/0x18}, scoped_ptr<CodeProfiler> 0x1c ("Joint Update"); ctor/dtor 100%.
  **MovingAssemblyStage 0x28**: the same list type over MovingAssemblyStageHook at 0x10, std::set<Assembly*>
  movingGroundedAssemblies 0x1c.
- **SpatialHash nodes derive RBX::NodeBase** {short level 0x00, int hash 0x04, Vector3int32 cell 0x08; 0x14}, whose
  destructor stores -2 to level and hash. SpatialNode adds prim 0x14 / next 0x18 / treeNode 0x1c (dtor nulls treeNode,
  prim, next: the inline body in the pool destructor 0x00502c50); TreeNode adds unsigned short children[8] 0x14,
  hasChild 0x24, refByPrimitives 0x28, next 0x2c, and its out-of-line dtor 0x007b36f0 asserts
  `refByPrimitives == 0` / `next == NULL` (SpatialHashMultiRes.h:287/:288). getLevel's `level >= -1` is :51.
- **AssemblyHistory is 0x20** (Assembly::reset news 0x20; ctor stores 0 at 0x18, 15 at 0x1c).
- **Profiling::Mark is 0x30** (section&, enclosing Mark* from a TLS slot, frameTick, startTime 0x10, enabled 0x18,
  childrenElapsed 0x20, logInclusive 0x28); ctor 0x007454f0 100%.

## Assert-line layout

- **SpatialHashMultiRes.h** restructured so NodeBase::getLevel :51, safeExtents :105/:107 and ~TreeNode :287/:288 land
  (node structs forward-declared at the top of the class, defined before the data members). .inl: ~SpatialHash :91.
- **World.cpp**: every assert the 2013 strings list for the bodies present is on its line (assert_lines: 0 misses),
  including destroyJoint :806 (`j`) and destroyAutoJoints :910-:948. Still missing (bodies absent): :445/:446,
  :514-:625 (the step functions), :682, and :781 (0x00746ba0, `!Joint::isKinematicJoint(j) || (parent && child &&
  parent != child)`, no 2016 counterpart).
- **Primitive.cpp**: getSizeMultiplier moved to its 2013 slot (:120 `0`), setGuid after getGuid (:144, :152 `!world`).
- **Joint.cpp** Joint() :77/:78 now stringize `jointCoord0 == CoordinateFrame()` (a file-scope `using G3D::CoordinateFrame`).
- **Assembly.h** visitPrimitives asserts `p` at :203.
- **Kernel.cpp** :303-:393 not done: the 2013 stepWorld / stepWorldThrottled / preStep index KernelData through
  IndexArray out-of-line accessors (0x0082e150, 0x0082e1d0, 0x0082e250 ...), ours are G3D::Arrays (see blockers).

## Patterns proven (rule, tell, function)

- **A template body in a .inl included under a non-canonical spelling cannot carry a FUNCTION/STUB marker on Linux**:
  2013's `include\v8World/SpatialHashMultiRes.inl` spelling (needed for the assert string) makes the PDB line records
  name a path that does not resolve to Client/App/include/V8World/; the marker silently stays unpaired. Pair such
  functions with a `// TEMPLATE:` + decorated nameref in the header instead (~SpatialHash 0x007b95d0).
- **Small static predicates in a header are emitted out of line and called** (`Joint::isKernelJoint` 0x00745c70,
  `isSpringJoint` 0x0071b680, `isAutoJoint` 0x00745cb0, cdecl, `push e; call; add esp,4`): give them header markers,
  otherwise the caller (World::insertJoint) scores the calls as unpaired.
- **2013 SimBody clears accumulators with `Vector3(0.0, 0.0, 0.0)`, not `Vector3::zero()`**: the tell is inline
  `xorps; movss` stores at +0xcc..+0xe0 (SleepStage::onAssemblyAdded 0x008e6e60); `zero()` is an out-of-line call here.
- **World owns contacts**: destroyContact 0x00745a80 deletes the contact after onEdgeRemoving (ours leaked it).
- **A ternary bool argument is an if/else of two literal calls in 2013** (GroundStage::onPrimitiveFixedChanging
  0x008eaf70: `removeGroundJoint(p, true)` / `(p, false)`).
- Kernel::insertBody's body-count sum is `contact + joint + realTime + freeFall` (load order [+0x34],[+0x24],[+0x14],[+4]).

## What blocks the rest

- **KernelData is IndexArray-based in 2013** (out-of-line operator[] per list, 0x0082e150...; fastRemove 0x0082f810):
  Kernel::stepWorld 0x00830b20 (14%), stepWorldThrottled 0x0082ec40, preStep 0x008318e0, preStepThrottled 0x0082ebc0,
  insertPoint/removePoint, and the :303-:393 assert expressions (`validateConnector(contactConnectors[j])`) wait on it.
- **Joint ctors** (0x008215c0 94.8%, 0x00821410 98.5%): `Primitive::isGeometryOrthogonal` is out of line in 2013
  (0x007542c0: `geometry->vtable[0x44]`), ours is a switch over a stub Geometry; needs the real Geometry vftable.
  Joint() is otherwise pairing-only (`Matrix3::operator==` vs its ICF twin `GLight::operator==`).
- **Pairing-only near misses** (body identical, a callee is an ICF twin our link names differently):
  World::destroyJoint 0x007492d0, createAutoJoints 0x0074aa80 99.1%, SendPhysics::onMovingAssemblyRootAdded/Removing
  0x008e5fe0/0x008e60b0 (`signal_with_args<1, void(T*)>::operator()` folded with the lua_State* instance in ours, not in
  2013). ContactManager::~ContactManager flips between 98.7% and 100% with the unordered `delete_nodes` twin choice.
- **World::destroyAutoJoints 0x00749340 (86.7%)**: 2013 reads the owner through an out-of-line getJointOwner (folded
  at 0x00501530) as an IJointOwner* and adjusts by -0x78 to the JointInstance.
- **MovingAssemblyStage::jointsStepUi 0x008eb2a0 (79%)**: a0 lives in ebx and the bool on the stack in 2013; not found.
- **Primitive::setGuid 0x00753c10 (98.4%)**: needs `Guid::copyDataFrom` to be `assign(other.data)` (Util/Guid.h).
- **Profiling::Mark::~Mark 0x00745150 (69%)**: 2013's `Time::now() - startTime` returns Time::Interval through a hidden
  pointer (non-POD Interval) and keeps the elapsed value for both log and the parent; Base's Time.h decides it.
- **ContactManager::releasePair 0x007b35f0**: its `RBXASSERT(0)` is ContactManager.cpp:637; the file is not laid out.
- **SleepStage** still lacks the 2013 bodies with asserts :168-:531 (wake/sleep/step passes).

## Outside my directories (notes for the owners)

- **Client/App/include/Util/SpanningEdge.h** (Util, not listed for anyone this wave): the interrupted run removed its
  four Mac-model hook pointers (stepNext/stepPrev/moveNext/movePrev) because Joint now carries the two boost hooks
  itself; I kept that edit (Joint's layout and the 14 stage functions above depend on it). Please review/commit with
  this file set.
- Util/Guid.h: `copyDataFrom(other)` should be `assign(other.data)` (no default-constructed Data, no
  Name::getNullName call): setGuid 0x00753c10.
- Base Time.h: Time::Interval is returned through a hidden pointer in 2013 (Mark::~Mark 0x00745150).
- Util/ConcurrencyValidator.h ctor store order is still the blocker for ContactManager ctor 0x007b63a0 (80.4%).
- Integrator: regenerate the shared reccmp/ CSVs (new markers: TEMPLATE rows for ~SpatialHash, ~TreeNode, both pool
  dtors; header FUNCTION markers on the Joint predicates and Body accumulator resets).

## Files changed (mine)

include/V8World: Assembly.h, AssemblyHistory.h, AssemblyStage.h, EdgeBuffer.h, GroundStage.h, Joint.h,
MovingAssemblyStage.h, Primitive.h, SpatialHashMultiRes.h/.inl, StepJointsStage.h, World.h. include/V8Kernel: Body.h,
Kernel.h, SimBody.h. v8world: Assembly.cpp, AssemblyHistory.cpp, AssemblyStage.cpp, Contact.cpp (marker),
ContactManager.cpp (marker), EdgeBuffer.cpp, GroundStage.cpp, Joint.cpp, MovingAssemblyStage.cpp, Primitive.cpp,
SendPhysics.cpp, SleepStage.cpp, StepJointsStage.cpp, World.cpp. v8kernel: Kernel.cpp, Point.cpp, SimBody.cpp
(marker). Util/Profiling.h, util/Profiling.cpp. Plus Util/SpanningEdge.h (above). Checks: check_2016_copies silent,
reccmp_addr_padding 0 bad, decomplint shows no new issue on any address added here.
