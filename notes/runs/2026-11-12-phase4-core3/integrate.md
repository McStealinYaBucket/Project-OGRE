# integrate (workflow phase4-core3, wf_4f741962-18b)

All three steps are done, and no function or vftable that was at 100% in the baseline lost it (checked against build/sweep4/P3.json and build-studio/sweep4/S3.json). Every heavy command ran through tools/capped.sh with the default 8G cap, and none hit it. I ran one build at a time with the job pool.

**Step 1: clean rebuild, CSV regeneration, sweep**
- I rebuilt both targets from clean. tools/gen_link_stubs.py added a PLAYER stub for registerNewLightAPI 0x0075ed40. STUDIO has 0 unresolved.
- I regenerated the CSVs, scored, and repeated until nothing more was gained: PLAYER P1→P8 and STUDIO S1→S8, pooling the last three runs each time. I then flipped every marker to FUNCTION, rebuilt, and ran two or three more rounds (PF1–PF3, SF1–SF3) before applying the sweep.
- I made three changes to tools/ida/gen_reccmp_csv.py (its selftest passes):
  - **vt_slot_refs:** every vftable a --json run paired now gives one vote per slot to the function in that slot.
  - **Unpairable markers:** a non-STUB marker that reccmp cannot pair (the run's .txt log says 'Debug data out of sync') no longer blocks a position row at its address.
  - **Oracle labels on marker addresses:** an oracle name with no symbol is no longer written at an address a marker owns. Such a name hid the nameref of SYNTHETIC markers (player-symbols.csv went from 6,438 to 5,381 rows).
- What these fixed:
  - **Stale rows:** the stale shared_ptr row (RelativePanel symbol) and EdgeBuffer row that vtables-3 reported are gone.
  - **shared_ptr chain:** shared_ptr<Instance>::operator= (PLAYER 0x006eda50, STUDIO 0x00751110) is back at 100%, along with its callers 0x00567ca0, 0x0067aea0, 0x0088cce0 and STUDIO 0x005cb7e0, 0x007e2dc0.
  - **AssemblyStage:** its vftable (0x00b94b28 / 0x00c67d48) and both deleting destructors (0x008ea3c0 / 0x008ea600; STUDIO 0x00a085d0 / 0x00a08810) are at 100%.
  - **EnumDesc:** 7 PLAYER EnumDesc vftables are at 100% (CreatorType, Genre, GearGenreSetting, GearType, SoundType, Material, ChatColor). EnumDesc 0x00772590 and 0x006c8c60 are now paired and at 100%.
  - **GuiBuilder:** the GuiBuilder ctor is at 100% in both targets (0x00644f30 / 0x00761ef0).
- The sweep was `app_sweep set FUNCTION`, rebuild, score, then `apply PF3 SF3`, then rebuild and rescore. The rescore after apply (PA/SA) matched PF3/SF3 exactly: 0 lost, 0 gained.
- I added `// FUNCTION: PLAYER 0x005c30c0 FOLDED` / `STUDIO 0x008a8950 FOLDED` for GuiBuilder::Initialize in GuiBuilder.cpp. They replace two comment lines, so the line count is unchanged. check_folded: 0 problems; scores unchanged.

**Step 2: function order (new tool tools/reorder_markers.py, with plan / apply / --strict / selftest)**
- How it works:
  - It moves whole marked definitions, with the comments directly above them, only within runs separated by blanks, comments or complete unmarked items. A run never crosses preprocessor lines, pragmas or REFLECTION_BEGIN/END.
  - It sorts by PLAYER address and refuses a run when STUDIO would get more descents.
  - It refuses whenever a chunk containing an assert or any other user of `__LINE__` would change line.
  - It reads and writes CRLF files unchanged.
- I reordered 28 files.
  - Four of them would not compile under the loose run rule (type.cpp C2908, LuaAtomicClasses, LuaSignal, LuaInstanceBridge). I re-applied those with --strict, which kept the compile-safe part for LuaAtomicClasses (25→21 descents) and LuaSignal (6→4). type.cpp and LuaInstanceBridge are unchanged.
  - After the rebuild, every PLAYER and STUDIO score was identical to before (0 entries differ).
- decomplint function_out_of_order went from 328 to 254 (PLAYER) and from 334 to 267 (STUDIO).

**Step 3: final figures** (build/sweep5/P-final.json, build-studio/sweep5/S-final.json)

| | PLAYER | STUDIO |
|---|---|---|
| Implemented, with libraries | 4,116 / 40,468 (baseline 3,923) | 3,989 / 42,329 (baseline 3,791) |
| Implemented, --nolib | 2,008 / 38,303 (baseline 1,834) | 1,988 / 40,162 (baseline 1,812) |
| At effective 100%, counted the baseline's way (functions + vftables) | 3,387 (baseline 3,145) | 3,263 (baseline 3,032) |
| Non-library, counted the baseline's way | 1,488 (baseline 1,280) | 1,427 (baseline 1,229) |
| Functions only / non-library functions only | 3,195 / 1,296 | 3,085 / 1,249 |
| App markers | 263 FUNCTION + 30 FOLDED + 838 STUB | 255 + 30 + 823 |
| Vftables at 100% | 192 / 371 | 178 / 322 |
| Vftable mismatches, structural / pairing-only | 32 / 147 | 14 / 130 |

**Near-miss shapes** (app_sweep report on PF3/SF3)
- PLAYER, 838 markers short of 100%: inlining-size 282, other 243, ebp-frame 203, unpaired 63, x87-vs-sse 29, data-ref 9, unpaired-callee 4, assert-file-line 3, stack-cookie 2. 71 of them are at 90% or more.
- STUDIO, 823 short: inlining-size 279, other 244, ebp-frame 201, unpaired 51, x87-vs-sse 29, data-ref 9, unpaired-callee 4, assert-file-line 3, stack-cookie 3. 71 at 90% or more.
- Pairing-only, 13 per target. PLAYER:
  - FWBase ctor 0x00619740
  - startCoreScripts 0x006400f0
  - ServiceProvider::onChildAdded 0x006a9770
  - MovingStage::onMechanismRemoving 0x0099a5b0
  - ~SimBody 0x00976a00
  - AsyncHttpQueue::isUrlBad 0x0086e540
  - ContentId::convertToLegacyContent 0x0074ecf0
  - Extents::separatedByMoreThan 0x00796580
  - Face::fuzzyContainsInExtrusion 0x008fc120
  - Face::cornersAligned 0x008fc240
  - Math::radiansToQuadrant 0x00758430
  - Math::intersectRayConvexPolygon 0x0075a070
  - Math::intersectRayPlane 0x00759190

**Checks**
- decomplint: PLAYER 254 out-of-order + 20 duplicate_offset, STUDIO 267 + 12, plus 3 unicode_decode errors on G3D files that were already there.
- reccmp-vtable: runs on both, no traceback.
- datacmp: completes. PLAYER 689 variables, 24 issues; STUDIO 661 variables, 26 issues.
- check_folded: PLAYER 1,082 annotated addresses, STUDIO 1,064, 0 problems.
- reccmp_addr_padding: 1,652 files, 0 bad markers.

**Where things are**
- Copies of the scored exe/pdb/map: /tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/scored-i5/{P,S}/
- Backups of the files before reordering: /tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/i5/pre-reorder/
- Near-miss CSV: /tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/i5/app-sweep-final.csv

## Files changed

- tools/ida/gen_reccmp_csv.py
- tools/reorder_markers.py
- reccmp/player-symbols.csv
- reccmp/player-libdata.csv
- reccmp/studio-symbols.csv
- reccmp/studio-libdata.csv
- Client/_generated/player/stubs_00.cpp
- Client/_generated/player/unresolved.txt
- Client/_generated/studio/stubs_00.cpp
- Client/_generated/studio/unresolved.txt
- Client/App/v8datamodel/GuiBuilder.cpp
- Client/App/Gui/AdornBillboarder.cpp
- Client/App/Gui/ChatOutput.cpp
- Client/App/Gui/GUI.cpp
- Client/App/script/LuaArguments.cpp
- Client/App/script/LuaAtomicClasses.cpp
- Client/App/script/LuaEnum.cpp
- Client/App/script/LuaLibrary.cpp
- Client/App/script/LuaSignal.cpp
- Client/App/script/ScriptContext.cpp
- Client/App/script/Script.cpp
- Client/App/script/ThreadRef.cpp
- Client/App/util/AsyncHttpQueue.cpp
- Client/App/util/NormalId.cpp
- Client/App/util/Region3.cpp
- Client/App/v8datamodel/Camera.cpp
- Client/App/v8datamodel/CustomStatsGuiJSON.cpp
- Client/App/v8datamodel/DataModel.cpp
- Client/App/v8datamodel/DataModelJob.cpp
- Client/App/v8datamodel/GuiBase2d.cpp
- Client/App/v8datamodel/GuiObject.cpp
- Client/App/v8datamodel/JointsService.cpp
- Client/App/v8datamodel/LuaStatsItem.cpp
- Client/App/v8datamodel/PartInstance.cpp
- Client/App/v8datamodel/StatsItem.cpp
- Client/App/v8tree/Service.cpp
- Client/App/v8world/World.cpp
- Client/App/v8xml/SerializerV2.cpp
- build/sweep5/P-final.json
- build/sweep5/P-final.txt
- build/sweep5/P-final-nolib.txt
- build/sweep5/PF3.json
- build/sweep5/vtable-PLAYER.txt
- build/sweep5/datacmp-PLAYER.txt
- build/sweep5/lint-PLAYER.txt
- build/sweep5/lint-STUDIO.txt
- build-studio/sweep5/S-final.json
- build-studio/sweep5/S-final.txt
- build-studio/sweep5/S-final-nolib.txt
- build-studio/sweep5/SF3.json
- build-studio/sweep5/vtable-STUDIO.txt
- build-studio/sweep5/datacmp-STUDIO.txt

## Unresolved

- Files I could not reorder because a function containing an assert (or another __LINE__ user) would change line (40 files): Gui/GuiDrawImage, reflection/reflection_object, script/CoreScript, ScriptContext (27->26 after a partial reorder), util/ContentProvider, Face, IndexedMesh, InterpolatedCFrame, Math, NormalId, SpanningTree, v8datamodel/DataModel (19->17 after a partial reorder), GuiBuilder (9->3), PartInstance, SafeChat, Workspace, v8kernel/Body, ContactConnector, Kernel, v8tree/Instance, Verb, v8world/Assembly, CleanStage, ContactStage, EdgeBuffer, GroundStage, IMoving, Joint, JointStage, MovingAssemblyStage, Primitive, SendPhysics, SimulateStage, SpatialFilter, StepJointsStage, TreeStage, World, v8xml/SerializerV2, Base/rbx/TaskScheduler, Network/Players. Fixing these means moving functions between files or fitting assert lines again.
- Files whose descents cross a namespace, preprocessor, pragma or REFLECTION_BEGIN boundary (19): LuaInstanceBridge, LuaLibrary, ScriptEvent, ThreadRef, AsyncHttpQueue, Extents, Name, ProtectedString, Region3, Sound, DataModelJob, TextService, SimBody, Service, Contact, ContactManager, IPipelined, SleepStage, RbxOgre/RbxTypesetter.
- Files where a marked function exists only in STUDIO, so the PLAYER sort cannot place it: Gui/GUI.cpp, v8datamodel/BaseScript.cpp, PartInstance.cpp. In no file did the PLAYER sort add STUDIO descents ('targets disagree' never fired).
- Reorders that break the compile through declaration order, so only the strict (comment-only runs) form was kept: reflection/type.cpp (explicit specialization Type::getSingleton<void> after instantiation, C2908), LuaAtomicClasses.cpp (closestPointVector3/distanceVector3 statics; the loose rule would go 25->0, strict gives 25->21), LuaSignal.cpp (g_allowSignalConnect; 6->0 loose, 6->4 strict), LuaInstanceBridge.cpp (YieldFunctionStateObject::execute is private).
- EnumDesc<T>::convertToItem PLAYER 0x007761a0 is still unpaired, and its marker went back to STUB. Its template-line marker fails with 'Debug data out of sync' because InputObject.cpp, LuaArguments.h and LuaEnum.h include 'Reflection/EnumConverter.h' while 2013 used 'reflection/enumconverter.h'. Those include lines are not mine to change. Changing them would also bring the InputObject EnumDesc vftables 0x00b5c5a4/0x00b5c5bc above 0.6.
- Vftables: PLAYER 32 structural / 147 pairing-only, STUDIO 14 / 130. Most of the pairing-only ones are unpaired `??_E...W<n>@` adjustor thunks (the generator filters them out of function-size) and twin bodies (orig 0x004027c0 with 102 slots, 0x0070c430 with 64), where a body pair at 100% outvotes the slot votes. STUDIO's ModelInstance Selectable vftable 0x00c14f64 needs a STUDIO-only isSelectable3d override 0x00817390.
- DataModel ctor 0x0064ec50 is at 62.5% and its dtor 0x0064d850 at 92.8%. They wait on items outside my ownership: the PhysicsInstructions member, Workspace(IDataState*), and STUDIO's layout difference.
- decomplint still reports 20 (PLAYER) and 12 (STUDIO) duplicate_offset warnings, plus 3 unicode_decode errors on G3D latin-1 files (CollisionDetection.cpp, Matrix3.h, Ray.h). All were already there before this run.
