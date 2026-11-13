# core-datamodel (workflow phase4-core2, wf_fef33d15-1aa)

I changed the layouts of DataModel, ScriptContext, Diagnostics::Countable and the FW classes. PLAYER gained 16 functions at 100% against today's 17:15 baseline and lost 3, none of which my edits caused. STUDIO gained 19 and lost 4, and those 4 come from another agent's PartInstance edits.

Final scores (17:50 builds): PLAYER 3,829 / 40,468 (was 3,828), 1,752 with --nolib (was 1,751). STUDIO 3,693 / 42,329 (was 3,687 at the 17:16 baseline), 1,725 with --nolib (was 1,723).

**Measurement caveats.** Other agents edited PartInstance, PVInstance, Joint, MultiJoint, SoundService, Event.cpp and ModelInstance while I was building. So the totals include their changes; the per-function comparisons below are the ones to trust.
- PLAYER lost 3:
  - protReader 0x00875f10 (99.0%) and luaL_loadprotected 0x00876140 (99.0%): the --verbose diff is one call that now pairs with the `std::deque<ChatLine>::size` twin instead of `getLastCompletedHash`. That is twin-body pairing churn, not a layout change.
  - The ChatLine deque entry 0x0066ef40 is now unpaired.
- STUDIO lost 4 PartInstance UI functions (0x0079f2f0 and the next three). That comes from the other agent's PartInstance.h/.cpp edits made at 17:50.
- I built nothing in build-vt.

**1. Diagnostics::Countable** (Client/Base/include/rbx/Countable.h)
- The ctor now calls RbxInterlockedIncrement(&count) and the dtor calls RbxInterlockedDecrement. Evidence: the calls into 0x009b8f10 / 0x009b8f00 from the Instance ctor (0x00620f05), DataModel ctor/dtor (0x0064ecc2 / 0x0064dda5), SoundChannel ctor/dtor (0x006f2ffc / 0x006ededd), ThreadRef(lua_State*) (0x00673934, before its members) and Job::Job (0x009bac28).
- SoundChannel now derives Countable<SoundChannel>; its hand-written counts in Sound.cpp are gone.
- I deleted every hand-written count in ThreadRef.cpp, ScriptEvent.cpp and ScriptContext.cpp, together with the helper templates. The one exception is the FunctionScriptSlot count, which belongs to no Countable class.
- PLAYER scores, before → after:

| Function | Address | Before | After |
|---|---|---|---|
| ThreadRef(lua_State*) | 0x00673910 | 79.5% | 97.2% |
| LiveThreadRef ctor | 0x008713a0 | 45.1% | 57.1% |
| LiveThreadRef dtor | 0x008714a0 | 39.3% | 61.8% |
| ~WeakThreadRef | 0x00871c40 | 61.3% | 82.5% |
| WeakThreadRef(L) | 0x008723d0 | 42.3% | 47.2% |
| ~SoundChannel | 0x006eddb0 | 77.4% | 84.0% |
| Job::Job | 0x009babf0 | 89.8% | 93.1% |

- The only diff left in ThreadRef's ctor is that RbxInterlockedIncrement 0x009b8f10 is unpaired (it has no marker in Base/rbx/atomic.cpp).
- One score went down: callCallback 0x0087b940, from 23.3% to 12.1%. It is a STUB that was never at 100%; its hand-written count pairs used to stand in for the temporaries.

**2. FW** (Client/App/include/Util/FeatherweightDict.h, plus the two setters in Instance.cpp)
- FWBase is now vfptr, weak_ptr at 4, holder at 0xc, flag at 0x14, ending at 0x18. The `_reserved16` member is gone (FWBase ctor 0x00619740).
- The parent-lock, RobloxLocked and Archivable bytes are FWValue<bool> at 0x18 / 0x19 / 0x1a, with the name at 0x1c (FWInstance ctor 0x0061d7e0). A probe compile confirms these offsets and FWInstance = 0x38. The three bytes are still declared in FWBase, with padding before them, so Instance's accessors through shared_ptr<FWBase> keep compiling. The header records that 2013 has them in FWInstance.
- FWValue<T> is now a value holder with the 2013 member `set(const T&, FWRef*)`, written from 0x0052c000 including both FWUpdate logs. A `// TEMPLATE: PLAYER 0x0052c000` mangled nameref pairs it, and it scores 76.6%. The old static form stays for PartInstance and the name string.
- IFWHolder now has the 2013 slot order from vftable 0x00b79100: destroyFlyweight, valueUpdate, insertCopy, checkIfInDict, checkHash, with no virtual destructor.
- setRobloxLocked 0x0061d780 and setIsArchivable 0x0061d7b0 now follow the 2013 bodies and no longer do the caller check.

