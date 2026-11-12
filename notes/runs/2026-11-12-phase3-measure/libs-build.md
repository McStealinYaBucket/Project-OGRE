# libs-build (workflow phase3-measure, wf_271afd88-e3a)

The matched third-party libraries are now compiled into the default build, and reccmp pairs and compares their functions itself instead of relying on the .obj match. The default build (OGRE_PRIOR_APP=OFF) is green (`cmake --build build` exit 0). Both exes and a new boost.dll link. CLAUDE.md, DECOMP_PLAN.md and the .i64 databases were not touched.

**Final scores (no --nolib).** reccmp now pairs PLAYER 1,656 functions (was 15), STUDIO 1,541 (was 16). `--nolib` gives 1 paired, 0 at 100% in both targets: the one pair is RBX::RbxRay::fromOriginAndDirection at 96.3%.

PLAYER, per library:

| library | rows | paired | 100% | below 100% | unpaired |
|---|---|---|---|---|---|
| lua | 514 | 514 | 497 | 17 | 0 |
| raknet | 463 | 463 | 345 | 118 | 0 |
| g3d | 131 | 131 | 124 | 7 | 0 |
| zlib | 45 | 45 | 44 | 1 | 0 |
| libzip | 65 | 65 | 65 | 0 | 0 |
| boost | 483 | 302 | 289 | 13 | 181 |
| callees found from paired code (libdata) | 113 | 112 | 26 | 86 | 1 |
| **total** | 1,814 | 1,632 | 1,390 | 242 | 182 |

STUDIO, per library:

| library | rows | paired | 100% | below 100% | unpaired |
|---|---|---|---|---|---|
| lua | 514 | 514 | 497 | 17 | 0 |
| raknet | 464 | 463 | 347 | 116 | 1 (0x00606c90 ~CCRakNetSlidingWindow) |
| g3d | 131 | 131 | 124 | 7 | 0 |
| zlib | 45 | 45 | 44 | 1 | 0 |
| libzip | 65 | 65 | 65 | 0 | 0 |
| boost | 402 | 218 | 205 | 13 | 184 |
| callees (libdata) | 81 | 79 | 28 | 51 | 2 |
| **total** | 1,702 | 1,515 | 1,310 | 205 | 187 |

- **Boost unpaired rows:** 181 (PLAYER) and 184 (STUDIO) are boost-thread header bodies. The original got them from Roblox source files, which our build does not compile yet.
- **Spot checks with --verbose (PLAYER), all 100%:** lua_gettop 0x00873b00, ReadFloat16 0x005c3910, 0x009d7e50, deflate 0x009e07a0, zip_fread 0x009e8460, zlib_error::check 0x006130e0, error_with_option_name::what 0x00478880, codecvt_error_category 0x009c0f20.
- **0x009d7e50:** this is G3D::fnan, but reccmp shows it under the name G3D::finf. Our linker folded the two bodies together and the PDB keeps one name per address.
- **luaO_int2fb:** the placeholder's `// FUNCTION:` marker is gone. It is now paired by its library row and scores 100%.

**Steps 1–4: what goes where.**
- **Install script.** `tools/lib/install_libs.py` (new) rebuilds each tree from the pristine upstream copy plus the staged patches, exactly as the matchers did.
- **Lua.** All 24 linked TUs go to `Client/App/Lua-5.1.4/src`, compiled as C++ with the App flags in the App library.
  - ldblib, liolib, loslib, loadlib, linit, lua.c, luac.c and print.c are on disk but not compiled.
  - Stand-ins used instead of real Client headers:
    - patch 01: `<intrin.h>` in luaconf.h, in place of rbx/Debug.h → RbxPlatform.h
    - patch 10: the RobloxExtraSpace declaration
    - patch 11: FastLog.h
    - patch 14: RbxProtectedString.h
  - The two stand-in headers sit in `Client/App/Lua-5.1.4/standin`, added as a source-level include directory. CMake puts that ahead of App's own include roots, so it reaches only the Lua files and never shadows `Client/Log/include/FastLog.h` for other App code.
