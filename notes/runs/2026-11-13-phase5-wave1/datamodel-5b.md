# datamodel-5b (workflow phase5-wave1, wf_322cba92-6a4)

I finished Phase 5b's first pass on DataModel and the core services. 16 PLAYER and 11 STUDIO functions in my files now score effective 100%, and their markers are flipped to FUNCTION. Neither target loses a 100% function to my changes.

**Measurement.** Builds were in build-dm and build-dm-studio. PLAYER final JSON: scratchpad/dm5b/P7.json; STUDIO: scratchpad/dm5b/S2.json.
- **Effective-100% entries:** PLAYER 3,646 → 3,725; STUDIO 3,527 → 3,590. These totals also include other agents' concurrent work (mostly ContentProvider).
- **Row 5b coverage (PLAYER):** 129 → 145 of 1,372 functions (9.4% → 10.6%). I extended `tools/row_coverage.py` with `--row 5b`: it lists the row's 15 files and allows a 0xa000-byte gap when finding a file's .obj range from its markers, because DataModel.obj spans 0x0062d1e0–0x00650660. Before/after are measured on the same ranges.
- **Per file (PLAYER, after):** DataModel.cpp 114/797, Team.cpp 2/23, Players.cpp 2/65, Service.cpp 5/8, DataModelJob.cpp 7/9.
- **Function inventory:** DataModel has 116 oracle-named functions; 43 were at 100% before, 18 had no body, the rest were partial. Team, Teams, Lighting, Debris, StarterGui, Player and RunService had almost no bodies; Workspace 0/30 at 100%. Full per-function lists are in scratchpad/dm5b/dm-funcs.txt and c-*.txt.
- **Losses checked with --verbose:** the only lost 100% entries are 5 boost library rows in each target (tss_data_node map/_Tree members, a vector `_Buy`, a `_Tree_nod` dtor). They're gone because another agent added `BOOST_THREAD_DYN_LINK` to CMakeLists, so those boost bodies are no longer linked; the 2026-11-13 03:23 PLAYER map has 79 `tss_data_node` symbols, ours has 0.
- **Checks:** `reccmp_addr_padding.py` 0 bad. `decomplint` reports only `function_out_of_order`, where the 2013 assert lines force the source order.