| Function | Address | Before | After |
|---|---|---|---|
| setRobloxLocked | 0x0061d780 | 9.8% | 94.1% |
| setIsArchivable | 0x0061d7b0 | 8.0% | 94.1% |
| FWInstance::operator== | 0x00619140 | 81.8% | **100%** |
| FWInstance ctor | 0x0061d7e0 | 52.8% | 61.5% |

- The only diff left in both setters is the unpaired property-descriptor global (propRobloxLocked 0x00de2a04).

**3. ScriptContext** (size 0x288; every member offset confirmed by a probe compile against ctor 0x0068ad20 and dtor 0x0068b1a0)
- The byte at 0x64 is Service::isPublic.
- Members in order: globalStates 0x68, commandLineSandbox 0x70, scripts 0x8c, nextPendingScripts 0x98, pending/loading vectors 0xa0 / 0xb0, runService 0xc0, yieldEvent 0xc8, waitingThreads 0xcc.
- Four bools at 0xf0–0xf3: m_unk0xf0, robloxPlace, scriptsDisabled, contentClearing. Then gcFullStepThreshold 0xf4, gcFrequencyBase 0xf8, lastGcTime 0x100, lastGcKb 0x108, statsItem 0x10c, collectStats 0x114, scriptStats 0x118, reentrancy 0x120.
- The timeout block runs 0x124–0x14c. Then three RunningAverages at 0x150 / 0x180 / 0x1b0, threadsResumedRate 0x1e0 and threadsThrottled 0x220.
- errorSignal 0x250, camelCaseViolation 0x254, libraryService 0x258, heartbeatConnection 0x25c, luaAllocator 0x260, gcJob 0x264, waitingScriptsJob 0x26c, scriptStatsMap 0x274, camelCaseViolationCount 0x280, camelCaseViolationConnection 0x284.
- `allocatorView` is gone; the code uses `luaAllocator` instead. `disabled` and `gcStepThreshold` have no 2013 slot, so they are now statics so that LuaStatsItem.cpp keeps compiling.
- DECOMP_SIZE_ASSERT(RBX::ScriptContext, 0x288) passes.
- I replaced 11 hard-coded Mac assert paths with a local `RBXASSERT_LINE(expr, line)` macro. It has RBX_LOG_ASSERT's shape, takes the path from `__FILE__` and the 2013 line from the PLAYER strings (1373, 1381, 1476, 1490, 1517, 1606, 1651, 1725, 1747, 1768, 2437).
- PLAYER gains at 100%:
  - GcJob ctor 0x00679c00: 99.1% → 100%
  - setRobloxPlace 0x0066fd10: 92.3% → 100%
  - onChangedScriptEnabled 0x006880c0: 75% → 100%
- Other PLAYER improvements: the ctor 23.5% → 59.3%, setTimeout 88.8% → 96.5%, onHeartbeat 62.9% → 74.1%, ypcall 40.1% → 46.4%, onServiceProvider 35.1% → 41.5%. STUDIO moves the same way.