- **RakNet.** 4.034 plus patches 0001–0003 replaces `Client/Network/raknet/Source`: 26 TUs plus every header, with plugins left out. It is compiled into Network with the proven App set. The .gitignore entry is kept.
- **G3D.**
  - Placement:
    - G3D: patches 0001–0005 (including the Vector3 `__fastcall operator=`) on the prior tree's copy, in `Client/Rendering/g3d/{G3Dcpp,include}`.
    - zlib 1.2.3: `Client/Rendering/g3d/zlib`.
    - libzip 0.9: `Client/Rendering/g3d/zip`. The zlib/ and zip/ directory names come from the 2016 tree; no 2013 `__FILE__` records either.
    - IJG 6b: `ijg/`. It is compiled only so GImage_jpeg links; none of it is linked in PLAYER.
  - Flags: C files as C, App set + ZLIB_WINAPI.
  - App code gets G3D headers from the include root `Client/Rendering/g3d/include`, so the Vector3 change now reaches App.
- **G3D build-only workarounds.**
  - GImage_png.cpp is left out, and the prior tree's png/ directory is deleted (PLAYER links no libpng). A hand-written `shim/png.h` and `shim/png_absent.cpp` resolve GImage.cpp's three references so the exe links.
  - The fork's CoordinateFrame.h includes a Bullet header (a 2016 addition). It only resolves through a read-only include root, `2013 55% App/Rendering/RbxG3D/include`. It has to stay: Bullet's header makes `<emmintrin.h>` visible, and the 161-identical G3D match was measured with it.
- **boost.**
  - The pristine sources are copied to `3rdparty/boost_1_53_0`.
  - `boost_static` (filesystem + iostreams + program_options): /GS on, BOOST_SYSTEM_DYN_LINK, BOOST_THREAD_DYN_LINK, ZLIB_WINAPI, BOOST_ALL_NO_LIB.
  - `boost` is a SHARED target producing boost.dll (thread + system, /GS-, BOOST_THREAD_BUILD_DLL, BOOST_SYSTEM_SOURCE, BOOST_CHRONO_HEADER_ONLY). Both exes link its import library and ws2_32.
  - program_options was not built /GL.
- **Network flags.** They are now the proven App set.
- **Linking with `/FORCE:UNRESOLVED`.** It is on by default as a CMake option (OGRE_FORCE_UNRESOLVED), because the library files reference code no default-build library defines yet:
  - FLog::* and FFlag::MarkRakNetSocketOnDelete (Log.dll)
  - RBX::ProtectedString, MD5Hasher, the RobloxExtraSpace ctor/dtor, DataModel::sendStats
  - RakNet PacketizedTCP, TCPInterface and CSHA1, whose only callers the original's linker dropped

**Step 5: how pairing works.** In reccmp a0f494c0, `match_symbols` pairs a CSV `symbol` only with a PDB PUBLICS name, and `match_functions` pairs a CSV `name` with a PDB proc name. So `tools/ida/gen_reccmp_csv.py` now resolves every library row against our own build's map and PDB.
- A new helper, `tools/lib/pdb_view.py`, dumps exactly what reccmp reads from the PDB.
- Rows whose bodies our linker folded together take the name reccmp actually keeps at that address.
- Anonymous-namespace names (`?A0x<hash>`) take our hash, which depends on the source path.
- Static functions have no PUBLICS entry, so they get an empty symbol and the PDB proc name (334 PLAYER / 329 STUDIO).
- Row size is the larger of IDA's size and the .obj length, because IDA stops before in-function jump tables.
- A new `reccmp/<t>-libdata.csv` (770 PLAYER / 663 STUDIO rows) adds what the paired functions reference, found by position: globals, vtables, strings, floats (with size), throw info, vftable slot functions and callees.
  - It also adds 101 nameless placeholder rows for the `__ehhandler$` stubs that /GS functions push. reccmp's handler finder misses them, so this is a workaround.
  - These rows took PLAYER from 850 to 1,390 at 100%.

