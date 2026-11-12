# app-cxx03 (workflow phase3-measure, wf_271afd88-e3a)

All 15 failing non-vendored TUs are fixed. 13 now compile and 2 are taken out of the build, with reasons below. Three other App TUs now fail, and the cause is outside my files: the Lua agent's new `luaconf.h`.

**Census** (`tools/app_compile_census.py`, `build-census`, reconfigured once after my CMake edit, 3m41s):
- **Non-vendored: 397 of 400 compile.** App 363 of 366, Base 21/21, Log 2/2, Network 5/5, GfxBase 1/1, RbxG3D 3/3, RbxOgre 2/2.
- The census now counts 520 TUs, because the two excluded files are no longer in `compile_commands.json`.
- **The 3 new failures:** `script/ScriptContext.cpp`, `ThreadRef.cpp` and `LuaInstanceBridge.cpp` (first error: `'context' : is not a member of 'RobloxExtraSpace'`). `Client/App/Lua-5.1.4/src/luaconf.h` now carries the Lua agent's opaque 44-byte `RobloxExtraSpace`. These App files need the full class: `context()`, `getThreadCount()`, `eraseRefsFromAllNodes`, `script`, `continuations`. With that include dir dropped from the command line, all three compile with 0 errors. Both copies of the file are outside what I may edit.
- **Vendored:** RakNet 26/26. g3d 93 of 94; `GImage_png.cpp` fails on `png_structp`, which belongs to the G3D agent.

**Fixes by category.** Each is tagged `// OGRE vc9:`; a construct VC9 rejects is evidence of what the 2013 source looked like.
1. **C++11 lambdas (3 TUs).**
   - `reflection_object.cpp`: two identical lambdas became `static bool compare2(const ClassDescriptor*, const ClassDescriptor*)`, the 2016 name.
   - `JointStage.cpp` `onPrimitiveAdded` (PLAYER 0x00998360): now `boost::bind(&JointStage::visitAddedPrimitive, this, _1, _2, boost::ref(found))`, as the 2016 source has it.
   - `Network/Players.cpp` `getNumPlayers`: now `(int)numChildren()`. Mac 0x008a3c66 is 37 bytes: it calls the `copy_on_write_ptr` at this+0xb4 and returns the vector's size, with no null test and no visitor.
2. **`nullptr`:** `util/Name.cpp` (3 uses) and `util/Http.cpp` (11 uses, all pointers) now use `NULL`. Name.cpp also lacked `rbx/Debug.h`, so `RBXASSERT` and `InterlockedIncrement` were missing.
3. **`shared_ptr` built from 0:** `DataModel.cpp` had `runService(NULL)`, which needs C++11's `nullptr_t` constructor. It is now `runService()`.
4. **`ofstream::open(std::string)`:** `CoreScript.cpp:165` now passes `fileName.c_str()`.
5. **`_InterlockedCompareExchange8`**, changed to what the PLAYER bytes show. `Body::getPV_Spin_Lock` at PLAYER 0x007554e0 holds a `tbb::spin_mutex::scoped_lock` on +0x1c: it takes the lock byte from `__TBB_LockByte` at 0x00402240, and the unlock (`mov [esi],bl`) runs inside the exception-handling frame.
   - `Body.h` member `unknown28` changed from `bool` to `tbb::spin_mutex` (the same 1 byte), and Body.h now includes `<tbb/spin_mutex.h>`.
   - The function body is now `tbb::spin_mutex::scoped_lock lock(unknown28); updatePV(); return ...`.
   - The 2016 name for this member is `mutex`; I did not rename it.
