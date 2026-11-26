# Wave 18: App and Base follow-ups (app-18)

- **Owner and scope:** Client/App and Client/Base (no Base file changed). One edit in App for Network (NetworkSettings.h, below). No commits, no IDA database opened, the shared reccmp/ not written, nothing under Client/Rendering/ touched.
- **Build dirs:** build-ap/ (PLAYER) and build-ap-studio/ (STUDIO). Logs, helper scripts and comparison lists are in build-ap/w18/.
- **How I measured:** private root build-ap/w/root (tools/, reccmp/ and reccmp-project.yml refreshed from HEAD f709dfd). Snapshots build-ap/w/snaps/V6 (PLAYER) and SV2 (STUDIO); for each, two reccmp runs, `gen_reccmp_csv.py` from that pair into the private root, then two more runs (V6c/d/e, SV2c/d/e), all equal.
- **Every link deleted the .pdb first** (see item 1d: link.exe keeps stale type records in an existing PDB).
- **reccmp-reccmp crashes on the current shared tree** (`assert element.size` in type_layout.get_name_for_offset). The cause is a zero-size element type `ATL::CRegParser::VTFromRegType::__l2::typemap` (an array of an incomplete local struct), which arrived with the shell agent's uncommitted code. I scored through build-ap/w18/rc.py, a wrapper that catches only that assert and names the offset `+N`. The integrator needs the same (or the shell fix) before scoring.

## Figures (effective 100%, against sweep24)

| | sweep24 | this wave (V6e / SV2e) | lost |
|---|---|---|---|
| PLAYER | 12,766 | **12,815** (+49) | 6: Lua no-.reloc immediates only |
| STUDIO | 12,014 | **12,052** (+38) | 10: see below |

