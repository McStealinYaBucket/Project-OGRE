# vtables-3 (workflow phase4-core3, wf_4f741962-18b)

Phase 4 vftable pass for PartInstance/PVInstance/ModelInstance, EnumDesc<T>, Edge/EdgeBuffer/AssemblyStage. Builds: build-vt (PLAYER), build-vt-studio (STUDIO). Every heavy command ran through tools/capped.sh. None hit the cap. Snapshots of the scored exe/pdb/map and JSONs: scratchpad r4/ (PLAYER) and s2/ (STUDIO).

FINAL COUNTS (vftable rows at 100% / rows compared; I added 12 VTABLE markers per target)
- PLAYER, shared CSVs: 180/351 (baseline 174/339). reccmp-vtable: 171 not matching, 35 structural / 136 pairing-only. Implemented 3931/40468. Functions at 100% (my count from the JSON): 3060, of which 1180 non-library (baseline P3.json, same counting: 2971 / 1106).
- PLAYER with a private CSV copy (3 stale rows removed, 3 EnumDesc symbol rows added, see unresolved): 189/351, 162 not matching (27 structural), 3065 functions at 100%.
- STUDIO, shared CSVs: 168/295 (baseline 163/283). 127 not matching, 14 structural / 113 pairing-only. Implemented 3800/42329. 2957 functions at 100%, 1142 non-library (baseline 2869 / 1066).
- STUDIO with the private CSV: 170/295, 125 not matching.

LOSSES
- AssemblyStage vftable (PLAYER 0x00b94b28 / STUDIO 0x00c67d48) reads 1.0 -> 0.833 with the current CSVs.
  - Cause: the stale row player-libdata 008ea3c0 -> ??_EEdgeBuffer, plus the D0 rows for 008ea3c0/008ea600 in player-symbols (STUDIO 00a085d0/00a08810).
  - With those rows removed, both deleting destructors and both vftables (AssemblyStage, EdgeBuffer) are 100% in both targets.
  - 0x008ea3c0 was 95.65% before my change (never 100%).
