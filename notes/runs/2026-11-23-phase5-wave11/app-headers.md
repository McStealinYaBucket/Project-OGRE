# Wave 11: App-wide header fixes (agent "ah")

Build dirs `build-ah/` (PLAYER) and `build-ah-studio/` (STUDIO). Scoring used a private root
(`build-ah/w/root`, symlinks plus its own `reccmp/` copy and `reccmp-project.yml`) and the ab protocol:
two runs on the shared CSVs, a regeneration, two more runs, a second regeneration, then the final run.
Scripts: `build-ah/w/{snap,score,ab}.sh`. Runs: `build-ah/w/runs/`. Scored exe/pdb/map:
`build-ah/w/snaps/{P1,P2,S1}`. Regenerated CSVs: `build-ah/w/csv-{P1,P2}-PLAYER`, `csv-S1-STUDIO`.

## Result

| | sweep17 | after | gained | lost |
|---|---|---|---|---|
| PLAYER (P2r, final link) | 11,435 | **11,647** | 220 | 8 |
| STUDIO (S1r) | 10,908 | **10,981** | 85 | 12 |

These links also contain other agents' uncommitted work from this wave (Network::Player/Server,
PhysicsInstructions, ReflectionMetadata classes, MainLogManager, Verb). The P1 link (13:40) was closer
to my changes alone: 11,435 → 11,549 (+152 / −38). 22 of the 38 losses came from the EnumProperty
include order, which I reverted (item 4). P2 then recovered them.

I read every loss with `--verbose`:
- **PLAYER, 8 losses:**
  - 6 Lua functions (symbexec, luaV_execute, constructor, patchtestreg, removevalues, luaK_setreturns). These are no-`.reloc` coincidences: `and edx, 0x7fffff` now reads as `crc_table[5][2]`, and the same for the other masks.
  - 2 are shell LTCG frame-size changes, see Unresolved 1:
    - `throw_exception<std::exception>` 0x0044d400: frame 0x4c in the original, 0x50 in ours.
    - `GlobalAdvancedSettingsItem<CRenderSettingsItem>` ctor 0x00456ee0: frame 0x2c in the original, 0x28 in ours.
- **STUDIO, 12 losses:**
  - 11 are `Network::Player` functions: the setters, setName, `calculatesSpawnLocationEarly`, two `EnumDesc` ctors. These are in-progress edits by the network agent in Player.cpp/Player.h; none of my changes touches those bodies.
  - 1 is the `enable_current_exception<error_info_injector<std::exception>>` twin.
- **P1 extras that recovered in P2:**
  - RakNet `IsOlderOrderedPacket`, `Math::isDenormal`/`isNanInfDenormVector3` and `httpGetPostWinHttp` were immediate/no-`.reloc` coincidences.
  - The Type<T> ctors were the EnumProperty include order.

## Changes, each with what it moved

1. **Util/Object.h: the eight `shared_ptr<T> (*thisFunction)(…) = &create;` lines are blank lines now**
   (the task named five; all eight are the same pattern). The asserts stay on lines 231 and 360.
   - PLAYER: `create<CRenderSettingsItem>` 0x0040d4f0 and `create<Reflection>` 0x0040d7b0 are 100%.
   - `shared_ptr<Reflection>::operator=` 0x00449790 is 100%.
   - About 34 `Creatable`/`Creator`/`FactoryProduct`/`DescribedCreatable` entries gained in PLAYER, 10+ in STUDIO.
   - `safe_static_do_get_singleton` 0x00447970: 87.0 → 91.9.
2. **Name:**
   - `toString()` returns `const std::string&`.
   - `std::ostream& RBX::operator<<(std::ostream&, const Name&)` is declared in Util/Name.h and defined first in util/Name.cpp, in address order.
   - The 2013 body inserts `name.c_str()`: `cmp [eax+0x1c],0x10`, then a tail jump to `operator<<(ostream&, const char*)`.
   - PLAYER 0x006131f0: unpaired → **100%**, marker FUNCTION.
   - It has no STUDIO copy: the byte pattern is absent, so it is folded or inlined there.
