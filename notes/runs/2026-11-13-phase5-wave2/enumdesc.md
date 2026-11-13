# enumdesc (workflow phase5-wave2, wf_cba1073c-3f1)

I added 61 EnumDesc<T> instantiations and fixed the 2 InputObject ones. All 63 ctors score 100% in PLAYER and 62 do in STUDIO. These figures come from my own regenerated copy of the reccmp CSVs, because the shared reccmp/*.csv are stale for this build. Integrate needs to regenerate them before these gains appear in the shared numbers.

**Inventory.** The PLAYER binary has 100 EnumDesc ctors (functions that call EnumDescriptor::EnumDescriptor at 0x006fa1a0); STUDIO has 104 (0x006dcae0).
- 7 are CRenderSettings ctors in the client shell, which is built /GL. 2 are D3D enums. STUDIO adds 4 that only RobloxStudio has.
- That leaves 91 App/Network ctors. For each one I listed the type name, ops (registration calls: addPair/addLegacy/addLegacyName, with values and names), vftable, source file and the matching STUDIO ctor.
- 89 of the 91 have identical ops in both targets. Material differs (STUDIO registers an older list), and STUDIO's RuntimeUndoBehavior ctor calls its own addPair copy at 0x006d5030.
- Which addPair copy a ctor calls tells you how its file spelled the include: 0x007097e0 is "reflection/enumconverter.h" (most), 0x008217f0 is "reflection/EnumConverter.h" (Joint), 0x008b5df0 is "Reflection/EnumConverter.h" (Axis, InputObject, KeywordFilterType, SurfaceConstraint, SurfaceType).
- New tool `tools/enumdesc_ctors.py [--target PLAYER|STUDIO] [--json OUT]` prints this list.

**Instantiations: 9 before, 70 after** (of the 91).
- Each ctor goes in the file its 2013 address range points to. I appended them at the end of each file with their getSingleton specialization and registrars, so no 2013 assert line above them moves.
- New files: `v8datamodel/Enums.cpp` (6 ctors at 0x008e5250–0x008e58d0, the same set as the 2016 file), `v8world/MacroTypes.cpp` and `v8datamodel/legacy.cpp` (2016 paths, since no 2013 assert names them). `v8datamodel/ChangeHistory.cpp` was empty and now has the RuntimeUndoBehavior ctor.
- I added the missing enum declarations to their owner classes, using the 2013 values. Enumerator names come from the 2016 headers where the values agree; I wrote the HttpContentType, ButtonStyle and BinType names by hand from the 2013 registration strings.
- Every new ctor carries `// FUNCTION: PLAYER`. STUDIO gets FUNCTION too, except RuntimeUndoBehavior 0x006d57d0, which stays STUB.

**Measurements.** The baseline is the Work agents' tree, built and scored first; both sides went through the same procedure (a run with the shared CSVs, a private CSV regeneration with that run's JSON, then a rescore).

| | PLAYER before | PLAYER after | STUDIO before | STUDIO after |
|---|---|---|---|---|
| Effective 100% | 6,619 | 6,733 (+121 / −7) | 6,484 | 6,605 (+121 / −0) |
| Implemented | 6,699 | 6,994 | 6,453 | 6,748 |
| Oracle-named EnumDesc functions at 100% | 58 / 560 | 118 / 560 | 65 / 541 | 124 / 541 |
| EnumDesc rows at 100% | 97 | 221 | 104 | 226 |
| EnumDesc vftables at 100% | 8 of 11 | 65 of 72 | 8 of 11 | 64 of 72 |
| All vftables at 100% | 1,498 | 1,555 | 1,551 | 1,607 |

- The 7 PLAYER losses are layout coincidences: Math::isDenormal 0x00759a40, isNanInfDenormVector3 0x00759b40, Http::httpGetPostWinHttp 0x00826990, luaK_setoneret 0x009a6cc0, luaK_nil 0x009a7020, exp2reg 0x009a7430 and luaK_posfix 0x009a7e90. Each diff is only an `and reg, 0x7fffff` or `0x800000` immediate that reccmp reads as an address, because the binary has no `.reloc`.
- The same 7 dropped after the first batch, before any code they use changed, so they track image layout, not code.
- With the stale shared CSVs, PLAYER effective goes from 6,146 to 6,077, because the ICF-folded PUBLICS names were reassigned.
- Final runs, to pool for the regeneration: `build/_enum/b5/{shared,q}.json` and `build/_enum/b5s/{shared,q}.json`, plus exe/pdb/map copies in the same folders.

**The three open causes:**
1. **convertToValue 0x00636ab0 (81.8%) has two diffs:**
   - 2013 calls Type::getSingleton<T> out of line (0x0078bc10, one of 58 23-byte bodies in the central factoryregistration TU at 0x0076f000–0x0078c100). Ours inlines it, because the owner TU defines it.
   - rbx::placement_any<Region3>::operator=<Enum> 0x00633680 has no oracle name, so it needs a CSV symbol row.
2. **EnumDesc<Material>::convertToItem 0x007761a0** was already at 100% in both baselines and still is (it is now named under another T after ICF).
3. **InputObject:**
   - Fixed: the ctors carried the 2016 contents. InputObject.h now has the 2013 values (UserInputType 0–6 plus None=7, UserInputState 0–2 plus None=3), and the ctors 0x008b6130 / 0x008b62a0 (STUDIO 0x00938100 / 0x00938270) went from 78.4% / 95.4% to 100% in both targets.
   - Still at 0.6: vftables 0x00b5c5a4 / 0x00b5c5bc, and the same holds for all 6 types spelled other than lowercase (Joint 0x00b5c3dc, SurfaceConstraint 0x00b5c40c, Axis 0x00b5c424, SurfaceType 0x00b5c4fc). Their lookup(const char*) slot 0x00773130 and convertToString slot 0x00772590 are the lowercase copies that the central factoryregistration TU emitted, because it was linked before the owners.
   - Fixing that takes a factoryregistration.cpp holding the getSingleton<T> specializations, plus a per-target position in the link order (PLAYER and STUDIO differ). That is a CMakeLists.txt change I don't own.

**Checks:** both targets build with no errors. check_2016_copies prints nothing. decomplint reports nothing new for the touched files; it still flags out-of-order functions at NormalId.cpp:275/468 and TextService.cpp:133, which point at existing functions, not the new blocks.

## Files changed

- tools/enumdesc_ctors.py
- Client/App/v8datamodel/Enums.cpp
- Client/App/v8world/MacroTypes.cpp
- Client/App/v8datamodel/legacy.cpp
- Client/App/v8datamodel/ChangeHistory.cpp
- Client/App/v8datamodel/InputObject.cpp
- Client/App/include/V8DataModel/InputObject.h
- Client/App/v8datamodel/DataModel.cpp
- Client/App/v8datamodel/DebugSettings.cpp
- Client/App/v8datamodel/GuiService.cpp
- Client/App/v8datamodel/UserInputService.cpp
- Client/App/v8datamodel/FriendService.cpp
- Client/App/v8datamodel/Camera.cpp
- Client/App/v8datamodel/Frame.cpp
- Client/App/v8datamodel/Hopper.cpp
- Client/App/humanoid/Humanoid.cpp
- Client/App/util/SoundService.cpp
- Client/App/v8datamodel/GameSettings.cpp
- Client/App/v8tree/EnumProperty.cpp
- Client/App/v8datamodel/StarterGuiService.cpp
- Client/App/v8datamodel/DataModelJob.cpp
- Client/App/v8datamodel/TextService.cpp
- Client/App/v8datamodel/DataModelMesh.cpp
- Client/App/v8datamodel/CharacterMesh.cpp
- Client/App/v8datamodel/SocialService.cpp
- Client/App/v8datamodel/PersonalServerService.cpp
- Client/App/v8datamodel/GuiObject.cpp
- Client/App/v8datamodel/MarketplaceService.cpp
- Client/App/util/NormalId.cpp
- Client/App/v8world/Joint.cpp
- Client/App/v8datamodel/AssetService.cpp
- Client/App/v8datamodel/HttpService.cpp
- Client/App/v8datamodel/Explosion.cpp
- Client/App/v8datamodel/Feature.cpp
- Client/App/v8datamodel/SkateboardPlatform.cpp
- Client/App/v8datamodel/NetworkSettings.cpp
- Client/Network/Player.cpp
- Client/Network/Players.cpp
- Client/Network/include/Network/Player.h
- Client/Network/include/Network/Players.h
- Client/App/include/V8DataModel/AssetService.h
- Client/App/include/V8DataModel/BasicPartInstance.h
- Client/App/include/V8DataModel/Camera.h
- Client/App/include/V8DataModel/ChangeHistoryService.h
- Client/App/include/V8DataModel/CharacterMesh.h
- Client/App/include/V8DataModel/DataModelMesh.h
- Client/App/include/V8DataModel/DebugSettings.h
- Client/App/include/V8DataModel/Explosion.h
- Client/App/include/V8DataModel/ExtrudedPartInstance.h
- Client/App/include/V8DataModel/Feature.h
- Client/App/include/V8DataModel/FriendService.h
- Client/App/include/V8DataModel/GameSettings.h
- Client/App/include/V8DataModel/GuiButton.h
- Client/App/include/V8DataModel/GuiObject.h
- Client/App/include/V8DataModel/GuiService.h
- Client/App/include/V8DataModel/Handles.h
- Client/App/include/V8DataModel/HopperBin.h
- Client/App/include/V8DataModel/HttpService.h
- Client/App/include/V8DataModel/Humanoid.h
- Client/App/include/V8DataModel/MarketplaceService.h
- Client/App/include/V8DataModel/NetworkSettings.h
- Client/App/include/V8DataModel/PersonalServerService.h
- Client/App/include/V8DataModel/PrismInstance.h
- Client/App/include/V8DataModel/PyramidInstance.h
- Client/App/include/V8DataModel/SkateboardPlatform.h
- Client/App/include/V8DataModel/SocialService.h
- Client/App/include/V8DataModel/StarterGuiService.h
- Client/App/include/V8DataModel/TextService.h
- Client/App/include/V8DataModel/UserInputService.h
- build/_enum/b5/shared.json
- build/_enum/b5/q.json
- build/_enum/b5s/shared.json
- build/_enum/b5s/q.json

## Unresolved

- Integrate (CSV): regenerate reccmp/player-*.csv and studio-*.csv with tools/ida/gen_reccmp_csv.py --json, pooling build/_enum/b5/shared.json and build/_enum/b5s/shared.json or fresh runs. Measured gain against an equally regenerated baseline: PLAYER effective 100% 6,619 to 6,733, STUDIO 6,484 to 6,605. Without regeneration the shared-CSV PLAYER figure drops from 6,146 to 6,077, because the ICF-folded PUBLICS names (addPair, convertToString 0x00772590, lookup 0x00773130, the std::map<Name*,...> helpers 0x0091cc50 / 0x00697570) were reassigned to new types.
- Integrate (CSV): rbx::placement_any<Region3>::operator=<Enum> 0x00633680 has no oracle name; a symbol row is needed for convertToValue 0x00636ab0, 0x006efe20 and 0x007216b0 (81.8%).
- convertToValue 59-byte copies (46 of 97 in PLAYER): 2013 calls Type::getSingleton<T> out of line (0x0078bc10 and the 58-body run 0x0078b910-0x0078c030 in the central factoryregistration TU at 0x0076f000-0x0078c100). Ours inline it because each owner .cpp defines it. The fix is a v8datamodel/factoryregistration.cpp holding the getSingleton<T> specializations (2016 has 122 RBX_REGISTER_ENUMs there, and Network/API.cpp has 9), placed per target in the link order (PLAYER after DataModel/PartInstance/Sound, STUDIO before them). CMakeLists.txt is not mine.
- 6 EnumDesc vftables stay at 0.6 (InputObject 0x00b5c5a4 / 0x00b5c5bc, Joint 0x00b5c3dc, SurfaceConstraint 0x00b5c40c, Axis 0x00b5c424, SurfaceType 0x00b5c4fc). Their lookup(const char*) and convertToString slots are the lowercase-spelling copies 0x00773130 / 0x00772590 that the central TU emitted; this has the same fix as the previous item.
- lookup(const Variant&) rows are at 41.7-50%: Type.h (not mine) needs the 2013 Variant::cast<T>, which calls the cdecl rbx::any_cast at 0x00636a20 (throws bad_cast) and dereferences &value+4.
- 21 App/Network ctors are still missing. PacketPriority 0x005b69c0 and PacketReliability 0x005b6880: our NetworkSettings.cpp sits in App, which cannot see RakNet's PacketPriority.h (2013 has it in Network). FilterResult 0x005b7f80: no network api.h. The rest have no owner class in our tree: EThrottle 0x0065cb40, Controller::Button 0x006e6d20, GameBasicSettings ControlMode/RenderQualitySetting 0x006f7c20 / 0x006f7d60, SpecialShape::MeshType 0x00744a80, Voxel Cell* 0x007640a0-0x00764640 (5), KeyframeSequence::Priority 0x008bd640, DialogRoot x2 0x00914b50 / 0x00914c70, Action::ActionType 0x009524f0, LegacyController::InputType 0x00952630, KeywordFilterType 0x009527e0.
- PartInstance::FormFactor 0x007221d0: PartInstance.h names CUSTOM=1 and BRICK=2, but 2013 registers Symmetric=0, Brick=1, Plate=2, Custom=3. The values used in PartInstance.cpp code are right; the names are swapped. Renaming them touches PartInstance.cpp functions I don't own.
- STUDIO EnumDesc<ChangeHistoryService::RuntimeUndoBehavior> 0x006d57d0 stays STUB: it calls its own lowercase addPair copy 0x006d5030, separate from 0x00775a00, cause not found. STUDIO Material 0x007bde50 registers an older 9-name list and needs an OGRE_TARGET branch.
- tools/ida/gen_reccmp_csv.py (not mine) crashes without --json: UnboundLocalError on `collections` at line 1105, because the `import collections` at line 966 inside gen() makes the name local.
- The new ctors sit at the end of their files to keep 2013 assert lines fixed, so they are not in PLAYER address order relative to each file's other functions. decomplint flagged no new out-of-order functions in these files.
