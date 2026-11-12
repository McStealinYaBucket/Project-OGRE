# buildpath (workflow phase4-pairing, wf_4baec7d9-68d)

Assert strings now carry the 2013 build-machine paths. Both targets were rebuilt, their CSVs regenerated and both rescored.

**How VC9 forms `__FILE__`.** Measured under wine with a probe; it matches the shapes in the 2013 binaries.
- A main file's `__FILE__` is its command-line spelling.
- A header found on an /I path gets that /I spelling, a backslash, then the `#include` spelling, e.g. `C:\BuildAgent\work\<hash>\Client\App\include\Util/Extents.h`.
- A header found next to the file that includes it gets that file's directory in lowercase, a backslash, then the `#include` spelling. That is where `c:\buildagent\...\client\network\raknet\source\DS_List.h` and `client\app\lua-5.1.4\src\luaconf.h` come from.
- So header paths in the 2013 strings come from absolute /I spellings, not from relative `..\` paths.

**What changed.**
- **`winewrap.py`.** With `OGRE_TARGET` set to PLAYER or STUDIO, repo paths become `C:\BuildAgent\work\407e17641b466b38` (PLAYER) or `fa0a2141fe3d2f4d` (STUDIO) instead of `O:`.
  - The boost headers become the 2013 Contrib2 path: PLAYER `C:\Users\build\build_SM-UTIL14\build_SM-UTIL14\Contrib2\boost_1_53_0\include`, STUDIO `C:\Users\build\build_SM-UTIL13\...`. The PLAYER path comes from the Oct 23 binary itself; the player column of `notes/paths-2013.txt` is the Oct 15 build.
  - Each source file the binary names is passed in its 2013 spelling from `tools/msvc9/paths-<T>.txt` (291 PLAYER, 383 STUDIO files, written by `gen_paths.py`).
  - Compiler output is translated back to host paths with the on-disk case, so Ninja's dependency tracking still works; every recorded dependency path exists.
  - The wrapper creates the symlinks it needs in the wine prefix's `drive_c` on first use.
- **`cmake/msvc9-wine.cmake`.** New cache variable `OGRE_TARGET` (default PLAYER), passed to the compiler through `env` as a launcher.
- **`CMakeLists.txt`, include spellings.** App's include root follows each 2013 project's spelling: RbxOgre `Client/App/Include`, GfxBase `Client/app/include`, everything else `include`. This was attributed from which library's functions use each spelling.
- **`CMakeLists.txt`, link order.** Each exe now links in its 2013 `.text` order, which also decides which copy of an inline function the linker keeps:
  - PLAYER: client, RbxOgre, Network, GfxAdapters, App, Base, AppDraw, GfxBase, g3d, RbxG3D, then boost_static.
  - STUDIO: client, RbxOgre, Network, GfxAdapters, RbxG3D, g3d, GfxBase, App, Base, AppDraw, then boost_static.
- **`CMakeLists.txt`, configure fix.** A fresh configure failed to link `Log.dll` because `OGRE_FORCE_UNRESOLVED` was declared after Log read it. It is now declared earlier.

**Commands.**
- PLAYER is unchanged: `build/`, `gen_reccmp_csv.py PLAYER`, and reccmp from `build/`.
- STUDIO now has its own directory: `cmake -G Ninja -S . -B build-studio -DCMAKE_TOOLCHAIN_FILE=cmake/msvc9-wine.cmake -DOGRE_TARGET=STUDIO`, then `cmake --build build-studio --target studio`, then `python3 -P tools/ida/gen_reccmp_csv.py STUDIO --build build-studio`, then reccmp from `build-studio/`.
- The `CMakeLists.txt` header shows these commands. CLAUDE.md's "same for STUDIO" line needs to say "from build-studio". `build/RobloxStudioBeta.exe` still builds, but with PLAYER's checkout path in its strings.

**Results** (the reccmp JSON's own 100% counts; reccmp's summary line in brackets):

| | Before | Paths only | Paths + link order |
|---|---|---|---|
| PLAYER, all functions at 100% | 1,741 (1930/40468, 97.30%) | 1,799 (97.50%) | 1,838 (1960/40468, 97.74%) |
| PLAYER, library rows at 100% | 1,560 / 1,829 | 1,621 | 1,656 / 1,859 |
| STUDIO, all functions at 100% | 1,641 (1817/42329, 97.84%) | 1,703 (98.25%) | 1,739 (1848/42329, 98.30%) |
| STUDIO, library rows at 100% | 1,485 / 1,718 | 1,547 | 1,581 / 1,749 |

- Functions whose only difference was the file string: **59 per target before, 0 after.** All 59 now match in both targets: 55 RakNet and 4 DataStructures. That is below the 75 your brief quoted; my classifier only counts a function when every changed line is a path inside an assert string.
- App markers (`app_sweep.py report`): PLAYER 112 → 115 at 100%, STUDIO 108 → 111, no App marker dropped. The +3 are Region3.cpp and came from another agent's concurrent work, not from this change. The assert-file-line shape went 41 → 10 in PLAYER; those functions still have other differences.
- Other agents rebuilt G3D and other sources during these runs, so the totals include some of their changes.

**Previously-100% entries that dropped, none from the code itself:**
- **Three vftables per target** (G3D::Plane, RakNet::RakPeerInterface, boost::bad_lexical_cast; STUDIO's Plane one later came back with the STUDIO link order). The tables themselves are unchanged. reccmp guesses a vftable's size from the gap to the next symbol in our build, and the new path strings moved the data that follows.
- **G3D::MemoryManager::alloc, ::free and CoordinateFrame::inverse.** They left the G3D CSV because `ida/exports/PLAYER-lib-resolved.csv` was rewritten at 13:15 by another agent (status "span"). alloc and free are still in our map at the same address.

**`__LINE__`** (PLAYER; STUDIO is the same apart from one or two pairs):
- 8 functions now differ only by line number, plus 1 by file and line.
- Across all functions, 190 assert-string pairs differ only in the line (126 functions); another 191 differ in message and line (117 functions), which are different asserts at that spot.
- Examples (2013 line, ours):

| Function | File | 2013 | Ours |
|---|---|---|---|
| `Extents::getCornerIndex` | `Util\Extents.cpp` | 53 | 54 |
| `Joint::resetLink` | `V8World/Joint.h` | 183 | 155 |
| `GuiResponse::wasSunkAndFinished` | `Gui/GuiEvent.h` | 66 | 68 |
| `GuiItem::getChildPosition` | `Gui/GUI.h` | 69 | 71 |
| `Math::safeDirection` | `Util\Math.cpp` | 811 | 526 |
| `SpanningEdge::getConstChildSpanningNode` | `util\SpanningEdge.cpp` | 23 | 36 |
| `IndexedTree::getOneBelowRoot` | `Util/IndexedTree.h` | 78 | 109 |
| `SpanningTree::swapTree` | `util\SpanningTree.cpp` | 86 | 64 |

- Line-only pairs elsewhere include `Util/Guid.h` 144 vs 218, `World.cpp` 432 vs 295, `Math.cpp` 1521 vs 1036, `Body.cpp` 95 vs 337.

**Remaining file-string differences** (150 pairs in PLAYER):
- 101 are code in the wrong file, e.g. `KernelData.h` vs `KernelData.cpp`, `SimJob.cpp` vs `SimJobTracker.cpp`, `SerializerV2.h` vs `Reference.h`.
- 29 are path literals hard-coded in the prior tree's source: `/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/ScriptContext.cpp` and `include/Script/LuaArguments.h`.
- 20 are case only: 12 are RbxOgre's copies of App inline headers, which our RbxOgre cannot supply yet (only 3 non-empty .cpp files); 8 are `#include "util/FeatherweightDict.h"` vs `Util/`.

