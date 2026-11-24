# world-5d (workflow phase5-wave9), 2026-11-24

Phase 5d, sixth pass: joints, contacts, World and sleep bodies. Builds in build-wld/ (PLAYER) and build-wld-studio/
(STUDIO). The private reccmp project is build-wld/proj. Its PLAYER CSVs were regenerated from 2 pooled runs of each
scored link (P3a/b, P4a/b, P5a/b) and its STUDIO CSVs from S1/S1b. The shared reccmp/ was never written. Every heavy
command ran through tools/capped.sh, one build at a time, and nothing hit the cap.

Scored copies:
- PLAYER final: build-wld/w9/s5/ (exe/pdb/map), runs P5a/P5b (pooled) and P5 (final, private CSVs).
- STUDIO final: build-wld-studio/w9/S3.json, on the link of the final source. S2 is the run after the CSV regeneration.
- Helpers (not repo tools): build-wld/w9/
  - dis.py, rtti.py (COL/CHD dump from a vftable), callers.py, q.py
  - vd.sh (verbose diff hunks only), cmp.py, realloss.py, flip.py

**build-wld's cache had `OGRE_BOOST_PO_GL=OFF`.** The option's default changed in b47a4dd, but the cached value stayed.
With it off, about 300 client-shell and program_options entries show as losses. I set it ON. Any other agent's old
build dir needs the same `cmake -DOGRE_BOOST_PO_GL=ON <dir>`.

## Row 5d (PLAYER, tools/row_coverage.py --row 5d)

| | functions at effective 100% |
|---|---|
| before (build/sweep15/P-final.json) | 421 / 1,554 (27.1%) |
| after (build-wld/w9/P5.json, private regenerated CSVs) | **464 / 1,554 (29.9%)**, +43 / -0 |

The per-function lists are world-5d-row-before.csv and world-5d-row-after.csv in this folder.