- **Shared-tree caveat:** the tree also held the network and shell agents' uncommitted work (Replicator, ConcurrentRakPeer, Teleporter, MainLogManager, ...), so the totals include some of their gains.
- **Lists:** build-ap/w18/V6e-vs-sweep24.txt and SV2e-vs-sweep24.txt.
- **App-attributable gains:** about 40 PLAYER and 40 STUDIO entries. These are LegacyLock ×2 and Implementation dtor plus its two static-init helpers, Game ×8, the signal<void(DataModel*)> helpers, LuaAllocator ×6 + vftable, setRunState, getBaseUrl/setBaseUrl, openScriptLibrary + its helper, panic, the classDescriptor copies, YieldingThreads ctor, wait/tick/rbxTime/load, SingletonBridge push and fastDynamicCast<OverlayDataModel>.
- **Losses, each read with --verbose:**
  - **Both targets:** symbexec, luaV_execute, constructor, patchtestreg, removevalues, luaK_setreturns. These are Lua immediates (`and edx,0x7fc000`) read as our data addresses: no-.reloc coincidences.
  - **STUDIO only:** `list<rbx::signals::connection>::_Buynode` 0x006d13d0 (App's Http.h TEMPLATE row) and the three bodies that call it (chain_base ctor 0x0068db20, GuiService list ctor 0x00859be0, map ctor 0x0085c890).
    - Our only copy now comes from the shell agent's modified `Client/ClientShared/placeholder.cpp` (map: 0x00427210 placeholder.cpp.obj; it was NetworkPacketCache.obj at sweep24).
    - That copy now pairs with the shell original 0x0049de90 (gained), and the engine twin loses.
    - This is the trade-off the shell agent chose. It is not App code.
  - **Intermediate links lost the prime_list one-past-end constructors** (PLAYER DataModel ctor, packet caches, LRUCache, ContactManager, ...). The cause was "circular_buffer" data following prime_list in ConcurrentRakPeer.obj (network agent). The final link does not have them.

## Changes

### 1. FactoryRegistration, comments, the 0x20 GuiBuilder, setRunState
- **ClassRegistrar<ControllerService>::registrar** is defined (it was unresolved data in both links):
  - **Addresses:** PLAYER 0x00e0c968 / STUDIO 0x00f8330c. Each is the operand of the `inc` in DescribedNonCreatable<ControllerService>'s ctor (PLAYER 0x006e73b3, STUDIO 0x00814433).
  - **2013 initializes it in FactoryRegistration.obj** (PLAYER initializer 0x00a75820, between ClickDetector and ChatService; .bss run 0x00e0c7ac..0x00e0ce14). The run also holds FriendService's registrar (initializer 0x00a76040, PLAYER 0x00e0c7ac / STUDIO 0x00f8314c).
  - **The two targets keep different classDescriptor copies:**
    - PLAYER keeps UserController.obj's and FriendService.obj's (0x006e6e90 and 0x006afe70 push the literal). PLAYER's FactoryRegistration.obj links after both.
    - STUDIO keeps FactoryRegistration.obj's (0x00736940 and 0x00734740 read the s-name from data).
  - **So the registrar definitions are per target:**
    - PLAYER: UserController.cpp and FriendService.cpp, `#ifndef OGRE_TARGET_STUDIO`.
    - STUDIO: FactoryRegistration.cpp, `#ifdef OGRE_TARGET_STUDIO`, which also gates its UserController.h/FriendService.h includes.
  - **Result:**
    - STUDIO `Described<FriendService>::classDescriptor` 0x00734740 0.86 → 100. STUDIO ControllerService classDescriptor 0x00736940 → 100.
    - PLAYER 0x006e6e90 and 0x006afe70 stay at 100.
    - **Rule:** when PLAYER and STUDIO keep different .obj copies of one COMDAT, put the definition per target in the .obj whose copy that target kept.
- **0xe0c comments:** DataModel.h's parentDataModel now says "0xe0c (STUDIO 0xe08, ctor 0x0076bb1a; the class pads to 0xe10 in both)", and GuiBuilder.h says DataModel is 0xe10 in both.
- **The 0x20 GuiBuilder record is not a TU. It is a stale PDB.**
  - link.exe reuses an existing .pdb and keeps the type records of objects that are no longer linked.
  - **Evidence:**
    - build-studio's PDB holds the strings `v8datamodel\PhysicsJob.cpp`. That source is gone, and its stale `PhysicsJob.cpp.obj` (Nov 22, the only .obj with a 0x20 `RBX::GuiBuilder` record) is not in build.ninja.
    - Every STUDIO TU is compiled with -DOGRE_TARGET_STUDIO.
    - With the PDB deleted before the link, build-ap-studio's PDB has only the 0x1c record (`grep -a -P '\x20\x00RBX::GuiBuilder\x00'` = 0).
  - Fix for the integrator / IDA owner: delete RobloxStudioBeta.pdb / RobloxPlayerBeta.pdb before each scoring link (or add it to the build), then re-export types.
- **RunService::setRunState takes `RunState`** (iOS signature). The body is rewritten from the 2013 bytes:
  - It branches RUNNING → PAUSED → STOPPED.
  - It stores totalWallTime, totalGameTime, totalGameTimeAtLastHeartbeat, skippedTimeAccumulated in that order.
  - Result: FUNCTION in both targets (PLAYER 0x0072db60 0.56 → 100, STUDIO 0x0079b7e0).
  - RunService.h has a TEMPLATE marker for `signal_with_args<1, void(RunTransition)>::operator()` (PLAYER 0x0072d780, STUDIO 0x0079b400; 0.63, a shell-style twin).

### 2. Game (v8datamodel/Game.cpp, V8DataModel/Game.h)
- **Game::m_unk0x30 is an `rbx::signal<void(DataModel*)>`.** iOS 10-29 Game::Game calls `signal<void(DataModel*)>::safe_static_do_get_mutex` and ~Game calls its `disconnectAll`. No 2016 counterpart, so the member stays unnamed.
  - The Unk0x30 stand-in struct and its call_once STUB are gone.
  - **TEMPLATE markers in Game.h:**

    | Member | PLAYER | STUDIO |
    |---|---|---|
    | signals::signal ctor | 0x00653790 | 0x007cc250 |
    | dtor | 0x00654e80 | 0x007cd5c0 |
    | disconnectAll | 0x00654790 | 0x007cd150 |
    | operator() | 0x00654dc0 | none |
    | safe_static_do_get_mutex | 0x004265c0 (/GL shell copy) | 0x007cb980 |
    | safe_static_init_mutex | 0x00426660 | 0x007cba10 |

  - **At 100%:** the ctor, dtor, disconnectAll and init_mutex in both targets.
  - **Below 100%:** PLAYER's shell do_get_mutex is 0.52 (it carries a /GS cookie). operator() is 0.62.
  - The STUDIO ctor row collides with a generated `studio-stl.csv` library row (`??1strstreambuf@std@@UAE@XZ_28` at 0x007cc250); the marker won after my regen.
- **New and rewritten bodies (iOS 10-29 thumb callees + 2013 bytes):**
  - **`template<class T> shutdownDM(shared_ptr<T>&)`:** PLAYER 0x00653600, both instantiations fold.
    - The body is the LegacyLock(dm.get(), Write) + closeDataModel(dm, true) block, then `dm.reset()`.
    - STUDIO keeps two copies (0x007cc000, 0x007cc090) that differ only in the folded reset twin they call, so STUDIO is not marked.
  - **doClearVerbs:** PLAYER 0x006542b0 / STUDIO 0x007ccca0. It does Impersonator(COM), commonVerbs.reset(), deletes each verb with the end cached, then verbs.clear().
  - **clearVerbs(bool):** 0x00654360 / 0x007ccd50.
  - **shutdown():** inline into ~Game in 2013 (iOS keeps it out of line).
  - **~Game:** PLAYER 0x00655d30 → 100. STUDIO 0x007cdc90 stays STUB at 99.07: it calls the two unfolded shutdownDM copies, and ours folds them.
  - **setDataModel(shared_ptr<DataModel>):** PLAYER 0x00655bf0, called by View::resetScheduler. It assigns, passes the weak_ptr to the overlay, then `ServiceProvider::create<ContentProvider>(overlay)` → submitTask(bind(setupDataModel, getBaseUrl())), and finally fires m_unk0x30(dm.get()).
  - **SecurePlayerGame ctor:** PLAYER 0x00656250, `call_once(&Network::initWithPlayerSecurity, flag)`. The flag is a file static at global scope, GLOBAL 0x00de68b4.
  - **STUDIO has UnsecuredStudioGame instead:** RTTI, vftable 0x00c0a928, ctor 0x007ce190 using `call_once(&Network::initWithoutSecurity, flag 0x00f89340)`. The classes are per target (`OGRE_TARGET_*`).
- **Deleting destructors:** Game's folds with SecurePlayerGame's (PLAYER 0x00656230) and with UnsecuredStudioGame's (STUDIO 0x007ce200). The PDB keeps the subclass name, so the SYNTHETIC marker sits on the subclass.
  - VTABLE markers: Game STUDIO 0x00c0a920, SecurePlayerGame PLAYER 0x00b3104c.
  - All four vftables and both D0s are at 100%.
  - `FOLDED` SYNTHETIC namerefs pair in reccmp but fail check_folded, so I did not use them.
- **Still STUB:**
  - setupDataModel 0x00655630. It needs the 2013 Commands.h camera verbs: CameraVerb(std::string, Workspace*) 0x00857810 with a ServiceClient<Selection> at +0x10, size 0x1c; six inline Camera*Command ctors whose vftables 0x00b30f64..0x00b31004 are emitted by Game.obj; CommonVerbs ctor 0x00864450; ToggleViewMode 0x00856130.
  - shutdownGameDataModel 0x00654400 (0.76). 2013 has an EH frame and no tail call to reset(); the explicit-temporary variant probe made no difference.
- **`Network::initWithPlayerSecurity`** (PLAYER 0x00526e10, iOS name) is declared locally in Game.cpp. It is unresolved in our link (/FORCE:UNRESOLVED, no map symbol), yet SecurePlayerGame is at 100%: both `push` operands are unpaired addresses, so reccmp masks them. See Unresolved.

### 3. LuaAllocator and openState
- **LuaAllocator 2013 layout:** a vptr (vftable PLAYER 0x00b7f5f8 / STUDIO 0x00c52fa8, one slot: the virtual `alloc(void*, size_t, size_t)`), then heapSize..maxHeapCount, then memPools at 0x14. Size 0x24.
  - The ctor loops `for (int size = 28; size <= 88; size += 4)`.
  - The dtor deletes, nulls and then clears the pools.
  - Ctor (0x0086b300 / 0x009ad320), dtor (0x0086b240 / 0x009ad260), the vftable, alloc, getHeapStats and clearHeapMax are all at 100% in both.
- **openState 0x00688370: 0.691 → 0.983 (STUDIO 0x007edad0 0.977). It is still a STUB.**
  - **What changed:**
    - `if (!luaAllocator)`.
    - DebuggerManager is registered only under `FFlag::LuaDebugger` (0x00688d05).
    - `ObjectBridge::registerInstanceClassLibrary` is now inline in LuaInstanceBridge.h. 2013 inlines it at 0x00688860 and has no out-of-line copy.
    - The panic handler is `panic` (iOS name), FUNCTION PLAYER 0x00672dc0 / STUDIO 0x007dac90.
    - **The 2013 tail is added:** `contentClearing = false`, then gcJob = GcJob(shared_from(this)) and waitingScriptsJob, each `TaskScheduler::add`ed. The frame is now 0x60 and the length is 3195 bytes, both equal to 2013.
    - openScriptLibrary is split as 2013 has it: FUN_0066fb00 (PLAYER 0x0066fb00 / STUDIO 0x007d7ec0, the locked-proxy half, L in esi and name in edi) and openScriptLibrary (0x0066fb80 / 0x007d7f40, L in eax). Both are at 100%.
  - **Remaining difference (probe_compare on a one-TU compile):** 14 bytes of register choice. These are the two lua_gc stores, the RBXASSERT(i==1) FLog load, and the three shared_from(getRootAncestor()) pushes.
- **Implementation ctor 0x00641e60 (STUDIO 0x0075f6e0): 0.992, still a STUB.** The only difference is the assert string. 2013 asserts `job` at DataModel.cpp:1054, and our DataModel.cpp places it about 450 lines later.

### 4. DataModel group
- **The 162 "DataModel" unnamed functions are almost all generic template bodies** shared across many classes (list/vector _Tidy, scoped_ptr deleters through vbases, member getters). Several are reached only from unwind funclets: 0x00660500 has 12 callers, all funclets.
- **Instead I took the two that mattered:**
  - **LegacyLock::Implementation, 2013 layout:**
    - +0 dataModel, +4 job, +8 previousJob, +0xc events, +0x14 `Limits::Counter::Activator`, +0x1c the saved writeRequestingThread.
    - The ctor takes a third argument, `shared_ptr<Limits::Counter>` = DataModelArbiter::m_unk0x830, now protected.
    - When the job type is Write, the ctor sets `writeRequestingThread = GetCurrentThreadId()` and the dtor restores it.
    - "Acquired" is logged with FASTLOG2 into the 3-argument format, as 2013 does.
    - **Result:** both LegacyLock ctors (PLAYER 0x006421c0 / 0x006422c0, STUDIO 0x0075fa40 / 0x0075fb40; 0.60 → 100) and ~Implementation 0x0063a380 / 0x00758fd0 are at 100%.
  - **ContentProvider::getBaseUrl** 0x0065fd00 is marked. setBaseUrl compares `baseUrl != url`; it is at 100% in both.
- **Security::Impersonator, 2013 shape:**
  - 4 bytes: `previous`, a `new Context(identity)`, `ptr().release()`, then `ptr().reset(c)`.
  - It stays out of line. The engine calls the /GL shell's copies (PLAYER 0x0041cf40 / 0x0041cf80, STUDIO 0x00461ce0 / 0x004619a0). Defined inline in the header it was inlined into callers, and doClearVerbs dropped to 0.71.
  - The bodies are STUB (0.31): the shell copy calls out-of-line `thread_specific_ptr::release`/`reset`.

### For Network (App edit made for them)
- **NetworkSettings.h:** `m_unk0x78` (+0x78, 2016's isQueueErrorComputed) is now public. ConcurrentRakPeer::PacketJob's ctor 0x005c5610 reads it, and the network agent's build was failing on it (C2248). The line count is unchanged.

## Patterns proven
- **A COMDAT can come from a different .obj in each target.** 2013 PLAYER links FactoryRegistration.obj after FriendService.obj and UserController.obj; STUDIO links it before them. Each target keeps the classDescriptor copy of whichever .obj it links first. Define the template's instantiating static per target (FactoryRegistration.cpp vs the class's own .cpp).
- **link.exe keeps stale types in an existing PDB.** Delete the .pdb before a link that is scored or exported.
- **When identical deleting destructors fold** (base and subclass), the PDB's proc name for the folded address is the subclass's. A SYNTHETIC marker on the subclass pairs both vftables; FOLDED SYNTHETIC namerefs pair, but check_folded rejects them.
- **openState's register conventions for file statics:** when 2013 splits a static in two, VC9 gives each half its own registers (L in eax for one, L in esi and name in edi for the other). Our single merged body had been getting edi.

## Checks
- `check_2016_copies.py`: silent.
- `reccmp_addr_padding.py`: 1,708 files, 0 bad.
- `check_folded`: PLAYER 3,106 annotated, 0 problems; STUDIO 2,832, 0.
- **`reccmp-decomplint` against sweep24, per file I touched:**
  - **DataModel.cpp +1 (PLAYER):** ~Implementation 0x0063a380 now sits ahead of the Implementation ctor, but the marker before it is higher. 2013 emits the class-inline dtor far earlier.
  - **SecurityContext.cpp +1 PLAYER / +2 STUDIO:** the Impersonator markers are shell-copy addresses after isInRole. Moving them above isInRole would shift its assert at line 33, which is on its 2013 line.
  - No other file changed.
- **Assert lines:** DataModel.cpp (3145/3151/3162), ScriptContext.cpp (all 35 anchors), LuaMemory.cpp (121..157) and SecurityContext.cpp (33) keep every 2013 line.
  - Each edit replaced lines one for one, or dropped stale porting comments inside openState (12 long "batch/section" lines) to make room for the job tail.
- **STUB → FUNCTION:** 30 markers in my files, each at effective 100% in both final runs of its target. No FUNCTION marker in my files is below 100%.

## Unresolved
1. **Integrator:**
   - Regenerate the shared CSVs (my private regen gave the figures above).
   - Delete the exe PDBs before linking (stale GuiBuilder/PhysicsJob types).
   - reccmp needs the zero-size-element guard (build-ap/w18/rc.py) or the shell agent's ATL CRegParser code changed.
   - The STUDIO `list<connection>::_Buynode` twin (shell placeholder vs Http.h) costs 4 STUDIO entries.
   - **The Job `_internal_accept_owner` group gained GcJob and WaitingScriptsJob members** (openState now builds those jobs, as 2013 does). It pairs correctly only after a regen; check MATCH_FIX 0x005bd080.
2. **Network owner:**
   - Declare and define `RBX::Network::initWithPlayerSecurity()` in API.h/API.cpp: STUB PLAYER 0x00526e10, iOS 0x00957b2c. The body calls initVersion1 0x00520790 and initWithoutSecurity, then two versionB calls through imports.
   - Then remove the local declaration in Game.cpp.
   - NetworkSettings::m_unk0x78 is now public.
3. **IDA owner (names):**
   - **Game:**

     | Address | Name |
     |---|---|
     | 0x00653600 | `RBX::shutdownDM<RBX::DataModel>` (folded with OverlayDataModel) |
     | 0x006542b0 | Game::doClearVerbs |
     | 0x00654360 | Game::clearVerbs(bool) |
     | 0x00654400 | Game::shutdownGameDataModel |
     | 0x00655bf0 | Game::setDataModel |
     | 0x00656250 | SecurePlayerGame ctor |

   - **signal<void(DataModel*)>:** ctor 0x00653790, dtor 0x00654e80, disconnectAll 0x00654790, operator() 0x00654dc0, do_get_mutex 0x004265c0, init_mutex 0x00426660, once flag 0x00e24fac.
   - **LegacyLock::Implementation:** `safe_static_init_currentJob` 0x0063a370 (IDA has a `j_` thunk name), once flag 0x00de58b8.
   - **Other PLAYER names:**

     | Address | Name |
     |---|---|
     | 0x00672dc0 | `panic` |
     | 0x0066fb00 | FUN_0066fb00 (helper) |
     | 0x0066fb80 | openScriptLibrary |
     | 0x0086b300 | LuaAllocator ctor |
     | 0x0086b240 | LuaAllocator dtor |
     | 0x0041cf40 / 0x0041cf80 | Impersonator ctor/dtor (shell) |
     | 0x0065fd00 | ContentProvider::getBaseUrl |
     | 0x00526e10 | Network::initWithPlayerSecurity |

   - **Registrars:** ControllerService PLAYER 0x00e0c968 / STUDIO 0x00f8330c; FriendService 0x00e0c7ac / 0x00f8314c.
   - **STUDIO counterparts** are in the markers.
4. **App, next:**
   - Game::setupDataModel (Commands.h CameraVerb family first).
   - openState's 14 register bytes.
   - shutdownGameDataModel's EH frame.
   - Moving the Implementation ctor's assert to DataModel.cpp:1054, which needs DataModel.cpp laid out to its 2013 lines.
   - STUDIO ~Game (two unfolded shutdownDM copies).
   - Game::m_unk0x30 needs a name.
