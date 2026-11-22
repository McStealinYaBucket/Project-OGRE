# datamodel-5b-3 (phase5-wave3, 2026-11-14)

PHASE 5b THIRD PASS AND BASE (datamodel-5b-3). All builds were in build-dm (PLAYER) and build-dm-studio (STUDIO). I compared against the baseline JSONs build/sweep8/P-final.json and build-studio/sweep8/S-final.json. Final runs: build-dm/w3/P3.json (exe/pdb/map copies in build-dm/w3/scored-P3/) and build-dm-studio/w3/S2.json (copies in build-dm-studio/w3/). The S2 build started before my last batch of edits (Log.h vftable markers, the Lighting/Crypt TEMPLATE markers), so those were not rebuilt into the STUDIO run. Markers in my files were flipped FUNCTION/STUB per those runs.

LEFTOVERS: all of my leftover edits build in both targets and are kept. Two headers the earlier agent had converted from CRLF to LF were put back to CRLF (TaskScheduler.Job.h, boost.hpp). The earlier agent's unmeasured FUNCTION markers in Base were all measured at effective 100%; none had to be downgraded. My builds hit two compile errors in other agents' files on the way (GuiObject.cpp, HumanoidStage.cpp); both were fixed by their owners before my final builds.

RESULTS (PLAYER):
- Row 5b: 447/1605 (27.9%) before, 490/1605 (30.5%) after.
  - DataModel.cpp 276 -> 308; Service.cpp 5 -> 8; Lighting 3 -> 4; RunService 6 -> 8; Teams 15 -> 17; Workspace 15 -> 17; GuiBuilder 25 -> 27; Debris 5 -> 6.
  - Team.cpp 19 -> 18: PropDescriptor<Team,BrickColor>::GetSetImpl 0x006aa1b0 is no longer paired. I did not touch Team; it looks like a template fold from another agent's edits.
- Base block (0x009b1000-0x009bd680, 292 IDA functions): 111 before, 185 after at 100%. STUDIO Base block (0x00a68000-0x00a74700, 307 functions): 122 before, 183 after.
- In my files' address ranges: 124 PLAYER functions gained 100% against the baseline, 5 lost (none from my edits, see below). Whole exe, effective 100%: 7,784 -> 7,994. That total includes every other agent's concurrent changes.

NEW CODE, written from the bytes:
- Base/rbx/Log.cpp (2016 Base/util/Log.cpp read for intent only):
  - formatMem 0x009b0f50 (11.3 -> 100) and formatTime 0x009b10e0 (29.2 -> 100).
  - timeStamp 0x009b15e0 (16.9 -> 100).
  - Log ctor 0x009b1740 and dtor 0x009b1800 (new -> 100 in both targets).
  - writeEntry(Severity, const char*) 0x009b1890 and writeEntry(Severity, const wchar_t*) 0x009b1920 (97.8).
  - Log.h gets VTABLE markers 0x00bb8800 / 0x00c784d0, plus TEMPLATE markers for c_time::localtime 0x009b1240 and second_clock::create_time 0x009b12b0 (both 100).
- New Client/Base/rbx/Tasks/Coordinator.cpp: SequenceBase::isInhibited 0x009bb710, advance 0x009bb760 and onRemoved 0x009bb7a0 are 100 in both targets. onAdded 0x009bb9e0 is 97.9: our linker folds its vector<Job*>::_Insert_n with the CallbackDescriptor* copy.
- Markers added on existing code:
  - RbxDbgInfo setters 0x009bc860/880/8a0/8c0/8e0 (STUDIO 0x00a72c50..0x00a72cd0): 100.
  - Crypt::verifySignatureBase64 0x009bcbc0 (STUDIO 0x00a73420): 100.
  - ATL::Base64Decode TEMPLATE 0x009bc940: 100.
  - ServiceProvider::findServiceByClassName 0x006a9360: 96.9 -> 100, with the 2013 ::iterator form.
  - findPublicServiceByClassNameString 0x006a93d0 and getPublicServiceByClassNameString 0x006a9b40: new -> effective 100.
  - Lighting::getTimeStr 0x00706190 (STUDIO 0x007d1d30): 100, via a TEMPLATE marker for to_simple_string 0x00706150. The iOS name list puts getTimeStr's name on 0x00706150, which is really to_simple_string.
  - threadsafe.h TEMPLATE head_waittime 0x0063f200 / 0x0075ca90.