**Functions raised to 100% (PLAYER / STUDIO, before → after):**
- **DataModel line layout:** I moved functions and padded blank lines so their asserts land on the 2013 lines.
  - httpGetAsync 0x0064a350 / 0x00766f40: 98.9 → 100 / 98.9 → 100
  - httpPostAsync 0x0064a870 / 0x00767460: 98.7 → 100 / 98.7 → 100
  - onChildChanged 0x0063e180: 96.0 → 100 (PLAYER only; STUDIO's 2013 lines differ)
  - onDescendantRemoving 0x00645e00: 96.4 → 100 (PLAYER only)
  - onDescendantAdded 0x00645d50: 58.4 → 100. Added its missing `RBXASSERT(write_requested==1)`; STUDIO 94.4.
  - getGenericJob 0x00630d70 / 0x00750a00: 24.8 → 100. Rewritten as lock + array read + `RBXASSERT(false)` + throw, at line 1150.
  - onRunTransition 0x0062ef20: 72.2 → 100. Removed the invented RunService mirror store and the workspace NULL checks; STUDIO 93.4 (its lines are one lower).
- **DataModel body fixes:**
  - closeDataModel 0x00649af0 / 0x007666e0: 88.3 → 100 / 88.3 → 100. 2013 log strings, early return, and a STUB marker on the LegacyLock ctor 0x006421c0 / 0x0075fa40 so its call pairs.
  - startCoreScripts 0x006400f0: 92.9 → 100. Added a marker for its VMProtect-touched callee loadCoreScripts 0x0063fbb0 / 0x0075d440.
  - setUiMessageBrickCount 0x0062d270 / 0x0074d590: no body paired → 100. Its marker had pointed at renderGuiRoot 0x0062ee30.
  - onChildAdded 0x0062ee00 / 0x0074eb20: 0 → 100. Written as `ServiceProvider::onChildAdded`, then `Players::isNetworkClient` → `workspace->setNullMouseCommand()`.
  - doDataModelSetup 0x0064f7f0 / 0x0076c2a0: 98.8 → 100. PLAYER stores `LegacyLock::mainThreadId = GetCurrentThreadId()` (the Mac build doesn't); GLOBAL markers added at 0x00d314c8 / 0x00ebd70c.
- **Other files:**
  - ServiceProvider::createChild 0x006a9650 / 0x0076f690: 77.5 → 100. 2013 calls `Creatable<Instance>::createByName` directly.
  - Players::frontendProcessing 0x00568900 / 0x005ccb00: 85.1 → 100. Assert text `serviceProvider!=NULL` and lines 182/189; both functions moved to the end of Players.cpp.
  - Team::getTeamColor 0x006aa0d0 / 0x00826c50 and Team::getAutoAssignable 0x006aa100 / 0x00826c80: new → 100.

**Raised but not 100% yet (PLAYER, STUDIO in brackets):**
- DataModel ctor 0x0064ec50: 65.0 → 95.5 (64.1 → 93.8).
  - `addHackFlag` is now `__forceinline` on WIN32, as in 2016.
  - `isContentLoaded` (+0xbe6) is now `volatile`; private-compile probes showed this is the only change (of five tried) that puts the 0xbe4/0xbe5 stores before the EH-state store.
  - `FLog::CloseDataModel`, `LegacyLock`, `CoreScripts` and `DataModelJobs` are now defined in a new `Client/App/v8datamodel/FastLogSettings.cpp`, written out with GLOBAL markers at their 2013 addresses (0x00d30f17, 0x00de3f76, 0x00d30f1a, 0x00d30ee9). Until then they were unresolved and bound to one address, which made different channels compare equal.
  - What's left: `Instance::fw()` is out of line in 2013 (0x006beaa0, 194 call sites) but inline in Instance.h, plus pairing-only calls.
- setPlaceVersion 0x0062d910: 6.9 → 95.5 (added the 2013 FASTLOG; what's left is the static `prop_placeVersion` at 0x00de4870, which doesn't pair).
- GcJob::stepDataModelJob 0x00679e60: 87.9 → 97.8. WaitingScriptsJob::stepDataModelJob 0x0068a4e0: 89.5 → 98.9. Both now use `dynamic_cast<DataModel*>(sc->getParent())`.
- Players::backendProcessing 0x005689c0: 83.7 → 98.6.
- New, pairing-only so far:
  - Team (new layout: score +0x60, color +0x64, autoAssignable/autoColorCharacters +0x68/+0x69, size 0x6c):
    - ctor 0x006aa970: 19.2 → 90.2
    - setters 0x006aa0b0 / 0x006aa0e0 / 0x006aa110: 85.7
    - askSetParent 0x006aa8b0: 96.7
  - Teams (`DescribedCreatable<…, PERSISTENT_HIDDEN>` + Service, `copy_on_write_ptr<Instances> teams` at +0x64, size 0x6c):
    - ctor 0x007a4f10: 88.6
    - ~Teams 0x007a44f0: 85.1
    - askAddChild 0x007a4610: 96.7
  - Players::isNetworkClient 0x0056b8c0: 96.7
  - Workspace::setNullMouseCommand 0x00710080: STUB body only, so onChildAdded's call has a target.

**Written, then reverted:** Teams::onChildAdded 0x007a4790 (96.8%) and onChildRemoving 0x007a48c0 (96.1%). Instantiating `shared_from<Team>` changed which folded `shared_from<>` copy our linker names, and Instance::onAncestorChanged, Instance::onDescendantAdded and DataModel::onChildChanged lost their call pairing. The bodies are kept in scratchpad/dm5b/Teams.cpp.withhooks, and a comment in Teams.cpp records why they're out.

**What blocks the rest:**
- **Pairing only (Integrate's CSV regeneration):** new vftables, `Described<>` / `FactoryProduct<>` template ctors, static property descriptors, signal ctors. This covers the Team/Teams functions, setPlaceVersion, isNetworkClient and the DataModel ctor's remaining template calls.
- **Headers I don't own:** Instance.h `fw()` out of line; Object.h `className()` inline form, which keeps 24 `ServiceProvider::find<T>` instances at 87.3%; threadsafe.h `head_waittime()` should return an Interval with no time argument (GenericJob::error 70.4%, processTasks 83.4%); the Variant/`placement_any` holder dtor slot (getJobsExtendedStats 94.2%); PhysicsSettings is missing (PhysicsInstructions ctor 83.8%).
- **Workspace layout:** the 2013 Workspace derives the ModelInstance/RootInstance chain (VerbContainer at +0x150, world at +0x148, size 0x278). That blocks completeShutdown 96.2%, loadWorld 95.9% and GuiBuilder::getVerb 75%, and needs ModelInstance's members first.
- **Larger classes not started:** RunService (2013 layout from ctor 0x0072efd0 is in my notes; the six signal types are still unknown), Lighting (0x1f0 with G3D::LightingParameters), DebrisService (layout equals 2016, but TimerService doesn't exist in the tree), StarterGui/StarterPack (need the GuiItem/Hopper chain), Player.

## Files changed

- Client/App/v8datamodel/DataModel.cpp
- Client/App/include/V8DataModel/DataModel.h
- Client/App/v8datamodel/DataModelJob.cpp
- Client/App/v8tree/Service.cpp
- Client/Network/Players.cpp
- Client/Network/include/Network/Players.h
- Client/App/v8datamodel/Team.cpp
- Client/App/include/V8DataModel/Team.h
- Client/App/v8datamodel/Teams.cpp
- Client/App/include/V8DataModel/Teams.h
- Client/App/v8datamodel/Workspace.cpp
- Client/App/include/V8DataModel/Workspace.h
- Client/App/include/V8DataModel/RunService.h
- Client/App/v8datamodel/FastLogSettings.cpp
- tools/row_coverage.py

## Unresolved

- Instance.h owner: Instance::fw() is non-inline in 2013 (0x006beaa0, called 194 times); making it out-of-line takes the DataModel ctor 0x0064ec50 from 95.5% toward 100%.
- Object.h owner: NonFactoryProduct/Creatable::className() in 2013 is `if (!sClassName) getNullName(); else { call_once(callDoDeclare); doDeclare(); }` (find<AssetService> 0x00639abe-0x00639ae0); this blocks the 24 ServiceProvider::find<T> instances at 87.3%.
- Client/Base threadsafe.h owner: in 2013 timestamped_safe_queue::head_waittime() returns a Time::Interval through a hidden pointer and takes no timeNow argument. This blocks DataModel::GenericJob::error 0x0063f3c0 (70.4%) and processTasks 0x0063f4e0 (83.4%).
- Integrate: regenerate the CSVs. These new functions differ only in vftable, template-ctor, static-descriptor or signal pairing: Team ctor/setters/askSetParent, Teams ctor/dtor/askAddChild, setPlaceVersion (prop_placeVersion 0x00de4870), Players::isNetworkClient, the DataModel ctor's template calls. The 5+5 lost boost library rows (tss_data_node map/_Tree, vector _Buy 0x008b3120, _Tree_nod 0x008db820; STUDIO 0x005b88e0/0x00628b70/0x0066ee90/0x00987a10/0x00a15430) come from BOOST_THREAD_DYN_LINK now in CMakeLists.txt, not from my files.
- Players::clientIsPresent 0x005adc70 lives in Client.cpp in 2013 (between Client::findClient and Client::onServiceProvider); its body is `return Client::findClient(context, testInDatamodel) != NULL`. Ours sits in Players.cpp and is inlined into backendProcessing (98.6%). Moving it needs an edit to Client.cpp, which I don't own.
- Teams::onChildAdded/onChildRemoving (0x007a4790/0x007a48c0, 96.8%/96.1%) are written but kept out: instantiating shared_from<Team> changes which folded shared_from<> copy our linker names, and 3 PLAYER plus 1 STUDIO 100% functions lose call pairing. A CSV row for the shared_from<> twin would let them back in (bodies in scratchpad/dm5b/Teams.cpp.withhooks).
- Workspace's 2013 bases (ModelInstance/RootInstance chain, VerbContainer at +0x150, world at +0x148, size 0x278) block completeShutdown 0x00641c90 (96.2%), loadWorld 0x00640430 (95.9%, joinAllHack 0x0070ebd0 has no NULL check), and GuiBuilder::getVerb 0x007e4d40. The getVerb assert should be `workspace` at GuiBuilder.cpp:116. ModelInstance members must be laid out first (not mine).
- PhysicsInstructions ctor 0x0084d2f0 (83.8%) needs GlobalAdvancedSettingsItem<PhysicsSettings>::singleton()+0x68 for +0x40; PhysicsSettings is not in the tree (not mine). WaitingScriptsJob ctor 0x00679f90 needs LuaSettings+0x84.
- getJobsExtendedStats 0x0064c180 (94.2%): the 2013 Variant/placement_any holder dtor goes through vtable slot +4, ours through +0 (reflection Variant, not mine). DataModelArbiter::preStep/postStep 0x0070cbd0/0x0070cc10 need the base Arbiter meter to call RbxInterlockedIncrement/Decrement out of line, plus the FFlag-gated OnScreenProfiler hook.
- Not started: RunService (2013 ctor 0x0072efd0: physicsJob +0x64, heartbeatTask +0x6c, runState +0x74, four doubles +0x78..+0x90, six signals +0x98..+0xac whose types are unidentified; stopTasks 0x0072bb60; onServiceProvider 0x0072bc30 is just `if (old) stopTasks()`), Lighting (0x1f0, G3D::LightingParameters at +0x68), DebrisService (2016 layout matches, size 0x94, but TimerService is missing), StarterGui/StarterPack (need the GuiItem/RelativePanel/TopMenuBar/Hopper chain), Player, GuiBuilder bodies.
- DataModel.cpp: initializeContents (872), the LegacyLock::Implementation ctor (1054; its 2013 signature takes a third shared_ptr<Limits::Counter> from dm+0x8d8), loadContent (1406), get(ContentId) (1445) and renderPass* need deeper rewrites before their line layout pays off. STUDIO's tail asserts sit at 3044/3050/3061, so onChildChanged/onDescendant* can only match PLAYER from one file.
