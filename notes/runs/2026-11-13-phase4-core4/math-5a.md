# math-5a (workflow phase4-core4, wf_cf7a5075-c0f)

PHASE 5a (math and G3D): measured, fixed and scored in build-math/ (PLAYER) and build-math-studio/ (STUDIO).

**Row 5a coverage (PLAYER, tools/row_coverage.py, one 2013 .obj range per file, same ranges before and after)**
- Before: 53/154 functions at effective 100% (34.4%), from build/sweep5/P-final.json.
- After: 106/154 (68.8%), from build-math/runs/P5.json scored with the repo CSVs as they are.
- With the CSV fixes listed below (tested in a scratch copy of the reccmp project, not in reccmp/): 121/154 (78.6%), +31 PLAYER entries and 0 lost (scratchpad/rbuild/P4y.json).
- Per file after: Math.obj 67/81, Extents 7/10, Face 8/10, NormalId 5/11, IndexedMesh 10/15, Region3 4/5, InterpolatedCFrame 2/18, Frustum 0/1 (98.4%, pairing only).

**Whole build**
- PLAYER: Implemented 4,116 → 4,154; effective-100 entries 3,387 → 3,500. This includes other agents' concurrent changes.
- STUDIO: 3,989 → 4,023; 3,263 → 3,382.
- Lost against the baseline: PLAYER 9, STUDIO 2. I read every one with its diff, and none comes from a code change in my files:
  - 7 are Lua and RakNet immediates that now coincide with our RTTI addresses, the no-.reloc problem (e.g. luaV_execute 0x0098ad00, `test edi,<OFFSET>` vs `??_R0...`).
  - RakNet::RecvFromLoop 0x005fdbb0 and LocklessUint32_t::Decrement 0x00608150 lost their pairing. Regenerating the CSVs restores both (they appear as gains in the scratch run).

**Added or raised to 100% (PLAYER, STUDIO = PLAYER+0x13f180 for Math.obj)**
- Math.cpp, 40 functions (35 new, 5 raised):
  - New: sumDeltaAxis 0x00756e20, mulMatrixDiagVector 0x00756f30, mulMatrixMatrixTranspose 0x00756fc0, lessThan 0x007570c0, getIBodyAtPoint 0x00757240, toDiagonal 0x007573b0, isAxisAligned 0x007573d0, getOrientId 0x007574a0, rotateAboutZ 0x00757600, angle 0x00757b20, isOrthonormal 0x00757c00, fuzzyEq(Matrix4) 0x00757d10, fuzzyEq(CFrame) 0x00757da0, rotateAboutYGlobal ×2 (0x00757df0, 0x00757e40), vector3Abs 0x00757ff0, get/setHeadingElevation (0x00758010, 0x00758060), intersectLinePlane 0x00758500, polygonStartingPoint 0x00758740, evenWholeNumberFuzzy 0x00758830, getFocusSpace 0x007588b0, toGrid ×2 (0x00758b20, 0x00758bc0), snapToGrid ×2 (0x00758c60, 0x00758d40), fuzzyAxisAligned 0x00758e20, orthonormalizeIfNecessary 0x00758f60, isDenormal 0x00759a40, isNanInf 0x00759aa0, isNanInfDenormVector3 0x00759b40, hasNanOrInf(Matrix3) 0x00759be0, fixDenorm 0x00759c40, fromDirectionCosines 0x00759e40, fromVectorToVectorRotation 0x0075a460.
  - Raised: iRoundVector3 0x00757ab0 (24.6%), snapToAxes 0x00757690 (74%), isNanInfVector3 0x00759ad0 (15%), fromRotationAxisAndAngle 0x00759ca0 (95.6%), hasNanOrInf(CFrame) 0x0075a410 (29.6%).
