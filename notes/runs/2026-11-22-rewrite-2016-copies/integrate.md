# integrate (rewrite of the 2016-identical files), 2026-11-22

Two agents (Base+Log: 44 files; App: 30 files) rewrote the owner-side files that `.gitignore` listed as byte-identical to the 2016 source. The task text says 77; the `.gitignore` block held 74 owner-side paths (44 + 30) plus the 7 `Client/Rendering/RbxG3D` paths, which stay ignored. I integrated both rewrites, rebuilt both targets from clean, regenerated the CSVs and compared with sweep9. Nothing regressed. Every heavy command ran through `tools/capped.sh`. Nothing is committed.

## Builds

- **PLAYER** (`build/`): configured, `--target clean`, then a full build: 679 steps, 0 errors. I deleted the exe and ran `gen_link_stubs.py PLAYER`. The stubs went from 8 to 10 addresses: `SystemUtil::getVideoMemory` 0x009b59b0 and `CRenderSettings()` 0x009c5ce0, the two names.csv rows ida-sync-6 added. Then I rebuilt and relinked.
- **STUDIO** (`build-studio/`): the same steps with `--target studio`: 674 steps, 0 errors. The same two stubs were added, at 0x00a68be0 and 0x006c20b0.
- **Ninja rebuilds everything on each run.** `.ninja_log` reports "premature end of file; recovering" (sweep9's build logs show the same), so every `cmake --build` and every `gen_link_stubs` run recompiles all of `build/`. `ninja -t recompact` removes the warning, but the next run still rebuilt everything. This costs about 20 minutes per run; I did not chase the cause.

## Regeneration rounds (two fresh reccmp runs of the current link per round; both runs agreed in every round)

| Target | sweep9 | Rounds (at effective 100%) | Final |
|---|---|---|---|
| PLAYER | 8,999 | 9,029 → 9,029 → 9,029 | **9,029** (`build/sweep10/P-final.json`, 12,283 entries) |
| STUDIO | 8,783 | 8,813 → 8,825 → 8,826 → 8,826 | **8,826** (`build-studio/sweep10/S-final.json`, 11,981 entries) |

The scored exe, pdb, map and pdbview are in `build*/sweep10/scored/`. The baselines `build*/sweep9/` are untouched.

## Against sweep9, by original address

**PLAYER: 0 lost, 30 gained.**
- 19 `rbx::signals::signal<…>::disconnectAll` instantiations (0.9831 → 1.0).
- The islot `intrusive_ptr_target::operator delete` 0x00406c40, `intrusive_ptr_add_ref` / `add_weak_ref` / `weak_release` for islot (0x0040dd30 / 0x0040d2d0 / 0x0040d380), and `intrusive_ptr_release` for WeakThreadRef 0x00670490 and LiveThreadRef 0x00672670.
  - Cause: the Base rewrite moved the `intrusive_ptr_target.h` asserts to their 2013 lines.
- Six Lua functions: `symbexec`, `luaV_execute`, `constructor`, `patchtestreg`, `removevalues` and `luaK_setreturns`. These are no-`.reloc` immediate coincidences, so this part of the gain is not real progress.
- `CRenderSettingsItem()` 0x004567d0 went from 0.925 to 0.9625 because of the new stubs. It is still below 100%.

**STUDIO: 0 lost, 43 gained.**
- The same signal and intrusive_ptr family: 22 functions.
- `RegistryUtil::keyExists` / `read32bitNumber` / `readString` (0x00a766b0 / 0x00a76790 / 0x00a769d0). These were absent from sweep9's JSON and now pair, from the RegistryUtil.cpp rewrite.
- Nine other functions newly paired, each absent from sweep9's JSON, including `signal_with_args<2,…>::fireItem` 0x005d86d0 and 0x00976f90, `ServiceProvider::getPublicServiceByClassNameString` 0x0076fb40, `Primitive::setSurfaceType` 0x00893650 and `GuiMain::~GuiMain` 0x00960960.
- Nine no-`.reloc` coincidences that flipped back to 100%: `IsOlderOrderedPacket`, `httpGetPostWinHttp`, `Math::isDenormal`, `isNanInfDenormVector3`, `luaK_setoneret`, `luaK_nil`, `luaK_posfix`, `luaK_setreturns` and `constructor`.

**Losses: none in either target, so no `--verbose` reads or fixes were needed.** The 7 losses both agents reported from their private builds (`luaK_*`, `Math::isDenormal`, `isNanInfDenormVector3`, `IsOlderOrderedPacket`, `httpGetPostWinHttp`) were no-`.reloc` coincidences. They are at 100% in this link.

## Checks (sweep10 vs sweep9; outputs beside the JSONs)

| Check | PLAYER sweep9 → sweep10 | STUDIO sweep9 → sweep10 |
|---|---|---|
| reccmp-decomplint `--warnfail` | 276 function_out_of_order + 12 duplicate_offset → same | 284 + 4 → same |
| reccmp-vtable | 998 mismatched vftables, 11 adjuster-thunk notes → same | 939, 11 → same |
| reccmp-datacmp | 1,719 variables / 51 issues → 1,721 / 51 | 1,667 / 53 → 1,669 / 53 |
| check_folded.py | 2,302 annotated / 0 problems → 2,304 / 0 | 2,165 / 0 → 2,167 / 0 |
| reccmp_addr_padding.py | 0 badly padded (1,669 files) | — |
| check_2016_copies.py | prints nothing (exit 0) | — |

The +2 annotated addresses and the +2 datacmp variables come from the two new link stubs.

## .gitignore

I removed the 74 owner-side paths from the block "files byte-identical to the 2016 source". The comment and the 7 `Client/Rendering/RbxG3D` lines remain, and `Client/shaders/` is still ignored. All 74 files now show as untracked, and `python3 -I tools/check_2016_copies.py` prints nothing.

## Similarity to the 2016 counterpart

Method: difflib `SequenceMatcher(None, ours, theirs).ratio()` with `\r` stripped from both files. The counterpart is the 2016 file with the same basename (case-insensitive) and the longest common path suffix, with ties broken by the higher ratio.
- 74 files, mean ratio 0.307.
- The highest ratio among the 55 files over 30 lines is 0.576 (`V8Tree/Property.h`). **No file over 30 lines reaches 0.6, so I rewrote none further.**
- Only two files are at or above 0.6, both under 30 lines: `FastLogStream.h` (15 lines, 0.620) and `Quaternion.inl` (19 lines, 0.602).

| File | Lines | Ratio | 2016 counterpart (under ref/src2016/extracted) |
|---|---|---|---|
| `Client/Log/include/FastLogStream.h` | 15 | 0.620 | `robloxsrc/Trunk2012/Build/Log/include/FastLogStream.h` |
| `Client/App/include/Util/Quaternion.inl` | 19 | 0.602 | `robloxsrc/Trunk2012/Build/App/include/util/Quaternion.inl` |
| `Client/App/include/V8Tree/Property.h` | 50 | 0.576 | `robloxsrc/Trunk2012/Build/App/include/v8tree/Property.h` |
| `Client/App/include/Util/BiMultiMap.h` | 72 | 0.572 | `robloxsrc/Trunk2012/Build/App/include/util/BiMultiMap.h` |
| `Client/App/include/V8World/ContactManagerSpatialHash.h` | 28 | 0.551 | `robloxsrc/Trunk2012/Build/App/include/v8world/ContactManagerSpatialHash.h` |
| `Client/App/include/Util/Vector3int32.h` | 120 | 0.533 | `robloxsrc/Trunk2012/Build/App/include/util/Vector3int32.h` |
| `Client/Base/include/rbx/Crypt.h` | 29 | 0.531 | `robloxsrc/Trunk2012/Build/Base/include/rbx/Crypt.h` |
| `Client/App/include/Voxel/Util.h` | 171 | 0.517 | `robloxsrc/Trunk2012/Build/App/include/voxel/Util.h` |
| `Client/Base/include/RbxHash.h` | 12 | 0.491 | `robloxsrc/Trunk2012/Build/Base/include/RbxHash.h` |
| `Client/Base/rbx/ProcessPerfCounter.cpp` | 217 | 0.489 | `robloxsrc/Trunk2012/Build/Base/rbx/ProcessPerfCounter.cpp` |
| `Client/Base/include/RbxPlatform.h` | 32 | 0.483 | `robloxsrc/Trunk2012/Build/Base/include/RbxPlatform.h` |
| `Client/App/include/V8DataModel/NumberSequence.h` | 48 | 0.481 | `robloxsrc/Trunk2012/Build/App/include/v8datamodel/NumberSequence.h` |
| `Client/Base/util/RegistryUtil.cpp` | 336 | 0.478 | `robloxsrc/Trunk2012/Build/Base/util/RegistryUtil.cpp` |
| `Client/App/include/Util/Region3int16.h` | 40 | 0.471 | `robloxsrc/Trunk2012/Build/App/include/util/Region3Int16.h` |
| `Client/App/include/Util/Faces.h` | 25 | 0.463 | `robloxsrc/Trunk2012/Build/App/include/util/Faces.h` |
| `Client/Base/include/SelectState.h` | 15 | 0.446 | `robloxsrc/Trunk2012/Build/Base/include/SelectState.h` |
| `Client/App/include/v8xml/WebSerializer.h` | 23 | 0.441 | `robloxsrc/Trunk2012/Build/App/include/v8xml/WebSerializer.h` |
| `Client/Base/include/util/SafeToLower.h` | 16 | 0.440 | `robloxsrc/Trunk2012/Build/Base/include/util/SafeToLower.h` |
| `Client/Base/include/rbx/Intrusive/Set.h` | 235 | 0.437 | `robloxsrc/Trunk2012/Build/Base/include/rbx/Intrusive/Set.h` |
| `Client/Base/include/rbx/MathUtil.h` | 15 | 0.424 | `robloxsrc/Trunk2012/Build/Base/include/rbx/MathUtil.h` |
| `Client/App/include/Util/HeapValue.h` | 42 | 0.424 | `robloxsrc/Trunk2012/Build/App/include/util/HeapValue.h` |
| `Client/Base/include/rbx/ProcessPerfCounter.h` | 84 | 0.423 | `robloxsrc/Trunk2012/Build/Base/include/rbx/ProcessPerfCounter.h` |
| `Client/Log/include/FastLog.h` | 200 | 0.411 | `robloxsrc/Trunk2012/Build/Log/include/FastLog.h` |
| `Client/App/include/Util/SpatialRegion.h` | 132 | 0.411 | `robloxsrc/Trunk2012/Build/App/include/util/SpatialRegion.h` |
| `Client/Base/include/rbx/object_pool.h` | 69 | 0.406 | `robloxsrc/Trunk2012/Build/Base/include/rbx/object_pool.h` |
| `Client/Base/include/HardwareInfo.h` | 11 | 0.403 | `robloxsrc/Trunk2012/Build/Base/include/HardwareInfo.h` |
| `Client/Base/include/rbx/SystemUtil.h` | 39 | 0.394 | `robloxsrc/Trunk2012/Build/Base/include/rbx/SystemUtil.h` |
| `Client/Base/include/util/RegistryUtil.h` | 35 | 0.388 | `robloxsrc/Trunk2012/Build/Base/include/util/RegistryUtil.h` |
| `Client/App/include/Util/standardout.h` | 64 | 0.382 | `robloxsrc/Trunk2012/Build/App/include/util/standardout.h` |
| `Client/App/include/script/ExitHandlers.h` | 56 | 0.368 | `robloxsrc/Trunk2012/Build/App/include/script/ExitHandlers.h` |
| `Client/Base/include/rbx/GlobalVectorItem.h` | 49 | 0.359 | `robloxsrc/Trunk2012/Build/Base/include/rbx/GlobalVectorItem.h` |
| `Client/Base/include/rbx/CEvent.h` | 57 | 0.357 | `robloxsrc/Trunk2012/Build/Base/include/rbx/CEvent.h` |
| `Client/Base/include/rbx/RbxDbgInfo.h` | 60 | 0.349 | `robloxsrc/Trunk2012/Build/Base/include/rbx/RbxDbgInfo.h` |
| `Client/Base/include/RbxBase.h` | 13 | 0.349 | `robloxsrc/Trunk2012/Build/Base/include/RbxBase.h` |
| `Client/Base/include/rbx/Tasks/Coordinator.h` | 60 | 0.341 | `robloxsrc/Trunk2012/Build/Base/include/rbx/Tasks/Coordinator.h` |
| `Client/Base/include/rbx/intrusive_ptr_target.h` | 270 | 0.332 | `robloxsrc/Trunk2012/Build/Base/include/rbx/intrusive_ptr_target.h` |
| `Client/Base/include/rbx/Thread.hpp` | 52 | 0.321 | `robloxsrc/Trunk2012/Build/Base/include/rbx/Thread.hpp` |
| `Client/Base/include/CPUCount.h` | 7 | 0.319 | `robloxsrc/Trunk2012/Build/Base/include/CPUCount.h` |
| `Client/Base/include/rbx/memory.h` | 272 | 0.317 | `robloxsrc/Trunk2012/Build/Base/include/rbx/Memory.h` |
| `Client/App/include/Util/Axes.h` | 34 | 0.316 | `robloxsrc/Trunk2012/Build/App/include/util/Axes.h` |
| `Client/App/include/Util/md5.h` | 38 | 0.295 | `robloxsrc/Trunk2012/Build/App/include/util/md5.h` |
| `Client/Base/include/RbxFormat.h` | 64 | 0.294 | `robloxsrc/Trunk2012/Build/Base/include/RbxFormat.h` |
| `Client/App/include/V8Tree/EnumProperty.h` | 9 | 0.273 | `robloxsrc/Trunk2012/Build/App/include/v8tree/EnumProperty.h` |
| `Client/Base/include/rbx/Nil.h` | 6 | 0.266 | `robloxsrc/Trunk2012/Build/Base/include/rbx/Nil.h` |
| `Client/App/include/Util/gpc.h` | 70 | 0.260 | `robloxsrc/Trunk2012/Build/App/include/util/gpc.h` |
| `Client/Base/include/util/ScopedSingleton.h` | 51 | 0.255 | `robloxsrc/Trunk2012/Build/Base/include/util/ScopedSingleton.h` |
| `Client/Base/include/util/stringbuffer.h` | 49 | 0.253 | `robloxsrc/Trunk2012/Build/Base/include/util/stringbuffer.h` |
| `Client/Base/include/rbx/intrusive_weak_ptr.h` | 99 | 0.245 | `robloxsrc/Trunk2012/Build/Base/include/rbx/intrusive_weak_ptr.h` |
| `Client/Base/include/RbxAssert.h` | 31 | 0.224 | `robloxsrc/Trunk2012/Build/Base/include/RbxAssert.h` |
| `Client/Base/include/rbx/trie.h` | 233 | 0.222 | `robloxsrc/Trunk2012/Build/Base/include/rbx/trie.h` |
| `Client/App/include/Util/Quaternion.h` | 84 | 0.216 | `robloxsrc/Trunk2012/Build/App/include/util/Quaternion.h` |
| `Client/Base/include/rbx/make_shared.h` | 104 | 0.203 | `robloxsrc/Trunk2012/Build/Base/include/rbx/make_shared.h` |
| `Client/App/include/Util/utilities.h` | 94 | 0.175 | `robloxsrc/Trunk2012/Build/App/include/util/Utilities.h` |
| `Client/App/include/Util/ObscureValue.h` | 63 | 0.173 | `robloxsrc/Trunk2012/Build/App/include/util/ObscureValue.h` |
| `Client/Log/FastLogStream.cpp` | 106 | 0.169 | `robloxsrc/Trunk2012/Build/Log/FastLogStream.cpp` |
| `Client/Base/include/rbx/Profiler.h` | 55 | 0.156 | `robloxsrc/Trunk2012/Build/Base/include/rbx/Profiler.h` |
| `Client/Base/include/XStudioBuild.h` | 6 | 0.148 | `robloxsrc/Trunk2012/Build/Base/include/XStudioBuild.h` |
| `Client/Base/include/rbx/RunningAverage.h` | 679 | 0.139 | `robloxsrc/Trunk2012/Build/Base/include/rbx/RunningAverage.h` |
| `Client/App/include/Util/G3DCore.h` | 77 | 0.138 | `robloxsrc/Trunk2012/Build/App/include/util/G3DCore.h` |
| `Client/App/include/lua/lua.hpp` | 57 | 0.138 | `robloxsrc/Trunk2012/Build/App/include/lua/lua.hpp` |
| `Client/Base/include/rbx/Log.h` | 75 | 0.135 | `robloxsrc/Trunk2012/Build/Base/include/rbx/Log.h` |
| `Client/App/include/Util/Handle.h` | 59 | 0.127 | `robloxsrc/Trunk2012/Build/App/include/util/Handle.h` |
| `Client/App/include/security/FuzzyTokens.h` | 82 | 0.123 | `robloxsrc/Trunk2012/Build/App/include/security/FuzzyTokens.h` |
| `Client/Base/include/rbx/Declarations.h` | 15 | 0.115 | `robloxsrc/Trunk2012/Build/Base/include/rbx/Declarations.h` |
| `Client/Base/include/rbx/BaldPtr.h` | 63 | 0.114 | `robloxsrc/Trunk2012/Build/Base/include/rbx/BaldPtr.h` |
| `Client/App/include/Util/BinaryString.h` | 28 | 0.109 | `robloxsrc/Trunk2012/Build/App/include/util/BinaryString.h` |
| `Client/App/include/security/JunkCode.h` | 56 | 0.091 | `robloxsrc/Trunk2012/Build/App/include/security/JunkCode.h` |
| `Client/App/include/v8xml/WebParser.h` | 41 | 0.077 | `robloxsrc/Trunk2012/Build/App/include/v8xml/WebParser.h` |
| `Client/Base/include/rbx/callable.h` | 99 | 0.040 | `robloxsrc/Trunk2012/Build/Base/include/rbx/callable.h` |
| `Client/App/util/md5.c` | 222 | 0.035 | `robloxsrc/Trunk2012/Build/App/util/md5.c` |
| `Client/App/include/Util/ProgramMemoryChecker.h` | 55 | 0.035 | `robloxsrc/Trunk2012/Build/App/include/util/ProgramMemoryChecker.h` |
| `Client/Base/include/rbx/ArrayDynamic.h` | 175 | 0.030 | `robloxsrc/Trunk2012/Build/Base/include/rbx/ArrayDynamic.h` |
| `Client/Base/include/rbx/DenseHash.h` | 19 | 0.012 | `robloxsrc/Trunk2012/Build/Base/include/rbx/DenseHash.h` |
| `Client/Base/include/rbx/RbxStrings.h` | 43 | 0.005 | `robloxsrc/Trunk2012/Build/Base/include/rbx/RbxStrings.h` |