Whole image against sweep15 (this includes other agents' uncommitted work):
- PLAYER 10,729 -> 10,819.
- STUDIO 10,192 -> 10,227.

I listed every lost 100% with realloss.py: 78+7 in PLAYER, 56 in STUDIO. None is in v8world/v8kernel/Joint/Contact or
any other range I touched. I read samples with --verbose, and they are ICF-twin pairing churn:
- boost::function `assign_to_own`.
- `shared_ptr<Instance>::operator=`.
- vector<shared_ptr<Instance>>.
- The ServiceProvider::create / ScriptContext / ScreenGui families, which other agents' Base and Gui edits move.

## Functions raised (PLAYER effective 100%; STUDIO FUNCTION where S2/S3 measured 100%)

World.cpp:
- notifyMoved(Primitive*) 0x00746b40 (new).
- jointCoordsChanged 0x00746ba0 (new, the :781 kinematic assert).
- sendClumpChangedMessage 0x007462e0 (new).
- AppendFallen 0x007471e0 (new).
- computeFallen 0x00747ca0 (new, :682).
- onPrimitiveCollided 0x00749270 (marked).
- insertJoint 0x0074a5d0 (98% -> 100).
- Through these: visitPrimitivesImpl<bind AppendFallen> 0x00747500, and the JointInstance Array resize/append
  0x00746940 / 0x00747010.

ContactManager:
- createContact 0x007b9120 (80% -> 100).
- The five pool operator news 0x007b7170 / 71f0 / 7270 / 72f0 / 7370, and the Allocator ctors 0x007b89e0 / 0x007b8ac0
  (TEMPLATE in Contact.h).
- BlockBlockContact ctor 0x00853750.
- Paired through these: the singleton_pool get_pool / malloc / free instances (0x007b5c80..0x007b6950), BallBall's D0
  0x007b8fb0, BallPoly's D0 0x00968490, and releaseMemory / Edge pieces in Contact.cpp (0x008524b0..0x008527f0).

Joints:
- Joint::isRigidJoint 0x0071b640 and Joint::isMotorJoint 0x0071b6c0. They are out-of-line COMDATs in 2013 and are now
  marked in Joint.h without moving its :106/:111/:183 asserts.
- Joint::getJointOwner 0x00501530 (FOLDED in PLAYER, STUDIO 0x009131f0).
- RigidJoint::getChildInParent 0x008ee7b0.
- TreeStage::sendClumpChangedMessage 0x008ebb60.

Other files:
- SpanningEdge::getChildSpanningNode 0x008ee900 and getParentSpanningNode 0x008ee9a0.
- Profiling::Mark::~Mark 0x00745150 (69% -> 100).
- SleepStage::changeContactState 0x008e8cc0 (97.5% -> 100).
- SimBody::updateSymmetricContactState 0x0082e000 (new; FFlag::SymmetricContact now exists). It is 93.7% in STUDIO
  because the Constants calls do not pair there yet, so it stays a STUB in STUDIO.
- Constants::impulseSolverSymStateTorqueBound / ForceBound 0x0084d430 / 0x0084d440 (paired once called).
- ContactManagerSpatialHash ctor 0x008ef090.

## Layouts proven

- **IJointOwner is an empty class in 2013 (no vftable).** JointInstance's CHD lists 18 bases, with IAdornable at mdisp
  0x60 and IJointOwner at mdisp 0x78. No `??_7JointInstance@@6BIJointOwner@` vftable exists. The JointInstance ctor
  0x007fccd0 passes `this+0x78` to setJointOwner 0x0081fae0, and that same offset is the first `weak_ptr<PartInstance>`
  member (the empty base shares its offset). Joint::jointOwner (+0x30) is an `IJointOwner*`. getJointOwner is out of
  line (0x00501530 `mov eax,[ecx+0x30]; ret`, folded). World::destroyAutoJoints casts back with `add eax,-0x78`.
- **JointInstance = Instance (0x60) + IAdornable (+0x60, 0x18 bytes) + IJointOwner (+0x78); joint at +0x88.** The
  ctor stores the IAdornable vftable 0x00b6c90c at +0x60, -1/-1/-1/0 at +0x64..+0x70, **a float 0 at +0x74**, and the
  joint at +0x88. PartInstance's ctor 0x007268d0 shows the same 2013 IAdornable shape: vptr at +0x74, then -1/-1/-1/0,
  then float 0.0 at +0x88. So the 2013 IAdornable is 0x18 bytes. Ours is 0x14 (GfxBase, Rendering's file).
- **Contacts: Edge 0x1c + Contact members to 0x2c.**
  - `numTouchCycles` is a 16-bit field at +0x24 (Contact ctor 0x008507e4 `mov word`; changeContactState
    `cmp word ptr [esi+0x24],3`).
  - BallBall, BallBlock and BlockBlock are 0x30, BallPoly 0xd0, PolyPoly 0xd4. These are the sizes createContact pushes
    to each `Allocator<T>::operator new`, and DECOMP_SIZE_ASSERTs in ContactManager.cpp now check them.
  - The concrete contacts derive `Allocator<T>`.
  - Block-poly pairs use PolyPolyContact.
- **Kernel is 0x30** (World ctor `push 0x30` at 0x00998e2d, then Kernel(IStage*) 0x00831590; SIZE + DECOMP_SIZE_ASSERT).
- **SpatialHash<Primitive,Contact,ContactManager,4> is 0xdc.** ContactManager's ctor news 0xdc for
  ContactManagerSpatialHash, which adds no members (SIZE on the template, DECOMP_SIZE_ASSERT on the subclass).
- **SimBody +0x118 symmetricContact, +0x119 verticalContact, +0x11c penetrationTorque, +0x128 penetrationForce**
  (updateSymmetricContactState).
- **Assembly +0x6c is the recursive pass id** (our `unknown104`) and +0x70 recursiveDepth (SleepStage::traverse
  0x008e9820). Not renamed yet.
- Body.cpp asserts: setMeInParent(CF) :234/:245 and setMeInParent(Link*) :251/:264, now on their lines. World.cpp:
  :682, :781 and :732 are on their lines (56 of 68 asserts).

## Patterns proven (rule, tell, function)

- **An empty CRTP `Allocator<T>` after a single polymorphic base shares its offset with the first member** (EBO
  applies). Tell: BallBallContact's ctor 0x007b8e40 calls the out-of-line `Allocator<BallBallContact>()` 0x007b89e0 with
  `ecx = this+0x2c`, then stores the connector at +0x2c; the size is 0x30. (Compare Assembly, where noncopyable is
  followed by another non-empty base and takes 4 bytes.)
- **`double s = e.seconds(); if (!flag) s = e.seconds() - c.seconds();` gives `fld; fld st(0)` and pops one copy on
  each arm** (`fstp st(0)` + `fsub` / `fstp st(1)`). The ternary gives no duplicate. A `Time now = Time::now<>()` local
  makes the now() call precede the `lea` of `&startTime` (Profiling::Mark::~Mark 0x00745150).
- **`int ok = set.insert(x).second; RBXASSERT(ok);` widens the bool**: `movzx eax, byte [eax+4]` is hoisted above the
  FLog gate, then `test eax,eax` (World::insertJoint 0x0074a5d0). A `bool` gives `mov al / test al,al`.
- **createContact reads both collide types into locals before the switch** (`t0`/`t1`, 2016 names). Calling
  `p1->getCollideType()` inside each inner switch reorders the calls and swaps esi/edi (0x007b9120).
- **VC9 keeps Joint's static type predicates out of line even though they are header-defined**: isRigidJoint and
  isMotorJoint (two virtual calls plus a compare chain) are called at 0x0071b640 / 0x0071b6c0. isKinematicJoint
  (a range test) is inlined in the same function (World::jointCoordsChanged).
- **An argument that is a call, held in a local, is evaluated before the vptr load.** For
  `otherConstNode(getConstChildSpanningNode())` written inline, VC9 loads `[esi]` into edi before the inner call. With
  `const SpanningNode* child = ...;` first, the vptr load follows the call (SpanningEdge::getParentSpanningNode
  0x008ee9a0, 72.7% -> 100).
- **2013 World::computeFallen compares to the literal -500.0f** (`comiss` against the .rdata float 0x00b537a8). There is
  no fallenPartDestroyHeight member yet. It walks the stage chain to SLEEP_STAGE (13) inline, through getSleepStage()
  const, and has no `RBXASSERT(sleepStage)`.
- **World::jointCoordsChanged in 2013** puts the kinematic assert (:781) before the parent/child test. It calls
  notifyMoved(parent) in both the rigid and the motor arm, and sendClumpChangedMessage(child) after either. The motor
  arm has no `RBXASSERT(parent)`.

## What blocks the rest

- **World::destroyAutoJoints 0x00749340 (97.7% PLAYER, 98.8% STUDIO).** It needs the 2013 IAdornable float at +0x14
  (Client/Rendering/GfxBase/include/GfxBase/IAdornable.h, not mine). Then IJointOwner moves from +0x74 to +0x78 and
  JointInstance::joint from +0x84 to +0x88. The remaining diffs are `add eax,-0x74` and `[ecx+0x84]`, and two calls that
  differ only by pairing: shared_from<JointInstance> folded with shared_from<GuiItem>, and Array::append.
- **Body::setMeInParent 0x008ed160 / 0x008ed250 (91.4% / 92.1%).** Only the makeStateDirty tail differs. 2013 loads the
  root into edx, interleaves the `pop edi/esi`, and the cold assert path uses eax. Ours puts the root into ebp. I did
  not find the source form (I tried nothing beyond `root->advanceStateIndex()`; next to try: `getRoot()->...` or an
  out-of-line advanceStateIndex call). Both stay STUB.
- **SleepStage::traverse 0x008e9820** is fully decoded (2013: the depth >= max arm demotes SLEEPING_DEEPLY to
  SLEEPING_CHECKING; pass id at Assembly+0x6c). It is blocked on Assembly::getAssemblyEdges 0x007b2500 (assert
  Assembly.cpp:331), which needs:
  - gatherPrimitiveExternalEdges 0x007b2180 (assert :311; our :311 is notifyMovedFromExternal, which must move below).
  - Primitive::getFirstEdge 0x007547a0 and getNextEdge 0x007547d0 out of line. 2013 checks joints (+0xbc) first through
    a call to EdgeList::getEdge and contacts inline. Primitive.cpp has no free lines between its asserts there.
- **SleepStage** stepSleepStage / wakeAssemblies / stepContacts / stepAssembliesAwake / stepAssembliesSleepingChecking
  (:168-:486) and **World** uiStep / doWorldStep / step (:514-:625) are not written. They are large (688-1685 bytes
  each).
- **ContactManager::createContact in STUDIO (97.1%)**: the STUDIO addresses of the allocator instances and the
  BallBall/BallBlock ctors are not in the body-match CSV, so they have PLAYER markers only.
- **BallBall/BallBlock/PolyPoly ctors (85.7% / 85.7% / 71.9%)**: 2013 BallBall/BallBlock call an out-of-line
  Contact(p0,p1) 0x008507c0, which our inline Contact ctor does not reproduce. PolyPoly and BallPoly derive a
  PolyContact base in 2013 (ctor 0x00967e20).
- Joint() 0x00821410 (98.5%): unchanged, pairing only (Matrix3::operator== against its ICF twin GLight::operator==).

## Outside my directories (notes for the owners)

- **Client/Rendering/GfxBase/include/GfxBase/IAdornable.h (second contributor):** the 2013 IAdornable is 0x18. After
  bucket it has a float initialised to 0.0f: JointInstance ctor 0x007fcd0f `movss [esi+0x74], xmm0`; PartInstance ctor
  0x007269a7 `movss [esi+0x88], xmm0`, with the IAdornable vptr at +0x74. Adding it gives JointInstance its 2013
  IJointOwner offset and fixes World::destroyAutoJoints.
- **Client/App/include/V8DataModel/JointInstance.h:** I edited this file because task item 1 needs it. It now derives
  `IAdornable` and `IJointOwner` after DescribedNonCreatable and includes V8World/Joint.h and GfxBase/IAdornable.h.
  This makes the JointInstance-family vftables pair (now 75%; slot +0x20 is JointInstance::render3dAdorn 0x007fbb30,
  not written). The datamodel owner may want to take it over.
- **Integrator:** regenerate the shared reccmp/ CSVs. New markers:
  - TEMPLATE rows in Contact.h.
  - World.cpp's new functions; Joint.h/Joint.cpp; Body.cpp; SimBody.cpp.
  - SpanningEdge.cpp; RigidJoint.cpp; TreeStage.cpp; ContactManagerSpatialHash.cpp.
- **Any agent with an older build dir:** `-DOGRE_BOOST_PO_GL=ON` (see the top of this report).

## Files changed (mine)

- include/V8World:
  - Joint.h: IJointOwner; out-of-line getJointOwner; isRigidJoint/isMotorJoint markers. Line count kept.
  - Contact.h: Allocator bases; 16-bit numTouchCycles; ctor markers; TEMPLATE rows.
  - World.h: notifyMoved / jointCoordsChanged / sendClumpChangedMessage / computeFallen.
  - SleepStage.h: getAwakeAssemblies.
  - SpatialHashMultiRes.h: SIZE.
- include/V8Kernel:
  - Kernel.h: SIZE.
  - Body.h: setMeInParent out of line.
  - SimBody.h: comment.
- v8world:
  - World.cpp, ContactManager.cpp, ContactManagerSpatialHash.cpp, Joint.cpp, TreeStage.cpp (marker), RigidJoint.cpp
    (marker).
- v8kernel: Body.cpp, Kernel.cpp (size assert), SimBody.cpp.
- util/Profiling.cpp (~Mark). util/SpanningEdge.cpp (two markers, a local).
- Outside the list: V8DataModel/JointInstance.h (see above).

## Checks

- check_2016_copies is silent and reccmp_addr_padding reports 0 bad.
- assert_lines: no touched file lost a line, and World.cpp gained :682/:781. Body.cpp :234/:245/:251/:264 and
  SimBody.cpp :54-:58 are in place.
- decomplint on v8world/v8kernel/include V8*: only function_out_of_order (assert-pinned descents) plus the
  pre-existing duplicate_offset 0x0096bde0 in Joint.h/CleanStage.h.
- Both targets build from the final source: PLAYER build-wld b8, STUDIO build-wld-studio bs2.