**4. DataModel** (members 0xac8–0xe10 from ctor 0x0064ec50 and dtor 0x0064d850; every offset up to guiBuilder 0xdec confirmed by a probe compile)
- 0xac8–0xaec: lockVerb (an `auto_ptr`), then 8 signals. allowedGearTypeChanged 0xadc, gameLoadedSignal 0xae4 and the 0xad8 signal are proven by their registered events (0xad8 is "GraphicsQualityChangeRequest"). luaExceptionSignal is at 0xae8, which reportError fires.
- 0xaf0–0xbb8: MouseStats 0xaf0. Then 9 shared_ptrs from 0xb70: playerHud, coreGuiService, localBackpack, workspace, runService, starterPack, starterGui, guiRoot, guiRoot2.
- 0xbb8–0xc60: forceArrowCursor 0xbb8, uiMessage 0xbbc, drawId, tempMetric, networkMetric, dirty/isInitialized/isContentLoaded at 0xbe4–0xbe6, and a PhysicsInstructions placeholder at 0xbe8. The counters run 0xc48–0xc54, then isGameLoaded 0xc58, isPersonalServer 0xc59 (getter 0x0062d240), game 0xc5c and networkStatsWindowsOn 0xc60.
- 0xc64–0xd68: hackFlagSetMutex / hackFlagSet 0xc64 / 0xc6c, jobId 0xc84 (getter 0x0062d1e0), write_requested 0xca0, itemChangedSignal 0xcac, requestShutdownCallback 0xcb0, saveFinishedSignal 0xcd0, guiTargetInstance 0xcd4, mouseOverGui 0xcdc, suppressNavKeys 0xcdd, genericJobsLock / genericJobs 0xce0 / 0xcf8, debugLock 0xd50, reporting map 0xd68.
- 0xd74 onward, in the existing tail order: remoteBuildMode 0xd74, then the strings and IDs, guiBuilder 0xdec, parentDataModel 0xe0c.
- The ctor's initializer list now follows 2013: genre, gear and experimental flags are left uninitialized, as the binary leaves them.
- **STUDIO:** the Oct 17 build lacks networkStatsWindowsOn, so its members from 0xc64 to 0xcac sit 4 bytes lower (ctor 0x0076b720). The 8-aligned boost::function at 0xcb0 absorbs the shift. This is a real source difference, recorded in the header. The header follows PLAYER.
- **Size assert not enabled:** DECOMP_SIZE_ASSERT(DataModel, 0xe10) is left commented out. GuiBuilder is 0x24 in our tree against 0x20 in PLAYER and 0x1c in STUDIO, so parentDataModel lands at 0xe10.
- PLAYER gains at 100%: removeCustomStat, writeStatsSettings, setPlaceID, loadGame.
- Other PLAYER changes:
  - ~DataModel: 11.9% → 78.6%
  - ctor: 18.5% → 48.8%
  - startCoreScripts: 78.6% → 92.9% (it reaches 100% in STUDIO)
  - onDescendantRemoving: 91.1% → 96.4%
  - onRunTransition: 63% → 72%
  - doDataModelSetup: 97.3% → 98.8%
  - LuaStatsItem::update: 56% → 67%
- In both targets, two DataModel base vftables and the ServiceProvider base vftable now pair.

**Markers:** I flipped STUB to FUNCTION only where both fresh JSONs (P3/S3) show effective 100%, and only in my files: 8 PLAYER markers and 9 STUDIO markers.
- FeatherweightDict.h: FWInstance::operator==.
- DataModel.cpp: setPlaceID, removeCustomStat, writeStatsSettings, loadGame, plus startCoreScripts in STUDIO only.
- DataModelJob.cpp: the GcJob ctor.
- ScriptContext.cpp: setRobloxPlace, onChangedScriptEnabled.

`app_sweep apply` wanted 13 more flips in other agents' files (PartInstance, Joint and so on); I reverted those. reccmp/*.csv was not regenerated.

**IDA updates to make:**
- **Name these functions:**
  - 0x009b8f10 / 0x009b8f00 RBX::RbxInterlockedIncrement / RbxInterlockedDecrement
  - 0x00641bd0 DataModel::MouseStats::MouseStats()
  - 0x00644f30 GuiBuilder::GuiBuilder
  - 0x0056b810 GuiBuilder::~GuiBuilder
  - 0x0062d240 / 0x0062d250 DataModel::getIsPersonalServer / setIsPersonalServer
  - 0x0062d1e0 DataModel::getJobId
  - 0x0068b1a0 ScriptContext::~ScriptContext (the body; currently sub_68B1A0)
  - 0x006f2fb0 Soundscape::SoundChannel::SoundChannel
  - 0x0084d2f0 the PhysicsInstructions ctor
  - 0x00619a70 FWValue<std::string>::set
- **Fix a wrong name:** 0x005c30c0 is the folded GuiBuilder::Initialize(DataModel*) (stores dataModel at +0); the database calls it RakNet::BitStream::SetWriteOffset.
- **Comment, name unknown:** 0x009bc7d0 / 0x009bc810 register and unregister a watched value. setPlaceID and ~DataModel call them on placeID.
- **Types to import:**
  - FWBase: 0x18
  - FWInstance: 0x38, with the bytes at 0x18 / 0x19 / 0x1a and the name at 0x1c
  - IFWHolder: 5 slots
  - ScriptContext: 0x288, layout above
  - DataModel: 0xe10, layout above
  - DataModel::MouseStats: 0x80
  - RunningAverage<double>: 0x30
  - The STUDIO DataModel variant: 4 bytes lower from 0xc64 to 0xcac.

## Structured fields

