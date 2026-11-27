# Wave 21: App and Base follow-ups (app-21)

- **Owner and scope:** Client/App only. Client/Base was not touched.
- **What I did not do:** no commits, no IDA database opened, shared `reccmp/` not written, nothing under
  Client/Rendering touched.
- **Header note for the other agents:** `app-21-headers.md`. Written at 01:35; measured results added at 02:50.
- **Build dirs:** `build-ap/` (PLAYER) and `build-ap-studio/` (STUDIO).
  - Scripts: `build-ap/w21/`.
    - `probe.sh`: compiles one TU with build-ap's flags.
    - `plink.sh`: relinks from the rsp.
    - `q.py` and `diff.py`: JSON lookups and run diffs.
    - `ttv_globals.py` and `write_tools.py`: the tool-name globals.
    - `w21-funclet.py`: EH funclet targets.
  - Snapshots: `build-ap/w/snaps/W21*`. Runs: `build-ap/w/runs/W21*`.
- **How I measured:**
  - Private root `build-ap/w/root`, with `tools/` and `reccmp-project.yml` refreshed from HEAD 1ba2c85.
  - Every scoring link deleted the .pdb first.
  - A1 got the full protocol (`ab.sh`: shared CSVs, 2 runs, regenerate, 2 runs, regenerate, final run).
  - Later snapshots used `ab2.sh`: 2 runs on the previous snapshot's final CSVs, regenerate, final run.
- **Memory:** every heavy command went through `tools/capped.sh`, one build at a time, no cap hits.

## Figures (effective 100%)

| | sweep27 | final | won | lost |
|---|---|---|---|---|
| PLAYER | 13,117 | **W21A5r 13,266** | 149 | **0** |
| STUDIO | 12,280 | **W21S3r 12,381** | 106 | 5 (pairing only, see below) |

- **Shared tree:** the totals include the shell and network agents' uncommitted work (UserInput, FunctionMarshaller,
  StatsUpdateJob, ServerReplicator).
- **Items in my files:** about 125 PLAYER and 95 STUDIO entries.

**The 5 STUDIO losses are all the `shared_from<T>` twin.**
- **What reccmp shows:** each loss is a 99%+ body whose only diff is a call to our folded `shared_from` body.
  reccmp names that body `shared_from<RunService>`, while 2013 calls its separate `shared_from<Instance>`.
- **Affected:** PacketJob ctor 0x00620870, StatsUpdateJob ctor 0x006211c0, PacketReceiveJob ctor 0x0064b030,
  YieldFunctionStateObject ctor 0x009deec0, and `shared_ptr<RunService>(RunService*, Deleter)` 0x0082cdf0.
- **Known already:** network-5f-11 reported the same twin (StatsUpdateJob ctor at 99.36%), as did shell-5k-11
  Unresolved 3.
- **Probably not from my files:** taking Commands.h out of the new tool TUs (S2 → S3) changed nothing.
  - I did not build a STUDIO link without my changes, so this is not proven.

**Network entries that dropped and came back.** They were not real losses.
- In A1–A3, the Replicator/ServerReplicator entries dropped and came back in A4.
- Cause: the network agent was editing Replicator.cpp while my snapshots were being scored. reccmp resolves markers
  through the current source lines, so the markers went out of sync.
- **Lesson:** never edit a file while a snapshot that contains it is being scored.

## 1. Headers others needed: landed (app-21-headers.md)

- DFLog::NetworkJoin in ScriptContext.cpp.
- NetworkSettings `physicsSendMethod` + `getPhysicsSendMethod()`.
- `PartInstance::fromPrimitive`/`fromConstPrimitive`.
- `Mechanism::getConstRootMovingPrimitive`/`getRootMovingPrimitive`.
- `OverlayDataModel::DualLegacyLock`.

Every new function is FUNCTION in both targets.

**getConstRootMovingPrimitive needed two separate early returns.** `if (!a) return NULL; if (a->computeIsGrounded()) return NULL;`.
- The `||` form scores 86%: VC9 shares the return tail.
- Two returns duplicate the tail, as 2013 does. This was probe-identical (`probe.sh` + `probe_compare.py`).

## 2. Event.h markers and LegacyLock::Implementation: done

- **Event.h:** `Event::getInstance` 0x00572d90 / 0x00654d90 and `EventInvocation::replicateEvent` 0x00572dc0 /
  0x00654dc0 are FUNCTION header markers.
  - One blank line was taken from each of three places, so the line count is unchanged.
  - Nothing after line 524 is a 2013 anchor. The `RBXASSERT(0)` at 1108 is already off: 2013 has it at 797.
