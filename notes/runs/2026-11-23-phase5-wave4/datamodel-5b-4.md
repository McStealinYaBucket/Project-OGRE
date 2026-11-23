# datamodel-5b-4 (phase5-wave4, 2026-11-23)

Phase 5b fourth pass: DataModel and core services. All builds were in build-dm (PLAYER) and
build-dm-studio (STUDIO). The baseline is sweep10 (build/sweep10/P-final.json, PLAYER 9,029 at
effective 100%). The working tree is shared, so my builds also compiled other agents' unfinished
edits (ScriptContext, v8xml, Lua); their gains and losses show up in the whole-exe numbers below.

How I measured:
- Two reccmp runs of a link (PLAYER build-dm/w4/P9.json and P10.json, binaries in build-dm/w4/s5/).
- tools/ida/gen_reccmp_csv.py on those runs, written into a private project root (build-dm/priv/: symlinks to the repo plus its own reccmp/ copy).
- A run against the private CSVs (config in build-dm/w4/privrun/).

The final runs are build-dm/w4/P12.json (PLAYER) and build-dm-studio/w4/S8.json (STUDIO, generated the same way from S4 and S5, binaries in build-dm-studio/w4/s2/; S8 is the final link, after the CSV generation). The shared reccmp/ was not written.

## Row 5b, PLAYER (tools/row_coverage.py)

| File | Before (sweep10) | After (P12) |
|---|---|---|
| Lighting.cpp | 3 / 25 | 35 / 62 (the range now starts at getMoonPhase 0x00704ed0) |
| RunService.cpp | 8 / 67 | 22 / 79 |
| everything else in the row | unchanged | unchanged |
| **Total** | **512 / 1,604 (31.9%)** | **558 / 1,653 (33.8%)** |

The denominator grew because the new markers widened the Lighting and RunService ranges.

Whole exe, effective 100%:

| Target | Before (sweep10) | After | Gains in my address ranges | Losses there |
|---|---|---|---|---|
| PLAYER | 9,029 | 9,164 (P12) | 54 (functions, templates, vftables) | 0 |
| STUDIO | 8,826 | 8,953 (S8) | 39 (functions and templates) | 0 |

The rest of the change in both totals is other agents' work in the shared tree (TextXml*, XmlWriter, ArchiveBinder, SerializerV2, Lua bridges, ScriptContext).

Every STUDIO marker I added scores the same as its PLAYER twin. The one exception is RunService::start, which is at 100% in STUDIO (STUDIO 0x0079af80, FUNCTION) and 99.4% in PLAYER.

## Layouts proven (constructor and destructor byte-identical)

- **Lighting, 0x1f0.** The ctor 0x00707ba0 and the implicit dtor 0x00707e40 are identical to the
  original; reccmp scores both at effective 100% with the private CSVs. 2013 layout:
  skyParameters @0x68 (G3D::LightingParameters, 0xf8 bytes in 2013), hasSky @0x160,
  clearColor/shadowColor/fogColor as **G3D::Color4** @0x164/0x174/0x184, fogStart @0x194,
  fogEnd @0x198, timeOfDay @0x1a0, topColorShift @0x1a8, bottomColorShift @0x1b4,
  globalBrightness @0x1c0, globalAmbient @0x1c4, globalOutdoorAmbient @0x1d0, globalShadows
  @0x1dc, outlines @0x1dd, lightingChangedSignal @0x1e0, sky @0x1e4. Names from iOS 10-29
  (setShadowColor(Color4), setShadowColor3(Color3), setFogColor3, getFogColor3, ...). The 21
  reflection descriptors are laid out in their 2013 initializer order (0x00a6ffd0..0x00a705b0).
- **PhysicsJob, 0x228** (ctor 0x0072b5e0, dtor 0x0072b780, both identical): a DataModelJob
  ("Physics", Physics, stepBudget 0.01) with stepsPerSec @0x218 (it caches
  Constants::uiStepsPerSec() at construction) and weak_ptr<DataModel> @0x220.