- Extents: closestFace 0x00795fb0, faceCenter 0x007967f0.
- Face: toObjectSpace 0x008fc5f0.
- NormalId: validNormalId 0x008c6ca0, normalIdOpposite 0x008c6cb0, Matrix3ToNormalId 0x008c7740.
- IndexedMesh:
  - Written from the 2013 bytes: setLower 0x008f9490 (placed so its assert lands on line 78), setUpper 0x008f9590, getConstIndexedMeshParent 0x008f9300.
  - Marked: setComputedUpper 0x008f93a0, attachChildren 0x008f93f0, severeChildren 0x008f9440, and the inline lowersChanged copy 0x008f9130 (marker in IndexedMesh.h).
- G3D: png_sig_cmp 0x009e96c0 written from the bytes in shim/png_absent.cpp; it is 100% once paired (scratch regeneration).
- Markers were flipped to FUNCTION only at effective 100% in the target's own JSON: build-math/runs/P5.json and build-math-studio/runs/S2.json. Asserts stay on their 2013 lines (tools/assert_lines.py reports 0 deltas in Math, NormalId, Face, Extents and IndexedMesh). check_folded reports 0 problems for both targets, and the address-padding check passes.

**Float and codegen patterns proven (for CLAUDE.md "Codegen Patterns")**
1. **G3D::iRound is upstream's inline-asm lrintf, not the fork's ±0.5 plus cvttss2si.** The tell is `movss [esp+N]; fld dword; fistp dword`. Proof: Math::iRoundVector3 0x00757ab0 went from 24.6% to 100%. This is G3D patch 0011. App has 35 `fistp dword [esp+d8]` sites.
2. **Math's NaN/denormal tests are boost::math::fpclassify<float> and isfinite<float>** (integer tests on 0x7F800000/0x7FFFFF, FP_SUBNORMAL=4), not hand-written bit tests or _fpclass. Proof: isDenormal 0x00759a40 and isNanInf 0x00759aa0 went from 9–15% to 100%. fixDenorm(float&) does not exist in 2013 (no iOS symbol, no PLAYER body).
3. **VC9 /O2 inlines a function defined later in the same TU, never one from another TU.** The prior tree's OrientId.cpp split blocked inlining; with orthonormalizeIfNecessary moved to the end of Math.cpp, fromRotationAxisAndAngle 0x00759ca0 and fromDirectionCosines 0x00759e40 reach 100%. So the 2013 TU membership matters.
4. **G3D 2013 has upstream reference counting.** MemoryManager is a ReferenceCountedObject and MemoryManager::Ref is a ReferenceCountedPointer, so Array's constructor is called out of line (0x008e6db0, taken by value, released with InterlockedDecrement). Proof: Frustum ctor 0x009ee010 went from 97.3% to 98.4% and is now pairing-only. This is G3D patch 0012.
5. **Recursive inline helpers are inlined one level, and the inner call goes to the out-of-line copy.** Proof: IndexedMesh::setUpper 0x008f9590 and setLower 0x008f9490 at 100%. Not yet reproduced: 2013 compiles the out-of-line tail self-recursion as a loop (0x008f9080), ours as a `jmp`.
6. **Function order inside one .obj is not source order.** In IndexedMesh.obj, isUpperRoot (line 219) at 0x008f90b0 comes before ~IndexedMesh (lines 19–23) at 0x008f9160, and setLower (line 78) at 0x008f9490 comes after getConstIndexedMeshParent (line 173) at 0x008f9300. In Math.obj, matrixTiltQuadrant (line 1272) at 0x00758320 comes before rotationToByteBase (lines 368–375) at 0x00758940. So decomplint function_out_of_order cannot be cleared in such files without breaking __LINE__; my 5 files account for 28 of those reports.
7. **The 2016 degenerate-case early returns in snapToAxes are absent in 2013.** Proof: 0x00757690 went from 74% to 100%.
8. **/Oi inlines memcmp** as a dword loop plus a byte tail (png_sig_cmp 0x009e96c0).
9. **A double literal compare (`d > 1e-5`) stays x87 `fld qword` under /arch:SSE** (closestPointOnRay 0x00758ff0).