- **DataModel.cpp:** I swapped two blocks.
  - The `mainThreadId` / `LegacyLock::Implementation` / LegacyLock ctors block now sits at 976–1103, so the
    Implementation ctor's `RBXASSERT(job)` is on 2013 line 1054.
  - The clearContents + HTTP block took the old place, after `GenericJob::processTasks`.
  - 50 blank lines before onRunTransition absorb the difference.
  - Asserts 1150, 1270..1330, 1930 and 1953 are unchanged.
  - Result: Implementation ctor PLAYER 0x00641e60 / STUDIO 0x0075f6e0 went 99.25 → 100 (FUNCTION).

## 3. Commands

### TToolVerb ctors: all 26 at 100 in both targets

- **The 2013 body:** `Base(Tool::name().toString() + "Tool", dataModel), toggle(toggle)`.
- **Tool::name() is the inline `Name::declare<sXTool>()`:**
  - the NULL test on the name pointer;
  - `call_once(&callDoDeclare<sXTool>, flag)`;
  - then `doDeclare<sXTool>`.
- **Tool classes:** Commands.h now declares the 26 `RBX::sXTool` globals (iOS names) and 26 partial tool classes,
  each with only the inline `static const Name& name()`. The real tools derive Named<...>, 2016 shape.
- **Where each global is defined** (2013 reads it from memory, so it must be in another TU):

  | File | Status | Globals |
  |---|---|---|
  | `v8datamodel/ToolsSurface.cpp` | existing empty file | the 11 surface tools |
  | `tool/AdvMoveTool.cpp` | existing empty file | AdvMove |
  | `tool/AdvRotateTool.cpp` | existing empty file | AdvRotate |
  | `tool/MoveResizeJoinTool.cpp` | existing empty file | MoveResizeJoin |
  | `tool/NullTool.cpp` | existing empty file | NullTool |
  | `tool/ToolsArrow.cpp` | existing empty file | AdvArrow |
  | `tool/AxisRotateTool.cpp` | new, 2016 path | AxisRotate |
  | `tool/GameTool.cpp` | new, 2016 path | Game |
  | `tool/GrabTool.cpp` | new, 2016 path | Grab |
  | `tool/CloneTool.cpp` | new, 2016 path | Clone |
  | `tool/HammerTool.cpp` | new, 2016 path | Hammer |
  | `v8datamodel/ToolsModel.cpp` | new, 2016 path | Anchor, Lock |
  | `v8datamodel/ToolsPart.cpp` | new, 2016 path | Fill, Material, Dropper |

  - Each global has GLOBAL markers in both targets.
  - The tool files do not include Commands.h: they declare the extern themselves.
- **No stand-in needed:** the `__declspec(noinline)` stand-in and the `static int unfold` stand-in are gone. The
  CommonVerbs ctor stays at 100 without them, because VC9 keeps the ctors out of line by itself.
- **Pairing:** the ctors, 27 `callDoDeclare<sXTool>` and 27 `doDeclare<sXTool>` pair through their oracle names,
  without markers. The PLAYER doDeclare copies sat at 82% (only their two statics) until a second regeneration.
  The extra callDoDeclare/doDeclare is AdvArrow's, which comes from Workspace.obj (0x0070e7a0 / 0x0070d920).

### Other Commands work (FUNCTION in both targets)

**Verb ctors**
- `BoolPropertyVerb` ctor PLAYER 0x00859130 / STUDIO 0x007839f0: `propertyName(Name::declare(propertyName))`.
  +0x24 is now `const Name* propertyName`.
- `TranslucentVerb`, `CanCollideVerb` and `AnchorVerb` ctors (0x00860d10 / 0x00860d90 / 0x00861a40; STUDIO
  0x009d28c0 / 0x009d2940 / 0x009d3580). They moved to CommonVerbs.cpp, where 2013 compiles them, so they call
  BoolPropertyVerb's ctor instead of inlining it.

**Grid and manual-joint doIts**
- `TurnOnManualJointCreation::doIt` and the six SetGrid / SetManualJoint doIts (0x00856900..0x00856cf0;
  STUDIO 0x00780f40..0x00781320).
- **The two globals are AdvArrowToolBase statics** (iOS 10-29 names):
  - `advGridMode`: 0x00e14628, .bss, ONE_STUD.
  - `advManualJointMode`: 0x00d98b40, .data, true.
  - `advManualJointType`: 0x00d98b44, STRONG_MANUAL_JOINT.
