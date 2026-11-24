# Phase 5b, fifth pass: DataModel services and Base near misses (wave 7)

Agent label: datamodel-5b-5. Build dirs: `build-dm/` (PLAYER), `build-dm-studio/` (not used this pass).
Scored links, JSONs and the private project are kept under `build-dm/w7/` (`C/`, `D/`, `scoredB/`, `priv` CSVs
in `build-dm/priv/reccmp/`). The measurement pipeline is `build-dm/w7/measure.sh NAME`: it copies the link,
runs reccmp twice with the shared CSVs, regenerates private CSVs with `gen_reccmp_csv.py --json J1,J2` into
`build-dm/priv/reccmp/`, then scores with them. The shared `reccmp/` was not written.

## Interrupted run's edits (reviewed first)

Kept, because each compiles and the verified run shows no lost 100%: InsertService layout (header and
`.cpp`), ContentFilter::MAX_CONTENT_FILTER_SIZE GLOBAL plus TextLabel::setText, DebrisService::setMaxItems
(permission 2), PhysicsJob::stepDataModelJob (read order), TaskScheduler::scheduleJob (no updatePriority),
and Job::preStep (updateError first).

Reverted: Workspace.h and Workspace.cpp, which padded the class to 0x278. That padding brought
`Creatable<Instance>::create<Workspace>` 0x00635a10 to 100%, but it moved the IHasLocation virtual base, and
the `ModelInstance::getLocation` vtordisp thunk in `Workspace::vftable{for IHasLocation}` 0x00b49448
dropped from 100% to 0% (the original thunk is `sub ecx,[ecx-4]; sub ecx,0x148`). The trade is not allowed.
The rule is never to lose a 100%, and the real fix is the 2013 base chain (see "Blocked").

Not mine and left alone: the DataModel.cpp include lines, StatsItem, StatsService, ProfilingItem,
NetworkSettings and GlobalAdvancedSettings.

## Measurement (PLAYER)

| | sweep13 | this pass (run D, private regenerated CSVs) |
|---|---|---|
| Entries at effective 100% (whole exe, all agents' uncommitted work) | 9,893 | 10,012 |
| Row 5b (`row_coverage.py --row 5b`) | 614 / 1,660 (37.0%) | 647 / 1,710 (37.8%) |

- Row 5b's denominator grew because the new StarterGuiService ctor marker (0x0070c150) widens
  StarterGuiService.cpp's range to 0x00708e60–0x0070c150 (4/15 → 21/65).
- Same ranges otherwise: DataModel.cpp 361 → 375, RunService.cpp 24 → 25, DebrisService.cpp 8 → 9.
- Part of DataModel.cpp's gain (the 11 `isNullClassName` copies, from another agent's `util/object.h`
  include) is not mine.
- Per-function CSVs: `build-dm/w7/row5b-before.csv` and `row5b-after.csv`.

