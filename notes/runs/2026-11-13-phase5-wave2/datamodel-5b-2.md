# datamodel-5b-2 (workflow phase5-wave2, wf_cba1073c-3f1)

Phase 5b second pass (DataModel and the core services). PLAYER row 5b went from 201 to 311 of 1,433 functions at effective 100%, measured over identical file ranges (13.8% to 21.7%). The baseline is build/sweep7/P-final.json and the final run is build-dm/dm-scores/P11.json. 102 of the 110 come from the first attempt's TEMPLATE markers, which tools/unpaired_twins.py wrote into my headers (DataModel.h, GuiBuilder.h, Workspace.h, Team.h and others); I rebuilt and measured those edits and kept them. The other functions are new this pass. Tree-wide effective-100 entries: PLAYER 5,972 -> 6,146 and STUDIO 5,848 -> 6,007 (S3.json). These totals include other agents' concurrent edits.

**Lost 100% entries.** None in my files, in either target. I checked every loss with jd/--verbose:
- The boost exception classes and vftables come from the BOOST_*_DYN_LINK change in CMakeLists.
- The Reflection PropDescriptor/TypedPropertyDescriptor vftables come from the reflection header edits.
- GuiMain/Frame vftables, Soundscape MethodProp<SoundChannel,bool>::setValue 0x005b5d60 and Instance::getPersistentDataCost come from GUI edits. PropDescriptor<GuiObject,bool>::setValue now folds into the same body.
- luaV_execute, symbexec, patchtestreg and others are the known 0x7fc000 immediate coincidence.

**Functions added or raised (PLAYER / STUDIO, before -> after):**
- **RunService**, new 2013 layout: Service @0x60, physicsJob @0x64, heartbeatTask @0x6c, runState @0x74, 4 doubles @0x78, 6 signals @0x98..0xac, SIZE 0xb0. runTransitionSignal @0xa4 is proven by initializeContents 0x0064f559 and heartbeatSignal @0xa0 by HeartbeatInstance 0x00871186.
  - onServiceProvider 0x0072bc30 / 0x00799740: new -> 100 / 100.
  - ctor 0x0072efd0 / 0x0079cc10: new -> 100 / 93.7. This took TEMPLATE markers for the five signal ctors 0x0072b9e0 / 0x0072ba40 / 0x0072baa0 / 0x0072bb00 / 0x0052e6c0, which score 86.4.
  - stopTasks 0x0072bb60 / 0x00799670: new -> 96.6. Only the call to TaskScheduler::remove(shared_ptr<Job>) 0x00573720 doesn't pair.
- **DebrisService**, 2013 layout SIZE 0x94; the RTTI functionality is 27, not RUNTIME_LOCAL:
  - cleanup 0x00844c20 / 0x0098a5f0: new -> 100 / 100.
  - static cleanup(weak_ptr) 0x008446f0 / 0x0098a220: 97.5.
  - setMaxItems 0x00844e90 / 0x0098a860: 96.4.
  - ctor 0x008455c0 / 0x0098af90: 97.8 (STUDIO 35.1 -> 97.8).
- **Teams:**
  - getTeamFromTeamColor 0x007a4660 / 0x008a2f50: new -> 100 / 100.
  - getTeamFromPlayer 0x007a4770 / 0x008a3060: new -> 100 / 100. Needed Player::teamColor as a BrickColor @0x68 (neutral @0x6c).
  - teamExists 0x007a4d40 / 0x008a3630: new -> 100 / 100.
  - assignNewPlayerToTeam 0x007a4fc0 / 0x008a37e0: new -> 100 / 100.
  - isTeamGame 0x007a4ae0: 98.9.
  - getNumPlayersInTeam 0x007a4bf0: 99.1, assert on its 2013 line 97.
  - getTeamColorForHumanoid 0x007a5130: 97.7, assert on line 160.
  - The last three differ only in calls whose original targets don't pair with ours.