- **Where they live:**
  - New partial header `include/Tool/ToolsArrow.h` holds the statics and the 2016 DRAG enums. The values match
    the 2013 stores.
  - Definitions with GLOBAL markers are in `tool/ToolsArrow.cpp`. STUDIO addresses: 0x00eb3b94 / 0x00eb3b98 /
    0x00f694f0.

**Rotation axis**
- `RotateSelectionVerb::getRotationAxis` 0x00856580 / 0x00780bb0: `return Math::matrixRotateY();`. This uses the
  out-of-line Matrix3 copy.

**Vftables**
- RunStateVerb, EditSelectionVerb and BoolPropertyVerb are at 100, plus the vftables listed in the PLAYER wins
  (Translucent, CanCollide, Anchor, MoveUp*, RotateSelection, RotateAxis, Delete*, CanNotSelect, PlayDelete).
- **New slot declarations:** `EditSelectionVerb::isEnabled` (+0x08) and `BoolPropertyVerb::isChecked` (+0x0c),
  STUB bodies at 0x0085c480 / 0x00786d70 and 0x0085bbd0 / 0x007864c0.
- **Why those two bodies are not written yet:**
  - Both go through `ServiceClient<Selection>::createService() const` 0x007145b0.
  - Both also go through an out-of-line Selection size helper, 0x0070eb00. It asserts `object.get() != 0` at
    `include\util/utilities.h:71` and reads `[[this+0x64]+8]-[+4] >> 3`. Selection's layout is not written yet.

### Workspace bases: not done (blocked on a layout)

- **2013 RTTI** (Workspace COL 0x00b49448) gives `RootInstance : Described<RootInstance, ModelInstance>, ICameraOwner`
  and `Workspace : DescribedNonCreatable<...>, Service, GuiTarget, VerbContainer, IMovingManager, IScriptFilter`, at
  ICameraOwner 0x128, GuiTarget 0x14c, VerbContainer 0x150, IMovingManager 0x164, IScriptFilter 0x178,
  Service 0x17c.
- **Our classes are too small:** ModelInstance is 0x9c (2013 0x128) and Workspace 0x100.
- **So the CameraVerb ctor (`Verb(workspace→VerbContainer*)`) and the camera doIts (ICameraOwner slot 1/2 on
  `workspace+0x128`) need ModelInstance's 2013 layout first.** The same goes for TToolVerb::isChecked: it calls
  `Workspace::getCurrentMouseCommand` 0x005290d0, which is out of line in 2013 with an assert at Workspace.h:251
  and +0x1c0.

## 4. TypedPropertyDescriptor\<SoundId\> copyValue: diagnosed, generator-side

- **The task's premise is wrong:** 2013 copyValue\<SoundId\> does not fold with copyValue\<ContentId\>.
  - The SoundId vftable 0x00b44c2c points at 0x006bc800, the TextureId/SoundId fold. Its funclet goes to the native
    thunk 0x004a2390.
  - copyValue\<ContentId\> 0x00669fd0 goes to the shell thunk 0x0041c8a0.
- **Our source is already right** (`build-ap/w21-funclet.py`):
  - our copyValue\<SoundId\> 0x558b60 unwinds to our native ~SoundId thunk;
  - our copyValue\<ContentId\> unwinds to the shell ~ContentId copy.
- **The pairing is what's wrong.** `player-libdata.csv` carries a sticky row,
  `00669fd0,,,?copyValue@?$TypedPropertyDescriptor@VSoundId@Soundscape@RBX@@@Reflection@RBX@@UBEXPBVDescribedBase@23@PAV423@@Z`.
  It gives our SoundId body to ContentId's original, so 0x6bc800 and our ContentId body stay unpaired.
  - That costs vftables 0x00b3388c / 0x00b33904 / 0x00b44c2c (each 13/14).
- **No source or shell stand-in can fix it.**
- **What I tried:** a private-generator probe that extended MATCH_FIX to the ios: rows. It did not take: the sticky
  libdata row still wins.

## 5. Not done

- openState and the DataModel unnamed group.

## Patterns proven

- **A template ctor's tool name inlines `Name::declare<sX>()` from a partial class.** A class with only
  `static const Name& name() { return Name::declare<sX>(); }` reproduces every TToolVerb ctor byte for byte, as long
  as `sX` is defined in another TU. In the same TU the NULL test folds away. 2013's `cmp dword [sX],0` needs the
  pointer read from memory.
