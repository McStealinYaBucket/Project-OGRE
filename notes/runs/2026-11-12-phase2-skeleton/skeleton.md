# skeleton (workflow phase2-skeleton, wf_e3afb0d0-6b4)

The build and reccmp skeleton (Phase 2, steps 2-6) works end to end. The 2026-11-12 run built PLAYER and STUDIO, scored luaO_int2fb at 100%, and passed every check.

**Commands** (run from the repo root):
- Patch the venv: `sh tools/reccmp-patches/apply.sh` (safe to run twice; already applied here).
- Generate the CSVs: `python3 -P tools/ida/gen_reccmp_csv.py` (`--selftest` passes).
- Configure: `cmake -G Ninja -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/msvc9-wine.cmake` (prints `MSVC for decompilation: TRUE`).
- Build: `cmake --build build`. This gives `build/RobloxPlayerBeta.exe`/`.pdb`/`.map`, `build/RobloxStudioBeta.exe`/`.pdb` and `build/reccmp-build.yml`.
- Score: `cd build && PYTHONIOENCODING=utf-8 ../toolchain/venv/bin/reccmp-reccmp --target PLAYER --nolib`.

**Results**
1. **Wrapper hang fixed.** `winewrap.py` now starts `wineserver -p` detached on /dev/null before each tool run. If a server is already running it exits at once with status 2, so the call is cheap. The tool's output is also captured through a temp file instead of a pipe, so a daemon wine leaves behind can no longer hold the wrapper open. Cold start check: I confirmed no compile or link was running, then killed the prefix's wineserver. The next `cl /Zi` returned in 0.76 s (rc=0) and `link /DEBUG` in 0.17 s. Afterwards wineserver, mspdbsrv, services and winedevice were all running.
2. **Scaffolding.** OpenRBX is GPLv3, so I copied none of its files; these are my own equivalents. If you are willing to license this repo GPLv3, copying becomes allowed.
   - `util/decomp.h`: `STUB(addr)` stores a different constant to `g_foldingDummyVariable` (with C linkage) so ICF cannot fold stubs; `DECOMP_SIZE_ASSERT` is active only under `_MSC_VER` 1500; plus `undefined`/`undefined2`/`undefined4`.
   - `util/decomp.cpp` and `util/compat.h` (the latter defines `MSVC_FOR_DECOMP` for `_MSC_FULL_VER` 150030729).
   - `.clang-format`: I set `IncludeBlocks: Preserve` and `SortIncludes: Never`, which avoids the include-regrouping gotcha CLAUDE.md warns about.
   - `tools/reccmp_addr_padding.py`, which uses reccmp's own marker regex (`--selftest` passes).
   - `tools/check_folded.py`, retargeted to `--target PLAYER/STUDIO`.
   - `cmake/reccmp.cmake`, which writes `reccmp-build.yml`.
   - `.gitignore` now also ignores `3rdparty/_downloads/`, `reccmp-build.yml` and `__pycache__/`. `staging/` is not ignored.
3. **CMakeLists.txt.**
   - It needs CMake 3.25 or later; below that, `/Z7` is silently dropped and the PDB has no line records.
   - Engine OBJECT libraries: App (proven flags), Base, Network, AppDraw, GfxBase, GfxAdapters, RbxG3D, RbxOgre and g3d, laid out as in `notes/paths-2013.txt`.
   - Each client compiles its own copy of ClientBase and Win with its own flags (`player_client`, `studio_client`); ClientShared is Studio only.
   - Non-App flags come from the 2016 Release|Win32 values and each carries `# TODO prove`. The exception is `/arch`: I used `/arch:SSE` everywhere, because the census found 0 scalar-double SSE2 instructions against 24,013 scalar SSE1. The 2016 `/Oy-` is kept but flagged, since it was wrong for App.
   - CMake's default flags are cleared. Runtime `/MD` and `/Z7` come from the CMake properties.
   - Link flags: `/DEBUG /INCREMENTAL:NO /BASE:0x400000 /FIXED:NO /DYNAMICBASE:NO /OPT:NOREF /OPT:ICF /MAP`. `OGRE_KEEP_UNREFERENCED` switches NOREF to REF. I chose `/FIXED:NO` so the recompiled exe keeps `.reloc`, which reccmp uses to tell addresses from constants; only the original lacks it.
   - Left out on purpose: Log (it ships as `Log.dll`, so it needs its own target), Ogre, boost.dll, tbb, Qt and the prebuilt D3DX.
