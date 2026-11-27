# Wave 20: App and Base follow-ups (app-20)

- **Owner and scope:** Client/App (Client/Base untouched this wave). No commits. No IDA database opened. Shared `reccmp/` not written. Nothing under Client/Rendering/ touched.
- **Header note for the other agents:** `app-20-headers.md`, written first, at 21:25.
- **Build dirs:** `build-ap/` (PLAYER) and `build-ap-studio/` (STUDIO).
  - Scripts, probes and logs: `build-ap/w20/` (`gen_cv.py` generated the CommonVerbs member classes; `plink.sh` is the probe relink).
  - Snapshots: `build-ap/w/snaps/W20*`. Runs: `build-ap/w/runs/W20*`.
- **How I measured:** private root `build-ap/w/root`, with `tools/` and `reccmp-project.yml` refreshed from HEAD 3327f67.
  - Each snapshot got the full protocol (`ab.sh`): shared CSVs, two runs, regenerate, two runs, regenerate, final run.
  - The last two snapshots per target (W20F, W20SF) got one run each on the CSVs regenerated from the previous full-protocol snapshot.
  - Every scoring link deleted the .pdb first.
- **Memory:** every heavy command went through `tools/capped.sh`, one build at a time, no cap hits.

## Figures (effective 100%)

| | baseline (same tree, my item 1 only) | final | gained | lost |
|---|---|---|---|---|
| PLAYER | W20B0r 13,044 | **W20Fa 13,112** | 68 (53 in my files) | **0** |
| STUDIO | W20S0r 12,223 | **W20SFa 12,274** | 51 (38 in my files) | **0** |

- **Comparison with sweep26:** 13,039 and 12,216.
- **Shared tree:** the totals include the network and shell agents' uncommitted work (ServerReplicator, StatsUpdateJob, `View::~View`).
- **Coincidences on the way:** intermediate PLAYER links lost and later regained 13 no-.reloc coincidences: Lua `0x7fffff` masks, `Math::isDenormal`/`isNanInfDenormVector3`, `IsOlderOrderedPacket`, `httpGetPostWinHttp`. I read each one with `--verbose`; each was an immediate equal to one of our data addresses. All 13 are back at 100 in W20Fa.

## 1. Headers others needed (landed 21:25; see app-20-headers.md)

- **ScriptContext.cpp:** gained `FLog::NetworkReadItem` (default 0, GLOBAL 0x00de3f85 / 0x00f84e5e) and `FLog::JoinSendExtraItemCount` (default 1, GLOBAL 0x00d30f1e / 0x00ec180a).
  - They sit in one `namespace FLog { ... }` block after NetworkCache's.
  - I freed the 9 lines it needed from blank lines and one comment, so the file kept its line count.
  - network-5f-11 had already deleted its copies when I first linked.
- **The 2013 initializer order at 0x00a66090..0x00a66170:** Network, HttpQueue, Player, NetworkCache, ReplicationDataLifetime (0x00de3f83), NetworkInstances (0x00de3f84), NetworkReadItem, JoinSendExtraItemCount.
  - HttpQueue and Player are still defined elsewhere in our tree.
- **Game.h:** `boost::shared_ptr<OverlayDataModel> getOverlayDataModel() const` is inline. The shell copy is PLAYER 0x0041ce60 / STUDIO 0x004d5770; no marker.
- **UserController.h:** ControllerService +0x64 is `UserInputBase* hardwareDevice`, with inline `setHardwareDevice`.

## 2. Security::Impersonator: cause narrowed, not fixed

Facts from the 2013 bytes:

- **release (0x0041e8a0) and reset (0x0041e8f0) are LTCG functions with a dynamically realigned frame** (`push ebp; mov ebp,esp; and esp,-8`).
- **They are never inlined anywhere in the 2013 shell.**
  - The shell's Impersonator ctor and dtor copies call them.
  - So does a shell function at 0x0041e302..0x0041e5d4, which inlines the Impersonator destructor (`push previous; call ptr; call reset` at 0x0041e4ef and 0x0041e5c8).
