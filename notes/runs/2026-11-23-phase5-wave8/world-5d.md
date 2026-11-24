# world-5d (workflow phase5-wave8), 2026-11-24

Phase 5d, fifth pass: kernel, joints, contacts. Own directories only (Client/App/v8world, v8kernel, include/V8World,
include/V8Kernel; Util/Profiling.h, util/Profiling.cpp and Util/SpanningEdge.h were not touched). Builds in build-wld/
(PLAYER) and build-wld-studio/ (STUDIO). The private reccmp project is build-wld/proj, and its CSVs were regenerated
there from 2 pooled runs of each scored link (the shared reccmp/ was never written). Every heavy command ran through
tools/capped.sh, one build at a time, and nothing hit the cap.

Scored copies:
- PLAYER: build-wld/w8/s8/ (exe/pdb/map), runs P8a/P8b (pooled for the CSVs) and P8 (final, private CSVs).
- STUDIO: build-wld-studio/w8/s8/, runs S8a/S8b and S8.
- Helpers (not repo tools): build-wld/w8/dis.py (disassembler with export names), assertfn.py (assert string to
  function), cmp.py / rowdiff.py (run diffs), flip.py (STUB to FUNCTION at effective 100%), addstudio.py
  (STUDIO markers from ida/exports/STUDIO-PLAYER-bodymatch.csv without moving lines).

## Row 5d (PLAYER, tools/row_coverage.py --row 5d)

| | functions at effective 100% |
|---|---|
| before (build/sweep14/P-final.json) | 368 / 1,553 (23.7%) |
| after (build-wld/w8/P8.json, private regenerated CSVs) | **418 / 1,553 (26.9%)**, +51 / -1 |

Per-function lists are in world-5d-row-before.csv and world-5d-row-after.csv here.

