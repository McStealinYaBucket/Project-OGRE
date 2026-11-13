# core-close (workflow phase4-core4, wf_cf7a5075-c0f)

Phase 4 close-out: Instance's vftable, FW, DataModel, ScriptContext, Time, TaskScheduler and Security. Measured with reccmp on my own snapshots of build/ and build-studio/ (scratchpad r4/ and s2/), using the shared CSVs as they are. The totals below also include other agents' concurrent work in the shared tree.

TOTALS (baseline -> now)
- PLAYER: implemented 4116 -> 4153 / 40468; --nolib 2008 -> 2047 / 38303; effective 100% 3387 -> 3498.
- STUDIO: implemented 3989 -> 4020 / 42330; --nolib 1988 -> 2023 / 40163; effective 100% 3263 -> 3382.
- Vftables at 100%: PLAYER 192/371 -> 192/372 (FWBase 0x00b2a214 newly compared and at 100%, FWInstance 0x00b2a234 newly compared at 0%); STUDIO 178/322 -> 178/323.
- Losses checked one by one with --verbose; none comes from my files:
  - RakNet LocklessUint32_t::Decrement (P 0x00608150, S 0x0069efc0): our linker now folds it with G3D::AtomicInt32::decrement from Frustum.cpp. RecvFromLoop follows from it.
  - luaK_setreturns, symbexec, constructor, patchtestreg, removevalues, luaV_execute and IsOlderOrderedPacket: an immediate now coincides with an RTTI ??_R0 address (no .reloc).
- Markers flipped to FUNCTION, all measured at effective 100%:
  - PLAYER (10): FWBase ctor 0x00619740 and dtor 0x00619690, FWInstance ctor 0x0061d7e0, checkSpeedHack 0x009bb050, Time::isSpeedCheater 0x009bb490, Time::isDebugged 0x009bb4a0, nowFast 0x009bb4b0, nowFastSec 0x009bb4d0, operator- 0x009bb4f0, startMMTimer 0x009bb510, now<Benchmark> 0x009bb6e0.
  - STUDIO (9): 0x006ee410, 0x006ee360, 0x006f2210, 0x00a71ea0, 0x00a722e0, 0x00a72300, 0x00a72320, 0x00a72340, 0x00a72510.
- addr_padding: 0 bad. decomplint: only the function_out_of_order descents that were already listed for Instance.cpp, Workspace.cpp and TaskScheduler.cpp.

STEP 1: INSTANCE
- Exit criterion: not reached.
  - All four Instance vftables (0x00b2a6ec, 0x00b2a6c4, 0x00b2a6d4, 0x00b2a6e0) are structurally complete in both targets (vtable_structural.py: 0 structural, pairing-only).
  - The PropertyChangedSignalData slot is a twin body the tool misreads: 0x006d5ed0 == 0x004027d0 byte for byte, STUDIO 0x0081ae10 == 0x0040cd70 too.
  - What is still red is twin and thunk pairing that markers in my files cannot fix (see Unresolved).
- Facts found:
  - The 2013 Instance ctors are Instance(FWInstance*) 0x00620da0 and Instance(const char*, FWInstance*) 0x00620ed0 (iOS mangles); Instance() and Instance(const char*) do not exist.
  - The 2013 ctor has no NULL check on name.
  - Implemented as `Instance(FWInstance* fw = NULL)` and `Instance(const char*, FWInstance* = NULL)`. Both use FWBase::init<FWInstance>, a FASTLOG2 "New Instance %p, fw ref: %p", and FWStringValue(name) assigned over fw+0x1c.
  - Dtor: the 2013 FASTLOG1 "Instance %p destroyed" added; Instance.cpp laid out by assert_lines --apply so all 13 asserts sit on their 2013 lines (parent==NULL on line 846, etc.).
  - Debugable is novtable (rbx/Debug.h): PLAYER has no Debugable vftable and the Instance ctor stores no Debugable vptr.
- Scores:
  - Instance(FWInstance*): P 0.589 -> 0.830, S 0.578 -> 0.819.
  - Instance(const char*, FWInstance*): P 0.535 -> 0.828, S 0.525 -> 0.808.
  - ~Instance: P/S 0.858 -> 0.905.