- **DataModel:**
  - addCustomStat 0x0062d7d0: 95.5 -> 100, through a STUB marker on GuiBuilder::addCustomStat 0x007f29a0 / 0x008b5780.
  - arbiterName 0x0064de40: 64.0 -> 81.5; it now returns getName(). The rest is the out-of-line Instance::fw().
- **Lighting**, partial layout: lightingChangedSignal @0x1e0, sky @0x1e4, SIZE 0x1f0.
  - onChildChanged 0x00706920: 93.8.
  - askAddChild 0x007074c0: 96.7.

**Written, measured, then backed out.** Each would have dropped other functions from 100%:
- Teams::onChildAdded/onChildRemoving (97.9 / 97.4) and Lighting::onChildAdded/onChildRemoving. Their shared_from<T> and shared_ptr<T>::reset instantiations join our linker's single folded body, which breaks its pairing. Instance::onAncestorChanged/onDescendantAdded, DataModel::onChildChanged and GuiItem::onDescendantRemoving left 100%.
- Typed RunService job members: they are now held as shared_ptr<TaskScheduler::Job> for the same reason.
- "util/object.h" spelling in DataModel.obj: FactoryProduct<Visit>::static_getCreator 0x00630f20 went to 100, but DataModel.obj then supplies the lowercase COMDAT copy of three other static_getCreator<T> bodies (0x00583dc0 and others) under our link order.
- Player RTTI functionality 19 ($0BD@) and Lighting 11 ($0L@): the CSV rows that pair by our old mangled names stop pairing, and Lighting's template bodies fold with Teams'.