- Functions that left 100% are not from my files.
  - PLAYER 0x006eda50 shared_ptr<Instance>::operator= is now unpaired. Its CSV row names ??4?$shared_ptr@VRelativePanel@..., which is defined in GuiBuilder.cpp (another agent's file) and no longer exists in our build.
  - 0x00567ca0, 0x0067aea0 and 0x0088cce0 call it, and the diff is only that call.
  - STUDIO: the same chain at 0x00751110, 0x005cb7e0, 0x007e2dc0.
  - STUDIO Name::Name 0x00702f90 dropped once because of a compile race with another agent's Name.cpp edit. It came back to 100% after recompiling.
- check_folded PLAYER: 0 problems. reccmp_addr_padding: 0 badly padded markers.

1. PARTINSTANCE / PVINSTANCE / MODELINSTANCE
- Primary vftable 0x00b4c784: now 64/64 slots with no structural slot (PLAYER 0.725 -> 0.797, STUDIO 0.768 -> 0.844). The remaining slots are identical-byte twins.
  - getSurface (+0xd4) has a STUB body (P 0x0071c220 / S 0x007b7c80) and a minimal 2013 Surface (V8DataModel/Surface.h, Surface.cpp; ctor 0x009b1a20).
- Secondary bases modelled from the RTTI base list and the ctor 0x007268d0 vptr stores at +0x60/+0x68/+0x74/+0x8c/+0xa0:
  - IHasPrimaryPart (new, in PVInstance.h), with virtual IHasLocation (vtordisp thunk 0x00723590).
  - IAdornable, CameraSubject (10 slots in iOS order) and Selectable.
  - New headers: Util/IHasLocation.h, Util/CameraSubject.h, Util/Selectable.h.
- New PartInstance vftables, PLAYER (STUDIO is the same):
  - 1.0: IHasPrimaryPart 0x00b4c754, IMoving 0x00b4c738, Selectable 0x00b4c6d0, IHasLocation 0x00b4c6c8.
  - IAdornable 0x00b4c704: 0.833, twins only.
  - CameraSubject 0x00b4c6d8: 0.7 (the destructor adjustor thunk plus `ret` / `ret 8` twins).
- STUB markers added for:
  - the IMoving overrides: 0x00726c10, 0x007228d0, 0x0071bee0, 0x00722a80, 0x0071aff0;
  - the CameraSubject/IAdornable overrides: 0x007235a0, 0x00723560, 0x00720ae0, 0x007235c0, 0x0071d570, 0x0071ed40, 0x00720340;
  - getPrimaryPart 0x00723500 and getLocation 0x00723520.
- New at 100%:
  - getPrimaryPart 0x00723500 (STUDIO 0x0079f430);
  - the getLocation vtordisp thunks 0x00723590 / 0x006d2960;
  - CameraSubject::stepLocationAndFocus, FUNCTION at PLAYER 0x006d2920 and STUDIO 0x00787c10.
- ModelInstance primary vftable 0x00b3f6bc, now compared: 0.424 -> 0.667 (STUDIO 0.758).
  - I added STUB overrides setName, getPersistentDataCost, onDescendantAdded/Removing, onChildAdded/Removing, onChildChanged, plus askSetParent (the same body, ICF-shared at 0x006de360). All 33 slots now line up; the rest are twins.
  - Secondary: IHasPrimaryPart, IHasLocation and Selectable at 1.0; IAdornable 0.833; CameraSubject 0.7.

2. ENUMDESC
- The bodies at the two slots are ICF-shared:
  - +0x10 = convertToString(size_t, string&) 0x00772590, which calls convertToString(const Enum&) 0x006c8c60;
  - +0x08 = lookup(const char*) 0x00773130.
- EnumConverter.h is now laid out on its 2013 assert lines. I added the 2013 RBXASSERT((int)value<=2304) in addPair 0x008217f0.
  - addPair 210/211, convertToString 262/263, convertToItem 273/274, convertToIndex 350.
- STUDIO: convertToString(const Enum&) 0x00719270 went 0.952 -> 1.0 (now FUNCTION). All 8 STUDIO EnumDesc vftables outside InputObject are 100%.
- PLAYER: 0x00772590, 0x006c8c60 and 0x007761a0 measure 100% once paired. With the 3 CSV rows, 7 EnumDesc vftables go 0.8 -> 1.0: CreatorType, Genre, GearGenreSetting, GearType, SoundType, Material, ChatColor.

3. EDGEBUFFER / EDGE
- User-declared ~AssemblyStage: the 2013 deleting destructor 0x008ea3c0 stores the vptr, so it does not fold with EdgeBuffer's 0x008ea600.
  - SYNTHETIC markers for both, PLAYER and STUDIO. Result as under Losses (needs the CSV regeneration).
- Edge is 0x1c in 2013, not 0x20: Sim::EdgeState and ThrottleType are `enum : unsigned char`.
  - Evidence: SleepStage 0x008e7e00 reads byte [edge+8] and byte [edge+9]==3; the Edge dtor 0x0081f840 finds prim0..index1 at +0x0c..+0x18; the original Joint SpanningEdge thunk 0x008eaa40 is `sub ecx,0x1c`.
  - Our thunk now adjusts by 0x1c as well.
  - Gains in both targets: AnchorJoint ctor 0x008ea6a0 and FreeJoint ctor 0x008ea6e0 (0.95 -> 1.0), BallBallContact::numConnectors 0x007b9050 and getConnector 0x00850a40.
  - The Joint/AnchorJoint/FreeJoint/MultiJoint SpanningEdge vftables are now pairing-only.

4. PAIRING-ONLY (PLAYER 136)
- 73 vftables mismatch only on deleting-destructor adjustor thunks (`??_E...W<n>@`).
  - The original `sub ecx,N; jmp` thunks are filtered out of player-function-size.csv, so no SYNTHETIC marker can pair them. The generator would need thunk rows that pair through their already-paired targets.
  - The only unpaired primary deleting destructors in my files were EdgeBuffer and AssemblyStage (markers added).
- 5 vftables are purecall plus thunk.
- 58 are twin bodies the generator pairs to a different original address. Main sources, as original address (slot count, our pairing):
  - 0x004027c0 (102 slots; ours pairs it to 0x00575d00)
  - 0x0070c430 (64; -> 0x00475080)
  - 0x00528720 (23), 0x00927cc0 (23), 0x00402730 (19), 0x0096bde0 (18), 0x0040b950 (15), 0x00402740 (7), 0x005b8900 (6) — unpaired
  - 0x0044e330 (3; -> 0x00962a50)
- The sp_counted_impl dtor 0x005830c0 vs 0x00435970 is byte-identical. tools/vtable_structural.py counts it as structural only because its body() read crosses a single int3 into the next function.

## Files changed

- Client/App/include/reflection/EnumConverter.h
- Client/App/include/V8DataModel/PartInstance.h
- Client/App/v8datamodel/PartInstance.cpp
- Client/App/include/V8DataModel/PVInstance.h
- Client/App/include/V8DataModel/ModelInstance.h
- Client/App/v8datamodel/ModelInstance.cpp
- Client/App/include/V8DataModel/Surface.h
- Client/App/v8datamodel/Surface.cpp
- Client/App/include/Util/IHasLocation.h
- Client/App/include/Util/CameraSubject.h
- Client/App/include/Util/Selectable.h
- Client/App/include/V8World/AssemblyStage.h
- Client/App/v8world/AssemblyStage.cpp
- Client/App/include/V8World/EdgeBuffer.h
- Client/App/include/V8World/Edge.h
- Client/App/include/V8World/Enum.h

## Unresolved

- Integrate (CSV regeneration): delete the stale rows player-libdata `008ea3c0,,,??_EEdgeBuffer@RBX@@UAEPAXI@Z`, studio-libdata `00a085d0,,,??_EEdgeBuffer...`, and the D0 name rows for 008ea3c0/008ea600 (player-symbols) and 00a085d0/00a08810 (studio-symbols). The SYNTHETIC markers then pair both deleting destructors at 100%, and the AssemblyStage and EdgeBuffer vftables reach 100% in both targets (measured with a private CSV copy).
- Integrate: add PLAYER libdata rows `00772590 -> ?convertToString@?$EnumDesc@W4ChatColor@ChatService@RBX@@@Reflection@RBX@@UBE_NIAAV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z`, `006c8c60 -> ?convertToString@?$EnumDesc@W4ChatColor@ChatService@RBX@@@Reflection@RBX@@QBE?AV?$basic_string@...@ABW4ChatColor@ChatService@3@@Z`, and `007761a0 -> ?convertToItem@?$EnumDesc@W4Material@RBX@@@Reflection@RBX@@QBEPBVItem@EnumDescriptor@23@ABW4Material@3@@Z`. These are the alias reccmp keeps as the recomp symbol at each folded address; it can change after a relink. STUDIO already has the equivalent rows. With them, 7 PLAYER EnumDesc vftables reach 100%.
- Client/App/v8datamodel/InputObject.cpp:16 (not my file) includes `Reflection/EnumConverter.h`; the 2013 strings show `reflection/enumconverter.h`. Because of the different __FILE__ string, our InputObject EnumDesc copies do not fold, so the template-line markers in EnumConverter.h are ambiguous to reccmp ('Debug data out of sync' at :128/:163/:260/:271/:335). It also leaves the InputObject EnumDesc vftables (PLAYER 0x00b5c5a4/0x00b5c5bc, STUDIO 0x00befe50/0x00befe68) at 0.6. LuaArguments.h and LuaEnum.h use the same spelling.
- EnumDesc::lookup(const char*) 0x00773130 (51%): in 2013, Name::lookup takes `const char* const&` (0x00613be0 dereferences the argument; iOS has `RBX::Name::lookup(char const* const&)`). Our Util/Name.h declares `lookup(const char*)`. Not my file.
- IAdornable (GfxBase, not my file) has no 2013 `float depth`. The 2013 subobject is 0x18 (the PartInstance ctor stores a float at +0x88), so our CameraSubject and Selectable sit 4 bytes low. The PartInstance and ModelInstance members after the bases are still the Mac layout (2013 PartInstance: primitive at +0xb8, IHasLocation at +0x15c, size 0x160). Diagnostics::Countable<PartInstance> and the Intrusive Hook base at +0xa4 are not modelled.
- STUDIO-only ModelInstance::isSelectable3d override 0x00817390 (checks a flag, then getPrimaryPart; PLAYER uses the default 0x006be550). The tree has no per-target switch for App code, so the STUDIO ModelInstance Selectable vftable 0x00c14f64 stays at 0.0.
- Losses that are not from my changes: PLAYER 0x006eda50, 0x00567ca0, 0x0067aea0, 0x0088cce0 and STUDIO 0x00751110, 0x005cb7e0, 0x007e2dc0. They all depend on the CSV row pairing shared_ptr<Instance>::operator= through `??4?$shared_ptr@VRelativePanel@RBX@@...`, which GuiBuilder.cpp (another agent's file) no longer emits.
- Pairing-only vftables need generator work, not headers: (a) adjustor-thunk rows for the `??_E...W<n>@` deleting-destructor thunks, about 73 PLAYER vftables; (b) twin pairing that follows vftable slots, for example original 0x004027c0 (102 slots) and 0x0070c430 (64). tools/vtable_structural.py also misclassifies byte-identical twins whose bodies are separated by a single int3 (sp_counted_impl dtor 0x005830c0 vs 0x00435970).
- New STUB bodies still need writing: PartInstance getSurface, shouldRender3dAdorn, render3dAdorn, render3dSelect, getCameraIgnorePrimitives; all the ModelInstance overrides (getPrimaryPart, getLocation, getRenderSize, onCameraNear, getCameraIgnorePrimitives, shouldRender3dAdorn, render3dAdorn, render3dSelect, setName, getPersistentDataCost, onDescendantAdded/Removing, onChildAdded/Removing, onChildChanged). ModelInstance::getRenderLocation has a real body (`return getLocation();`) and scores 75% (STUB).
- PartInstance.cpp markers are still not in ascending order, which is pre-existing; my new override bodies sit at the end of the file. ModelInstance.cpp is now ascending.