- Private-compile probe (tools/probe_compare.py; Instance.cpp built with an Object.h override in which Object and AbstractFactoryProduct are not RBXBaseClass):
  - ~Instance is byte-IDENTICAL.
  - Both ctors are 7 bytes off: only `lea edi,[esi+0x48]` is scheduled before `call init` instead of after.
  - So Util/Object.h (not mine) is the remaining blocker. 2013 stores AbstractFactoryProduct's vftable 0x00b2a2f0 at Instance+0x18 (0x00620df9) with its own EH state, and the dtor stores Object's 0x00b29f08 (0x0061d684).

STEP 2: FW
- FWInstance : FWBase, Allocator<FWInstance>. Evidence: RTTI base list with Allocator at mdisp 0x18; ctor 0x0061c800 = Allocator<FWInstance>::Allocator(), iOS _ZN3RBX9AllocatorINS_10FWInstanceEEC2Ev.
- FWPartInstance adds Allocator<FWPartInstance> (mdisp 0x38, pool 0x54; D0 0x00724030 gives 0x54 back to 0x00df44a0), with using-declarations so new/delete resolve to it.
- New FWFinal<T> : T. Its virtual dtor erases the dict entry (FWFinal<FWInstance> dtor 0x0061f1c0); a copy ctor (0x00845c00 shape) is used by valueUpdate.
- FWBase::init<T> news FWFinal<T> (0x0061fe70). FWDeleter is removed: 2013 uses plain shared_ptr<FWBase> (sp_counted_impl_p<FWBase> RTTI).
- Removed the fwHash virtual: the 2013 FWInstance vftable has one slot, so hashing now goes through the hash_value overloads.
- FWBase log strings set to the 2013 ones ("FW instance created: %p" / "FW instance destroyed: %p"). FWInstance() initializes name("Instance").
- Results:
  - FWInstance ctor: P 0.646 -> 1.0, S 0.646 -> 1.0.
  - FWBase ctor/dtor: P and S 1.0.
  - init<FWInstance>: P 0.969, S 0.938.
  - FWFinal dtor: P 0.962.
  - Allocator ctor (TEMPLATE markers added to memory.h without moving any line): P 0.897.
  - At obj level init, FWFinal dtor and FWInstance ctor are IDENTICAL; what is left is unpaired callees and vftables.

STEP 3: DATAMODEL
- New PhysicsInstructions (0x60) and Util/Average.h (0x18) from ctor 0x0084d2f0 and Average ctor 0x0084d260. The member at 0xbe8 replaces the m_unk block.
- New Workspace(IDataState*) STUB 0x00718e10 / 0x008357b0. 2013 has virtual bases: the ctor tests the most-derived flag. The ctor now calls create<Workspace>(this).
- The DataModel once_flags are no longer static (they now pair).
- Results:
  - DataModel dtor: P 0.928 -> 0.985 (obj-IDENTICAL), S 0.916 -> 0.971.
  - DataModel ctor: P 0.625 -> 0.647.
  - Probe: with addHackFlag inlined, the frame and most of the body line up. The rest is the AbstractFactoryProduct state, the 0xbe4/0xbe5 store scheduling, and fw() called out of line (2013 inlines addHackFlag but not fw()).
- registerNewLightAPI (382 B: assert at Light.cpp:25 plus two static property descriptors) was not written.

STEP 4: SCRIPTCONTEXT
- LuaSettings gets the 4 base bytes at 0x60, so the size is 0x88 and gcPause sits at +0x64.
- The ctor now reads gcFullStepThreshold/gcFrequencyBase (+0x78/+0x7c) through LuaSettings::singleton (paired to 0x00654600).
- IScriptFilter is RBXInterface (novtable).
- Score: P 0.889 -> 0.897. What is left is the safe_queue, which is really the spin_mutex type (see Unresolved).

STEP 5: INCLUDE SPELLINGS
- LuaArguments.h and LuaEnum.h now include 'reflection/enumconverter.h'. InputObject.cpp is left as it was: 2013's InputObject addPair 0x008b5df0 is the only function that uses 'Reflection/EnumConverter.h', so the old note was wrong for that file.
- Instance.h, Workspace.cpp and FWService.h include 'util/FeatherweightDict.h' (2013 string).
- EnumDesc<Material>::convertToItem 0x007761a0 is still unpaired, and the InputObject EnumDesc vftables are still 0.6.