- **HeartbeatTask, 0x230** (ctor 0x0072d040, dtor 0x0072d250, both identical): a DataModelJob
  ("Heartbeat", Write, stepBudget 0.003) with weak_ptr<RunService> @0x218, weak_ptr<DataModel>
  @0x220 and fps @0x228 (30.0).
  Both job classes now live in RunService.cpp as classes local to the .cpp, as in 2013 (their
  bodies sit between RunService's own functions). The old shells PhysicsJob.cpp/.h and
  HeartbeatTask.cpp/.h, which derived TaskScheduler::Job directly, are deleted.
- **RunService** (PLAYER) was already proven by its ctor 0x0072efd0. Its job members are now typed
  `shared_ptr<PhysicsJob>` / `shared_ptr<HeartbeatTask>` (iOS 10-29 has both operator=), which is what makes
  RunService::start 0x0072d300 byte-identical in a probe compile. With Job-typed members the original's single stack
  slot for both temporaries becomes two. stopTasks 0x0072bb60 stays at 100%, and the fold losses that the 2026-11-13
  note in RunService.h warned about (GuiItem::onDescendantRemoving 0x00768d00 and others) did not come back.
- **World, size 0xb8** (for world-5d; I did not edit v8world). This is a scout from the ctor
  0x0074ae30 and dtor 0x00749ae0, with `new 0xb8` in RootInstance::RootInstance 0x007e35f0:
  - Four signals at 0x00/0x04/0x08/0x0c, ints at 0x10..0x1c, a G3D::Array at 0x20, a bool at 0x30, pointers at 0x34/0x38.
  - G3D::Arrays at 0x3c and 0x50, a pointer at 0x4c, a std::set at 0x60, zeroed fields at 0x6c/0x70/0x74.
  - G3D::Arrays at 0x78 and 0x88, then five stage pointers at 0x98..0xa8 (the dtor deletes them in reverse order).
  - ContactManager at 0xac, **SendPhysics at 0xb0**, and one more field at 0xb4.
  - Our World is 0x7c with sendPhysics at 0x74. That is why World::getSendPhysics folds wrongly
    and SimulateStage::putFirstMovingRootInSendPhysics 0x00998920 stays STUB. The 2013 getter
    is 0x008bb760 `mov eax,[ecx+0xb0]`.

## Functions added or raised (PLAYER, effective 100% in P12)

- **Lighting:**
  - Already at 100%: getTimeStr and askAddChild.
  - New at 100%: the ctor and the implicit dtor and deleting dtor (SYNTHETIC markers).
  - Getters: getMoonPhase, getGeographicLatitude, getFogColor3, getMoonPosition, getSunPosition.
  - Setters: setFogStart, setFogEnd, setGlobalShadows, setFogColor3, setClearColor, setShadowColor, setShadowColor3, setTime, setGeographicLatitude, setMinutesAfterMidnight, setTimeStr.
  - Other members: onChildChanged (STUB before, now FUNCTION), onChildRemoving, onPropChanged, replaceSky, and
    onChildAdded 0x00707510 (byte-identical in a probe compile; 98.5% in both targets because shared_from<Sky> folds
    into our shared_from<GuiItem>; STUB). The Lighting Debugable vftable 0x00b478c4 now matches, because the child
    hooks fill its slots.
  - Templates: str_from_delimited_time_duration 0x00706330 (TEMPLATE), findConstFirstChildOfType<Sky> 0x007073e0, int_adapter::mult_div_specials.
  - Vftables: 9 Lighting reflection vftables (PropDescriptor<Lighting,*>, BoundProp*GetSet<Lighting>, FuncDesc<Lighting>, BoundFuncDesc<Lighting,void(double)>) and BoundProp<float>.
- **RunService.cpp:**
  - PhysicsJob: ctor, sleepTime, getDesiredConcurrencyCount, dtor, deleting dtor, vftable.
  - HeartbeatTask: ctor, sleepTime, dtor; error was paired by the CSV to its folded body at 0x005dc110.
  - RunService::start: 100% in STUDIO (FUNCTION). PLAYER is at 99.4% (STUB), because the shared_from<RunService>
    call pairs with our folded shared_from<GuiItem>.
- **StarterGear::StarterGear 0x006cb3e0:** the 2013 ctor calls setName("StarterGear"), and the
  inline empty ctor did not. Its Described<StarterGear> ctor 0x006ca3f0 is now a TEMPLATE marker.
- Supporting declarations:
  - DataModel::physicsStep(float, double, double, int): the iOS signature, declaration only.
  - DataModel::throttleAt30Fps (GLOBAL 0x00d314cc, initialized true) and RunService::parallelPhysicsUserEnabled (GLOBAL 0x00df4e68).
  - Both use the 2016 names. The 2013 bytes have the same structure: `flag ? stepsPerSec : 1000.0` and `flag ? 1024 : 1`.
  - The 21 Lighting descriptor objects sit at 0x00df2ea8..0x00df3298; the generated libdata rows pair them.
    reccmp's parser ignores a GLOBAL marker above a constructor-style definition (decomplint
    global_not_variable), so the file carries none.
  - throttleAt30Fps is defined in RunService.cpp, so no assert line in DataModel.cpp moves. A first try that defined it in DataModel.cpp shifted 4 asserts by 3 lines, and I reverted it.