**Step 6: why the 242 PLAYER below-100% rows differ** (`lib_score.py --classify`; a function can carry several causes, so these add up to more than 242):
- **`__FILE__` path, 75 (60 RakNet functions differ by nothing else):** the original's `C:\BuildAgent\work\407e17641b466b38\...` against our `O:\...`. The fix is in the compiler wrapper (`tools/msvc9/winewrap.py`, not mine). Even then one build can match only one target, because STUDIO's path is different.
- **Instructions differ, 84 (mostly callees found from paired code):** the original's linker kept a copy of an inline/template function compiled in another file, often a Roblox one. For example RakNet BitStream::IsNetworkOrder is out of line with an exception-handling frame in the original and inlined in ours.
- **Data, 58:** each file has its own static copy of a constant such as UNASSIGNED_RAKNET_GUID, or of a static-local guard, and the original's references point at another file's copy. Also zlib lenfix/distfix: infback.c's duplicate statics share the name, and reccmp pairs by name.
- **Callee, 43:** our linker or the original's folded identical bodies, or the callee is unpaired.
- **EH handler, 40:** the `__ehhandler$` stub that /GS functions push. These remain after the placeholder rows above.
- **Unresolved, 27:** calls to the symbols `/FORCE:UNRESOLVED` leaves at 0. They resolve once App, Base and Log exist.
- **Folded with Roblox code, 23:** the original folded a library body (operator delete, the CCRakNetSlidingWindow ctor) with a Roblox-named function. Those addresses are deliberately left unpaired.
- **Constants taken for addresses, 13:** the original has no .reloc, so reccmp reads immediates such as 0x7fc000 in Lua masks as addresses.
- **Different float constant, 3:** two of these are wrong matcher rows. G3D::Color4::one at 0x007681b0 loads 0.6f and Color3::one at 0x009d7c30 loads 0.7f in PLAYER; the matcher masks float constants, so it called them identical.

What I fixed on our side: statics, folded names, anonymous-namespace hashes, jump-table sizes, data, floats, throw info, vftable slots, callees and the /GS EH-handler pushes.

**Step 7: checks** (run from build/ with the venv python):
- **`reccmp-decomplint` (PLAYER and STUDIO):** exit 1. 336 `function_out_of_order` + 42 `duplicate_offset` (PLAYER), 341 + 34 (STUDIO). These are the same counts as before; they come from the imported App tree.
- **`reccmp-vtable`:** exit 1. PLAYER 99 vtables found, 41 not matching; STUDIO 56 found, 22 not matching (it was 0 found before the libdata rows). The failures are slots pointing at original functions that are unpaired or folded.
- **`reccmp-datacmp`:** exit 1. PLAYER 164 variables, 9 issues; STUDIO 150, 9. The issues are pointer-valued data: Lua opts/optsnum, rakMalloc_Ex/rakRealloc_Ex/rakFree_Ex/notifyOutOfMemory, separator_string, static_bl_desc. There are also WARNs on throw-info types reccmp doesn't know.
- **`check_folded`:** exit 0. PLAYER 976 annotated addresses, STUDIO 964, 0 folding problems.
- **`reccmp_addr_padding`:** 1,647 files, 0 badly padded.

**RakNet licence.** The RakNet_PC-4.034 archive ships no open-source licence.
- `readme.txt`: "Copyright 2002-2005 Kevin Jenkins ... Licensed under the "RakNet" brand by Jenkins Software and subject to the terms of the relevant licensing agreement available at http://www.jenkinssoftware.com".
- Every source file: "This file is part of RakNet Copyright 2003 Jenkins Software LLC / Usage of RakNet is subject to the appropriate license agreement."
- The only licence document is `RakNet_4.0_Eval_Clickthrough_1.1.doc`, an evaluation licence:
  - 1.1: "a non-exclusive, non-transferable, limited license ... to download, install, and evaluate"
  - 2.1: "shall not ... utilize the Program in connection with the development or maintenance of any product or application"
  - 2.5: "no copies of the Program except for those necessary to fulfill the purpose of this Agreement"
- `Client/Network/raknet/` stays gitignored. `staging/raknet` is not in .gitignore, so it would be committed if staging/ is ever added.

## Structured fields