3. **`Descriptor::name` is `const Name&`.**
   - The ctor sets it from `*Name::declare(name)`. The unused `Descriptor(const Name*)` ctor is removed.
   - The header is laid out so that `RBXASSERT(!this->name.empty())` sits on 2013's line 58 (`include\reflection/Descriptor.h line: 58`).
   - `Descriptor::Descriptor` PLAYER 0x00455710 and STUDIO 0x004113e0: 75.9 → **100%** in both, markers FUNCTION.
   - `reflection_property.cpp:92` is `!type.name.empty()`, and the EnumDescriptor ctor uses `allEnumsNameLookup()[&name]`.
   - Every user was fixed (`->name->` → `->name.`, `*d->name` → `d->name`), in:
     - member.h, property.h, Function.h, YieldFunction.h, Callback.h, Event.h, EnumConverter.h (`&item->name`) and LuaArguments.h;
     - type.cpp, reflection_object.cpp, LuaEnum.cpp, LuaSignal.cpp, LuaInstanceBridge.cpp, Instance.cpp, EnumProperty.cpp, SerializerBinary.cpp;
     - ClientBase/ReflectionMetadata.cpp.
   - `SignatureDescriptor::Item::name` stays a `const Name*`, because it is not a Descriptor.
   - `MemberDescriptorContainer::compare` now uses `Name::operator<` (string order), where it used to compare pointers (Unresolved 4).
4. **Include spellings and the ValueMap singleton:**
   - util/ProtectedString.cpp and util/ContentId.cpp now include `reflection/Type.h`. `Type<ProtectedString>` 0x0066ef90 and `Type<ContentId>` 0x006fe1a0 went from 96.8 to **100%**.
   - `getSingleton<shared_ptr<const ValueMap>>` moved into reflection/type.cpp, and its name is now **"Map"** (2013 string 0x00b60fe0; the prior tree had "ValueMap"):
     - The function: PLAYER 0x0079fe90 / STUDIO 0x0073e6b0, 95.7 → **100%**, FUNCTION markers.
     - `Type<ValueMap>` 0x0079fba0: 96.8 → **100%**.
     - type.cpp's asserts stay on lines 125/134/135/140. getAllTypes and the new body are one-liners.
   - **Reverted:** v8tree/EnumProperty.cpp spelling `Reflection/Type.h` first.
     - That spelling fixes `Type<Region3int16>` 0x00793a90, but costs 22 `Type<T>` ctors (int, bool, float, double, string, Vector3…). Those come from the same .obj and assert `reflection/Type.h`.
     - In 2013, `getSingleton<Region3int16>` (0x00793b60) sits beside BasicPartInstance, not in EnumProperty.obj (Unresolved 2).
5. **Variant converts** (EnumProperty.cpp), rewritten on the 2013 forms: `isType<T>()` (typeid), by-value `rbx::any_cast<T>(value)`, and `rbx::any_cast<T>(&value)`. PLAYER / STUDIO:
   - `convert<bool>` 0x00703ee0: 56.2 → 90.2, effective match (STUDIO 0x0070d450, the same).
   - `convert<double>` 0x00704080: 47.5 → 94.1, effective.
   - `convert<ContentId>` 0x00702b60: 40.0 → 95.8, effective. Only a string source, then genericConvert.
   - `convert<ProtectedString>` moved to util/ProtectedString.cpp, where 2013 has it (0x0066f960, after genericConvert 0x0066f7f0): 37.2 → 95.1, effective, in both targets.
     - 2013 stores the string itself (`placement_any::operator=<std::string>` 0x0044c6c0) under the ProtectedString type.
     - It needs `Util/Utilities.h`, for StringConverter.
   - `convert<int>` 0x00703dc0: 44.6 → 80.6. The int case is returned in place, each branch returns `*any_cast<int>(&value)`, and the fallback is genericConvert.
   - `convert<float>`: 53.8 → 82.7.
   - `convert<shared_ptr<Instance>>`: 44.4 → 82.9. 2013 uses dynamic_cast plus `shared_from`, and throws "Unable to cast value to Object".
   - `convert<shared_ptr<DescribedBase>>`: 54.6 → 73.9.
   - `convert<std::string>`: 54.4 → 62.2. Only the numeric arms use `rbx::any_cast<T>(value)`.