4. **reccmp config.** `reccmp-project.yml` defines PLAYER (Oct 23 SHA256 326d9c3e…) and STUDIO (68ba1c5f…). PLAYER's report ignores `WinMainCRTStartup`, because reccmp would otherwise pair it with VMProtect's entry at 0x00ee050a (it scored 0%). `reccmp-user.yml` (gitignored) points both targets at `bin/`.
5. **CSV generator (`tools/ida/gen_reccmp_csv.py`).** Addresses are absolute, written as 8 hex digits, which is what reccmp's CSV loader expects. The filter counts go in `#` comment lines at the top of each function-size file, which reccmp skips.
   - **PLAYER:** 48,868 IDA functions, 40,249 kept. Dropped: 6,884 outside `.text` (the `.vmp` sections), 27 touched by VMProtect, 40 unwind/catch funclets, 757 import thunks and 911 adjustor thunks. The funclets come from 6,339 FuncInfo records (19,336 unwind and catch targets); IDA folded almost all of them into their parent functions.
   - **STUDIO:** 44,599 functions, 42,115 kept (1,529 import thunks and 955 adjustor thunks dropped; 7,076 FuncInfo records).
   - Other files: `player-symbols.csv` has 6,697 rows and `studio-symbols.csv` 6,813 (demangled ios:/mac: names, no type). The flirt names split into crt/stl/atl as 55/6/19 for PLAYER and 92/104/19 for STUDIO, typed `library` with the decorated name in the `symbol` column. `player-imports.csv` names the 1,928 linker IAT slots.
   - Any `ida/exports/<T>-lib-<lib>.csv` turns into `reccmp/<t>-<lib>.csv` (only rows with `identical=yes`). The generator flags any output file that `reccmp-project.yml` does not list yet.
6. **Import table.** A CSV alone cannot fix this, because reccmp's CSV loader has no import type. So I wrote one small patch, `tools/reccmp-patches/0001-ogre-imports-and-vc9-funcinfo.patch`, applied by `apply.sh` and recorded in `tools/requirements.txt`. It changes five things:
   - CSV type `import` creates an import entry.
   - When two imports share a name, the lowest address wins, so recompiled imports pair with the `.rdata` IAT that `.text` uses and not with VMProtect's copy in `.vmp1`.
   - Import thunks are also found for slots named in the CSV: 1 before, 5,442 after.
   - The FuncInfo search accepts magic 0x19930521 and 0x19930522: PLAYER goes from 1 record to 6,339, STUDIO from 1 to 7,076. That 1-record figure is what the toolchain notes measured earlier; the upstream search only looks for 0x19930520, which VC9 never writes.

   **Verification:** I added a temporary function at PLAYER 0x00430a50 (`push ecx; call [DeleteCriticalSection]; ret`, written as `DeleteCriticalSection(&m_cs)`). With the imports CSV it scored 100%. Without it, it scored 66.67%: the original side showed `call dword ptr [<OFFSET1>]` where the recompiled side had `->KERNEL32.dll::DeleteCriticalSection (IMPORT)`. The temporary file has since been removed.
7. **Smoke test.**
   - Before adding the real function, the denominator was 40,168 and the only match was the bogus entry pair (0%), now ignored.
   - `luaO_int2fb` is now in `Client/App/Lua-5.1.4/src/lobject.c`, compiled as C++ with the App flags under `// FUNCTION: PLAYER 0x008a78c0`. After a clean rebuild reccmp reports it 100.00% similar, with Implemented 1/40,167 and Accuracy 100%. `--json` writes it with `matching: 1.0`.
   - STUDIO: Implemented 0/41,900.
   - Checks, all exit code 0:
     - `reccmp-decomplint Client --module PLAYER --warnfail`
     - `reccmp-vtable --target PLAYER` ("Vtables found: 0")
     - `reccmp-datacmp --target PLAYER` ("Variables: 0. Issues: 0")
     - `check_folded.py --target PLAYER` ("1 annotated addresses, 0 folding problems")
     - `reccmp_addr_padding.py` ("15 files, 0 badly padded markers")

**reccmp limitations found**
- Imports come from the `.vmp1` directory (fixed by the patch plus the CSV).
- FuncInfo is only searched for magic 0x19930520 (fixed by the patch).
- PLAYER has no `.reloc`, so reccmp falls back to a heuristic pointer scan: 274,328 pointers. 6,161 of them are read from `.vmp0` (1,955) and `.vmp1` (4,206), because both sections are marked executable and writable and get scanned as code. That scan only keeps pointers into data sections, so pointers into `.text` (vtable slots, function-pointer tables) are never found without relocations.
- The import-thunk scan marks every `FF 25` jump to an import slot as a thunk, including tail jumps inside functions. Unpatched reccmp already finds 9,647 such thunks in STUDIO; the patch extends the same behaviour to PLAYER.
- Entry points are paired automatically, which is wrong for PLAYER (handled with ignore-functions).
- reccmp runs `cvdump.exe` under plain wine with the default `~/.wine` prefix, not `toolchain/wineprefix`. It works but starts a second wineserver.
- About 82 CSV rows drop out of the denominator under `--nolib`: they are mostly the 80 crt/stl/atl `library` rows.