- From the leftovers, now measured:
  - 0x009b3880-0x009bae40 (TaskScheduler and Job): about 40 functions or templates from -100 to 100, including reschedule, areExclusive, enqueueWaitingJob, isLowerWakeTime, getJobsInfo, getJobsByName, disableThreads, setThreadCount, shouldDropThread, notifyCoordinatorsPreStep and isDisabled.
  - CEvent ~CEvent and Wait; PerfCounter/CProcessPerfCounter (5); RegistryUtil (6, were unscored); roblox_allocator::malloc; Crypt::~Crypt.
  - DataModel::GenericJob::error 70.4 -> 100 (head_waittime()).
  - DataModel::loadWorld 0x00640430 95.9 -> 100 (no workspace NULL check).
  - The DataModel/GuiBuilder/Workspace/Teams/RunService TEMPLATE twins (about 35).
  - DataModelJob::step 23.9 -> 72.4.
  - FLog::GuiTargetLifetime is now defined: STUDIO 0x00f84e1f, verified from the initializer at 0x00b1c390.

LOSSES I CHECKED (none from my edits):
- About 200 PLAYER vftables at 92.9 and Instance::canClientCreate/onServiceProvider 0x00402730/0x00402740: the shell trivial-body copies (shell-5k's ClientBase changes).
- STUDIO init_foo and sp_pointer_construct: they now fold with ClassRegistrar<CRenderSettingsItem>::~ (shell-5k's RenderSettingsItem.cpp).
- BillboardGui RefPropDescriptor (gui-5h).
- BoundFuncDesc<DataModel> D0s 0x0063d4c0/0x0063d530: CallbackDescriptor vs FunctionDescriptor dtor fold, from reflection edits by another agent.
- EnumPropDescriptor<DataModel,GearGenreSetting>: the convertToIndex pairing changed, which I did not cause.
- RakNet IsOlderOrderedPacket: a coincidence.
- Two intrusive-list erase twins swapped which copy pairs (PLAYER 0x008e5e80 -> 0x009b3d20, STUDIO 0x00a6bdc0 -> 0x00a6bd90), net 0.

CHECKS: tools/reccmp_addr_padding.py reports 0 badly padded markers.

## Files

- /home/thebigw/Documents/Project OGRE/Client/Base/rbx/Log.cpp
- /home/thebigw/Documents/Project OGRE/Client/Base/include/rbx/Log.h
- /home/thebigw/Documents/Project OGRE/Client/Base/rbx/Tasks/Coordinator.cpp
- /home/thebigw/Documents/Project OGRE/Client/Base/rbx/RbxDbgInfo.cpp
- /home/thebigw/Documents/Project OGRE/Client/Base/rbx/Crypt.cpp
- /home/thebigw/Documents/Project OGRE/Client/Base/include/rbx/threadsafe.h
- /home/thebigw/Documents/Project OGRE/Client/Base/include/rbx/TaskScheduler.Job.h
- /home/thebigw/Documents/Project OGRE/Client/Base/include/rbx/boost.hpp
- /home/thebigw/Documents/Project OGRE/Client/Base/include/rbx/TaskScheduler.h
- /home/thebigw/Documents/Project OGRE/Client/Base/include/rbx/Limits.h
- /home/thebigw/Documents/Project OGRE/Client/Base/rbx/TaskScheduler.cpp
- /home/thebigw/Documents/Project OGRE/Client/Base/rbx/TaskScheduler.Job.cpp
- /home/thebigw/Documents/Project OGRE/Client/Base/rbx/TaskScheduler.Thread.cpp
- /home/thebigw/Documents/Project OGRE/Client/Base/rbx/CEvent.cpp
- /home/thebigw/Documents/Project OGRE/Client/Base/rbx/Memory.cpp
- /home/thebigw/Documents/Project OGRE/Client/Base/rbx/boost.cpp
- /home/thebigw/Documents/Project OGRE/Client/App/v8tree/Service.cpp
- /home/thebigw/Documents/Project OGRE/Client/App/include/V8Tree/Service.h
- /home/thebigw/Documents/Project OGRE/Client/App/include/V8DataModel/Lighting.h
- /home/thebigw/Documents/Project OGRE/Client/App/v8datamodel/Lighting.cpp
- /home/thebigw/Documents/Project OGRE/Client/App/v8datamodel/DataModel.cpp
- /home/thebigw/Documents/Project OGRE/Client/App/include/V8DataModel/DataModel.h
- /home/thebigw/Documents/Project OGRE/Client/App/v8datamodel/DataModelJob.cpp
- /home/thebigw/Documents/Project OGRE/Client/App/include/V8DataModel/DataModelJob.h
- /home/thebigw/Documents/Project OGRE/Client/App/v8datamodel/FastLogSettings.cpp
- /home/thebigw/Documents/Project OGRE/Client/App/v8datamodel/Workspace.cpp
- /home/thebigw/Documents/Project OGRE/Client/App/include/V8DataModel/Workspace.h
- /home/thebigw/Documents/Project OGRE/Client/App/include/V8DataModel/GuiBuilder.h
- /home/thebigw/Documents/Project OGRE/Client/App/include/V8DataModel/RunService.h
- /home/thebigw/Documents/Project OGRE/Client/App/include/V8DataModel/Teams.h
- /home/thebigw/Documents/Project OGRE/Client/App/include/V8DataModel/DebrisService.h
- /home/thebigw/Documents/Project OGRE/tools/row_coverage.py
- /home/thebigw/Documents/Project OGRE/build-dm/w3/P3.json
- /home/thebigw/Documents/Project OGRE/build-dm/w3/scored-P3
- /home/thebigw/Documents/Project OGRE/build-dm-studio/w3/S2.json

## Unresolved

- Integrate (link stubs): Client/_generated/player/stubs_00.cpp still defines ogre_stub_009b1890 with its own STUB marker, so reccmp drops my marker on the real Log::writeEntry(Severity, const char*) 0x009b1890 as a duplicate address. Regenerate the link stubs (STUDIO 0x00a68980 likewise); writeEntry(wchar_t) 0x009b1920 (97.8) then pairs its call.
- Instance.h owner: Instance::fw() is out of line in 2013 (0x006beaa0). It is the remaining diff in the DataModel ctor 0x0064ec50 (97.3) and in arbiterName 0x0064de40 (81.5).
- Structural, cannot fix from my files: ~DataModel 0x0064d850 (99.76), ~TaskScheduler 0x009b4e00 (98.6; was 99.5 before network-5f's Item intrusive list joined the fold) and TaskScheduler::remove 0x009b52b0 (99.49) differ only in calls to a shared_ptr/_Destroy_range/trivial body. In the original those are native ICF copies (for example 0x007e4e40 and 0x00829d90); ours fold into boost program_options' copy, because boost_po is native in our build and LTCG in the original.
- TaskScheduler ctor 0x009b55e0 (97.8): (1) its member at +8 calls a second atomic ctor class, 0x005ba6f0 (assert atomic.h:182), whose type is unknown (no symbol), and I did not invent one; (2) the original stores two byte temporaries ([esp+0x4e]/[esp+0x4f]) around the intrusive-list inits; (3) the RunningAverageDutyCycle<2> ctor and the sampleRunningJobCount assign_to calls are unpaired.
- Integrate (CSV): Crypt ctor 0x009bca80 (98.9): the function-local keyB64 GLOBAL marker (0x00db4b60) gets the orig label RBX::keyB64 but our PDB name is keyB64, so the data operand does not pair.
- Integrate (CSV): the iOS name on 0x00706150 (Lighting::getTimeStr) belongs to 0x00706190; 0x00706150 is boost::posix_time::to_simple_string (94.4, its call to to_simple_string_type<char> is unpaired).
- Security owner: DebrisService::setMaxItems 0x00844e90 pushes permission 2 where our enum's LocalUser is 3 (the 2013 Security enum probably lacks a value). Still 98.2.
- network-5f owner: PhysicsInstructions::changeSimulationRadius 0x0084cb70 (assert line 55) and changeMaxSimulationRadius 0x0084cbe0 need Network::Player's simulation radius at +0xcc and updateSimulationRadius 0x0052e510 in Player.h.
- Still blocked: Workspace (2013 World* raw pointer at +0x148, not a shared_ptr; needs the ModelInstance/RootInstance bases), so joinAllHack/reset/assemble stay STUB. Lighting setters 0x007067e0..0x00706b00 need the 0x64..0x1a0 member layout (Color4 at 0x164/0x174, floats at 0x15c) and the reflection PropDescriptors. StarterGuiService needs the BasePlayerGui chain. GuiBuilder::getVerb 0x007e4d40 needs its assert on line 116 and Workspace's VerbContainer at +0x150. HeartbeatTask is a DataModelJob in 2013 (+0x228).
- Limits (App/util/Limits.cpp, owner unclear): Counter::Activator 0x009b96b0 (44%) and ~Activator 0x009b97f0 (92.9). DataModelJob::step 0x0070ccb0 (72.4) also needs the shared_dynamic_cast<DataModelArbiter> template 0x004fb420 paired.
- I did not run decomplint, vtable or datacmp this pass; addr padding is clean. The final STUDIO run (S2) does not include my last Log.h/Lighting.h/Crypt marker additions; a STUDIO rescore by Integrate will cover them.