- **Two early returns differ from one `||` return.** `if (!a) return NULL; if (f(a)) return NULL;` keeps two
  return tails. `if (!a || f(a)) return NULL;` shares one (Mechanism::getConstRootMovingPrimitive 0x007f9f90).
- **Scoring a snapshot while its sources change corrupts the run.** Markers resolve through the current source's line
  records, so edits made during a run (by you or another agent) unpair or misscore whole files.

## Checks

- `reccmp_addr_padding`: 1,726 files, 0 bad.
- `check_2016_copies`: silent.
- **assert_lines:**
  - ScriptContext.cpp: 321/356/506… unchanged, line count 3,404.
  - DataModel.cpp: 1054 (new), 1150, 1270–1330, 1930, 1953.
  - Mechanism.cpp: 88 (new), 127.
  - Commands.cpp: 1066/1073 (the per-target CameraVerb asserts).
  - Event.h: line count unchanged.
- **decomplint, changed files:**
  - 0 errors in Mechanism, CommonVerbs, OverlayDataModel (reordered), the tool files, Event.h and ToolsArrow.h.
  - PartInstance.cpp gains one `function_out_of_order` (fromPrimitive at the end of an already unordered file).
  - DataModel.cpp stays at its earlier out-of-order count.
- **Every FUNCTION marker I added is at effective 100** in W21A5r (PLAYER) and W21S3r (STUDIO).

## Unresolved

1. **Integrator / generator (`tools/ida/gen_reccmp_csv.py`):**
   - Drop the sticky libdata row for 0x00669fd0.
   - Pair 0x00669fd0 ↔ our `copyValue<ContentId>` and 0x006bc800 ↔ our `copyValue<SoundId>`. MATCH_FIX only
     rewrites match: rows, and 0x669fd0 / 0x6bc800 are ios: rows, so a new mechanism is needed.
   - Expected: +3 vftables and +1 function per target (section 4).
2. **Integrator:**
   - Regenerate the shared CSVs.
   - New TUs: tool/{AxisRotate,Clone,Game,Grab,Hammer}Tool.cpp and v8datamodel/Tools{Model,Part}.cpp, plus content
     in 6 formerly empty files. CMake picks them up through CONFIGURE_DEPENDS.
   - The STUDIO `shared_from<T>` twin (5 entries) needs the generator's twin rule.
3. **App, next:**
   - ModelInstance's 2013 layout (0x128), then RootInstance's ICameraOwner and Workspace's bases (section 3). That
     unblocks the camera verbs, the CameraVerb / CameraCenter / ZoomExtents ctors, TToolVerb::isChecked/doIt and
     Workspace::getCurrentMouseCommand.
   - Selection's layout (+0x64 container, size helper 0x0070eb00) for EditSelectionVerb::isEnabled and
     BoolPropertyVerb::isChecked.
   - `requireEdit` 0x00856010 (static, DataModel in esi) for SelectAll / UnlockAll / Run / Stop / Reset.
   - Run / Stop / Reset doIt assert at Commands.cpp:562, so they need the file laid out.
4. **IDA owner (names):**
   - `0x0070eb00`: Selection size helper (utilities.h:71 assert).
   - `0x007145b0`: `ServiceClient<Selection>::createService() const`.
   - Data: 0x00d98b40 / 0x00d98b44 / 0x00e14628 = `AdvArrowToolBase::advManualJointMode` / `advManualJointType` /
     `advGridMode`; STUDIO 0x00eb3b94 / 0x00eb3b98 / 0x00f694f0.
   - The 26 `RBX::sXTool` globals, at the addresses in `build-ap/w21/ttv.json`.

## Files

- **App, modified:**
  - include/V8DataModel/{Commands,NetworkSettings,OverlayDataModel,PartInstance}.h
  - include/V8World/Mechanism.h
  - include/reflection/Event.h
  - script/ScriptContext.cpp
  - v8datamodel/{Commands,CommonVerbs,DataModel,NetworkSettings,OverlayDataModel,PartInstance,ToolsSurface}.cpp
  - v8world/Mechanism.cpp
  - tool/{AdvMoveTool,AdvRotateTool,MoveResizeJoinTool,NullTool,ToolsArrow}.cpp
- **App, new:**
  - include/Tool/ToolsArrow.h
  - tool/{AxisRotateTool,CloneTool,GameTool,GrabTool,HammerTool}.cpp
  - v8datamodel/{ToolsModel,ToolsPart}.cpp
- **Notes:** notes/runs/2026-11-23-phase5-wave21/{app-21-headers,app-21}.md.