**CSV fixes for the Integrate agent (measured in the scratch project: +31 PLAYER entries, 0 lost)**
- (a) **libdata float rows with a blank size get size 4:** PLAYER 00be6a48 (0.0f) and 00be6ab0 (2.0f), STUDIO 00d2e9c4 and 00d2eab0.
  - Cause: our linker ICF-folds `__real@` with a named constant, so the generator finds no `__real@` name and writes no size.
  - This alone unblocks 17 PLAYER functions, including radiansToQuadrant 0x00758430, intersectRayPlane 0x00759190, intersectRayConvexPolygon 0x0075a070, Face::fuzzyContainsInExtrusion 0x008fc120, Face::cornersAligned 0x008fc240, Extents::separatedByMoreThan 0x00796580, SimBody::~SimBody, and 8 G3D functions (Matrix3(Quat), CoordinateFrame::lerp, Box::init, Line::intersection, LineSegment::closestPoint, Color4::operator/, collisionLocationForMovingPointFixedAABox, Array<string>::resize).
  - These are the six pairing-only functions from the integrate report.
- (b) **function-size rows should extend over a trailing in-function jump table:** 204 PLAYER and 224 STUDIO rows.
  - Examples: 00758320 245→264, 00758570 431→456, 00796190 969→996, 008c6cd0 109→136, 008c7290 489→516.
  - These unblock matrixTiltQuadrant, getAxisRotationMatrix, getFaceCorners, normalIdToMask, normalIdToU, uvwToObject, objectToUvw, normalIdToMatrix3, Axes::getAxisByNormalId and AsyncHttpQueue::dispatchGenericCallback.
- (c) **G3D rows:**
  - 009dae50 `??0ReferenceCountedObject@G3D@@IAE@XZ`
  - 009dae70 `?ReferenceCountedObject_zeroWeakPointers@ReferenceCountedObject@G3D@@QAEXXZ`
  - 0049be40 `?setPointer@?$ReferenceCountedPointer@VMemoryManager@G3D@@@G3D@@AAEXPAVMemoryManager@2@@Z`
  - 009daf10 `?create@MemoryManager@G3D@@SA?AV?$ReferenceCountedPointer@VMemoryManager@G3D@@@2@XZ`
  - vtables 00bbd3ec (ReferenceCountedObject, 1 slot) and 00bbd3f4 (MemoryManager, 4 slots)
  - png_sig_cmp 009e96c0 (the generator pairs it; a hand row `_png_sig_cmp` did not)
  - zlib inflate.c's static `order` table at PLAYER 00bbff50 / STUDIO 00be3ca0 (inflate 0x009e22e0 is 99.9%, held back only by this)
- (d) **A regeneration that pools my JSONs pairs the rest:** radWrap 0x006b3a60, intToNormalId copy 0x004a20c0, ClosestSqDistanceToAABB 0x0051ac90, the matrixRotateX / segSizeRadians statics, and floats 0x00b57f70, 0x00b582e8, 0x00b5837c (16 functions). Pool 2–3 runs: my 2-run scratch regeneration churned twin pairings, e.g. IndexArray::operator[] 0x0082e750.

## Files changed

- Client/App/util/Math.cpp
- Client/App/util/OrientId.cpp (deleted; bodies moved into Math.cpp, copy at /tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/bak/OrientId.after1.cpp)
- Client/App/util/Extents.cpp
- Client/App/util/Face.cpp
- Client/App/util/NormalId.cpp
- Client/App/util/IndexedMesh.cpp
- Client/App/include/Util/IndexedMesh.h
- Client/Rendering/g3d/include/G3D/g3dmath.h
- Client/Rendering/g3d/include/G3D/ReferenceCount.h
- Client/Rendering/g3d/include/G3D/MemoryManager.h
- Client/Rendering/g3d/G3Dcpp/MemoryManager.cpp
- Client/Rendering/g3d/G3Dcpp/ReferenceCount.cpp
- Client/Rendering/g3d/shim/png_absent.cpp
- Client/Rendering/g3d/shim/png.h
- staging/g3d-8.00/G3D/patches/0011-g3dmath-upstream-lrint-iround.patch
- staging/g3d-8.00/G3D/patches/0012-referencecount-and-memorymanager-ref-upstream.patch
- tools/row_coverage.py
- build-math/runs/P5.json
- build-math-studio/runs/S2.json

