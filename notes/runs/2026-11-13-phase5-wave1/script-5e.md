# script-5e (workflow phase5-wave1, wf_322cba92-6a4)

Phase 5e (script, Lua bridges, EnumDesc). I measured PLAYER and STUDIO in my own build dirs (build-scr, build-scr-studio). Each target was scored twice and the two runs agreed. I compared the results against the baselines P-final.json and S-final.json.

Totals (other agents' concurrent changes are included in these numbers):
- PLAYER 4,415 / 40,469 (baseline 4,348); --nolib 2,297 / 38,287 (baseline 2,223).
- STUDIO 4,250 / 42,330 (baseline 4,222); --nolib 2,238 / 40,148 (baseline 2,203).

Row 5e coverage (tools/row_coverage.py over Client/App/script/*.cpp, same file set before and after): 152/848 = 17.9% before, 161/848 = 19.0% after. Measured with the old marker set, the before figure was 144/827.

Inventory by oracle name (PLAYER, after):
- ScriptContext (with RuntimeScriptService, jobs, LuaProfiler): 92 named functions. 39 at 100% (38 before), 49 paired below 100%, 4 with no body.
- Script/BaseScript/CoreScript/StarterScript: 25 named. 6 at 100%, 14 paired, 5 with no body.
- EnumDesc<T>: 590 named. 68 at 100%, 30 paired, 492 with no body. Our tree instantiates only 9 enums; 2013 has about 162.
- RBX::Lua bridges: 246 named. 148 at 100% (142 before), 62 paired, 36 with no body.
- Unnamed functions remain as listed in PLAYER-app-unnamed-by-class.csv: ScriptContext 224, EnumDesc 143, Bridge 135.

Raised to effective 100% in both targets. Markers are flipped to FUNCTION; all 109 PLAYER and 107 STUDIO FUNCTION markers in my files were verified effective in the final runs.
- LuaArguments::getBool 0x008a41c0 (97.1%): the type test is a switch (`sub eax,1; jne`).
- LuaArguments::getLong 0x00877bb0 (33.8%): 2013 uses G3D::iRound, an inline-asm fld/fistp.
- LuaProfiler::~LuaProfiler 0x0066fca0 (90.9%): the assert expression is `instance == this` at line 356.
- CoordinateFrameBridge on_pointToWorldSpace 0x00886800, on_pointToObjectSpace 0x008869e0, on_vectorToWorldSpace 0x00886bd0, on_vectorToObjectSpace 0x00886e90 (about 87%): the count==0 arm calls Vector3Bridge::pushVector3 0x00877190 out of line; the loop inlines pushNewObject.
- The four on_tostring(lua_State*) wrappers, 0x0087f9d0, 0x0087fa40, 0x0087faa0, 0x0087fac0 (71.4%): their on_tostring(const T&, L) specializations 0x008a8f40 to 0x008a8fa0 moved to ScriptEvent.cpp, as in 2016 and as the 2013 link position shows, so the wrappers call them out of line. Line counts in ThreadRef.cpp and LuaSignal.cpp are unchanged.

Byte-identical by private-compile probe in both targets, but still blocked by reccmp pairing (CSV rows), so they stay STUB:
- 14 Bridge<T>::registerClass bodies (0x00880470 to 0x00880a90 and 0x00888160 to 0x008890a0): 84% to 95.9%. protect_metatable moved into ScriptContext.cpp between libraryWriteGuard and openScriptLibrary, where 2013 has it (0x0066fad0). It stays at 100%.
- LuaProfiler::hookCall 0x006774e0: 96.2%.
- EnumDescriptor on_index 0x0087d310: 77% to 98.4%.
- The 4 DataModel EnumDesc ctors (0x0063d6a0, 0x0063d7b0, 0x0063d970, 0x0063da80) and Material's 0x00722680: 39% to 49% before, 88% to 96% after. The fix was the 2013 two-argument EnumDesc::addPair(value, name) in reflection/EnumConverter.h; the line numbers of the header's asserts did not move.

Other raises:
- removeScript 0.873 → 0.897, setTimeout 0.971 → 0.994.
- newVector3FromNormalId 0.376 → 0.797 and FromAxis 0.438 → 0.787: rewritten to the 2013 shape, with Windows type_info::operator== and two throw sites; only register allocation differs now.
- newFaces 0.38 → 0.585.
- ScriptContext.cpp asserts respelled and laid out by tools/assert_lines.py --apply: 321, 356, 1231, 2304, 2549, 2621 and 2634 now land on their 2013 lines.

Tried and reverted: a central v8datamodel/factoryregistration.cpp holding Type::getSingleton<Enum>. PLAYER evidence is a run of 58 identical 23-byte bodies at 0x0078b910 to 0x0078c030. In PLAYER all 6 bodies were byte-identical, in STUDIO the 5 that had a STUDIO address. STUDIO, however, links that TU before its owners: its EnumDesc convertToValue copies at 0x0073c6b0 and up come from the central TU with getSingleton inlined, and moving the registrations dropped 6 STUDIO functions from 100% to 86%. PLAYER needs the opposite order. DataModel.cpp, Sound.cpp, PartInstance.cpp and ChatService.cpp were restored and the file was deleted.

Losses against the baseline: none in my files. The rest are pairing-only:
- tss_data_node map and tree rows (0x00493590, 0x0085ede0, 0x0085f680) and the boost-internal _Buy and ~_Node rows 0x008b3120 and 0x008db820 are now unpaired. These are boost.dll internals after the BOOST_THREAD_DYN_LINK change in CMakeLists.txt; there are matching STUDIO rows.
- STUDIO Instance::onDescendantAdded 0x006f11e0 is at 96.8%; Instance.cpp is not mine.

The ownership exception is CMakeLists.txt compile flags; I did not edit CMakeLists.txt, reccmp/ or ida/.

## Files changed

- Client/App/script/ScriptContext.cpp
- Client/App/script/LuaAtomicClasses.cpp
- Client/App/script/LuaArguments.cpp
- Client/App/script/LuaEnum.cpp
- Client/App/script/LuaSignal.cpp
- Client/App/script/ThreadRef.cpp
- Client/App/script/ScriptEvent.cpp
- Client/App/include/reflection/EnumConverter.h
- build-scr/runs/P5.json
- build-scr-studio/runs/S4.json

## Unresolved

- Integrate (CSV rows): these callees are unpaired, which keeps byte-identical bodies below 100%. (1) EnumDesc::addPair: the lowercase-spelling copy is PLAYER 0x007097e0. Our addPair instantiations ICF-fold under one name (?addPair@?$EnumDesc@W4SoundType...), and 2013 has 3 copies (0x007097e0, 0x008217f0, 0x008b5df0), one per include spelling. Pairing it lifts the 9 EnumDesc ctors to 100%. (2) Bridge<T>::on_gc/on_eq/on_tostring(L) function-pointer pushes inside registerClass (e.g. 0x0087fc70, 0x0087fcb0, 0x0087fc90 for Color3). Pairing them lifts 14 registerClass bodies to 100%. (3) rbx::placement_any::operator=<Enum> 0x00633680: pairing it lifts convertToValue 0x00636ab0, 0x006efe20 and 0x007216b0. (4) Singleton<EnumDesc<T>>::initSingleton (e.g. 0x0063e610) plus the call_once flag statics (e.g. 0x00de5e74). (5) LuaProfiler::stringCache data 0x00de8c8c (hookCall 0x006774e0).
- Header markers on EnumConverter.h templates can never pair: reccmp's lines DB matches files by lowercased name, so the 'reflection/enumconverter.h' and 'Reflection/EnumConverter.h' (InputObject.cpp) instantiations both map to the same marker line and the result is ambiguous. This is the real cause of EnumDesc<Material>::convertToItem 0x007761a0 staying unpaired; it needs a CSV symbol row.
- InputObject EnumDesc vftables 0x00b5c5a4 / 0x00b5c5bc: in 2013 their slots (D0 0x00776750, lookup 0x00776d70, convertToValue 0x0078e110) are COMDATs from the central factoryregistration TU (0x0076f000-0x0078c100, lowercase enumconverter.h spelling), linked before InputObject.obj. Ours come from InputObject.cpp (Reflection/ spelling). Fixing this needs the 2013 factoryregistration.cpp TU plus per-target link order: PLAYER links it after DataModel/PartInstance/Sound, STUDIO before (0x0073a190 run). Our alphabetical glob gives one order for both targets.
- Type.h (not mine): 2013 Variant::cast<T>() returns T by value via a cdecl rbx any_cast that throws bad_placement_any_cast (0x00636a20). Ours is an inline thiscall that throws runtime_error. This blocks every EnumDesc lookup(const Variant&) (0x00637790 etc., 41.7%).
- Client/Base (not mine): 2013 rbx::signal is 4 bytes, one pointer (ctor 0x0066a0e0 zeroes [this] and call_once-inits). BaseScript is 0x9c in 2013 (ctor 0x0066cf10: ContentId 0x60, workspace 0x80, weak_ptr 0x84, bytes 0x8c/0x8e/0x8f with 0x8d unset, threadNode 0x90, starting 0x94, stopped 0x98); ours is 0x1c larger, which blocks every Script member (requestHash 0x00669300 is 1 byte off, isRobloxScript 0x006692c0). rbx::atomic operator int must read volatile (mov+test), see ScriptContext::onHook 0x0067a150. Instance::getName goes through out-of-line fw() 0x006beaa0 (removeScript 0x00684700).
- ScriptContext ctor 0x0068ad20 (89.7%): still needs GlobalAdvancedSettingsItem<LuaSettings>::singleton 0x00654600 (LuaSettings base, not mine) and the inline tbb::spin_mutex safe_queue at 0xcc (Client/Base).
- callCallback 0x0087b940 and FunctionScriptSlot::operator() 0x0087dee0 (12.5% / 22.5%): 1833-byte and 1393-byte bodies that belong in LuaInstanceBridge.cpp (asserts at lines 659-702) and LuaSignalBridge.cpp (lines 124-192). 2013 has no LuaSignal.cpp at all: our LuaSignal.cpp content corresponds to 2013's LuaSignalBridge.cpp (0x0087dee0-0x0087ebd0). Not attempted; it needs a file split and a rewrite from the bytes.
- Bridge<AllEnumDescriptors>::on_index 0x0087d3f0 (50.6%) needs Name::lookup(const char* const&) 0x00613be0 (Name.h, not mine). newVector3FromNormalId/FromAxis (79.7% / 78.7%) differ only in register choice for the item->owner.typeInfo read; cause not found.
- decomplint: ScriptContext.cpp, ThreadRef.cpp and ScriptEvent.cpp keep function_out_of_order reports. The files are pinned by 2013 assert lines (ScriptEvent's queueWaiter at line 29; the moved 0x008a8f40-0x008a8fa0 specializations sit at the end of the file). ScriptContext.cpp now has blank-line padding up to line 2634; its later asserts (2776-2828, RuntimeScriptService) cannot be reached without reordering functions.
- STUDIO data-pairing churn not caused by code: getGlobalState 0x007d8c80 dropped 43% → 26% on FFlag data pairing. setRobloxPlace 0x007d80d0 dipped to 92.3% in one intermediate STUDIO run (FLog::CoreScripts is unresolved, listed in Client/_generated/studio/unresolved.txt) and was not in the final run's losses.
- Missing EnumDesc<T> work: about 150 enum instantiations (ctor + D0 + doGetSingleton + convertToValue + lookup each) need their owners' enum types and ctor specializations. Most owner headers (Humanoid, Camera, GuiObject, TextService, ...) lack the enums, and the 51 other central getSingleton bodies need the central TU.
