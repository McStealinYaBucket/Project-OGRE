# Phase 5b, sixth pass: DataModel services (wave 8)

Agent label: datamodel-5b-6. Build dirs: `build-dm/` (PLAYER), `build-dm-studio/` (STUDIO).
Scored links, JSONs and private CSVs:
- PLAYER: `build-dm/w8/A` and `build-dm/w8/B`. B is the final link.
- STUDIO: `build-dm-studio/w8/A` and `build-dm-studio/w8/B`. B is the final link.

The pipeline is `build-dm*/w8/measure.sh NAME` plus `round.sh NAME IN1,IN2 OUT`:
1. Two runs with the shared CSVs.
2. `gen_reccmp_csv.py --json` into `build-dm*/priv/reccmp/`.
3. Further regeneration rounds on the same link.

The shared `reccmp/` was not written. I opened no IDA database and edited nothing under Client/Rendering/.

## Measurement

Both targets score the whole exe, so these totals include every agent's uncommitted work.

| | sweep14 | this pass |
|---|---|---|
| PLAYER entries at effective 100% | 10,076 | **10,427** (B, round 4; 10,434 in round 3) |
| STUDIO entries at effective 100% | 9,803 | **10,175** (B, round 3, same as round 2); 10,143 on link A, round 4 |
| Row 5b, new file set (see below) | 662 / 1,923 (34.4%) | **695 / 1,923 (36.1%)** |
| Row 5b, old file set | 664 / 1,748 (38.0%) | not comparable: StarterGuiService.cpp no longer exists |

**Row 5b file set (`tools/row_coverage.py`).**
- StarterGuiService.cpp is gone. Its code is in PlayerGui.cpp, which row 5h counts.
- InsertService.cpp, TimerService.cpp and PhysicsSettings.cpp were added.

**About 228 of the PLAYER gains come from my files.** I counted the gained entries whose names carry one of my classes, on link A against sweep14. Most of the rest is base-pairing's `Creator::create` fix (`shared_ptr<Object>`).

**Row 5b per file, sweep14 → B:**

| File | sweep14 | B |
|---|---|---|
| DataModel.cpp | 375 | 383 |
| Service.cpp | 8 | 9 |
| Workspace.cpp | 20 | 21 |
| Player.cpp | 46 | 52 |
| DebrisService.cpp | 9 | 11 |
| PhysicsInstructions.cpp | 6 | 7 |
| InsertService.cpp | 36 | 46 |
| TimerService.cpp | 0 | 3 |
| PhysicsSettings.cpp | 0 | 1 |

All other files in the row are unchanged. Per-function CSVs: `build-dm/w8/row5b-before.csv` and `build-dm/w8/row5b-after.csv`.

**PLAYER losses against sweep14 (B round 4): 15.** I read every one with `--verbose`.
- **Two are mine, and both are twin-pairing churn:** `list<FailedUrl>::_Incsize` 0x0086bc70 and `list<FailedUrl>::push_front` 0x0086c5b0 (95.83).
  - TimerService's `list<Item>::_Incsize` has the same node size, so our link folds it with FailedUrl's.
  - The kept copy now comes from TimerService.obj, and nothing pairs the original 0x0086bc70 to it any more.
  - The 2013 binary folds them too: TimerService::delay calls 0x0086bc70.
  - **Fix for the generator owner:** add a row pairing 0x0086bc70 with our folded symbol, `0086bc70,,,?_Incsize@?$list@VItem@TimerService@RBX@@V?$allocator@VItem@TimerService@RBX@@@std@@@std@@IAEXI@Z,`. I tested this row in the private libdata (run `build-dm/w8/A/priv/T.json`). With it, _Incsize, push_front and TimerService::delay 0x007a6a30 all reach 100%.
- **Not mine:**
  - The Base `intrusive_ptr`/`signal` copies: 0x00406c40, 0x0040d2d0, 0x006695c0, 0x00670490, 0x00672670, the 0x00678310/0x006787a0 disconnectAll copies, and Bridge 0x0087da90. These come from base-pairing's in-progress edits to rbx/signal.h and the intrusive_ptr helpers. Link A, built before those edits, did not lose them.
  - The world agent's `World::reset`, `SimBody`, `IndexedTree::fastRemove`/`setIndexedTreeParent` and `KernelData::removeBody`. SimBody's assert lines are off by 1.

