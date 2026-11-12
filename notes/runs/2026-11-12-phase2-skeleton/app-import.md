# app-import (workflow phase2-skeleton, wf_e3afb0d0-6b4)

I imported the prior App decomp into `Client/`, added 1,034 STUB markers, and ran a VC9 compile census. **387 of the 402 non-vendored TUs compile; 15 fail.** Every remaining failure is specific to its own TU, and I left those bodies alone.

**1. Import** (`tools/import_prior_app.py`; it can be rerun and does not overwrite existing files unless given `--force`). It copied 1,552 files: 912 non-empty code files, 230 zero-byte TODO stubs and 410 vendored files. Bodies are unchanged. `notes/app-import-manifest.csv` records the old path, new path, kind and where each path came from.
- **Directory case** follows `notes/paths-2013.txt`: each path component takes its most frequent spelling, records that are lowercase end to end are ignored, and a tie keeps the tree's spelling. Changes: `App/gui` → `App/Gui`, `Base/include/Util` → `Base/include/util`.
- **8 non-empty .cpp files moved** to the one directory 2013 records for their file name. Six of them replace a 0-byte stub there:
  - `v8datamodel/ScriptContext.cpp`, `Script.cpp` and `DebuggerManager.cpp` → `App/script`
  - `v8datamodel/ScriptInformationProvider.cpp` and `ContentProvider.cpp` → `App/util`
  - `v8datamodel/Widget.cpp` → `App/Gui`
  - `v8datamodel/Humanoid.cpp` → `App/humanoid`
  - `v8datamodel/RenderSettings.cpp` → `Rendering/GfxBase`, which moves it from the App library to GfxBase.
- **One exception, not moved:** `App/v8datamodel/Item.cpp` defines RBX::GuiItem. The 2013 `Network/Item.cpp` is a different class.
- **Where the 402 TUs' paths came from:** 141 match an exact 2013 file record, 248 sit in a 2013-recorded directory without their own file record, 8 were moved, and 5 have no 2013 record and keep the tree's path: `Base/RbxAssert.cpp`, `Base/RbxFormat.cpp`, `Base/cpucount.cpp`, `Log/FastLog.cpp`, `Log/FastLogStream.cpp`. Headers with no record include `App/include/security/*`, `App/include/stdafx.h`, `Base/include/*.h` and `RbxG3D/include/RbxG3D/*`.
- **Vendored, copied as they were:** `Rendering/g3d` (210 files, including ijg and png), `Network/raknet` (33 .cpp but only 7 .h), `fmod/api`, `App/include/fmod`, `BuiltInPlugins` (27), `shaders` (135) and `App/lz4`.
- **Not copied:** `App.BulletPhysics`, `App/Lua-5.1.4`, the `G3D/` shadow copy (it is in no include path), `boostlibs/`, `PropertySheets/`, the .sln/.vcxproj files and `HANDOFF.md`.

**2. Markers** (`tools/convert_app_markers.py`; rerunnable, `--selftest` passes, a second run changes nothing).
- **Placed: 1,034 `// STUB: PLAYER` and 1,014 `// STUB: STUDIO`**, all eight lowercase hex digits; `reccmp_addr_padding.py` reports 0 badly padded markers.
- **How each was placed:** 1,005 because the demangled Mac name matches the definition's name, class and namespace; 29 because the comment block directly above the definition starts with an address in that Mac function. 32 of the placements rest on map rows whose own pairing class is callgraph-guess.
- **Templates:** 159 placements sit on template definitions, which Phase 3 must turn into TEMPLATE markers.
- **Listing:** `notes/app-marker-placements.csv` gives file, function, PLAYER, STUDIO, Mac address, pairing class, basis and a template flag for each marker.
- **Mac comments:** 2,259 comments were rewritten to `// mac: 0x…`. That is every comment starting with a Mac address, plus 7 prose comments that fit reccmp's marker pattern.
- **Map rows not placed (287 of 1,321):**
  - 124 have no definition in the tree.
  - 121 are further template instances or overloads of a definition that already has a marker.
  - 18 share both definition and PLAYER address with a placed row (C1/C2 and similar pairs).
  - 17 are ambiguous explicit specializations, e.g. `TypedPropertyDescriptor<ProtectedString>`.
  - 7 point only at files I did not import.