6. **`<thread>`:** `Base/cpucount.cpp` now uses `GetSystemInfo().dwNumberOfProcessors` with the same fallback. The PLAYER function differs and is recorded in a comment. `RbxTotalUsableCoreCount` is 0x009bd190; `TaskScheduler::setThreadCount` (0x009b7ec0) calls it with 1. It ignores its argument and returns a count from an Intel-style `CPUCount` at 0x009bceb0. It falls back to `dwNumberOfProcessors` only when `CPUCount` returns status 8.
7. **4-argument `vsnprintf_s`:** `FastLogFormatted` and `FastLogFormattedNoAdorn` in `Log/FastLog.cpp` are now inside `#if 0`. They are 2016 additions: the 2013 `Log.dll` exports 24 functions and neither of these, and it imports no `vsnprintf` variant.
8. **`extern "C"` in a C file:** `util/gpc.c` now wraps it in `#ifdef __cplusplus`. PLAYER names `gpc_polygon_clip` (0x0090b2c0) unmangled.
9. **zlib:** `Http.cpp` included `<zlib/zlib.h>` (2016 Contribs 1.2.8). It now includes `<zlib.h>` from `3rdparty/_downloads/zlib-1.2.3` with `ZLIB_WINAPI`. PLAYER's only zlib is stdcall: `deflateInit2_` at 0x009e1e80 ends `ret 0x20`.
10. **lz4: PLAYER does contain it, so I fetched it.** The binary deserializer is present in PLAYER and in the Mac build:
    - `"<roblox!"` is compared at 0x006fd4d5 (`SerializerV2::load`) and 0x008d2124.
    - The strings `"Malformed data (%d != %d)"`, INST/PROP/PRNT and others are present.
    - iOS 1029 has `LZ4_decompress_safe` and only `SerializerBinary::deserialize`. No compressor strings exist in PLAYER or Mac.

    **Version:** the prior tree's `App/lz4/lz4_decoder.h` is identical to upstream svn r94–r96, and the 2016 `App/lz4` set equals r94 (commit 40ae7043, 2013-04-26) apart from two casts in `lz4hc_encoder.h`. I downloaded the r94 GitHub archive into `3rdparty/_downloads/lz4-r94/`. GitHub publishes no checksum for it; sha256 is `e833bf59b9f9315839351f49fc29929ba4c8f391245ec72653147b4374866633`. The source is recorded in `SOURCE.txt` there.

    `SerializerBinary.cpp`'s writer called `LZ4_compress_HC` and `LZ4_compress_default`, which only exist from lz4 r129. It now calls the r94 `*_limitedOutput` names.
11. **rapidjson: absent from PLAYER (no rapidjson error strings).** iOS 1029 lists eight WebParser functions; all are XML or `boost::property_tree` based. So `WebParser.cpp` stays in the build, with:
    - the rapidjson includes and its 7 rapidjson functions in `#if 0`;
    - the `tag_Web*` pointer globals dereferenced (`*tag_WebList`), as the other v8xml TUs do;
    - `Serializer::loadInstancesFromText` replaced by `SerializerV2`'s (Mac 0x5AECFC).
12. **Out of the build (listed in CMake, files not deleted):**
    - `v8xml/XmlSerializer.cpp` is the dormant 2016 copy. Its 2013 bodies already live in `TextXmlParser.cpp` and `TextXmlWriter.cpp`, whose asserts cite XmlSerializer.cpp lines, so compiling it too would define the writer functions twice.
    - `util/StreamRegion.cpp` is 2013 code (iOS has both BitStream operators), but its headers are not 2013. `Util/StreamRegion.h` needs 2016 SpatialHash APIs, `Voxel/Cell.h` is a 0-byte stub, and `Util/StreamingUtil.h` does not exist.

**CMake:** my edit is inside `if(OGRE_PRIOR_APP)` only. It adds two `EXCLUDE` regexes for those files and `target_include_directories(App PRIVATE ${OGRE_DL}/lz4-r94/lz4-r94)`.

**Encoding:** both files had non-ASCII bytes only in comments, so I converted both to UTF-8. `Util/base64.hpp` was cp1251 (two Russian comments, lines 187 and 220). `V8DataModel/CookiesService.h` was cp1252 (one em dash, line 3). Line endings are unchanged.

**Found on the way:** in `Name::setOrderIndex` at PLAYER 0x00613530, `lock xadd [eax],ecx` increments the first dword of the Name object itself. So `sortIndex` is an inline int at +0, not the `int*` that `Name.h` declares.

## Structured fields