- **Native callers:**
  - `Context::current` (0x0053ac72, 0x0053ac85) calls the shell reset.
  - Other engine functions inline reset after calling ptr (0x0058fab2, 0x00649a55, 0x0068647c, 0x00686765).
  - Only the shell's Impersonator ctor calls release.
- **No 2013 shell function calls the Impersonator ctor.** All 9 call sites are engine code. The shell owns the copies only because LTCG emits them for the native references.
- **In our link reset is ICF-folded across every `thread_specific_ptr<T>`** (0x006cb990 = Job, GenericJob, Limits::Counter, string, Context). 2013's 0x0041e8f0 is the same folded body.

Probe relinks (`build-ap/w20/plink.sh`, private objects only):

- **Stand-ins:** `w19/shellimp.cpp`, and `w20/shellimp2.cpp`, whose caller has a `volatile double` local (an 8-byte-aligned frame).
- **Result:** in both, our LTCG inlines release and reset into the shell Impersonator copies, and never gives them an aligned frame.

Working hypothesis and status:

- **Hypothesis:** VC9's LTCG does not inline a callee that needs dynamic realignment. What gave 2013's release and reset that requirement is still unknown; it is not an aligned caller.
- **Next lead:** the shell's real callers (the 0x0041e302 function), as with the wave-12/13 singleton() frame.
- **Status:** the ctor and dtor stay STUB, and `Players::onChildAdded` stays at 99.28.

## 3. Game, CommonVerbs, Commands.cpp

### shutdownGameDataModel 0x00654400: FUNCTION

- It calls `clearVerbs(false)`. The inlined clearVerbs keeps the dead LegacyLock branch's EH frame, which is the 2013 frame.
- Probe-identical, then 100 in W20A.

### CommonVerbs: full layout written

**CommonVerbs.h** has the 62 real members, offsets 0x000..0x704, SIZE 0x724:

- Classes from the RTTI of each vptr the ctor stores or each ctor it calls.
- 26 of them are `TToolVerb<X, RunStateVerb>`.
- Against 2016: Group and Ungroup verbs, no Color or Material verbs, SetGridToOne / OneFifth / Off, and `TToolVerb<NullTool>`.
- STUDIO has the same layout.

**Commands.h** gained the verb classes:

- **Bases:**
  - RunStateVerb (0x1c, vftable 0x00b7c12c / 0x00bfa9bc).
  - EditSelectionVerb (0x24, vftable 0x00b7c14c / 0x00bfa9dc).
  - DeleteBase (0x28), BoolPropertyVerb (0x28), MoveUpSelectionVerb (0x28, inline ctor), RotateAxisCommand (0x24, with a `getRotationAxis` slot at +0x1c).
- **All 37 member classes and the TToolVerb template.**
- **Declared virtuals:** only the doIt overrides each 2013 vftable shows, plus RotateAxisCommand's `getRotationAxis`. Other overridden slots (isEnabled, isChecked, getText) are not declared yet.

**FUNCTION in both targets** (each one was probe-identical first, then measured in a link):

- `~CommonVerbs`: PLAYER 0x006531e0 / STUDIO 0x007cbbb0, Game.obj, now a user-declared empty dtor.
- `CommonVerbs::CommonVerbs`: 0x00864450 / 0x009d5e90.
- `~RunStateVerb` and `~EditSelectionVerb`: 0x00857430/0x008574b0 / 0x00781a70/0x00781af0.
- **Base ctors:** RunStateVerb 0x00857780, EditSelectionVerb 0x00858c10 (`workspace(shared_from(dataModel->getWorkspace()))`), DeleteBase 0x00858d10, RotateAxisCommand 0x008590c0.
- **Command ctors:** the 15 Verb-derived ones (FirstPersonCommand "FirstPersonIndicator", Stats, …, SetManualJointToInfinite), Group, Snap, MoveDown, Rotate, Tilt, and MoveUpPlate/MoveUpBrick (0.4f / 1.2f).
- **ToggleViewMode:** ctor, isEnabled (`getNumPlayers() > 0` through the DataModelArbiter base) and doIt (FASTLOG `Gui:ToggleViewMode`, then `switchViewMode()`).
- **CameraVerb::doIt** (`RBXASSERT(dataState!=NULL); dataState->setDirty(true)`).

Supporting changes:

- **New STUB for `DataModel::switchViewMode`** (0x0064f630 / 0x0076c0e0) in DataModel.cpp, so ToggleViewMode::doIt pairs. It took 4 lines (a blank line and a 3-line comment) and kept the file's line count.
  - An earlier version also took 2 blank lines inside onRunTransition. That moved its assert off 2013's line 1953 and cost it, so I restored them.
- **Rows paired with no work:** the generator paired these from the new code:
  - Folded deleting destructors 0x00861ac0 and 0x00862ed0.
  - `checked_delete<CommonVerbs>` and `sp_counted_impl_p<CommonVerbs>::dispose`.
  - Vftables: SelectAllCommand, UnlockAllVerb, AllCanSelectCommand, RunStateVerb, ToggleViewMode, CameraCenter, ZoomExtents.

**Layout and stand-ins:**

- **Commands.cpp:** CameraVerb::doIt's asserts sit on 2013 lines 1066 (STUDIO) and 1073 (PLAYER), in `#ifdef OGRE_TARGET_STUDIO` / `#ifndef` blocks. No macro: a macro would expand NULL to 0 inside the assert string. Blank padding before the function sets the lines.
- **TToolVerb ctor:** `__declspec(noinline)` is a stand-in. The real ctors are larger COMDATs in CommonVerbs.obj (0x00861c70..0x00864320), and our STUB bodies would otherwise inline into the CommonVerbs ctor.
- **TToolVerb bodies:** they use a function-local static, so `/OPT:ICF` cannot fold the 26 instantiations.
- **Anchor / Translucent / CanCollide ctors:** written but still STUB. 2013 compiles them in CommonVerbs.obj. Ours sit in Commands.cpp with BoolPropertyVerb's STUB ctor (0x00859130), which therefore inlines into them. BoolPropertyVerb takes `const std::string&`: its callers destroy the name temporary.

### STUDIO Game items: pairing only

| Function | Score | Cause |
|---|---|---|
| `~Game` | 99.07 | STUDIO's two shutdownDM copies 0x007cc000/0x007cc090 differ only in their `reset` callee (0x004b0120 vs 0x00a74630, two unfolded `shared_ptr::reset` copies) |
| `Game::Game` | 99.65 | One `shared_ptr::operator=` call pairs with a different folded twin |
| setupDataModel | 98.1 | The wave-19 `_Insert_n` twin |

## 4. Not done

- **openState:** no new idea.
- **DataModel/ScriptContext unnamed groups:** not started.
- **Near misses I read:**
  - **`LegacyLock::Implementation` ctor (99.25):** 2013's assert is at DataModel.cpp:1054. The whole Implementation class sat above getGenericJob (1150), where ours is at about 1525. Moving it means re-laying the file between 1000 and 1150.
  - **`ScriptContext::setTimeout` (99.41):** a `shared_from<DataModel>` twin (pairing).

## Patterns proven

- **An inlined function's dead branch can leave its EH frame behind.** Calling `clearVerbs(false)` inlines `if (useLock) { LegacyLock lock(...); doClearVerbs(); } else doClearVerbs();`. The lock's unwind frame survives with no state stores (`shutdownGameDataModel` 0x00654400); a direct `doClearVerbs()` has no frame.
- **A verb's implicit destructor inlines to its nearest user-declared base destructor.** The members of `~CommonVerbs` call `~RunStateVerb`, `~EditSelectionVerb` or `~Verb` directly. Deleting destructors fold per base (0x00861ac0, 0x00862ed0).
- **The two by-value-string conventions show in the caller.** A base ctor that takes `std::string` by value destroys the temporary itself (the caller has no EH frame; Group 0x00858cb0). One that takes `const std::string&` leaves the dtor call to the caller (Anchor 0x00861a40 → BoolPropertyVerb 0x00859130).
- **Assert lines that differ per target cannot go through a wrapper macro when the expression contains NULL.** The wrapper's parameter is macro-expanded first, so the string becomes `dataState!=0`. Use `#ifdef` around a plain RBXASSERT.

## Checks

