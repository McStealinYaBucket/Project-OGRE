# core-bodies (workflow phase4-core3, wf_4f741962-18b)

I finished steps 1 to 5 and part of step 6. One FW-layout change was added along the way. Both targets build cleanly, and the final score runs are newer than the 0b8a684 baseline.

**Final scores**
- PLAYER (build/dmsc/P2.json):
  - Implemented 3,924 / 40,468 (baseline 3,923). With --nolib 1,835 / 38,322 (baseline 1,834).
  - Entries at effective 100%: 3,224 (baseline 3,145); non-library 1,344 (baseline 1,280).
  - Vftables at 100%: 179 / 351 (baseline 174 / 339).
- STUDIO (build-studio/dmsc/S1.json):
  - Implemented 3,793 / 42,329 (baseline 3,791). With --nolib 1,818 (baseline 1,812).
  - Effective 100%: 3,115 (baseline 3,032); non-library 1,300 (baseline 1,229).
  - Vftables at 100%: 167 / 295 (baseline 163 / 283).
- The totals also include other agents' concurrent edits. The per-function results below are mine.
- check_folded PLAYER: 1,066 addresses, 0 folding problems. reccmp_addr_padding: 0 bad markers.

**Lost 100% (each read)**
- shared_ptr<Instance>::operator= (PLAYER 0x006eda50, STUDIO 0x00751110) and its three callers dropped to 97.8–98.9%:
  - copy_on_write_ptr::write 0x00567ca0
  - ScriptContext::scheduleResume 0x0067aea0
  - LibraryService::registerDevelopmentLibrary 0x0088cce0
  - Cause: the libdata CSV pairs that address through the symbol `??4?$shared_ptr@VRelativePanel@...`, which only existed because of GuiBuilder's 2016-only `chatMenu` member. That member is now gone. The code is unchanged; regenerating the CSVs pairs it with shared_ptr<Instance>::operator=.
- The PartInstance functions 0x007234c0 / 0x007234e0 and the AssemblyStage vftable belong to other agents' files.

**Step 1: atomic helpers, globals, ThreadRef**
- Client/Base/rbx/atomic.cpp now calls the kernel32 imports, as the original bytes do.
  - RbxInterlockedDecrement / Increment: PLAYER 0x009b8f00 / 0x009b8f10, STUDIO 0x00a70810 / 0x00a70820. Both 40% → 100% in both targets.
  - New RbxInterlockedExchange (PLAYER 0x009b8f20, STUDIO 0x00a70830) and RbxInterlockedCompareExchange (0x009b8f40 / 0x00a70850): 100%. Names and order come from iOS 1029.
- propRobloxLocked/propArchivable and ThreadRef(lua_State*) were already at 100% through the regenerated CSVs, so no GLOBAL markers were needed.
- RbxDbgInfo::RemovePlace (PLAYER 0x009bc810, STUDIO 0x00a72c00): 100% after adding markers. The DataModel destructor calls it.

**Step 2: GuiBuilder layout and DataModel size**
- GuiBuilder is now 0x20, from ctor 0x00644f30 / STUDIO 0x00761ef0: dataModel 0x0, workspace 0x4, shared_ptr<TopMenuBar> safeChatMenu 0x8, customStats 0x10, networkStatsPage 0x1c (nextNetworkStats 0x007f2e80).
- The ctor takes no arguments and is inline. Initialize(DataModel*) is out of line in GuiBuilder.cpp, folded at PLAYER 0x005c30c0 with RakNet SetWriteOffset.
- STUDIO's GuiBuilder is 0x1c, a real source difference recorded in the header. The STUDIO ctor stores parentDataModel at 0xe08.
- DECOMP_SIZE_ASSERT(GuiBuilder, 0x20) and DECOMP_SIZE_ASSERT(DataModel, 0xe10) are enabled and compile.
- The GuiBuilder ctor scores 97.1% in both targets. The only difference is the unpaired map `_Buynode` (orig 0x0055b6c0).