```json
{
 "files": [
  "Client/App/util/Name.cpp",
  "Client/App/reflection/reflection_object.cpp",
  "Client/App/v8world/JointStage.cpp",
  "Client/Network/Players.cpp",
  "Client/App/v8datamodel/DataModel.cpp",
  "Client/App/script/CoreScript.cpp",
  "Client/App/include/V8Kernel/Body.h",
  "Client/App/v8kernel/Body.cpp",
  "Client/Base/cpucount.cpp",
  "Client/Log/FastLog.cpp",
  "Client/App/util/gpc.c",
  "Client/App/util/Http.cpp",
  "Client/App/v8xml/SerializerBinary.cpp",
  "Client/App/v8xml/WebParser.cpp",
  "Client/App/include/Util/base64.hpp",
  "Client/App/include/V8DataModel/CookiesService.h",
  "CMakeLists.txt",
  "3rdparty/_downloads/lz4-r94/lz4-r94-40ae7043.tar.gz",
  "3rdparty/_downloads/lz4-r94/SOURCE.txt",
  "notes/app-compile-census.csv",
  "build-census/census-ninja.log"
 ],
 "unresolved": [
  "3 App TUs (script/ScriptContext.cpp, ThreadRef.cpp, LuaInstanceBridge.cpp) now fail because Client/App/Lua-5.1.4/src/luaconf.h (Lua agent, not my ownership) defines an opaque 44-byte RobloxExtraSpace stand-in. The App code needs the full class (context(), getThreadCount(), eraseRefsFromAllNodes, script, continuations). All three compile with that include dir dropped. The Lua agent needs a luaconf.h that gives App TUs the full 2013 class.",
  "util/StreamRegion.cpp (2013 code, PLAYER STUB 0x005a7eb0) is out of the build until someone writes 2013 versions of Voxel/Cell.h (0-byte stub now) and Util/StreamRegion.h (a 2016 copy needing SpatialHashStatic::hashGridSize/scaleExtents and ContactManagerSpatialHash::CoarseMovementCallback), plus Util/StreamingUtil.h (missing) and RakNet BitStream headers.",
  "v8xml/XmlSerializer.cpp is out of the build as the dormant 2016 copy. The 2013 file name is XmlSerializer.cpp, so TextXmlParser.cpp and TextXmlWriter.cpp should eventually be merged into it (Phase 3 file layout).",
  "lz4 r94 is wired in as headers only. lz4.c (the 2016 tree builds it in App, apparently as C++ given two casts in lz4hc_encoder.h) is not compiled and not matched against PLAYER's decoder; that belongs to whoever owns third-party library matching. The r94 pick rests on the 2016 tree's copy plus the prior lz4_decoder.h (identical in r94 to r96), not on PLAYER bytes.",
  "SerializerBinary.cpp's writer (writeChunk, the LZ4 compressor and the serialize path) is probably not 2013 code: iOS 1029 lists only SerializerBinary::deserialize, and PLAYER and Mac have no compressor strings. The 0x9e3779b1 multiplies in PLAYER sit in 0x0081d180 and 0x0081d310 (likely xxhash), not in the serializer. I left the writer compiling with the r94 names instead of removing it.",
  "WebParser.cpp still uses the 2016 function names legacyParseWebJSONResponse and ptreeParseWebJSONResponse. 2013 has parseWebJSONResponse(std::stringstream&, shared_ptr<unordered_map<...>>&) and populateValueTableFromPtree with an unordered_map ValueTable, so the signatures do not match 2013.",
  "Name.h layout conflict: PLAYER 0x00613530 increments the dword at Name+0 directly (lock xadd [eax]), so sortIndex is an inline int at +0, not the int* the prior Name.h declares. Not changed: a layout fix, not a compile fix.",
  "RbxTotalUsableCoreCount is at PLAYER 0x009bd190 (it calls CPUCount at 0x009bceb0; GetSystemInfo fallback on status 8), but cpucount.cpp has no marker and no CPUCount body. Base/cpucount.cpp's 2013 contents are Intel CPUCount code that is not in the tree.",
  "Players::getNumPlayers uses numChildren(), which adds a null test the Mac body lacks. Whether Instance::children sits at +0xb4 is not proven by a Windows constructor.",
  "Body::unknown28 keeps its placeholder name. The 2016 name 'mutex' plus the PLAYER scoped_lock make the rename likely, but I did not rename it."
 ]
}
```