```json
{
 "files": [
  "Client/Base/include/rbx/Countable.h",
  "Client/App/include/Util/FeatherweightDict.h",
  "Client/App/v8tree/Instance.cpp",
  "Client/App/include/Util/SoundChannel.h",
  "Client/App/util/Sound.cpp",
  "Client/App/script/ThreadRef.cpp",
  "Client/App/script/ScriptEvent.cpp",
  "Client/App/include/V8DataModel/ScriptContext.h",
  "Client/App/script/ScriptContext.cpp",
  "Client/App/include/V8DataModel/DataModel.h",
  "Client/App/v8datamodel/DataModel.cpp",
  "Client/App/v8datamodel/DataModelJob.cpp",
  "/tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/dmctx/P0.json",
  "/tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/dmctx/P4.json",
  "/tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/dmctx/S0.json",
  "/tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/dmctx/S4.json"
 ],
 "unresolved": [
  "DECOMP_SIZE_ASSERT(RBX::DataModel, 0xe10) is commented out. It waits on GuiBuilder, which I do not own: ours is 0x24 against 0x20 in PLAYER (ctor 0x00644f30: dataModel at +0 via Initialize 0x005c30c0, zeros at +8/+0xc that are probably shared_ptr chatMenu, customStats map at +0x10, networkStatsPage at +0x1c) and 0x1c in STUDIO (no networkStatsPage). With GuiBuilder fixed, parentDataModel moves from 0xe10 to 0xe0c.",
  "The DataModel ctor body (call_once API registrations, hackFlagSet.insert(0x800), ++DataModel::count at 0x00de4c0c, setName(\"Game\"), the parent lock through fw()->b21.set(true, fwRef()), guiBuilder.Initialize(this)) and the dtor body (--count, unregistering placeID through 0x009bc810) are not written. The ctor is at 48.8% and the dtor at 78.6%.",
  "The ScriptContext ctor body (setName(\"Script Context\"), gcFullStepThreshold and gcFrequencyBase from LuaSettings +0x78/+0x7c, the onCamelCaseViolation connection) is not written; it scores 59.3%. Two members have no 2013 slot and are statics so LuaStatsItem.cpp compiles: `disabled` (probably scriptsDisabled at 0xf2) and `gcStepThreshold`.",
  "The LuaArguments.h assert literals ('include/Script/LuaArguments.h' lines 146 and 178) live in Client/App/script/LuaArguments.cpp, which I do not own. In 2013 those bodies were inline in include/script/LuaArguments.h, so the right fix is to move getValues and the luaIndex>0 body into the header at those lines. __FILE__ in the .cpp cannot produce the header path.",
  "ThreadRef.cpp still holds 17 Mac literal paths. They cite LuaSignalBridge.cpp, LuaInstanceBridge.cpp and ThreadRef.cpp, so the bodies belong in other files before __FILE__ can produce them.",
  "Three PLAYER functions are short of 100% only because a reference is unpaired. setRobloxLocked and setIsArchivable (94.1%) need propRobloxLocked 0x00de2a04 and propArchivable 0x00de2da8 paired, as GLOBAL markers in Instance.cpp or in the CSVs. ThreadRef(lua_State*) (97.2%) needs RbxInterlockedIncrement 0x009b8f10 paired (no marker in Base/rbx/atomic.cpp).",
  "The CSVs need regenerating (Integrate): the new FWValue<bool>::set member makes the player-symbols.csv row for 0x0052c000 pairable by name. For now it pairs through the TEMPLATE mangled nameref in FeatherweightDict.h. Twin churn also cost 3 PLAYER entries (protReader 0x00875f10, luaL_loadprotected 0x00876140, the ChatLine deque 0x0066ef40).",
  "The FWInstance bytes b21/b22/b23 are still declared in FWBase, after padding, so Instance.h/Instance.cpp/Workspace.cpp keep compiling. 2013 declares them in FWInstance; moving them needs edits to Instance.h, Instance.cpp and Workspace.cpp, which belong to other agents.",
  "STUDIO lost 4 PartInstance UI functions (0x0079f2f0, 0x0079f330, 0x0079f370, 0x0079f390). That follows the other agent's PartInstance.h/.cpp edits made at 17:50, which my build picked up; they should check it.",
  "DataModel names not proven by a 2013 function and taken from the 2016 slot in the same position (marked src2016): the screenshot signals, mouseStats, drawId, tempMetric, networkMetric, isContentLoaded, physicsStepID, guiTargetInstance, mouseOverGui. Unknown: m_unk0xae0 (a void(const UIEvent&) signal fired by processUiEvent) and the PhysicsInstructions placeholder at 0xbe8.",
  "DataModel::MouseStats is placed by its out-of-line ctor 0x00641bd0, but I did not mark that ctor. ThreadRef.h still carries a stale comment about the manual count++. I left it alone to avoid another full rebuild."
 ]
}
```