## Losses I read (P4, P11 and P12 vs sweep10), none kept from my code

- Twin and fold churn from Lighting's new instantiations:
  - SendPhysics::onMovingAssemblyRootAdded/Removing 0x008e5fe0/0x008e60b0 now pair their signal_with_args<1,void(Primitive*)>::operator() call with our folded lua_State* body.
  - The same fold affects Lighting's setters, which reach 100% only through the CSV twin pairing.
  - With the shared (stale) CSVs a dozen PropDescriptor/BoundProp GetSetImpl rows and their vftables also went unpaired, because their names now share folded bodies. The private CSV regeneration re-paired all of them.
- From other agents in the shared tree (P11 also lost 65 Lua bridge rows while an agent was in the middle of editing LuaAtomicClasses.cpp; all of them were back in P12):
  - ScriptContext's Described ctor 0x0067fcd0.
  - SerializerV2::newRootElement ×2.
  - ArchiveBinder's vftable.
- No-.reloc coincidences: symbexec, luaV_execute, constructor, patchtestreg, removevalues and luaK_setreturns (Lua). Each one is an immediate that now equals one of our data addresses, for example `and eax, <Plugin RTTI>` in constructor 0x0098f9a0.

## What blocks the rest

- **Name::declare<sName>() is missing from Util/Name.h** (Util owner). 2013 inlines it at every className() site:
  - The body is `if (sName == NULL) return getNullName(); call_once(&callDoDeclare<sName>, flag); return doDeclare<sName>();`.
  - callDoDeclare<sContentFilter> is 0x0062fc40 (a `jmp` to doDeclare 0x0062e040).
  - Our NonFactoryProduct::className() calls doDeclare directly.
  - isNullClassName's assert in 2013 compares `(className() == Name::getNullName()) == (sClassName == NULL)`: 0x00632290, unpaired today.
  - Fixing both in Util/Name.h and Util/Object.h should lift every ServiceProvider::find<T> (14 rows at 87.3% in DataModel.cpp alone, e.g. 0x00638de0), the create<T> rows at 87%, and find<ServerStorage>-style rows at 93.4%.
- **TaskScheduler::Job::Stats has no timeNow in 2013** (Base owner). The Stats ctor 0x009b98d0 (ret 0xc) writes timespanSinceLastStep at +0 and timespanOfLastStep at +8 (16 bytes).
  - Base's Stats still has 2016's timeNow first.
  - PhysicsJob::error 0x0072b740 and PhysicsJob::stepDataModelJob 0x0072b880 are written for the 2013 layout and stay STUB until Base drops timeNow.
  - The same change affects Job::computeStandardError 0x009b98b0 and computeStandardSleepTime 0x009b9a90, and every job's error().