```json
{
 "files": [
  "CMakeLists.txt",
  "reccmp-project.yml",
  "tools/ida/gen_reccmp_csv.py",
  "tools/lib/install_libs.py",
  "tools/lib/pdb_view.py",
  "tools/lib/lib_score.py",
  "Client/App/Lua-5.1.4/src",
  "Client/App/Lua-5.1.4/standin/FastLog.h",
  "Client/App/Lua-5.1.4/standin/RbxProtectedString.h",
  "Client/Network/raknet/Source",
  "Client/Rendering/g3d/G3Dcpp",
  "Client/Rendering/g3d/include",
  "Client/Rendering/g3d/zlib",
  "Client/Rendering/g3d/zip",
  "Client/Rendering/g3d/ijg",
  "Client/Rendering/g3d/shim/png.h",
  "Client/Rendering/g3d/shim/png_absent.cpp",
  "3rdparty/boost_1_53_0",
  "reccmp/player-lua.csv",
  "reccmp/player-raknet.csv",
  "reccmp/player-g3d.csv",
  "reccmp/player-zlib.csv",
  "reccmp/player-libzip.csv",
  "reccmp/player-boost.csv",
  "reccmp/player-libdata.csv",
  "reccmp/studio-lua.csv",
  "reccmp/studio-raknet.csv",
  "reccmp/studio-g3d.csv",
  "reccmp/studio-zlib.csv",
  "reccmp/studio-libzip.csv",
  "reccmp/studio-boost.csv",
  "reccmp/studio-libdata.csv",
  "build/RobloxPlayerBeta.exe",
  "build/RobloxStudioBeta.exe",
  "build/boost.dll"
 ],
 "unresolved": [
  "`__FILE__` paths: 75 PLAYER library functions differ only or partly in the build-agent path (60 RakNet functions differ by nothing else). The fix is in tools/msvc9/winewrap.py (not mine): build from a path that maps to C:\\BuildAgent\\work\\407e17641b466b38\\ for PLAYER. STUDIO's path is fa0a2141fe3d2f4d, so one build cannot match both targets.",
  "`/FORCE:UNRESOLVED` is on by default (OGRE_FORCE_UNRESOLVED). Lua and RakNet call FLog::* and FFlag::MarkRakNetSocketOnDelete (Log.dll), RBX::ProtectedString, MD5Hasher, the RobloxExtraSpace ctor/dtor and DataModel::sendStats, plus RakNet PacketizedTCP/TCPInterface/CSHA1. 27 PLAYER diffs come from this. Turn it off once Log, Base and App provide these symbols.",
  "The prior-App block (not mine) still globs Network/*.cpp and Rendering/g3d/*.cpp|*.c. CMake de-duplicates the RakNet/G3D files, but under OGRE_PRIOR_APP=ON it also adds G3Dcpp/GImage_png.cpp, which cannot compile against the png shim. That block should exclude GImage_png.cpp, or drop its g3d and raknet globs now that the default build compiles those libraries. The prior-mode configure step succeeds; the compile was not run.",
  "reccmp limitations, suggested as reccmp patches in tools/reccmp-patches (not mine): (a) find_eh_handlers labels only the `mov eax, FuncInfo` inside /GS EH handler stubs, worked around here with nameless placeholder rows, and 40 diffs remain; (b) the original has no .reloc, so immediates such as Lua's 0x7fc000/0x7fffff masks are read as addresses (13 diffs).",
  "A CSV row cannot point reccmp at one specific recompiled static when several share a name: per-file const copies (UNASSIGNED_RAKNET_GUID, zlib lenfix/distfix from infback.c) can pair with the wrong copy, and the original also references other files' copies. 58 PLAYER diffs are labelled data.",
  "In 84 PLAYER diffs the instructions differ: the original's linker kept a copy of an inline or template function compiled in another file, mostly a Roblox one (e.g. RakNet BitStream::IsNetworkOrder and Write<T>, boost/std header bodies). These match only once the Roblox Network/App files exist and link in the original's object order.",
  "Two G3D library rows are wrong matcher placements: Color4::one at PLAYER 0x007681b0 loads 0.6f and Color3::one at 0x009d7c30 loads 0.7f. The matchers mask float constants, so they called these identical.",
  "23 PLAYER diffs come from the original folding library bodies (operator delete wrappers, the CCRakNetSlidingWindow ctor) with Roblox-named functions. Those addresses stay unpaired by design (lib_consistency 'conflict').",
  "program_options was built /GL (LTCG) in PLAYER but was compiled without /GL here (no /LTCG link). Its 5 non-identical functions and its unlocated internals are still unmatched.",
  "Licence decision needed: RakNet 4.034 is under the RakNet 4.0 Evaluation Clickthrough licence (evaluation only; no use in developing a product; no copies beyond evaluation). Client/Network/raknet stays gitignored, but staging/raknet is not in .gitignore. The user should decide whether to keep it.",
  "The G3D build depends on a read-only include root, '2013 55% App/Rendering/RbxG3D/include', for the fork's 2016 Bullet include in CoordinateFrame.h. Removing it needs a no-Bullet G3D patch plus a re-measure, because Bullet's btScalar.h makes <emmintrin.h> visible and that changes codegen."
 ]
}
```