- **Ascending order fails widely.** 79 of the 94 .cpp files with two or more markers are out of order for PLAYER (80 for STUDIO), plus 21 headers. `reccmp-decomplint` reports 336 PLAYER and 341 STUDIO `function_out_of_order` errors, which are these same descents. Worst files: `LuaAtomicClasses.cpp` (33 descents), `ScriptContext.cpp` (27), `PartInstance.cpp` (24), `LuaEnum.cpp` and `ThreadRef.cpp` (13 each), `DataModel.cpp` (12), `GUI.cpp` (10). The script prints the full list.
- **Other lint results:** 42 PLAYER `duplicate_offset` warnings, because some map rows pair distinct Mac functions to one PLAYER address (possible FOLDED cases). Two files are not UTF-8 and reccmp cannot read them: `Util/base64.hpp` and `V8DataModel/CookiesService.h`.

**3. CMake.**
- **New option `OGRE_PRIOR_APP` (default OFF).** With it OFF, the skeleton build is unchanged and still builds. With it ON, every non-empty imported file is globbed into App, Base, Network, GfxBase, RbxG3D, RbxOgre and g3d with the skeleton's flags, plus a new Log OBJECT library. Log uses the 2016 `Log.vcxproj` flags marked "TODO prove" and is not linked into either exe.
- **Include roots** follow the HANDOFF list:
  - boost 1.53.0 from `3rdparty/_downloads`.
  - TBB headers from 4.4, downloaded from the uxlfoundation/oneTBB GitHub tag (sha256 93c74b60…). The 2013 `tbb.dll` is TBB 2.1, interface 3011, and 4.4 is the oldest official tag; only `queuing_rw_mutex` is used.
  - zlib 1.2.3.
  - Lua: `Client/App/Lua-5.1.4/src` first, then the prior tree's Lua headers as a fallback, set by the cache variable `OGRE_PRIOR_LUA_FALLBACK`.
- **Census build tree:** `build-census` was configured with `-DOGRE_PRIOR_APP=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`. I added `build-*/` to `.gitignore`.

**4. Census** (`tools/app_compile_census.py` builds the objects only, under `ninja -k 0`, in about 4 minutes). It writes `notes/app-compile-census.csv` with 544 rows.

| | OK | Errors |
|---|---|---|
| App | 356 | 12 |
| Base | 20 | 1 |
| Log | 1 | 1 |
| Network | 4 | 1 |
| GfxBase | 1 | 0 |
| RbxG3D | 3 | 0 |
| RbxOgre | 2 | 0 |

Before the `auto&` fix the same run gave App 218 OK and 150 errors, with 142 TUs stopping at `Object.h:239`.

Vendored: 98 OK and 44 errors. Those failures are missing files in the prior copies: 25 RakNet TUs lack their headers, 17 g3d png TUs lack `png.h`, one TU lacks `jconfig.h`, and one has a duplicate `Color3::black`.

**Fixes made.** Each one blocked many TUs, and each edit is tagged `OGRE vc9:`. A construct VC9 rejects is evidence the 2013 source did not use it.
- **(a)** `rbx/boost.hpp` raises `#error` unless `ROBLOX_BOOST_CONFIGS` is defined; it blocked every TU. Roblox's `boost/config/user.hpp` defines only that macro (checked in the 2016 Contribs), so I define it on the compiler command line.
- **(b)** VC9 has no `<stdint.h>` or `<cstdint>` (examples: `Base/include/RbxBase.h:3`, `rbx/atomic.h:53`); it blocked every TU. I added typedef-only headers in `3rdparty/vc9compat`.
- **(c)** `App/include/security/ApiSecurity.h` `rbxGetNtdll` uses `PEB::Ldr`, `PEB_LDR_DATA` and `LDR_DATA_TABLE_ENTRY`, which need Windows SDK 7.0 or later; VC9's SDK 6.0A lacks them. The header is reached through `Util/Object.h`, so it blocked every App TU. The function is unreferenced and is 2016 code (it has `RBX_PLATFORM_DURANGO` guards), so I wrapped it in `#if 0`.
- **(d)** C++11 `auto&` in `Util/Object.h` (lines 239 and 252, `FactoryProduct::Creator` ctor and dtor) blocked 142 TUs. I replaced it with `std::map<const Name*, const ICreator*>&`.
- **(e)** `nullptr` in `Base/include/rbx/DenseHashSet.h` (two ctor init lists) became `NULL`.