**New tools.**
- `toolchain/venv/bin/python -P tools/msvc9/assert_diff.py P.json [--before OLD.json] [--lines N]` classifies diffs as file-only / line-only / file+line and lists drops.
- `python3 -P tools/msvc9/gen_paths.py` regenerates the spelling tables.

**Wine prefix.** Added symlinks `drive_c/BuildAgent/work/{407e17641b466b38,fa0a2141fe3d2f4d}` pointing at the repo, and `drive_c/users/build/.../Contrib2/boost_1_53_0/include` pointing at the downloaded boost headers.

**CSVs.** PLAYER's `reccmp/*.csv` were regenerated against `build/` and STUDIO's against `build-studio/`.

## Structured fields

```json
{
 "files": [
  "tools/msvc9/winewrap.py",
  "tools/msvc9/gen_paths.py",
  "tools/msvc9/paths-PLAYER.txt",
  "tools/msvc9/paths-STUDIO.txt",
  "tools/msvc9/assert_diff.py",
  "cmake/msvc9-wine.cmake",
  "CMakeLists.txt",
  "build-studio",
  "toolchain/wineprefix/drive_c/BuildAgent/work",
  "/tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/base-PLAYER.json",
  "/tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/lo2-PLAYER.json",
  "/tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/base-STUDIO.json",
  "/tmp/claude-1000/-home-thebigw-Documents-Project-OGRE/93f42955-134d-420f-91e4-d72f5119fbc9/scratchpad/lo2-STUDIO.json"
 ],
 "unresolved": [
  "Vftable 100% drops (PLAYER: G3D::Plane 0x00b032e0, RakNet::RakPeerInterface 0x00b27d54, boost::bad_lexical_cast 0x00b2cfe0; STUDIO 0x00bdf23c, 0x00bf3408): reccmp sizes a vftable by the gap to the next symbol in our build, and the path strings moved the data that follows. The tables are unchanged. Fix: gen_reccmp_csv.py should write vftable sizes (slot counts from the original) into the libdata CSV, which is outside my ownership.",
  "G3D::MemoryManager::alloc/free (PLAYER 0x009daed0/0x009daee0, STUDIO 0x006aedd0/0x006aede0) and CoordinateFrame::inverse (PLAYER 0x004a2dc0, STUDIO 0x00532180) dropped out of the G3D CSV because ida/exports/PLAYER-lib-resolved.csv was rewritten at 13:15 by another agent (status 'span'). Not caused by the path change.",
  "boost link-order trade-off: with boost_static before App, PLAYER gains about 14 more functions at 100% (e.g. boost::exception_detail clone_impl 0x0040e130, boost::any 0x00479b30, program_options vftables 0x00be63c0) but loses boost::filesystem::path::string 0x00660160, so I kept boost_static last. In 2013, program_options was compiled inside WindowsClient. Moving its sources into player_client is a source-list change for that list's owner and should get both.",
  "CLAUDE.md needs updating (I may not edit it): score STUDIO from build-studio/, configured with -DOGRE_TARGET=STUDIO, and run gen_reccmp_csv.py STUDIO --build build-studio. app_sweep.py apply reads build/<exe>.pdbview.tsv for STUDIO and should use build-studio/ instead.",
  "Remaining file-string mismatches are source-level, not build paths: 101 pairs are code in the wrong file (KernelData.h vs .cpp, SimJob vs SimJobTracker, SerializerV2.h vs Reference.h). 29 are hard-coded literals in the prior tree ('/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/ScriptContext.cpp', 'include/Script/LuaArguments.h'). 8 are the #include spelling util/FeatherweightDict.h vs Util/. 12 are RbxOgre copies of App inline headers that our RbxOgre cannot supply yet (only 3 non-empty .cpp files).",
  "__LINE__ misses are untouched by design: 8 functions per target differ only by line number, plus 1 by file and line (MergeBinder::announceIDREF). Across all functions, 190 assert-string pairs differ only in the line and 191 in message and line."
 ]
}
```

