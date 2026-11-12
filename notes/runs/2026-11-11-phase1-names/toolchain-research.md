# toolchain-research (workflow phase1-names, 2026-11-11)

TOOLCHAIN AND RECCMP RESEARCH. Nothing was written to the repo and no compiler was downloaded. Scratch only: reccmp master tarball, two comp-id tables, and a throwaway wine prefix.

## What the user must supply

1. **A VC9 SP1 x86 toolset whose `cl` banner reads 15.00.30729.01.** Least effort: `VCForPython27.msi` ("Microsoft Visual C++ Compiler for Python 2.7", 82.83 MB, SHA1 7800d037ba962f288f9b952001106d35ef57befe per the legacyupdate.net Download Center archive; Microsoft's own link is gone).
   - Unpack with `wine msiexec /a VCForPython27.msi /qb TARGETDIR=...`; no install is needed.
   - It is a per-user MSI with cl/link/lib, CRT and STL headers and libs, and a Windows SDK 7.0 subset. It has no ATL/MFC.
   - That it carries exactly 15.00.30729.01 is my recollection, not something a fetched source stated. Check the banner first.
2. **If the MSI falls short** (missing `ml.exe`/`nmake`, or ATL is needed): the Windows SDK for Windows 7 and .NET 3.5 SP1 (v7.0) ISO, or VS2008 Professional with SP1.
   - `RobloxProxy.dll` and `NPRobloxProxy.dll` will need ATL, which means VS2008 Professional SP1; Express has no ATL/MFC.
   - For the two main exes, `strings` found no `atlbase`/`afx`/`atlmfc` paths in `bin/player_1023/RobloxPlayerBeta.exe` or `bin/studio/RobloxStudioBeta.exe`, and the Player import list has no `mfc90.dll`.
3. **Optional:** a late-2004 DirectX 9.0c SDK, only if we later want FLIRT signatures for the static D3DX code (see section 3).
4. **`sudo apt install winbind`** (and optionally `winetricks`, `msitools`). None of these is installed.

## 1. Which distributions carry which compiler

| Build | Where it ships | Source |
|---|---|---|
| 15.00.21022.08 (RTM) | VS2008 RTM, all editions; Windows SDK for Server 2008 and .NET 3.5 (v6.1) | richprint table "MSVS2008 (9.0.21022.8 RTM)"; a forum post reports the same banner before and after installing SDK v6.1 |
| 15.00.30729.01 (SP1) | VS2008 SP1, including Express with SP1; Windows SDK for Windows 7 and .NET 3.5 SP1 (v7.0) | richprint "MSVS2008 SP1 (9.0.30729.1 SP)"; search result quoting "SP1 version (15.00.30729.01) is also available in Microsoft Windows SDK for Windows 7" |
| 15.00.30729.4056 | Hotfix KB967211: `Cl.exe`/`C2.dll` 15.0.30729.4056, `Link.exe` 9.0.30729.4056 (fixes `/fp:except` and `/arch:SSE2` problems) | support.microsoft.com/kb/967211 |
| 15.00.30729.4455 | Hotfix KB976656: `C1.dll`/`C1xx.dll` only (C1859 PCH fix) | support.microsoft.com/kb/976656 |
| 15.00.30729.207 | WDK 7.x, from memory only | not confirmed by any source this session |

- **The Rich header cannot separate these.** It stores only the build number 30729, so .01, .207, .4056 and .4455 look identical. Start with .01 and revisit only if a codegen diff points at the back end.
- **The shipped CRT is RTM.** `bin/player/Microsoft.VC90.CRT/msvcr90.dll` and its manifest are 9.0.21022.8, although the compiler is SP1. This is the SP1 default when `_BIND_TO_CURRENT_CRT_VERSION` is not defined. The Rich header agrees: Implib900 build 30729 x55 and build 21022 x2.

**How other projects package the compiler**
- **isledecomp/isle:** portable MSVC 4.2 repo (itsmattkc/msvc420), `VCVARS32.BAT`, CMake with NMake Makefiles or Ninja, wine or Docker on Linux. Its README warns about spaces in paths.
- **OpenRBX:** portable Asphaltian/MSVC800-SP1, `Common7\Tools\vsvars32.bat`, `cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo`, on Windows.
- **decomp.me** (decompme/compilers `values.yaml`): pulls portable git repos such as widberg/msvc8.0 and Drahsid/ffxiv_msvc8.0sp1, laid out as `bin` / `include` / `PlatformSDK`. The entries I saw stop at msvc8.0sp1; I did not see an msvc9 entry.
- **Ryzom Core** (wiki.ryzom.dev/nel/vs2008_linux_wine; `tool/wine_vs2008/` in ryzomcore has `toolchain.cmake`, `winecl-cc`, `-link`, `-lib`, `-rc`, `-mt`, `-env`): the only VS2008 + wine + CMake/Ninja recipe I found. It uses cl 15.00.30729.01.

## 2. reccmp with no relocations and MSVC 9

Read from reccmp master source (the docs do not cover this).

**Where relocations are used**
- `reccmp/formats/pe.py` `relocations` / `_relocated_addrs`: with no `.reloc` directory both sets come back empty. It does not crash.
- `reccmp/compare/functions.py` `create_valid_addr_lookup`: an immediate operand becomes an address placeholder if it is a relocation target. Otherwise it must lie inside the image and intersect a known entity (`db.intersects`). Displacements only need to lie inside the image.
- `reccmp/analysis/pointers.py` `find_pointers`: "If the image has a relocation table, use it... If not, search the image for pointers in any section that point to addresses in a data section." This heuristic scan feeds string and float discovery.
- `reccmp/compare/analyze.py` `create_seh_entities`: its docstring says it exists so images without a relocation table still get EH handler and FuncInfo addresses replaced.

**Concrete risk**
- In the original, a `push imm32` or `mov reg, imm32` that points at a global or function reccmp has no entity for stays a raw number. The recompile side gets a placeholder or name, so a correct function can score below 100%.
- The pointer scan is heuristic. It can produce false string entities, and it also walks `.vmp0`, which is VMProtect's code+data section.
- The import directory sits in `.vmp1`; KERNEL32 appears four times in our import list. reccmp's import-thunk naming depends on that table parsing correctly. I did not test it.

**Workaround** (not documented upstream; it follows from the code)
- Give reccmp an entity for every referenced address: generate `GLOBAL`/function/string rows in `reccmp/player-*.csv` from the IDA export, which is already the plan.
- Link the recompile `/FIXED /BASE:0x400000` so both sides behave the same.
- I did not find a GitHub issue about relocation-less images.

**MSVC 9 support**
- reccmp's README says old MSVC "like 4.20" is supported and newer versions are "in progress".
- The bundled `cvdump.exe` is 14.00.23611, which should read VC9 PDBs; this was not run on one.
- Issue #85 (still open) shows unknown type leaves from newer compilers are now logged, not fatal; `datacmp` was the weak spot.
- reccmp needs Python 3.11 or later; the host has 3.14.4.

## 3. Rich header product ids

Sources: dishather/richprint `comp_id.txt`, skochinsky's rich.py gist (`PRODID_MAP`), kirschju/richheader `prodids.py`.

| Id (build) | Name | Table entry |
|---|---|---|
| 0x0096 (20413) | prodidAliasObj900 | The id is listed; build 20413 is in no table |
| 0x0004 (8447) | prodidLinker600 | `000420ff [LNK] VC++ 6.0 SP5 imp/exp build 8447` |
| 0x009c (30319) | prodidImplib1000 | `009c766f [IMP] VS2010 build 30319` |
| 0x0019 (9210) | prodidImplib700 | `001923fa [IMP] VS2002 build 9210 (Import library)` |
| 0x001c (9178) | prodidUtc13_C | `001c23da [ C ] VS2002 XP SP1 DDK build 9178` |
| 0x008a | Utc1500_LTCG_CPP | confirmed |
| 0x0089 | Utc1500_LTCG_C | confirmed |
| 0x005f / 0x0060 | Utc1310_C / Utc1310_CPP | confirmed |

- **0x0096 build 20413** shows up in every VC9 binary here (the exe, `boost.dll`, `Log.dll`, `fmodex.dll`), which fits CRT alias objects from `msvcrt.lib`. That is an inference.
- **13.10.4035** is labelled "Windows Server 2003 SP1 DDK build 4035" by richprint (entries 005f0fc3, 00600fc3, 000f0fc3), with a `(*)` marker on each.
- **The 146 and 118 C++ objects are most likely static D3DX9**, though I did not attribute individual objects.
  - `RenderSystem_Direct3D9.dll` contains "Microsoft (R) D3DX9 Shader Compiler 5.04.00.3900", "D3DXInitialize() must be called first" and the "D3DX: (WARN) Overloaded ::new and ::delete" texts.
  - `bin/player_1023/RobloxPlayerBeta.exe` contains the same WARN block, "D3DX Effect Compiler" and "DisableD3DXPSGP".
  - So both link a static `d3dx9.lib` from a DirectX 9.0c SDK that predates the `d3dx9_NN.dll` era (February 2005 onward, from memory). Mapping 5.04.00.3900 to an exact SDK release (October or December 2004) is unconfirmed.

## 4. What the 2016 tree offers for the 2013 toolchain

Nothing was copied. There is no VC9 compiler, CRT or `msvcr90` anywhere under `ref/src2016/extracted/robloxsrc`.

- **`en_visual_studio_professional_2012_x86_dvd_2262334.iso`** (1.5 GB): VS2012 (v110) only. Not usable for matching.
- **`Trunk2012/Contribs/SDK`** (1.8 GB): a third-party repack (README dated 2020/2021) of a Windows 8.1-era SDK (`ntverp.h` `VER_PRODUCTBUILD` 9600) plus DirectX bits.
  - `d3dx9.lib` there is an 89 KB import library for `d3dx9_35.dll` (dated July 2007); headers say `D3DX_SDK_VERSION` 43. It is not the static 2004 library the 2013 build used.
  - It has `wmvcore.lib` and `wmsdk*.h` (WMF SDK headers), `dinput8.lib`, `dxguid.lib` and `DxErr*.lib`. These are good enough to compile and link a recompile, but they are not evidence about 2013.
- **`Contribs/DSBaseClasses`**: DirectShow base-class sources with `strmbase.lib`.
- **Other Contribs**, all too new for 2013: boost 1.56 and 1.84, Qt 4.8.5 `win_VS2012`, curl 7.50.2, zlib 1.2.8, SDL2, openssl-master, libwebm, `VMProtectWin` and `VMProtectWin_2.13` (SDK headers and libs, plus a `RobloxPlayer.exe.vmp` project file that may list the protected functions).
- **VC9-era leftovers in `Build`**:
  - 12 VS2008-format `.vcproj` files (Version="9.00") under `Build/Installer/` (RobloxProxy, nproblox, Bootstrapper*).
  - `PropertySheets/Common.vsprops` (`/MP`).
  - `RobloxProxy` Release there reads Optimization=2, RuntimeLibrary=2 (`/MD`), DebugInformationFormat=3 (`/Zi`), OPT:REF and ICF. These are real VC9 flag hints for the two proxy DLLs.
- **Flag hypotheses from the 2016 `.vcxproj` files**: `App.vcxproj` and `WindowsClient.vcxproj` say `/fp:fast`, `/arch:SSE2`, `OmitFramePointers=false`; `WindowsClient` says `FixedBaseAddress=true` and `RandomizedBaseAddress=false`. The last pair matches the 2013 PE (no `.reloc`, base 0x400000).

## The wine and CMake approach to plan for

**Measured on this host**
- wine-10.0 (Ubuntu), `wine64` package only, but it has `i386-windows` WoW64 support.
- A 32-bit console exe runs: reccmp's `cvdump.exe` printed its banner in a scratch prefix.
- cmake, ninja, cabextract and 7z are present. winbind, winetricks and msitools are not.
- No `c1xx.dll` turned up in a shallow search of the home directory.

**Plan**
1. Keep the portable toolset gitignored, e.g. `toolchain/msvc9/{VC/bin,VC/include,VC/lib,WinSDK}`, with `mspdb80.dll`, `mspdbcore.dll`, `msobj80.dll` and `mspdbsrv.exe` beside `cl.exe` (or on `WINEPATH`).
2. Use a dedicated `WINEPREFIX`, `WINEDEBUG=-all`, and set `INCLUDE`/`LIB` from the shell. No vcvars is needed.
3. Write wrapper scripts under `tools/` (cl, link, lib, rc) that translate absolute Unix paths to Windows form, because MSVC reads a leading `/` as a switch. Model them on ryzomcore `tool/wine_vs2008`.
4. Write a CMake toolchain file: `CMAKE_SYSTEM_NAME Windows`, compilers set to the wrappers, `-G Ninja`, find-root modes NEVER, and per-library OBJECT libraries with their own flags.
5. Strip `/MANIFEST:EMBED` or pass `/MANIFEST:NO`; CMake 3.30+ handles pre-VS2010 linkers itself. Avoid `/Fe:` with a colon, and delete stale `.pch` files after header edits.
6. Build from a path with no space. Symlink the repo, since "Project OGRE" has one.

**First gate, before any source work: can VC9 under wine produce a PDB?** reccmp cannot work without one.
- Ryzom reports `/Zi` hanging on `mspdbsrv` and `link /DEBUG` failing with LNK1101 under wine. Their workaround is `/DEBUG:NONE`, which we cannot use.
- msvc-wine documents that `mspdbsrv` communication needs winbind, and that otherwise C1902 appears.
- Probe: one hello.cpp, with winbind installed. Try `cl /Z7` plus `link /DEBUG /PDB:x.pdb /FIXED /BASE:0x400000 /OPT:REF /OPT:ICF /INCREMENTAL:NO`, and also `/Zi` with a pre-started `wine mspdbsrv.exe -start -spawn -shutdowntime -1`. Then run reccmp's cvdump on the result.
- If neither yields a PDB, compile under wine and link on a Windows VM or machine.

Sources:
- https://github.com/dishather/richprint/blob/master/comp_id.txt
- https://gist.github.com/skochinsky/07c8e95e33d9429d81a75622b5d24c8b
- https://github.com/kirschju/richheader
- https://github.com/isledecomp/reccmp (and issue #85)
- https://github.com/isledecomp/isle
- https://github.com/Asphaltian/OpenRBX
- https://github.com/decompme/compilers
- https://wiki.ryzom.dev/nel/vs2008_linux_wine
- https://github.com/mstorsjo/msvc-wine
- https://support.microsoft.com/kb/967211
- https://support.microsoft.com/kb/976656
- https://legacyupdate.net/download-center/download/44266/microsoft-visual-c-compiler-for-python-2.7

## Files



## Unresolved

- Whether VC9 cl/link under wine 10 on this host can emit a PDB (/Zi via mspdbsrv, or /Z7 + link /DEBUG). Not testable without the compiler; this is the first probe to run once the user supplies it, and winbind must be installed first.
- That VCForPython27.msi contains exactly cl 15.00.30729.01, and whether it includes ml.exe/nmake, is from memory and not confirmed by a fetched source. Check the cl banner after unpacking.
- The 15.00.30729.207 (WDK 7.x) attribution is unconfirmed. Whether Roblox's build machine had hotfix .4056 or .4455 cannot be read from the Rich header, which stores only 30729.
- Rich id 0x0096 build 20413: the id is AliasObj900, but that build number is in no comp-id table. Attributing it to the VC9 CRT import libs is an inference from its presence in every VC9 binary here.
- Attributing the 146 (exe) and 118 (RenderSystem_Direct3D9.dll) Utc1310 C++ objects to static d3dx9.lib rests on string evidence only; no per-object attribution was done. The exact DirectX SDK release for D3DX shader compiler 5.04.00.3900 (October vs December 2004) is unconfirmed.
- reccmp was not run against the VMProtect-wrapped PLAYER. Unknown: whether its import parser copes with the import directory in .vmp1, how noisy the no-reloc pointer scan is over .vmp0, and whether the bundled cvdump 14.00.23611 parses a VC9 PDB cleanly.
- No matching-decomp project that publicly packages MSVC 9 (15.00.30729) was found. The closest are decomp.me's msvc8.0sp1 entries and Ryzom's VS2008 wine toolchain.
- Whether the 2013 Player needs ATL headers is not settled. No ATL/MFC path strings were found in either exe, but that is weak evidence. RobloxProxy.dll and NPRobloxProxy.dll will need ATL from VS2008 Professional SP1.