Every entry lost against sweep13 was read with `--verbose`; none is in my files:
- StatsItem/StatsService `doDeclare`/`classDescriptor` (another agent's uncommitted StatsItem.h).
- `~ContactManager` and `Primitive::getGeometryType`/`getCollideType` (world agent's edits).
- `symbexec`, `luaV_execute`, `constructor`, `patchtestreg`, `removevalues` and `luaK_setreturns`: the known
  no-.reloc immediate coincidences.

My change did lose one entry: the folded Described/NonFactoryProduct dtor over BasePlayerGui (0x00708b60).
Adding StarterGuiService gives that body a second name, and the PUBLICS name our PDB keeps changed. I
updated the TEMPLATE nameref in CoreGuiService.h and it is back at 100%.

STUDIO was not built or scored in this pass. The new STUDIO addresses come from masked-byte twins
(`build-dm/w7/twin_studio.py`), and every STUDIO marker I added stays `STUB`.

## Functions raised (PLAYER, effective 100%, marker set to FUNCTION)

- **InsertService** (`v8datamodel/InsertService.cpp`):
  - setTrustLevel 0x007cfb40 and the seven url setters 0x007cfb60–0x007cfd40.
  - The ctor 0x007db980.
  - Through their markers: the implicit dtor 0x007dbbf0 (SYNTHETIC), the scalar deleting dtor 0x007dbe40,
    and the `Described<InsertService>` ctor 0x007d8030. Its TEMPLATE nameref was wrong (`V42` → `V32`).
  - Through the regenerated CSVs: the InsertService and FactoryProduct<InsertService> vftables (7) and the
    callback map's `_Buynode` 0x007d0550.
- **StarterGuiService** (`v8datamodel/StarterGuiService.cpp`, new 2013 layout):
  - getCoreGuiEnabled 0x007095e0 and setCoreGuiEnabled 0x00709c60.
  - Through the regenerated CSVs: `Creatable<Instance>::create<StarterGuiService>` 0x0058ea00 (`new 0x98`),
    `NonFactoryProduct<BasePlayerGui,sStarterGuiService>::getClassName` 0x0070bca0 and six vftables.
  - The ctor 0x0070c150 went from 20.2% to 97.4% (STUB, cause B below).
- **Base**: Job::preStep 0x009b9de0, 62.6% → 100%. 2013 keeps `Time now` in a named local before
  `updateError(now)`, so the call takes `lea ecx,[esp+0x14]` and not the returned pointer.
- **Verified and promoted from the interrupted run's edits**:
  - TaskScheduler::scheduleJob 0x009b43b0 (94.6% → 100%).
  - DebrisService::setMaxItems 0x00844e90 (98.2% → 100%).
  - TextLabel::setText 0x006beeb0 (94.8% → 100%).
  - PhysicsJob::stepDataModelJob 0x0072b880 (98.6% → 100%).
- **Written, still STUB**:
  - HeartbeatTask::stepDataModelJob 0x0072dc30: the full 2013 body, 71.5% → 99.45%. Byte-identical in a
    probe; the only diff is call pairing on the folded `weak_ptr<T>::lock`.
  - RunService::raiseHeartbeat 0x0072da30: 64.4%.
  - InsertService::insert 0x007d6a20: 93.9%.
  - RootInstance::publicInsertRaw is declared with the 2013 signature (two trailing bools).

## Layouts proven

- **InsertService, 0x188**. Proven by the ctor and dtor at 100%. Layout:
  - Described / DescribedCreatable<InsertService, Instance, INTERNAL>, with Service at 0x60.
  - Five remote_signals at 0x64–0x8c.
  - Seven url strings at 0x8c–0x150 and trustLevel (float, 0.5) at 0x150.
  - Five plain `connection`s at 0x154–0x164.
  - `boost::recursive_mutex` at 0x168 (its dtor's inline CloseHandle reads 0x174).
  - `std::map<string, Callback>` at 0x178 and `rbx::atomic<int> loadCount` at 0x184.
- **StarterGuiService, 0x98**. Proven by get/setCoreGuiEnabled and `create<>` at 100%. Layout:
  - `DescribedNonCreatable<StarterGuiService, BasePlayerGui, PERSISTENT>`; BasePlayerGui is 0x74, and
    Service sits at 0x74.
  - `boost::unordered_map<CoreGuiType,bool> coreGuiEnabledState` at 0x78 and
    `rbx::signal<void(CoreGuiType,bool)> coreGuiChangedSignal` at 0x90.
  - `showGui` at 0x94 and `resetPlayerGui` at 0x95, both true (iOS `setShowGui`/`setResetPlayerGui`).
  - In 2013 the code sits inside PlayerGui.obj (0x007083d0–0x0070c450), as in 2016's PlayerGui.cpp. It
    stays in StarterGuiService.cpp for now.

## Layouts found (not yet proven)

- **RunService's doubles.** 2016's `totalWallTime` and `skippedTimeAccumulated` swap places:
  raiseHeartbeat 0x0072da30 accumulates `wallStep` at 0x90. RunService.h now declares skipped at 0x88 and
  wall time at 0x90; the ctor's 100% is unaffected.
- **RootInstance, 0x14c** (ctor 0x007e35f0):
  - ModelInstance ends at 0x128, and RootInstance's own vptr is at 0x128.
  - Vector3 (zero) at 0x12c, floats at 0x138/0x13c (0), 0x140 ([0xb25074]) and 0x144 ([0xb69450]).
  - `World*` (`new 0xb8`) at 0x148.
- **Workspace, 0x278** (ctor 0x00718e10):
  - Bases after RootInstance: GuiTarget at 0x14c, VerbContainer at 0x150 (ctor 0x006f9200),
    IMovingManager at 0x164 (ctor 0x00823080), IScriptFilter at 0x178, Service byte at 0x17c, and the
    IHasLocation virtual base at the end.
  - Members from 0x180: a 0x30-byte object, Vector3s at 0x188/0x194, `double` at 0x1a0, the dataState at
    0x1b8, and the `G3D::Array<IndexedTree*>` at 0x214.
  - Doubles at 0x228–0x240, a `boost::shared_ptr`-like member from 0x70f150 at 0x24c, and a float at 0x250.
  - Three `CodeProfiler`s ("DataModel Step", "Workspace Step", "Workspace Assemble") at 0x25c/0x260/0x264.
  - The assert `dataState!=NULL` is at line 0xe6.
- **ModelInstance, 0x128** (ctor 0x006d4640): IAdornable at 0x68, CameraSubject at 0x80 (vbptr 0x84, byte
  0x88, float 10.0 at 0x90), Selectable at 0x94, members 0x98–0x128 (two CoordinateFrames at 0xa8/0xd8,
  Vector3 0x108, bools 0x114/0x115, 0x11c–0x124).
- **Hopper, 0x98**:
  - RelativePanel's ctor 0x006c8760 is out of line.
  - The NonFactoryProduct<Hopper, sStarterPackService> ctor stores +0x8c=4 and +0x90=1, and StarterPackService
    adds a Service byte at 0x98.

## Closest misses (task item 1): shared causes, none fixable in my files

All eight named closest misses were read with `--verbose`.

- **A. boost program_options is built native (it is `/GL` in 2013), so its template copies win our
  folds.**
  - In 2013 the engine set keeps its own copies:
    - `~shared_ptr<value_semantic const>`: engine 0x007e4e40, shell 0x00405b50.
    - `_Destroy_range<shared_ptr<T>>`: engine 0x00829d90, shell 0x00427f30.
    - `shared_ptr<T>::operator=`: 0x00469520.
    - `shared_from<T>`: 0x004618d0.
  - Our single folded body pairs with the shell copy, so every engine caller shows `call <OFFSET>`.
  - Affected:
    - DataModel and TaskScheduler: ~DataModel 0x0064d850, DataModel ctor 0x0064ec50,
      ~TaskScheduler 0x009b4e00, ~Job 0x009ba1f0, computeStandardSleepTime 0x009b9a90,
      onlyJobsLeftForThisArbiterAreGenericJobs 0x0063a120.
    - Services and RunService: ServiceProvider::onChildAdded 0x006a9770, RunService::start 0x0072d300,
      Lighting::onChildAdded 0x00707510.
    - Templates: the 15 `ServiceProvider::find<T>` copies (98.8%) and
      setJobsExtendedStatsWindow/enableThreads.
  - Needs the owner's side: build boost_po `/GL` (it cost 154 entries before), or pick the engine copy in the
    generator's twin rule when more engine callers point at it.
- **B. `prime_list_template<unsigned>::value` one-past-end (0x00bdb080).**
  - In the original, the word after the prime list is a vftable's COL pointer (0x00c6f764). reccmp renders
    it as the string `"d\xf7\xc6"`; ours is the `reflection_property.cpp` assert string.
  - 52 PLAYER functions load it (every inline `boost::unordered_map` ctor).
  - Affected: LegacyContentTable ctor 0x008f08a0 (99.97%), the DataModel ctor, the StarterGuiService ctor
    (97.4%), StatsService, ContactManager, Cloner, LibraryService.
  - Needs a reccmp-side rule: a one-past-end pointer of a sized variable should compare as `var+size`.
- **C. Trivial `ret` twins.** TaskScheduler::remove 0x009b52b0: `test eax, 0x528720` is boost::function's
  safe-bool (`&dummy::nonnull`), which equals the engine `ret` copy 0x00528720. Ours equals another twin.
- **D. Base keeps one STL copy per .obj in 2013.**
  - `vector<shared_ptr<T>>::push_back` exists at both 0x009b7020 (TaskScheduler.Thread.obj) and 0x009ba960
    (TaskScheduler.Job.obj), with identical bytes; our link folds them.
  - Affected: Job::addCoordinator 0x009baa80, SequenceBase::onAdded 0x009bb9e0, setJobsExtendedStatsWindow,
    enableThreads.
- **E. `FactoryProduct<…>::Creator::create` converts to the base at +0x18 (AbstractFactoryProduct), not
  +0x1c.** The 98.7% Creator::create rows (Teams 0x0053d1d0, Backpack 0x0053c830, Flag 0x007114c0, …) show `add ecx,0x18` against
  our `0x1c`. The return type in `Util/Object.h` is probably `shared_ptr<AbstractFactoryProduct<Base>>`.

## Blocked (needs other owners)

- **IAdornable (Client/Rendering/GfxBase/IAdornable.h, second contributor) is 0x18 in 2013:** a `float`
  after `bucket`.
  - Evidence: ModelInstance ctor `movss [esi+0x7c]` after `bucket` at 0x78 (0x006d46c8); PartInstance ctor
    `movss [esi+0x88]` after `bucket` at 0x84 (0x007269a7).
  - Every IAdornable subclass layout depends on it, including ModelInstance, RootInstance and Workspace.
    Workspace's VerbContainer must reach 0x150 for DataModel::completeShutdown 0x00641c90 (96.2%:
    `lea ecx,[workspace+0x150]` vs our 0x94).
  - Until then the Workspace row (20/208) cannot move.
- **`Util/RunTransition.h`: `Heartbeat::expirationTime` is a `Time` in 2013, not a double.**
  raiseHeartbeat 0x0072da30 copies the `Time::nowFast()+stepBudget` result as two dwords into
  `Heartbeat+0x20`.
- **`V8Tree/Instance.h`: `getRobloxLocked()` should return the FWValue<bool> itself.** The original's
  `mov al,[eax]; cmp al,bl` is the only diff left in InsertService::insert 0x007d6a20 (probe), against our
  `!= 0`.
- **`v8tree/Service.cpp`: ServiceProvider::create 0x006a98f0 calls `setAndLockParent`, not
  `setParentInternal`.**
- **PhysicsInstructions:**
  - The ctor 0x0084d2f0 needs PhysicsSettings (singleton +0x68), which is a settings file.
  - changeSimulationRadius 0x0084cb70 and changeMaxSimulationRadius 0x0084cbe0 need
    `Network::Player` +0xcc/+0xd0 and the setters 0x0052e510/0x0052e570. Their asserts are at lines 55 and 63
    of PhysicsInstructions.cpp.
- **DebrisService::onServiceProvider 0x00845380** needs TimerService (`ServiceProvider::create<TimerService>`
  0x0053d890).
- **StarterPackService** needs Hopper : RelativePanel (gui, row 5h).
- **InsertService::onServiceProvider 0x007de310** (assert line 178) needs the three backend handlers
  0x007ddf70/0x007de1c0/0x007de070, and those fire the remote event descriptors (`mov ecx,0x00e0fd3c`).
  The InsertService reflection block comes first.

## Checks

- `check_2016_copies`: silent.
- `reccmp_addr_padding`: 0 bad markers.
- `reccmp-decomplint` adds no new errors in my files; the `function_out_of_order` lines in
  TaskScheduler*.cpp were already there.

## Files

- `Client/App/include/V8DataModel/InsertService.h`, `Client/App/v8datamodel/InsertService.cpp`
- `Client/App/include/V8DataModel/StarterGuiService.h`, `Client/App/v8datamodel/StarterGuiService.cpp`
- `Client/App/include/V8DataModel/RunService.h`, `Client/App/v8datamodel/RunService.cpp`
- `Client/App/include/V8DataModel/RootInstance.h`, `Client/App/include/V8DataModel/CoreGuiService.h`
- `Client/App/include/V8DataModel/ContentFilter.h`, `Client/App/v8datamodel/ContentFilter.cpp`,
  `Client/App/v8datamodel/TextLabel.cpp`, `Client/App/v8datamodel/DebrisService.cpp`
- `Client/Base/rbx/TaskScheduler.cpp`, `Client/Base/rbx/TaskScheduler.Job.cpp`
- Workspace.h and Workspace.cpp: reverted to HEAD.