**Step 3: DataModel vftable (0x00b2ffec, 34 slots)**
- Added the 2013 virtuals in slot order:
  - +0x70 processAccelerators(const GuiEvent&) 0x0064fce0
  - +0x74 processCameraCommands(const GuiEvent&) 0x0062f010
  - +0x78 savePlaceAsync 0x0063f980
  - +0x7c renderPass2d(Adorn*, IMetric*) 0x00645770
  - +0x80 renderMouse(Adorn*) 0x00640970
  - +0x84 bool processEvent(const UIEvent&) 0x0064ae00
- Added overrides onChildAdded(Instance*) 0x0062ee00 and onChildChanged 0x0063e180, which scores 94%; only the assert line differs (2013 line 3145).
- Return types come from `ret` sizes (GuiResponse by value, bool).
- Removed the virtual getStatValue: it has no 2013 symbol, and GuiBuilder now uses getMetricValue.
- Our vftable now has 34 slots with every slot paired except twin bodies. vtable_structural reports only pairing-only rows.

**Step 4: constructor and destructor bodies**
- DataModel ctor PLAYER 0x0064ec50: 49.1% → 58.1% (STUDIO 0x0076b720: unpaired → 54.6%).
  - Body written from the bytes: FFlag NewLightAPI / ImageRectEnabled gates, FASTLOG CloseDataModel, then call_once on three statics (registerNewLightAPI, registerNewImageAPI, registerNetworkStreamingProp).
  - Then addHackFlag(0x800), DataModel::count++, setName("Game"), fw()->b21.set(true, fwRef()), guiBuilder.Initialize(this).
  - Added FASTFLAGVARIABLE NewLightAPI / ImageRectEnabled (registered in DataModel.cpp's initializers), the static registerNewImageAPI STUB (0x0064ea00) and the registerNetworkStreamingProp STUB in Workspace.cpp (0x007154c0).
- DataModel dtor 0x0064d850: 78.6% → 89.3% (STUDIO 0x0076a340: unpaired → 87.1%). Body: count--, then RemovePlace(placeID) if it is set.
- ScriptContext ctor 0x0068ad20: 62.6% → 88.1% in both targets.
  - Added nextPendingScripts(Time::now<Fast>()), setName("Script Context") and the camelCaseViolation connection.
  - New onCamelCaseViolation (0x00670df0 / 0x007d8d50) scores 63%.
- Also in Base: 2013's atomic(T) has no assert, and atomic() asserts `(((long)(&(this->v))) & 0x3) == 0` at atomic.h line 224 (the file is laid out to put it there).
  - rbx::atomic<int>() 0x004022d0: 72% → 96%. The remaining difference is one instruction scheduled differently; four probe variants did not reproduce it.
  - TaskScheduler ctor: 70.9% → 81.9%.

**Step 5: FW layout**
- b21/b22/b23 now live in FWInstance at 0x18–0x1a. FWBase ends at 0x18.
- Instance::fw() returns FWInstance* (0x006beaa0 is unchanged at 100%).
- FWValue's default ctor is trivial, matching 2013. FWInstance initializes the three bytes in its init list.
- FWBase ctor: 92.8% → 97.0%. FWInstance ctor: 61.5% → 64.6%. The remainder is a call to 0x0061c800 on this+0x18, probably an Allocator<FWInstance> empty base, not modeled.
- Instance.h, Instance.cpp and Workspace.cpp were adjusted. The setters stay at 100%.

**Step 6: assert homes (partial)**
- LuaArguments getValues and getVariant are now inline in include/script/LuaArguments.h, with RBXASSERT on lines 146 and 178. The header was reordered so the class comes before withVariantValue (forward-declared).
  - getValues: 97.6% → 100% in both targets (0x00683f90 / 0x007e9d50).
  - getVariant: 94.3% → 100% in both (0x00877b40 / 0x009dc240).
- ThreadRef.cpp is laid out so the real RBXASSERT lands on lines 112 and 419.
  - addRef: 97.2% → 100% (0x00871d60 / 0x008281e0).
  - LiveThreadRef ctor: 58.6% → 61.4%.
- Also at 100% in both: ScriptContext::crash 0x0067a190 / 0x007e20b0, from the ScriptContext edits.

**Markers**
- Flipped to FUNCTION from fresh P2/S1 runs, only in my files: 5 PLAYER and 5 STUDIO (the getValues, getVariant, addRef, RemovePlace and crash markers).
- The atomic.cpp markers were written as FUNCTION and measured at 100% in both targets.

**For the next IDA sync**
- Names:
  - 0x009b8f20 RBX::RbxInterlockedExchange(long volatile*, long) (STUDIO 0x00a70830)
  - 0x009b8f40 RBX::RbxInterlockedCompareExchange(long volatile*, long, long) (STUDIO 0x00a70850)
  - 0x009bc810 RBX::RbxDbgInfo::RemovePlace(long) (STUDIO 0x00a72c00)
  - 0x00644f30 / STUDIO 0x00761ef0 GuiBuilder::GuiBuilder()
  - STUDIO 0x008a8950 GuiBuilder::Initialize (the call target in the STUDIO ctor)
  - 0x00de4760 FFlag::NewLightAPI (STUDIO 0x00f83b31), 0x00de4761 FFlag::ImageRectEnabled
  - The call_once flags: 0x00de4768 image, 0x00de4770 light, 0x00de4778 network (STUDIO 0x00f83b38 / 0x00f83b40 / 0x00f83b48)
  - 0x00de4c0c RBX::DataModel::count (STUDIO 0x00f83fd8)
  - STUDIO 0x0076a340 ~DataModel
  - 0x005ba6f0: second atomic-style ctor (assert at atomic.h line 182), used only for TaskScheduler +8
  - 0x0061c800: FWInstance's +0x18 base/member ctor (pool registration)
- Fix: 0x009bb680 is Time::now<Fast> (it checks preciseOverride against 0; 0x009bb6e0 checks against 1). The CSV calls it now<2>, and our Time.cpp has the wrong 2013 shape.
- Types:
  - GuiBuilder 0x20 (STUDIO 0x1c), offsets as above
  - FWBase 0x18; FWInstance with the bytes at 0x18–0x1a and name at 0x1c
  - DataModel vftable with 34 slots, names as above
  - LuaSettings 2013 (ctor 0x0085e360): +0x64 / +0x68 int 200, +0x70 double, +0x78 int 2, +0x7c int 0, +0x80 bool, +0x84 float. Our header is 8 bytes short.
  - IScriptFilter has no vftable of its own in 2013 (novtable).

## Files changed

- Client/Base/rbx/atomic.cpp
- Client/Base/include/rbx/atomic.h
- Client/Base/rbx/RbxDbgInfo.cpp
- Client/App/include/V8DataModel/GuiBuilder.h
- Client/App/v8datamodel/GuiBuilder.cpp
- Client/App/include/V8DataModel/DataModel.h
- Client/App/v8datamodel/DataModel.cpp
- Client/App/v8datamodel/Workspace.cpp
- Client/App/include/V8DataModel/ScriptContext.h
- Client/App/script/ScriptContext.cpp
- Client/App/include/Util/FeatherweightDict.h
- Client/App/include/V8Tree/Instance.h
- Client/App/v8tree/Instance.cpp
- Client/App/include/script/LuaArguments.h
- Client/App/script/LuaArguments.cpp
- Client/App/script/ThreadRef.cpp
- build/dmsc/P2.json
- build/dmsc/P2-nolib.txt
- build/dmsc/vt.txt
- build-studio/dmsc/S1.json
- build-studio/dmsc/S1-nolib.txt

## Unresolved

- Integrate must regenerate reccmp/*.csv. player-libdata.csv:861 (0x006eda50) and studio-libdata.csv:820 (0x00751110) name ??4?$shared_ptr@VRelativePanel... That symbol was only emitted by GuiBuilder's 2016-only chatMenu member, which is now removed. Until regeneration, shared_ptr<Instance>::operator= and 3 callers per target sit at 97.8-98.9% instead of 100%: copy_on_write_ptr::write PLAYER 0x00567ca0 / STUDIO 0x005cb7e0, scheduleResume 0x0067aea0 / 0x007e2dc0, and registerDevelopmentLibrary 0x0088cce0. The GuiBuilder ctor (97.1%) also waits on a CSV row for its map _Buynode (orig 0x0055b6c0).
- registerNewLightAPI (PLAYER 0x0075ed40, STUDIO 0x007d7380) belongs in Light.cpp, which I do not own. The DataModel ctor's call to it stays unresolved until a STUB exists there or gen_link_stubs runs.
- ScriptContext ctor (88.1%):
- gcFullStepThreshold/gcFrequencyBase are not written. The ctor reads LuaSettings +0x78/+0x7c through GlobalAdvancedSettingsItem<LuaSettings>::singleton 0x00654600. Our LuaSettings.h (not mine) is 8 bytes short: the 2013 ctor 0x0085e360 stores 200/200 at +0x64/+0x68, a double at +0x70, 2 at +0x78, 0 at +0x7c, a bool at +0x80 and a float at +0x84.
- IScriptFilter.h (not mine) should be `class RBXInterface IScriptFilter` (novtable). Evidence: no IScriptFilter vftable exists in PLAYER, and the 2013 ctor has no store to [esi+0x60]; ours does.
- The safe_queue at 0xcc is constructed inline in 2013; ours calls an out-of-line ctor.
- DataModel ctor (58%) and dtor (89%):
- The PhysicsInstructions member at 0xbe8 (ctor 0x0084d2f0) is not modeled. It shifts the EH state numbers by one.
- 2013 creates the workspace with Creatable<Instance>::create<Workspace>(this), passing DataModel* to Workspace(IDataState*) 0x00718e10. Workspace.h (not mine) only has Workspace().
- STUDIO's DataModel is 4 bytes lower from 0xc64 (no networkStatsWindowsOn) and lacks networkStatsPage. The header follows PLAYER, so STUDIO's ctor and dtor cannot reach 100%.
- Step 6 left over:
- The LuaSignalBridge.cpp and LuaInstanceBridge.cpp bodies still carry Mac literal asserts inside ThreadRef.cpp: FunctionScriptSlot::operator() 0x0087dee0 and callCallback 0x0087b940, PLAYER lines 124/131/173/186/192/276 and 371/399/659/666/667/702/829/948/967/984. The 2013 assert order shows these are several helper functions that our tree merged, so moving them needs real decompilation, not a cut and paste.
- ThreadRef.cpp line 338 (callGenericAsyncFunctionBridge 0x008736a0, yieldCaptured) is still a literal.
- In LuaArguments.h, the line 114 (withVariantValue "0") and line 213 (pushArray "count == 1") asserts are still literals. withVariantValue is now defined after the class and would need compacting to end at line 114.
- LuaArguments.cpp line 436 ("!_First->first.empty()") is still a literal.
- rbx::atomic<int>() 0x004022d0 is at 96%. The 2013 body hoists `cmp byte [FLog::Asserts]` above the `v = 0` store. Five probe variants failed to reproduce it: v(0), body assignment, assert-first, volatile, and an interlocked store.
- The assert at atomic.h line 182 (0x005ba6f0, called only by TaskScheduler for its member at +8) is a second, unidentified class.
- Our Time.cpp now<Fast>/now<Benchmark> do not have the 2013 shape. The 2013 bodies check preciseOverride and cache a time through RbxInterlockedExchange (0x009bb680 / 0x009bb6e0).
- FWInstance ctor (64.6%): 2013 calls 0x0061c800 on this+0x18 before the bytes. This is probably an Allocator<FWInstance> empty base. I did not model it, because pool allocation conflicts with the tree's FWDeleter plain-delete path.
- DataModel's other placeholders are unchanged from core-datamodel.md: the src2016 member names, m_unk0xae0 and PhysicsInstructions.