**The 15 remaining failures**, each confined to one TU and left as written:
- C++11 lambdas: `reflection_object.cpp:45`, `JointStage.cpp:56`, `Network/Players.cpp:41`.
- `nullptr`: `util/Name.cpp:35`.
- `<thread>`: `Base/cpucount.cpp:10`.
- `ofstream::open(std::string)`: `CoreScript.cpp:165`.
- `_InterlockedCompareExchange8`: `v8kernel/Body.cpp:195`.
- A literal `0` initializing a `boost::shared_ptr`: `DataModel.cpp:388`.
- The 4-argument `vsnprintf_s`: `Log/FastLog.cpp:434`.
- `extern "C"` in a file compiled as C: `util/gpc.c:41`.
- Missing third-party headers: lz4 (`SerializerBinary.cpp`), rapidjson (`WebParser.cpp`), `zlib/zlib.h` (`Http.cpp`).
- Inconsistencies in the prior tree itself: `XmlSerializer.cpp` defines a member its header lacks, and `StreamRegion.cpp` fails because `Voxel/Util.h` lacks `CellMaterial`. These two and `WebParser.cpp` were not in the prior `AppCoreCheck` list, so they never compiled there either.

I did not link or score anything.

## Structured fields

```json
{
 "files": [
  "tools/import_prior_app.py",
  "tools/convert_app_markers.py",
  "tools/app_compile_census.py",
  "notes/app-import-manifest.csv",
  "notes/app-marker-placements.csv",
  "notes/app-compile-census.csv",
  "CMakeLists.txt",
  ".gitignore",
  "3rdparty/vc9compat/stdint.h",
  "3rdparty/vc9compat/cstdint",
  "3rdparty/_downloads/tbb-4.4/oneTBB-4.4.tar.gz",
  "Client/App/include/security/ApiSecurity.h",
  "Client/App/include/Util/Object.h",
  "Client/Base/include/rbx/DenseHashSet.h",
  "build-census/census-ninja.log"
 ],
 "unresolved": [
  "Ascending order per TU fails for 79 of the 94 .cpp files with two or more PLAYER markers (decomplint: 336 PLAYER and 341 STUDIO function_out_of_order errors). Either the prior tree's source order differs from the 2013 emission order, or some Mac-to-PLAYER pairings are wrong. I did not establish which. Reordering whole functions is Phase 3 work, and until then reccmp may drop markers after the first descent in each file.",
  "159 STUB markers sit on template definitions and need TEMPLATE markers plus namerefs before Phase 3 flips them. Another 121 instance or overload rows and 17 ambiguous explicit specializations (e.g. TypedPropertyDescriptor<ProtectedString>) were left unplaced. 124 map rows have no definition in the tree.",
  "42 PLAYER (34 STUDIO) duplicate_offset warnings: several Mac functions pair to one PLAYER address. These may be ICF FOLDED cases or wrong pairings; I did not check which.",
  "32 placements rest on map rows of class callgraph-guess, and 29 placements use the weaker comment-above basis. Both are flagged in notes/app-marker-placements.csv.",
  "The TBB headers are 4.4, not the 2.1 that the 2013 tbb.dll is built from (2.1 has no official upstream download). The Lua headers fall back to the prior tree's 2016-fork Lua until the Lua agent's 2013 headers land in Client/App/Lua-5.1.4/src. VC9's missing stdint.h is covered by a typedef-only compat header; what the 2013 build used in its place is unknown.",
  "15 non-vendored TUs still fail, each for a reason specific to it (C++11 lambdas, nullptr, <thread>, ofstream::open(std::string), _InterlockedCompareExchange8, shared_ptr from 0, 4-argument vsnprintf_s, extern \"C\" in a C file, missing lz4/rapidjson/zlib headers, prior-tree inconsistencies in XmlSerializer.cpp and StreamRegion.cpp). Fixing them means rewriting bodies, which was out of scope.",
  "The vendored prior copies are incomplete: RakNet has 33 .cpp but only 7 headers (25 TUs fail), and the g3d png directory has no png.h (17 TUs fail). The G3D and RakNet library agents are expected to replace both trees.",
  "OGRE_PRIOR_APP defaults to OFF so the skeleton build stays green. With it ON, the exes will not link (unresolved externals) until more code lands. I did not attempt a link or a score.",
  "I did not push the prior names into ida/PLAYER.i64; that step (snapshot -04-prior-app) is for the agent that owns the database. notes/app-marker-placements.csv has the rows for it.",
  "Two prior files are not UTF-8 (Client/App/include/Util/base64.hpp, Client/App/include/V8DataModel/CookiesService.h), so reccmp's parser skips them."
 ]
}
```