Whole image against sweep14 (this mixes in other agents' uncommitted work):
- PLAYER 10,076 -> 10,450.
- STUDIO 9,803 -> 10,006.

I read every lost 100% with --verbose. Mine are 2 functions in each target, both from one cause (the IndexArray
fastRemove fold, see "What blocks"). The rest are other agents' files: the FactoryProduct/Described/service rows, the
AsyncHttpQueue list nodes, and the Camera/TouchTransmitter deleting destructors.

## Functions raised (PLAYER effective 100%; STUDIO FUNCTION where S8 measured 100%)

**KernelData** (now 11 IndexArrays):
- addLeafBody 0x008306e0 (69% -> 100)
- removeLeafBody 0x008307d0 (new)
- removeLeafBodies 0x00830870 (new)
- removeBodyFromCurrentList 0x008308b0 (24% -> 100)
- addLeafBodies 0x00831180 (52% -> 100)
- addBodyToNewList 0x00831370 (14% -> 100)
- addConnectorToBody 0x008314c0 (new)
- removeConnectorFromBody 0x00831530 (new)
- addConnector 0x00831ba0 (27% -> 100)

**Kernel:**
- preStepThrottled 0x0082ebc0 (62% -> 100)
- stepWorldThrottled 0x0082ec40 (23% -> 100)
- removePoint 0x00830970 (57% -> 100)
- insertPoint 0x008309d0 (44% -> 100)
- stepWorld 0x00830b20 (14% -> 100)
- preStep 0x008318e0 (2% -> 100)
- step 0x00832430 (96% -> 100)

**IndexArray instances** (TEMPLATE markers in KernelData.h):
- fastAppend/fastRemove for all 10 lists, 0x0082f760-0x00830620: 20 functions, 100%.
- operator[] non-const: 0x0082e150, 1d0, 2d0, 3d0, 4d0, 950 at 100%.
- operator[] const: 12 instances paired but held at 95.5% by the IndexArray.h line (see blockers).
- realTime connector operator[] const 0x0082e7d0 and fastAppend 0x009997a0 now pair through the folded Connector name.
- The out-of-line index getters Body::getLeafBodyIndex 0x0082e130 and Point::getKernelIndex 0x0082e140 are 100%.

**SimBody / Point:**
- SimBody::updateAngMomentum 0x00976bf0: `angMomentum = world * angVel`.
- SimBody::accumulateForce 0x0084e340: out-of-line header inline, now marked.
- Point::forceToBody 0x00978900 and Point::step 0x00978920.
- ~SimBody 0x00976a00 is back to 100.

**Constants** (paired through the oracle names once called): kernelStepsPerWorldStep, impulseSolverMaxIterations,
impulseSolverAccuracy, impulseSolverAccuracyScalar (0x0084d3c0 / 400 / 410 / 420).

**Primitive / Geometry / Joint:**
- Primitive::isGeometryOrthogonal 0x007542c0 (new, out of line, tail jump through Geometry vtable+0x44).
- Joint ctor (Primitive*, Primitive*, CF, CF) 0x008215c0 (94.8% -> 100).
- Geometry::setSize base 0x00753630 pairs.

**World:** onPrimitiveGeometryChanged 0x00745950 (new, :445/:446).

**ContactManager:** releasePair 0x007b35f0 (95.8% -> 100). This followed from laying out ContactManager.cpp.

**SleepStage:** isAffecting 0x008e6790 (new, :505/:506) and wakeEdge 0x008e9030 (new, :519/:520/:531).

**MovingAssemblyStage:** jointsStepUi 0x008eb2a0 (79% -> 100).

Primitive::setGuid 0x00753c10 also reached 100%, but that came from the Util/Guid.h edit in the tree (the note from
the last wave), not from me.

STUDIO: I added STUDIO markers from the body-match pairs only where no line moved, then flipped them to FUNCTION when
S8 measured 100%: 25 markers across KernelData.h, Kernel.cpp, SimBody.h/.cpp, Body.h, Point.h, Primitive.cpp,
Joint.cpp, World.cpp, SleepStage.cpp and MovingAssemblyStage.cpp. Still without a STUDIO marker:
- removeLeafBody.
- insertPoint, removePoint and insertConnector (a marker line there would move :98-:116).
- ContactManager::onPrimitiveGeometryChanged.
- The IndexArray TEMPLATE rows that had no free line got STUDIO rows at the end of KernelData.h instead.

## Layouts proven

- **KernelData is eleven `IndexArray<T, &T::getXIndex>`** (0xb0), in this order: freeFall, realTime, joint, contact
  (SimBody +0xf0..+0xfc), leaf (Body +0x20), points (Point +4), humanoid, secondPass, realTime, joint, contact
  (Connector +4/+0xc/+8/+0x10/+0x14). There is no buoyancy list in 2013, but `Connector::BUOYANCY` = 4 exists
  (BuoyancyConnector slot 0 0x00962a80 returns 4, and addConnector routes it to the joint list).
- Every KernelData member is defined in the class, on the 2013 lines:
  - dtor :38-:48
  - addLeafBodies :56/:57
  - insertBody :68-:72
  - removeBody :77-:83
  - addConnector :88/:137/:150/:151
  - removeConnector :194/:212/:213
  - addLeafBody :220-:223
  - removeLeafBody :228/:230
  - addConnectorToBody / removeConnectorFromBody / addBodyToNewList / removeBodyFromCurrentList are after those.
- **KernelData::addConnector returns void** and so does Kernel::insertConnector (0x00832370 just forwards eax). Its
  early return is the 2016 three-type test (HUMANOID=2, JOINT=1, KERNEL_JOINT=3). The connector counters are
  `numConnectors[type] += d; numConnectors[0] += d` with d = +-1 kept in a register (`mov eax,1; add [..],eax`).
- **Geometry's vftable through +0x44**:
  - getGeometryType +4, getCollideType +8
  - setSize +0xc, base 0x00753630
  - +0x10..+0x3c: placeholders (setGeometryParameter, getGeometryParameter, then the pure getRadius..vertOverlapsFace)
  - getFaceFromLegacyNormalId +0x40
  - isGeometryOrthogonal +0x44: base `mov al,1` 0x006be550; the sloped polys return false.
  - Slots +0x48..+0x74 are listed in Geometry.h (from the vftable dump) but not modelled.
- Kernel.cpp: stepWorld's asserts land on :303/:337/:342/:353 and stepWorldThrottled's on :393 (step at :185 before
  them). ContactManager.cpp: createContact :534/:558/:573 (a switch on collide type, ball first) and releasePair :637.
  World.cpp: all 2013 asserts for the bodies present are on their lines (:445/:446 added). SleepStage.cpp: :505/:506
  and :519/:520/:531 added.

## Patterns proven (rule, tell, function)

- **IndexArray accessors are out-of-line template instances, and 2013's const/non-const choice follows the reference
  type.** Kernel functions bind `IndexArray<...>& x(data->x)` (non-const: assert line 103) or `const IndexArray<...>&`
  (const: line 113) exactly as 2016 does. Tell: `call 0x0082e1d0` vs `call 0x0082e250` for the same list
  (Kernel::stepWorld 0x00830b20, preStep 0x008318e0).
- **A getter named through a member pointer of a class with multiple inheritance stays a call.** Body and Point
  instances mangle `$H...A@` and call the getter (0x0082e130 `lea eax,[ecx+0x20]`, 0x0082e140 `lea eax,[ecx+4]`).
  SimBody and Connector getters (`$1`) inline. VC9 reproduces this by itself.
- **ICF-folded template instances pair only through the name our PDB keeps.** IndexArray<IndexedTree> and
  IndexArray<Connector, getRealTimeIndex> have identical bodies (index at +8), so both images fold them. A TEMPLATE
  nameref with the IndexedTree name fails ("Failed to find a match"), and the Connector name pairs (0x0082e7d0,
  0x009997a0).
- **A pointer inserted into `std::set<T*>` through a cast gets a stack temporary, and the pointer itself stays in a
  register.** jointsStepUi 0x008eb2a0 holds `const Assembly* a0` in ebx and does
  `insert(const_cast<Assembly*>(a0))`, which copies to `[esp+..]` at the call. A plain `Assembly* a0` has its address
  taken and lives on the stack (79%).
- **Kernel's impulse tolerance evaluates the scalar call first.**
  `Constants::impulseSolverAccuracy() * (contactBodies.size() / Constants::impulseSolverAccuracyScalar() + 1)` calls
  0x0084d420, then fild, then 0x0084d410 / fmul (stepWorld).
- **A same-line widening of SimBody.cpp or World.h breaks earlier 100% bodies through `__LINE__` alone** (~SimBody
  :54-:58 and World::reset World.h:166 each moved by one include or declaration line). Keep new declarations on an
  existing line, or check every touched header with tools/assert_lines.py before building.

## What blocks the rest

- **Util/IndexArray.h (base-pairing's file)** needs two edits:
  - **Line 113 must be `operator[](int) const`.** 2013 has non-const int :103, unsigned :108, const int :113, const
    unsigned :118 (our :113 is the unsigned one). All 12 const instances (0x0082e250, 350, 450, 550, 5d0, 6d0, 7d0,
    8d0, 9d0, ...) sit at 95.5% on the line literal alone.
  - **The fastRemove nameref** above `// FUNCTION: PLAYER 0x00830340` / `STUDIO 0x009a29a0` must become
    `?fastRemove@?$IndexArray@VConnector@RBX@@$1?getRealTimeIndex@12@QAEAAHXZ@RBX@@QAEXPAVConnector@2@@Z`. Our link now
    folds the IndexedTree instance with KernelData's realTime instance, as the original does, and only the Connector
    name resolves.

  These two lost 100%s (both targets) are pairing-only. The body is identical: fastRemove 0x00830340 (STUDIO
  0x009a29a0) and IndexedTree::setIndexedTreeParent 0x00999850 (STUDIO 0x00a44060), whose only diff is the call to it.
  KernelData::removeConnector 0x00831f80 (99.6%) waits on the same rename.
- **FFlag::SymmetricContact** (var 0x00d30f08, registered as "SymmetricContact" at 0x00a65a80, beside
  ClampControllerVelocityMag 0x00d30f07) is not declared in Util/FastFlags.h or defined in util/FastFlags.cpp. Until
  it is, SimBody::updateSymmetricContactState 0x0082e000 stays a declared STUB in SimBody.cpp (the 2013 body is a header
  inline: symmetric = penetrationTorque.squaredLength() < 0.05, vertical = |penetrationForce.x| and |.z| < 0.001, both
  false when the flag is off; it returns symmetric).
- **World::destroyAutoJoints 0x00749340 (87%)**: Joint::jointOwner is an `IJointOwner*` that JointInstance derives at
  +0x78 (the `add eax,-0x78` after the out-of-line getJointOwner 0x00501530). This needs an IJointOwner interface and
  JointInstance (v8datamodel) deriving it. Not done.
- **ContactManager::createContact 0x007b9120 (80%)** is now the 2013 switch on its 2013 lines. The rest is the
  per-class pool allocators (0x007b7170 / 0x007b71f0 / 0x007b7270 / 0x007b72f0 / 0x007b7370; `new` goes through
  Contact allocators) and the 0xd0/0xd4/0x30 object sizes.
- **ContactManager::onPrimitiveGeometryChanged 0x007b9490** is a STUB (declared and called from World, body not
  written).
- **World step functions** uiStep 0x00749e10, doWorldStep 0x007478a0, step 0x0074a0c0 and computeFallen 0x00747ca0 (the
  :514-:625 and :682 asserts), and 0x00746ba0 (:781), are not written. computeFallen walks the stage chain to stage
  type 13 and visits a set at stage+0xb8 with a bound function 0x007471e0 through 0x00747500.
- **SleepStage**: stepSleepStage 0x008e9d90, wakeAssemblies 0x008e9a40, stepContacts 0x008e9170, stepAssembliesAwake
  0x008e8620, stepAssembliesSleepingChecking 0x008e8930 and traverse 0x008e9820 (asserts :168-:486) are not written.
  changeContactState 0x008e8cc0 (97.5%) reads a 16-bit field `cmp word ptr [esi+0x24],3` where ours reads a dword, and
  calls an unpaired World::onPrimitiveCollided.
- **Near misses not chased**:
  - Joint() 0x00821410: pairing only (`Matrix3::operator==` vs its ICF twin `GLight::operator==`).
  - World::insertJoint 0x0074a5d0 (98%): a byte field read as `movzx eax` / `test eax`, i.e. an int-returning
    accessor.
  - ContactManager ctor 0x007b63a0: the ConcurrencyValidator store order, as before.

## Outside my directories (notes for the owners)

- Util/IndexArray.h: the two edits above (line 113 = const int operator[]; the fastRemove nameref).
- Util/FastFlags.h / util/FastFlags.cpp: add `SymmetricContact` (default false), PLAYER 0x00d30f08.
- v8datamodel JointInstance: an IJointOwner base at +0x78 (for World::destroyAutoJoints).
- Integrator: regenerate the shared reccmp/ CSVs. New markers are the KernelData.h TEMPLATE rows for the IndexArray
  instances; the getters in Body.h, Point.h and SimBody.h; and the Primitive, World, SleepStage and ContactManager
  functions above.

## Files changed (mine)

- include/V8Kernel: KernelData.h (rewritten on IndexArray), Kernel.h, SimBody.h, Body.h, Point.h, Connector.h
  (BUOYANCY, index getters), ContactConnector.h (clearImpulseComputed).
- include/V8World: Geometry.h (vftable slots, GeometryType enum), Primitive.h (isGeometryOrthogonal out of line, line
  count kept), World.h, SleepStage.h, ContactManager.h.
- v8kernel: Kernel.cpp, SimBody.cpp.
- v8world: Primitive.cpp, ContactManager.cpp, World.cpp, SleepStage.cpp, MovingAssemblyStage.cpp, Joint.cpp (marker
  flip).

## Checks

- check_2016_copies is silent and reccmp_addr_padding reports 0 bad.
- assert_lines shows no new miss in any touched file. Body.h's 7 misses are pre-existing.
- decomplint on v8kernel/v8world/include V8* reports only function_out_of_order (descents pinned by the 2013 assert
  lines, most of them pre-existing) and no marker errors.