STEP 6: TIME AND TASKSCHEDULER
- Time.cpp rewritten to the 2013 Windows object (0x009bb050-0x009bb707):
  - QPC/QPF clock, multimedia-timer callbacks, startMMTimer, the speed-hack and debugger checks, now<Fast> with startMMTimer + FLog::Init(nowFastSec) through RbxInterlockedExchange, now<Benchmark>.
  - GLOBAL markers for the file-scope variables. They have external linkage because reccmp pairs namespace-scope statics only by their bare name.
- now<Fast>: P 0.083 -> 0.923 (obj-IDENTICAL; only the function-local `inited` is unpaired). now<Benchmark>: 1.0.
- TaskScheduler:
  - 2013 has no errorCalculationPerSec, sortFrequency or lastSortTime, and the lerp is a pool constant. The ctor's assert lines fitted (4 of 6).
  - Ctor: P/S 0.819 -> 0.955. What is left is the atomic.h:182 class 0x005ba6f0 (taskCount), not identified.

STEP 7: SECURITY::CONTEXT
- Context::current() is inline in 2013 (the kept copy is in Network, between Player.cpp's functions) and heap-allocates Context(Anonymous). Now header-inline with boost/thread/tss.hpp included.
- Score: P/S 0.449 -> 0.897. The rest is boost.dll calls through the thunk instead of `call [imp]` (no BOOST_*_DYN_LINK define).
- Context::ptr 0x0041ceb0 is an LTCG client-shell copy (/GS cookie), so it cannot match.
- Ctor/dtor/scalar-deleting-dtor work for the TaskScheduler core classes beyond the ctor was not reached.

IDA NAMES FOR THE NEXT SYNC (PLAYER; source)
- FW and Allocator:
  - 0x0061c800 RBX::Allocator<RBX::FWInstance>::Allocator() (ios:)
  - 0x0061bbd0 Allocator<FWInstance>::operator new(size_t)
  - 0x0061ba30 Allocator<FWInstance>::operator delete(void*)
  - 0x0061c2a0 Allocator<FWInstance>::releaseMemory
  - 0x0061fe70 FWBase::init<FWInstance>(FWInstance*) (ios:)
  - 0x0061f1c0 FWFinal<FWInstance>::~FWFinal (match: obj-identical)
  - 0x00845c00 FWFinal<FWPartInstance>::FWFinal(const FWPartInstance&)
  - 0x00721e50 Allocator<FWPartInstance>::operator new; 0x00721800 Allocator<FWPartInstance>::operator delete
- PhysicsInstructions and Average:
  - 0x0084d260 Average<double>::Average(size_t, double)
  - 0x0084d0a0 vector<double>::_Insert_n; 0x0084d020 vector<double>::erase
- Time.cpp (all match: or obj-identical):
  - 0x009bb050 checkSpeedHack (static)
  - 0x009bb130 RBX::getStart
  - 0x009bb180 directCallback (CALLBACK)
  - 0x009bb230 interpolatedCallback
  - 0x009bb490 Time::isSpeedCheater; 0x009bb4a0 Time::isDebugged
  - 0x009bb510 startMMTimer
  - Globals: 0x00e1b608 ticksPerCorrection, 0x00e1b610 millisecondsPerTick, 0x00e1b618 prevSpeedHackCheckpointTime, 0x00e1b620 currentSeconds, 0x00e1b628 cheater, 0x00e1b629 isDebuggedValue, 0x00e1b62c lastSpeedCheckCounter, 0x00e1b630 violationsCount, 0x00e1b634 prevTickTime, 0x00e1b638 recguard, 0x00e1b640 getStart::start, 0x00e1b664 now<Fast>::inited.
  - STUDIO equivalents are at 0x00f9e7f0-0x00f9e84c (in the markers).
- Instance:
  - 0x007bc3e0 Instance `scalar deleting destructor'
  - 0x007bc400 / 0x00958d00 its adjustor thunks (-0x1c / -0x18)
- Other:
  - 0x00402240 tbb __TBB_LockByte (TBB 2.1, COMDAT); 0x009eeea0 thunk tbb.dll::__TBB_machine_trylockbyte
  - 0x005ba6f0 the atomic.h:182 class ctor
  - Global 0x00e0c9dc ClassRegistrar<Instance>::registrar

IDA TYPES FOR THE NEXT SYNC
- FWInstance 0x38: FWBase@0, Allocator<FWInstance>@0x18 empty, b21/b22/b23@0x18-0x1a, name@0x1c.
- FWPartInstance 0x54 (+Allocator<FWPartInstance>@0x38).
- FWFinal<T> : T.
- PhysicsInstructions 0x60 (averageDt@0, averageDutyDt@0x18, doubles@0x30/0x38/0x40/0x48, bool@0x50, double@0x58).
- Average<double> 0x18.
- LuaSettings 0x88 (gcPause@0x64).
- Debugable and IScriptFilter: novtable.
- TaskScheduler: runningJobCount@0x180, nextWakeTime@0x188, threads@0x190.

## Files changed

- Client/App/include/V8Tree/Instance.h
- Client/App/v8tree/Instance.cpp
- Client/App/include/Util/FeatherweightDict.h
- Client/App/include/V8DataModel/FWService.h
- Client/Base/include/rbx/Debug.h
- Client/Base/include/rbx/memory.h
- Client/Base/rbx/Time.cpp
- Client/Base/include/rbx/TaskScheduler.h
- Client/Base/rbx/TaskScheduler.cpp
- Client/App/include/V8DataModel/DataModel.h
- Client/App/v8datamodel/DataModel.cpp
- Client/App/include/V8DataModel/PhysicsInstructions.h
- Client/App/v8datamodel/PhysicsInstructions.cpp
- Client/App/include/Util/Average.h
- Client/App/include/V8DataModel/Workspace.h
- Client/App/v8datamodel/Workspace.cpp
- Client/App/include/V8DataModel/LuaSettings.h
- Client/App/include/V8DataModel/IScriptFilter.h
- Client/App/script/ScriptContext.cpp
- Client/App/include/script/LuaArguments.h
- Client/App/include/script/LuaEnum.h
- Client/App/include/security/SecurityContext.h
- Client/App/security/SecurityContext.cpp

## Unresolved

- EXIT CRITERION (Instance vftables green) needs generator/Integrate work, not headers. Every remaining red slot is a twin or thunk pairing. The original keeps two copies of each trivial body, probably an LTCG (client shell, 0x40xxxx) copy and an engine copy that ICF did not fold together; ours folds them into one. Original slot counts per copy: ret 4 0x004027c0 (3620 slots) vs 0x00575d00 (930); xor al,al;ret 0x00402730 (781) vs 0x00927cc0 (1799); ret 8 0x00402740 (499) vs 0x008c8450 (233); xor al,al;ret 4 0x0070c430 (2121) vs 0x00475080 (1); ret 0x0040b950 (590) vs 0x00528720 (331). Today our single ret-4 body is claimed by the FOLDED markers at 0x00575d00 (IAdornable.h, IndexedTree.h, Item.h; not my files). The CSV pairs our xor al,al;ret body with 0x006690f0 (a 252-byte EH function used in 0 slots), which is wrong. Instance's Described/AbstractFactoryProduct slot 0 needs rows for the adjustor thunks 0x007bc400 (sub ecx,0x1c) and 0x00958d00 (sub ecx,0x18), both of which jmp to ??_GInstance at 0x007bc3e0.
- Util/Object.h (not mine): drop RBXBaseClass from Object and AbstractFactoryProduct. Evidence: the 2013 Instance ctor stores AbstractFactoryProduct's vftable 0x00b2a2f0 at 0x00620df9 with an extra EH state, and the dtor stores Object's vftable 0x00b29f08 at 0x0061d684. A private compile with that change makes ~Instance 0x0061d510 byte-identical and leaves both ctors 7 bytes off (lea edi scheduling only). It also shifts the EH states of every Instance subclass, so expect gains tree-wide.
- Integrate: in player-symbols.csv rows 0061c320 (FWInstance D0) and 0061f220 (FWFinal<FWInstance> D0) have empty symbol columns, which blocks the SYNTHETIC markers. They need ??_GFWInstance@RBX@@UAEPAXI@Z and ??_G?$FWFinal@VFWInstance@RBX@@@RBX@@UAEPAXI@Z (STUDIO 007be5f0 / 006f3ae0). This is what keeps the FWInstance vftable 0x00b2a234 at 0%. Nothing in reccmp/ was edited.
- rbx::spin_mutex is tbb::spin_mutex on Windows 2013. Evidence: RTTI sp_counted_impl_p<tbb::spin_mutex> and shared_ptr<tbb::spin_mutex> in the ThreadPool bind types; safe_queue locks call __TBB_LockByte 0x00402240 (pause backoff, SwitchToThread) -> tbb.dll __TBB_machine_trylockbyte; the ScriptContext ctor zeroes a 1-byte flag at waitingThreads+0x20. The TBB 2.1 headers are gone and 4.4's LockByte differs, so I did not switch it. This blocks the inline safe_queue ctor in ScriptContext 0x0068ad20 and every safe_queue lock site.
- Boost.Thread imports: 2013 calls get_tss_data and other boost.dll functions as `call [imp]` (dllimport), ours goes through the thunk. A CMake define (BOOST_THREAD_DYN_LINK or BOOST_ALL_DYN_LINK) is needed; CMakeLists.txt is not mine. This affects Security::Context::current (0.897) and every TSS user.
- DataModel ctor 0x0064ec50 (0.647). A private probe with addHackFlag forced inline matches the frame and nearly all of the body. What is left: (a) 2013 inlines addHackFlag(0x800) (param temp at [esp+0x10]) but calls Instance::fw() 0x006beaa0 out of line, while our compiler does the opposite; (b) the 0xbe4/0xbe5 byte stores come before the PhysicsInstructions lea/state; (c) the AbstractFactoryProduct state from Object.h. registerNewLightAPI 0x0075ed40 / STUDIO 0x007d7380 (382 B: assert at Light.cpp:25, two static descriptors through 0x0075e640/0x0075ebe0) is still a generated stub.
- PhysicsInstructions ctor 0x0084d2f0 (0.784) needs GlobalAdvancedSettingsItem<PhysicsSettings,sPhysicsSettings>::singleton() 0x00654470 (+0x68). LuaSettings needs the same template base: ctor 0x0085e220, bool at +0x60, singleton 0x00654600. GlobalAdvancedSettings.h is not mine.
- Time.cpp: getStart 0x009bb130 (2013 zeroes the static before the QPC copy; source form not found), now<Precise> 0x009bb400 (getStart is inlined and evaluated before the QPC), directCallback/interpolatedCallback (2013 calls getStart out of line). now<Fast> 0x009bb680 is obj-identical but stays at 0.923 because its function-local `inited` (0x00e1b664 / STUDIO 0x00f9e84c) has no pairing.
- TaskScheduler: taskCount (+8) uses a second atomic class whose ctor 0x005ba6f0 asserts at atomic.h:182 (rbx::atomic<int> is at line 224). Its name and type are unproven, so it is not modelled. The ctor stays at 0.955. Two TaskScheduler.cpp asserts are still off their 2013 lines (339 vs 337, 557 vs 535). Ctor/dtor/scalar deleting dtor for the remaining TaskScheduler core classes (Job, Thread, Arbiter) were not reached.
- EnumDesc<Material>::convertToItem 0x007761a0 is still unpaired and the InputObject EnumDesc vftables 0x00b5c5a4/0x00b5c5bc are still 0.6 after the LuaArguments.h/LuaEnum.h spelling fix. 2013 InputObject.cpp itself uses 'Reflection/EnumConverter.h': its addPair 0x008b5df0 is the only function with that string. So the remaining cause is elsewhere, probably the folded-alias CSV rows. Name::lookup(const char* const&) is still open.
- Security::Context::ptr 0x0041ceb0 is an LTCG client-shell copy (/GS cookie, inlined thread_specific_ptr ctor), so it cannot match from an App object.
- Not done: VTABLE markers for FWFinal<FWInstance> 0x00b2a8b4 / 0x00bed674 (template class). STUB markers for Allocator<FWInstance>::operator delete 0x0061ba30 / 0x006f0660.