## Structured fields

```json
{
 "files": [
  "tools/msvc9/winewrap.py",
  "util/decomp.h",
  "util/decomp.cpp",
  "util/compat.h",
  ".clang-format",
  ".gitignore",
  "CMakeLists.txt",
  "cmake/reccmp.cmake",
  "reccmp-project.yml",
  "reccmp-user.yml",
  "tools/ida/gen_reccmp_csv.py",
  "tools/reccmp_addr_padding.py",
  "tools/check_folded.py",
  "tools/reccmp-patches/0001-ogre-imports-and-vc9-funcinfo.patch",
  "tools/reccmp-patches/apply.sh",
  "tools/requirements.txt",
  "Client/App/Lua-5.1.4/src/lobject.c",
  "Client/WindowsClient/placeholder.cpp",
  "Client/RobloxStudio/placeholder.cpp",
  "Client/Base/placeholder.cpp",
  "Client/Network/placeholder.cpp",
  "Client/ClientBase/placeholder.cpp",
  "Client/ClientShared/placeholder.cpp",
  "Client/Win/placeholder.cpp",
  "Client/Rendering/AppDraw/placeholder.cpp",
  "Client/Rendering/GfxBase/placeholder.cpp",
  "Client/Rendering/GfxAdapters/placeholder.cpp",
  "Client/Rendering/RbxG3D/placeholder.cpp",
  "Client/Rendering/RbxOgre/placeholder.cpp",
  "Client/Rendering/g3d/placeholder.cpp",
  "reccmp/player-function-size.csv",
  "reccmp/player-imports.csv",
  "reccmp/player-symbols.csv",
  "reccmp/player-crt.csv",
  "reccmp/player-stl.csv",
  "reccmp/player-atl.csv",
  "reccmp/studio-function-size.csv",
  "reccmp/studio-symbols.csv",
  "reccmp/studio-crt.csv",
  "reccmp/studio-stl.csv",
  "reccmp/studio-atl.csv"
 ],
 "unresolved": [
  "Every non-App library's flags are 2016 hypotheses marked '# TODO prove'. That includes /arch:SSE (chosen from the PLAYER-wide census, not per library), /Oy-, /GF, /Gy- and /OPT:ICF. STUDIO's /GL (127 LTCG objects) is not modelled.",
  "Log ships as Log.dll, so it has no target yet and needs its own DLL target and reccmp target. Its __FILE__ paths are absent from notes/paths-2013.txt.",
  "The 2013 __FILE__ path for Lua is lowercase 'client/app/lua-5.1.4/src'. The tree uses 'Client/App/Lua-5.1.4/src' to match the prior App decomp; I did not decide this case question. The placeholder lobject.c will be replaced when Phase 2 step 7 imports that decomp.",
  "STUDIO's report does not ignore the automatic entry-point pairing. Its entry point is a real CRT start in .text, so the pairing may be legitimate, but our placeholder WinMainCRTStartup does not match it yet. I did not check this further.",
  "reccmp runs cvdump.exe under the default ~/.wine prefix. It worked, but the winewrap hang fix does not cover it. If cvdump ever hangs, run reccmp with WINEPREFIX set to toolchain/wineprefix.",
  "Pointers into .text in the original (vtable slots, function-pointer tables) are invisible to reccmp's pointer heuristic because PLAYER has no .reloc. reccmp-vtable and datacmp will need RTTI-based CSV rows (vtable/global types) once classes land.",
  "reccmp's import-thunk scan treats every 'jmp [IAT]' byte pattern as a thunk, including tail jumps inside functions (5,442 in PLAYER after the patch, 9,647 in STUDIO upstream). I did not check whether this misattributes any comparisons.",
  "Unwind/catch funclets: IDA made only 40 of the 19,336 FuncInfo targets into separate functions. The rest sit inside parent function extents, so parent sizes may include funclet code. I did not measure this.",
  "No CI workflow, .clang-tidy, .editorconfig or tools/ncc was written. Those are Phase 2 step 2 items outside this task's list.",
  "OpenRBX is GPLv3, so nothing was copied from it. Licensing this repo is your decision."
 ]
}
```