- reccmp_addr_padding: 1,717 files, 0 bad.
- check_2016_copies: silent.
- **FUNCTION markers:** every one I added or flipped is at 100 in its target's final run (W20Fa PLAYER, W20SFa STUDIO), and was probe-identical (`tools/probe_compare.py`) before.
- **Assert lines kept:**
  - ScriptContext.cpp: line count unchanged.
  - DataModel.cpp: line count unchanged, and onRunTransition is back at 1953.
  - Commands.cpp: 1066 and 1073.
- **Not run:** decomplint, check_folded, vtable and datacmp. That is left to the integrator.
- **Expected decomplint:** Commands.cpp markers ascend by PLAYER address. STUDIO ascends too, except the CommonVerbs.obj COMDAT ctors (STUDIO 0x009d28c0..0x009d3580) placed after Commands.obj ones, which is still ascending.

## Unresolved

1. **Integrator:**
   - Regenerate the shared CSVs. Commands.cpp now has 56 PLAYER STUB markers and 32 PLAYER FUNCTION markers (STUDIO: 32 FUNCTION), plus CommonVerbs.cpp and Game.cpp.
   - The two new VTABLE markers (RunStateVerb, EditSelectionVerb) will not be 100: their other slots are not declared yet.
2. **Shell (shell-5k-11):**
   - Delete the `OGRE_APP_INPUT_ACCESSORS` stand-ins in View.cpp and UserInput.cpp, then measure (app-20-headers.md).
   - The Impersonator lead is in item 2.
3. **App, next:**
   - The remaining Commands.cpp bodies: doIts and the undeclared isEnabled/isChecked/getText slots. Their 2013 slot addresses come from `build-ap/w20/rtti.py` (RTTI chain + first 9 slots per vftable) and `gen_cv.py`.
   - The camera doIts and isEnabled, and the CameraVerb, CameraCenter and ZoomExtents ctors. All need Workspace's 2013 bases: a camera-owner base at +0x128 whose slots 1/2 are getCamera/getConstCamera, and VerbContainer at +0x150.
   - The SetGrid / ManualJoint doIts, which need the globals 0x00e14628 (drag grid mode) and 0x00d98b40 (manual joint flag) named.
   - BoolPropertyVerb's ctor 0x00859130 (`Name::lookup` 0x006138c0) and the Ungroup / SelectChildren ctors (extra callees 0x00858dc0 / 0x00858f40).
   - Moving the Anchor / Translucent / CanCollide / MoveUp ctors (and the 26 TToolVerb ctors) to a CommonVerbs.cpp-side home, since 2013 compiles them in CommonVerbs.obj.
   - Re-laying LegacyLock::Implementation to DataModel.cpp:1054.
4. **IDA owner (names):**
   - **TToolVerb ctors:** the 26 TToolVerb ctors at 0x00861c70..0x00864320 (AxisRotate, AdvMove, AdvRotate, AdvArrow 0x00862100, MoveResizeJoin 0x008621a0, Flat, Glue, Weld, Studs, Inlet, Universal, Hinge, RightMotor, LeftMotor, OscillateMotor, SmoothNoOutlines, Anchor, Lock, Fill, Material, Dropper, Null, Game, Grab, Clone, Hammer).
   - **Other functions:**

     | PLAYER | STUDIO | Name |
     |---|---|---|
     | 0x00857780 | 0x00781ee0 | RunStateVerb ctor |
     | 0x00857430 | 0x00781a70 | ~RunStateVerb |
     | 0x00858c10 | 0x007834c0 | EditSelectionVerb ctor |
     | 0x008574b0 | 0x00781af0 | ~EditSelectionVerb |
     | 0x00858d10 | 0x00783740 | DeleteBase ctor |
     | 0x00859130 | | BoolPropertyVerb ctor |
     | 0x008590c0 | 0x00783980 | RotateAxisCommand ctor |
     | 0x0041e302 | | the shell function that inlines the Impersonator dtor |

## Files

- **App, modified:** include/V8DataModel/{Commands,CommonVerbs,Game,UserController}.h, script/ScriptContext.cpp, v8datamodel/{Commands,CommonVerbs,DataModel,Game,UserController}.cpp.
- **Notes:** notes/runs/2026-11-23-phase5-wave20/{app-20-headers,app-20}.md.