**STUDIO losses against sweep14 (final link B, round 3): 2.** Both are the world agent's: `IndexArray<IndexedTree>::fastRemove` 0x009a29a0 and `IndexedTree::setIndexedTreeParent` 0x00a44060.
- Link A had lost 23, mostly the Base `signal::disconnectAll`/`intrusive_ptr` copies from base-pairing's work in progress. Those are back on link B.
- **None is in my files.** STUDIO pairs the folded `_Incsize` correctly: TimerService::delay is 100% there.

## Done

### 1. StarterGuiService moved into PlayerGui.cpp
- Its four bodies (getCoreGuiEnabled, the EnumDesc ctor, setCoreGuiEnabled and the ctor), plus sStarterGuiService and its registrar, now live in `v8datamodel/PlayerGui.cpp` in address order. StarterGuiService.cpp is deleted (`git rm`).
- Every 100% held in both targets.
- **The ctor 0x0070c150 (STUDIO 0x00798860) stays at 97.44%.** Its only diff is cause B from last wave: the `prime_list_template<unsigned>::value` one-past-end operand (orig `"d\xf7\xc6"` against our assert string). That is a reccmp-side rule, so the marker stays STUB.

### 2. RunService heartbeat
- base-pairing's `Heartbeat::expirationTime` is now a `Time` (it is in the tree).
- **raiseHeartbeat 0x0072da30: 64.4% → 97.87%.** The bytes show that 2013 passes **gameStep**, not wallStep, to `scriptHeartbeatSignal`: the `fld [esp+0x48]` is the gameStep local.
- The only diff left in raiseHeartbeat is call pairing: the original's `signal_with_args<1, void(const Heartbeat&)>::operator()` 0x0072d630 is unpaired, and ours folds with the `lua_State*` copy.
- **HeartbeatTask::stepDataModelJob 0x0072dc30 stays at 99.45%, for a pairing reason.** It calls the engine copy of `weak_ptr<T>::lock` (0x0049ccc0, the variant with `push ecx`, 273 callers). Our single folded lock body pairs with the shell LTCG copy 0x00439eb0 instead (37 callers), through a libdata row named `weak_ptr<AsyncHttpQueue>::lock`.
  - I measured the trade with `build-dm/w8/lockcost.py`. Re-pairing the lock to 0x0049ccc0 would gain 5 (HeartbeatTask, DebrisService's `cleanup` 0x008446f0, GcJob 0x00679e60, WaitingScriptsJob 0x0068a4e0, 0x0088edf0) and lose the 5 callers of 0x00439eb0 that are at 100% now.
  - It is the LTCG/native fold problem, the same as cause A. It is not fixable from source.

### 3. InsertService, TimerService, PhysicsSettings, DebrisService
- **InsertService::onServiceProvider 0x007de310 (STUDIO 0x00879f40): FUNCTION in both targets.**
  - Backend only: it disconnects the three backend connections and reconnects them through `boost::bind` to the three handlers.
  - The assert is on 2013 line 178, placed by layout.
  - The handlers backendInsertRequested 0x007ddf70, backendInsertAssetVersionRequested 0x007de070 and backendInsertAssetRequested 0x007de1c0 have iOS signatures. They are STUB bodies, because they fire RemoteEventDescs that wait on the reflection block.
- **TimerService: new class, `V8DataModel/TimerService.h` and `v8datamodel/TimerService.cpp`.**
  - Layout proven, 0x78: `DescribedCreatable<..., PERSISTENT_HIDDEN>` (RTTI `$0L@`), Service at 0x68, HeartbeatInstance at 0x60 (polymorphic, so placed first), and `std::list<Item>` at 0x6c. An Item is `{Time; boost::function0}` in a 0x30 node.
  - FUNCTION in both targets: onHeartbeat 0x007a6b70 / 0x008bb290, the ctor 0x007a6d10 / 0x008bb430, and the inline onServiceProvider 0x007a6da0 / 0x008bb4c0. That last one is `add ecx,0x60` plus a tail jump, because Instance::onServiceProvider is empty and inline.
  - delay 0x007a6a30 is FUNCTION in STUDIO (0x008bb150). It stays STUB in PLAYER at 98.28%, because of the `_Incsize` pairing above.
  - It registers in FactoryRegistration.cpp: its 2013 classDescriptor copy is 0x00789270, in that block.
- **PhysicsSettings: new class, `V8DataModel/PhysicsSettings.h` and `v8datamodel/PhysicsSettings.cpp`.**
  - Layout proven, 0x70: `GlobalAdvancedSettingsItem<PhysicsSettings>` plus a double at 0x68 (throttleAdjustTime, 1.0).
  - The ctor 0x0085da60 / 0x009424e0 is FUNCTION in both targets.
  - These reach 100% through the regenerated CSVs: the item ctor 0x0085d920, singleton 0x00654470, and about 30 vftables, Creator and descriptor rows. The ctor and the singleton carry TEMPLATE markers.
  - PhysicsSettings registers in FactoryRegistration.cpp (2013 classDescriptor 0x007874f0).
- **PhysicsInstructions ctor 0x0084d2f0 / 0x009a4fe0: 83.8% → 100%**, now that it reads the PhysicsSettings singleton. FUNCTION in both targets.
- **DebrisService gained addItem 0x00845000 and onServiceProvider 0x00845380.** Both are STUB:
  - addItem is at 98.98%, because the folded `function0` ctor pairs with the DataModel binder twin.
  - onServiceProvider is at 99.22%, because `shared_from` 0x0091d170 is unpaired.
  - Both bodies match byte for byte; only the pairing is missing.

### 4. STUDIO scored this pass
- These STUDIO twins measured 100% and their markers flipped: TimerService ×4, PhysicsSettings, PhysicsInstructions and InsertService::onServiceProvider.
- getCoreGuiEnabled 0x007960b0 and setCoreGuiEnabled 0x007963f0 were already FUNCTION and stay at 100%.
- These stay STUB:
  - The StarterGuiService ctor 0x00798860 (97.44%, cause B).
  - raiseHeartbeat 0x0079b6b0 and HeartbeatTask 0x0079b850 (call pairing, as in PLAYER).
  - The InsertService ctor 0x008775b0 (99.28%: one signal ctor call pairing).

### 5. Template arguments: wrong functionality values in V8DataModel headers
`build-dm/w8/func_check.py` compares each header's `Described(Non)Creatable<…>` arguments with the 2013 RTTI names. It found 29 wrong classes, all now fixed. These template arguments were the largest single gain of this pass, about 140 PLAYER entries: classDescriptors, Described ctors/dtors, the vftables that pair by RTTI name, the find/create copies and the Creator rows.

| What was wrong | Classes | Fix |
|---|---|---|
| RUNTIME_LOCAL where 2013 has the default 27 | AssetService, CollectionService, ContextActionService, CookiesService, FWService, GamePassService, GeometryService, PersonalServerService, PhysicsService, RenderHooksService, ScriptInformationProvider, ScriptService, SocialService, SpawnerService, TeleportService | Default (27) |
| Wrong functionality | JointsService | INTERNAL |
| | Camera | PERSISTENT_LOCAL (25) |
| | HttpService | PERSISTENT_HIDDEN (11) |
| | MegaClusterInstance | PERSISTENT_HIDDEN (11) |
| | TouchTransmitter | INTERNAL (3) |
| | MarketplaceService | RUNTIME (19) |
| Non-creatable in our tree, creatable in 2013 | BadgeService, ChangeHistoryService, ServerScriptService, TestService, UserInputService, ReplicatedStorage | Creatable, default 27 |
| | FriendService | Creatable, INTERNAL |
| | ServerStorage | Creatable, PERSISTENT_LOCAL |

The two DataModel.h TEMPLATE namerefs for `Described<ScriptService>` and `Described<PhysicsService>` were updated from `$0BB@` to `$0BL@`.

- **Left as they are:**
  - Lighting: 2013 says 11. The header documents a measured fold trade with Teams.
  - LocalBackpack and PlayerHUD: 2013 has `NonFactoryProduct<RelativePanel, …>`, but our RelativePanel is a non-Instance GUI widget (Client/App/include/Gui, row 5h).
- **Other near misses:**
  - DataModelJob::getPriorityFactor 0x0070ce10 was rewritten from the bytes: `shared_dynamic_cast`, `isPerPlayer ? 1 : std::max(1, players)`, one fild. It went from 33% to 97.8%.
  - What is left is one EH state. The original registers no state for the `getArbiter()` temporary, and DataModelJob::step 0x0070ccb0 shows the same pattern. Not solved.

## Layouts proven
- TimerService 0x78.
- PhysicsSettings 0x70.
- 2013 template arguments (functionality and Creatable versus NonCreatable) for the 29 service classes listed in section 5.

## Blocked / for other owners
- **Generator (owner):**
  - The `_Incsize` row above.
  - The `weak_ptr::lock` twin: the shell LTCG copy 0x00439eb0 against the engine copy 0x0049ccc0.
  - The `signal_with_args<1, Heartbeat const&>` copy 0x0072d630.
  - The DebrisService `function0` and `shared_from` twins.
- **Include spelling of Util/Object.h (util-target / base-pairing):**
  - Many Creator/static_getCreator/getClassName/isNullClassName copies differ only in the `util/object.h` / `Include\util/object.h` / `Util/Object.h` spelling of the TU that emitted them. Examples: ChangeHistoryService 0x0063c720 (in DataModel.obj in 2013), Decal/Texture/Sky/FileMesh 0x00489360…, InsertService 0x00565c70, RenderHooksService isNullClassName 0x004852e0.
  - The fix is per TU and per target; I did not attempt it.
- **Network::Player (owner of Client/Network):** PhysicsInstructions::changeSimulationRadius 0x0084cb70 and changeMaxSimulationRadius 0x0084cbe0 need:
  - The Player floats +0xcc (simulationRadius) and +0xd0 (maxSimulationRadius).
  - The setters 0x0052e510 and 0x0052e570, which clamp against 0xb01654 and the max, then raise a property change.
  - Bodies, from the bytes: `timeSinceLastRadiusChange = 0; RBXASSERT(player) /*line 55*/; player->setSimulationRadius(player->get…()*factor)`. The max variant has no timer reset and its assert is on line 63.
- **Base (base-pairing):** DataModelArbiter::preStep/postStep 0x0070cbd0/0x0070cc10 call RbxInterlockedIncrement/Decrement out of line and then OnScreenProfiler::UpdateJobStart/End. Arbiter::preStep's counter must use the helper call, and OnScreenProfiler is not in the tree.
- **Rendering (second contributor):** Workspace, ModelInstance and RootInstance still wait on IAdornable 0x18. completeShutdown 0x00641c90 stays at 96.2%.

## Checks
- `python3 -I tools/check_2016_copies.py`: silent.
- `reccmp_addr_padding`: 0 bad markers in 1,669 files.
- `reccmp-decomplint`: my new files are clean. DataModelJob.cpp's two `function_out_of_order` errors (lines 135 and 158) and the RunService/DebrisService warnings were already there.

## Files
- **Edited:**
  - Client/App/v8datamodel/: PlayerGui.cpp, InsertService.cpp, DebrisService.cpp, PhysicsInstructions.cpp, FactoryRegistration.cpp, RunService.cpp, DataModelJob.cpp.
  - Client/App/include/V8DataModel/: StarterGuiService.h, InsertService.h, DebrisService.h, DataModel.h, and the 29 service headers listed in section 5.
  - tools/row_coverage.py (row 5b file set).
- **New:** V8DataModel/TimerService.h, V8DataModel/PhysicsSettings.h, v8datamodel/TimerService.cpp, v8datamodel/PhysicsSettings.cpp.
- **Deleted:** Client/App/v8datamodel/StarterGuiService.cpp.