6. **`Limits::Counter::canAdd` returns int:**
   - Body: `if (countLimit > 0 && getCurrentCount() + n > countLimit) return 0; return 1;`
   - Limits.cpp is reordered into 2013 address order: ~Countable, Countable, current/getCurrentCount, canAdd, Activator.
   - canAdd PLAYER 0x009b9680 and STUDIO 0x00a6b5d0 (found by byte pattern): unpaired → **100%**, FUNCTION.
   - `ObjectBridge::newInstance` 0x0087af00: 65.8 → 67.5. Its remaining diffs are the EBP/EDI allocation and the body (script owner).
7. **Security:**
   - **Permissions** are now 2013's: None 0, RobloxPlace 1, LocalUser 2, WritePlayer 3, RobloxScript 4, Roblox 5. There is no Plugin.
   - **Identities:** `StudioPlugin` is unconditional, no longer under `RBX_STUDIO_BUILD`.
     - Both targets' `isInRole` compare the identity with 9 (10 identities). Jump table slot 6 shares LocalGUI_/CmdLine_'s arm.
     - PLAYER had 9 identities before, so COM/WebService/Replicator_ were off by one there.
   - **isInRole:**
     - The arms are now {1}, {1,2,4}, {1,2} and {1,3,4}.
     - The `default:` assert is on line 33.
     - PLAYER 0x0079f650: 74.2 → **100%**. STUDIO 0x008b5c60: 74.2 → **100%**. Both FUNCTION.
   - **Every user, checked against the 2013 push:**
     - Descriptor initializers: the first `push N` before `sub esp,0xc` / the name push.
     - Functions: the `push N` before `call isInRole` 0x0079f650.
     - Sites whose old name already gave the right number were renamed to keep that number.
     - Sites where the bytes disagreed with the old number took the 2013 name:
       - LocalUser: GetJobs*, SetPlace*, SetCreator*, SetGenre, SetGearSettings, GetDebugId, RobloxLocked, AddStarterScript, SimulationRadius/MaximumSimulationRadius, Script.Source, SetTimeout, ScriptsDisabled, Shutdown and the SetMessage family.
       - RobloxScript: the Http* functions, IsPersonalServer, LocalSaveEnabled, HasBuildTools, the ScriptContext WritePlayer group.
       - Roblox: Visit's Set/GetUploadUrl and SetPing.
       - RobloxPlace: DataCost and ScriptContext.Error.
     - Files: DataModel.cpp, Visit.cpp, Service.cpp, GuiObject.cpp, Instance.cpp, Script.cpp, ScriptContext.cpp, LuaSignal.cpp, LuaLibrary.cpp, StatsService.h, CoreGuiService.h, Network/Client.cpp, Server.cpp and Player.cpp.
8. **`LOGVARIABLE(Network, 1)` in ScriptContext.cpp**, right after LuaScriptTimeoutSeconds:
   - In the original, its initializer 0x00a66090 sits between those of LuaScriptTimeoutSeconds and HttpQueue.
   - One blank line above `LOGGROUP(UseLuaMemoryPool)` was dropped, so every later line, and every assert, keeps its number.
   - This defines `FLog::Network`; it was unresolved before.
9. **DECOMP_SIZE_ASSERTs:**
   - `Reflection::Type` 0x1c and `SignatureDescriptor::Item` 0x4c, at the end of type.cpp.
   - `EnumDescriptor` 0x34, at the end of reflection_property.cpp.
   - All three compile.

## Checks

- check_2016_copies: silent. reccmp_addr_padding: 1,679 files, 0 bad.
- reccmp-decomplint: type.cpp, Name.cpp and ProtectedString.cpp have the same number of `function_out_of_order` entries as at HEAD (3, 1 and 1, all pre-existing). security/ is clean.
- Both targets build and link (`build-ah`, `build-ah-studio`).
- The last edits were marker words only (STUB to FUNCTION for operator<<, isInRole, canAdd and Descriptor). Each was verified at 100% on a scored link before it was flipped. They change no line numbers.