## Unresolved

- CSV fixes above (a: 4 float sizes, b: 204/224 jump-table sizes, c: G3D/zlib rows, d: regeneration from build-math/runs/P1,P2,P4,P5.json) are for the Integrate agent; tested only in a scratch copy of the reccmp project (scratchpad/rproj, rbuild/P4y.json).
- tools/ida/gen_reccmp_csv.py: (1) give a float row size 4 when the ICF-folded name is not __real@; (2) extend function sizes over trailing in-function jump tables (4-aligned dwords pointing into the function after the IDA end).
- InterpolatedCFrame (16 of 18 functions, 0x008dbc40-0x008dc8c0): the 2013 class keeps history in boost::circular_buffer<FrameInfo 0x40> (push_back 0x008dc3e0, uninitialized copy 0x008dc480, erase 0x008dc500, buffer reset 0x008dc610). The prior tree uses a fixed ring from a Mac-guessed layout. PLAYER and STUDIO bodies differ (different source revisions). Needs a rewrite from the 2013 bytes.
- IndexedMesh: computeConstUpper 0x008f9080 is 51.9% (2013 turns the tail self-recursion into a loop, ours emits jmp). isUpperRoot 0x008f90b0 is 72.3% (2013 calls 0x008f9080 out of line, ours inlines one level). onParentChanged 0x008f9600 is 43.9% (2013 inlines lowersChanged one level, ours calls it; the simpler inline form of lowersChanged in IndexedMesh.h did not change this). The ctor 0x008f9050 is pairing-only (IndexedTree ctor 0x00999950).
- G3D alwaysAssertM is live in 2013 (Array::realloc's 'Memory manager returned NULL' at Array.h:158, _failureHook at 0x00db54b4, _releaseInputGrab_ 0x009d62e0, 11 App call sites). The fork defines it away. Restoring it needs upstream debugAssert.h/.cpp plus G3D Log and FileSystem (upstream Log.cpp calls FileSystem::fopen; PLAYER has Table<string,FileSystem::Dir>). That is the 101-function unpaired G3D cluster 0x009d5b70-0x009de560, and it also blocks Array<string>::realloc 0x009d73e0 (32.7%).
- G3D still not identical: LightingParameters::setTime 0x009e3b70 (47%), BinaryInput ctor 0x009dbc50, findRayCapsuleIntersection 0x009dffe0, DialogTemplate ctor 0x009e5870 (reccmp labels the 2-byte "\0" literal "" vs "\x00", so a string-row issue), and Color4 zero-static at 0x009e3820 (unplaced).
- Upstream AlignedMemoryManager/CRTMemoryManager are left out of patch 0012 because the fork's System lacks alignedMalloc/alignedFree, although PLAYER has System::alignedMalloc at 0x009d6de0.
- Quaternion, Units, PV, Velocity, Region3int16 and Vector3int16 have iOS symbols but no PLAYER body was found (inlined on Windows). Region3 0x007fa6b0 is Reflection::TType<Region3> (reflection code, not math).
- decomplint: 28 function_out_of_order reports in Math/Extents/Face/NormalId/IndexedMesh are inherent (see pattern 6); fixing them would move asserts off their 2013 lines.
- tools/lib/install_libs.py's docstring still says patches 0001-0010; it applies staging/g3d-8.00/G3D/patches/*.patch by glob, so 0011 and 0012 are picked up. Patch 0012 was checked to reproduce the installed files.
- STUDIO markers for Math.obj use PLAYER+0x13f180, a uniform delta verified for every function in 0x00756db0-0x0075a460. IndexedMesh isUpperRoot STUDIO 0x00a0e170 is not an IDA function in STUDIO-functions.csv (same bytes as PLAYER), so it needs a function-size row.