- **G3D::LightingParameters** (Rendering contributor): 2013's is 0xf8 bytes. setTime 0x009e3b70 writes nothing past geoLatitude at +0xf4; the fork's header adds `skyAmbient2` (0x108).
  - Lighting.h holds the parameters in a 0xf8-byte SkyParameters2013 holder that constructs and hands out the G3D type.
  - Replace the holder with a plain member once the G3D header drops skyAmbient2. Dropping it may also raise setTime 0x009e3b70 (47%).
- **Constants::uiStepsPerSec 0x0084d380 and Constants::uiDt 0x0084d450** have no marker (v8kernel owner). Both test the byte 0xe12490.
  - The PhysicsJob ctor reaches 100% only through the CSV pairing.
  - Add markers in Client/App/v8kernel/Constants.cpp.
- **Folded template bodies in our link**, so they stay out or stay below 100%:
  - signal_with_args<1,void(bool)>::operator() 0x00439b20 is folded with the lua_State* instantiation.
  - Lighting::onChildAdded 0x00707510 and RunService::start 0x0072d300 are byte-identical but call shared_from<T>, which folds into shared_from<GuiItem> (both STUB in PLAYER).
  - Teams::onChildAdded and onChildRemoving stay out for the same reason.
  - The weak_ptr::lock and _Allocate twins hold back GcJob::stepDataModelJob 0x00679e60 (98.9), DebrisService cleanup 0x008446f0 (98.8) and deque::_Growmap 0x006341a0 / 0x00844a70 (99.3).
- **Reflection templates** (reflection owner): the 2013 PropDescriptor/BoundProp/FuncDesc/BoundFuncDesc/EventDesc constructors are longer than ours.
  - The original inlines the `Class::classType()==typeid(Class)` assert. Lighting's are 0x00706b40..0x00707b00 (184 vs 278 bytes, and so on).
  - The asserts carry include spellings `include\Reflection/Reflection.h` and `include\reflection/property.h`.
- **Include spelling:** DataModel.obj's FactoryProduct<Visit>::static_getCreator 0x00630f20 (90.5) asserts with `include\util/object.h`, and ours is `Util/Object.h`.
  - The 2013 binary has all four spellings (include/Include × util/object.h / Util/Object.h), so the first includer per TU decides.
  - I did not change V8Tree's includes, because that would move other TUs.
- **DataModel's remaining misses:**
  - The ctor 0x0064ec50 (97.3) needs Instance::fw() out of line (0x006beaa0).
  - onlyJobsLeftForThisArbiterAreGenericJobs 0x0063a120 asserts on 2013 line 615 (ours is 1609), plus a _Destroy_range fold.
  - WaitingScriptsJob's ctor 0x00679f90 reads ScriptContext+0x84 (ours is +0xac; ScriptContext owner).
- **Not started, too large for this pass:**
  - InsertService (94 unnamed functions, 23 KB). It is an empty shell in the tree today, and it needs the HTTP and yield-function machinery.
  - Workspace still needs the ModelInstance/RootInstance bases and World* at +0x148.
  - StarterPackService needs Hopper's layout. Hopper derives RelativePanel (GUI), whose ctor 0x006c8760 stores RelativePanel's five vftables (the fifth at +0x60) and calls RelativePanel::init(Layout) 0x007682c0. Its NonFactoryProduct ctor 0x006ca370 stores +0x8c=4 and +0x90=1.
  - In 2013, StarterGear, StarterPackService, HopperBin and BackpackItem bodies all sit in Hopper's .obj (0x006c8xxx..0x006cb7xx).

## Checks

- python3 -I tools/check_2016_copies.py: silent.
- tools/reccmp_addr_padding.py: 0 badly padded markers.
- reccmp-decomplint on my six files, both targets: no errors. It prints 8 byname_function_in_cpp warnings for the
  SYNTHETIC destructor namerefs of PhysicsJob and HeartbeatTask, which can only live in the .cpp because the
  classes are local to it.
- Marker order inside RunService.cpp ascends. HeartbeatTask's ctor and sleepTime are defined out of line after
  onServiceProvider, so the class can be complete before stopTasks.
- I did not run vtable or datacmp.