**Checks:** reccmp_addr_padding.py reports 0 bad markers. decomplint on my files shows only function_out_of_order, and only in DataModel.cpp and GuiBuilder.cpp (the prior agent's 2013-line layout). I removed the GLOBAL markers I had put on file statics (prop_creatorId/Type/Genre, prop_MaxItems) because they fail to parse (global_not_variable) and never paired; DataModel.cpp's line layout is back to what it was.

Builds were in build-dm and build-dm-studio. Final JSONs are in build-dm/dm-scores/ (P0, P10, P11, S3), moved there because tmpfs /tmp hit its quota mid-run.

## Files changed

- Client/App/v8datamodel/RunService.cpp
- Client/App/include/V8DataModel/RunService.h
- Client/App/v8datamodel/DebrisService.cpp
- Client/App/include/V8DataModel/DebrisService.h
- Client/App/v8datamodel/Teams.cpp
- Client/App/include/V8DataModel/Teams.h
- Client/App/v8datamodel/Lighting.cpp
- Client/App/include/V8DataModel/Lighting.h
- Client/Network/Player.cpp
- Client/Network/include/Network/Player.h
- Client/Network/Players.cpp
- Client/App/v8datamodel/DataModel.cpp
- Client/App/v8datamodel/GuiBuilder.cpp
- Client/App/include/V8DataModel/DataModel.h
- Client/App/include/V8DataModel/GuiBuilder.h
- Client/App/include/V8DataModel/Workspace.h
- Client/App/include/V8DataModel/Team.h
- Client/App/include/V8DataModel/StarterGuiService.h
- Client/Network/include/Network/Players.h
- build-dm/dm-scores/P11.json
- build-dm/dm-scores/S3.json
- build-dm/dm-scores/cmp-P10-vs-base.txt
- build-dm/dm-scores/cmp-S3-vs-base.txt

## Unresolved

- Instance.h owner: Instance::fw() is out of line in 2013 (0x006beaa0). It is the only remaining diff in the DataModel ctor 0x0064ec50 (97.3%, besides two pairing-only operands) and in arbiterName 0x0064de40 (81.5%), and it also appears in Players::onDescendantRemoving 0x00569260.
- Base TaskScheduler.Job.h owner: 2013 Job::getArbiter() returns shared_ptr<Arbiter> by value and is out of line (0x005837a0). This blocks DataModel::onlyJobsLeftForThisArbiterAreGenericJobs 0x0063a120 (39.3%; its 2013 assert sits on DataModel.cpp line 615) and DataModelJob::step 0x0070ccb0. TaskScheduler::remove(shared_ptr<Job>) 0x00573720 is out of line too and needs a pairing; that alone keeps RunService::stopTasks 0x0072bb60 at 96.6%.
- Util/Limits.h owner: DataModelJob::step 0x0070ccb0 (23.9%) needs Limits::Counter::Activator (ctor 0x009b96b0, dtor 0x009b97f0), constructed from DataModelArbiter+0x830, around the virtual stepDataModelJob call.
- Integrate (CSV): the libdata rows for 0x0063e220 and 0x0064de30 are swapped. 0x0063e220 is DataModel::getNumPlayers (the IMetric-adjusted body). 0x0064de30 is the ??_E thunk (sub ecx,0xa8; jmp).
- Integrate (CSV): the row for 0x00526230 names Described<Network::Player,...,Functionality 19>, but our PDB has the classDescriptor at Functionality 27. Pairing it would raise Teams::isTeamGame (98.9), getNumPlayersInTeam (99.1) and getTeamColorForHumanoid (97.7).
- Integrate (CSV): getTeamColorForHumanoid also needs pairings for Instance::findConstFirstChildOfType<Humanoid> (0x005413f0) and BrickColor::color3 (0x00792510).
- Integrate (CSV): Lighting::askAddChild 0x007074c0 and Lighting::onChildAdded 0x00707510 need Described<Sky>::classDescriptor paired at 0x00706950.
- Integrate / build owner: 2013 RTTI says Player is functionality 19 (RUNTIME) and Lighting 11 (PERSISTENT_HIDDEN); ours use the default 27. Switching drops the CSV-paired ??_E/??_G thunks 0x00529040/0x00529050 and 0x00705750/0x00705760/0x007057a0/0x00705b00, plus FactoryProduct<Player>/<Lighting>'s vftables, until the CSVs are regenerated with the new names and the Lighting/Teams template fold is handled.
- Build owner (our /OPT:ICF fold vs the original's separate copies): any new shared_from<T> or shared_ptr<T>::reset instantiation breaks the pairing of the single folded body (0x00652fe0 reset; the shared_from twin). Teams::onChildAdded/onChildRemoving (0x007a4790 / 0x007a48c0 at 97.9 / 97.4) and Lighting::onChildAdded/onChildRemoving stay out until that is solved.
- Link order / CMakeLists owner: the 2013 DataModel.obj spells Object.h as 'include\util/object.h' (2016 GuiBuilder.h includes "util/object.h"). Our link puts DataModel.obj ahead of MarketplaceService/GuiImageButton/NotificationBox, so its lowercase COMDAT copy wins for those static_getCreator<T> (0x00583dc0 and others). Fixing the spelling needs those objects linked before DataModel.
- Security owner: DebrisService::setMaxItems 0x00844e90 pushes permission 2 where the 2016 source says LocalUser, which is 3 in our enum. The 2013 enum is probably missing a value (possibly Plugin). The setter stays at 96.4% until that is settled.
- Players layout not started: the 2013 dtor at 0x005705c0 shows a large class with localPlayer @0x19c and signals @0x1c0..0x1e8. Players::findLocalPlayer 0x0056baa0 needs that layout, and DataModel::renderGuiRoot 0x0062ee30 needs findLocalPlayer. The 2013 RTTI says Players is DescribedNonCreatable, not Creatable.
- Player::setTeamColor 0x00535090 and setNeutral 0x005350e0 are STUBs. Their 2013 object file is unknown; they sit among Network templates, not in the Player.obj range 0x005493e0-0x0054b9e0.
- Still blocked by other classes: StarterPack/StarterGui need the Hopper : RelativePanel : TopMenuBar : GuiItem chain (Hopper is 0x98 bytes). Workspace needs the ModelInstance/RootInstance bases, which also block renderStep 0x0062eea0 and processWorkspaceEvent 0x0062f100. DebrisService::addItem and onServiceProvider 0x00845380 need TimerService. The Lighting members 0x64..0x1e0 are not laid out.