## Files changed

- Mine:
  - Client/App/include/Util/{Object.h, Name.h, Limits.h};
  - Client/App/util/{Name.cpp, Limits.cpp, ProtectedString.cpp, ContentId.cpp};
  - Client/App/include/reflection/{Descriptor.h, EnumConverter.h, member.h, property.h, Function.h, YieldFunction.h, Callback.h, Event.h};
  - Client/App/reflection/{type.cpp, reflection_property.cpp, reflection_object.cpp};
  - Client/App/include/security/SecurityContext.h, Client/App/security/SecurityContext.cpp;
  - Client/App/v8tree/EnumProperty.cpp;
  - Client/App/script/ScriptContext.cpp: FLog::Network, plus permission names.
- Outside my list, forced by the tree-wide renames (users of `Descriptor::name` or of Permissions). All edits are mechanical, one token each:
  - Client/App/include/script/LuaArguments.h;
  - Client/App/script/{LuaEnum, LuaSignal, LuaInstanceBridge, LuaLibrary, Script}.cpp;
  - Client/App/v8tree/{Instance, Service}.cpp, Client/App/v8xml/SerializerBinary.cpp;
  - Client/App/v8datamodel/{DataModel, Visit, GuiObject}.cpp;
  - Client/App/include/V8DataModel/{StatsService, CoreGuiService}.h;
  - Client/Network/{Client, Server, Player}.cpp: the network agent is editing Player.cpp and Server.cpp in parallel, so check that the permission names survive a merge;
  - Client/ClientBase/ReflectionMetadata.cpp: shell; its local `operator<<` declaration is now redundant but harmless.

## Unresolved

1. **Shell LTCG frames.**
   - `throw_exception<std::exception>` 0x0044d400 (PLAYER) and `GlobalAdvancedSettingsItem<CRenderSettingsItem>` ctor 0x00456ee0 lost 100%.
   - In both, the only difference is the frame size (±4 bytes in `sub esp`).
   - These are /GL shell functions. Most likely the Object.h change moved the LTCG allocation, but this is not proven.
   - The Object.h change is clearly net positive, at +30 or more in that same shell.
2. **`Type<Region3int16>` 0x00793a90** needs `Reflection/Type.h`.
   - Its `getSingleton<Region3int16>` (0x00793b60) belongs in the TU next to BasicPartInstance (v8datamodel), not in EnumProperty.cpp.
   - Moving it there, with that file spelling `Reflection/Type.h`, is the v8datamodel owner's call.
3. **Converts still below 100%:**
   - `convert<int>` 80.6: block order and register choice.
   - `convert<float>` 82.7: where the float temporaries are spilled.
   - `convert<std::string>` 62.2: the 2013 arms for long/double/Instance/enum differ, and there is a 0x005b61d0 call in the double arm.
   - `convert<shared_ptr<Instance>>` 82.9 and `convert<shared_ptr<DescribedBase>>` 73.9.
   - These have no markers; they pair through CSV rows.
4. **`Name::operator<`** compares strings. 2013 Name keeps an ordered `sortIndex`, so 2013's `operator<` may compare that instead. MemberDescriptorContainer's compare now goes through it.
5. **LuaInstanceBridge.cpp** has `(Security::Permissions)2` casts, which could now be written `Security::LocalUser`. That is cosmetic and leaves the bytes unchanged.
6. **Integrator:**
   - Rerun `tools/gen_link_stubs.py` for both targets (delete the exe first): `RBX::operator<<(ostream&, const Name&)` and `FLog::Network` are now defined.
   - Regenerate the CSVs with pooled runs.
   - Raise the import_types.py CLASSES note to the size-assert bar for Type, EnumDescriptor and SignatureDescriptor::Item.
7. **`ObjectBridge::newInstance`** 0x0087af00 (67.5): the remaining diffs are the register allocation and the body. That is the script owner's.
