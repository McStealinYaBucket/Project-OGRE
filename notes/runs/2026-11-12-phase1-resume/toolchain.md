# toolchain (workflow phase1-resume, 2026-11-12)

The toolchain is built and the PDB gate passes. VC9 SP1 builds under wine 10.0 with a full PDB (functions with section:offset plus line records) using either /Zi or /Z7. The fix that made it work is a 10-line stand-in for Samba's ntlm_auth, so winbind is not needed.

**1. Earlier work checked, and the combined SP1 image was wrong.** The earlier toolchain/vsadmin-sp1 came from one wine `msiexec /a` with both SP1 patches in PATCH=. It returned success, but wine silently dropped every file the VC patch (KB947888) changes or adds: VC/include had 145 of 188 entries, VC/lib 25 of 58, atlmfc/lib 5 of 32, and crtdefs.h and libcmt.lib were missing. Its compiler binaries were correct SP1. Do not use that image or toolchain/vc9.
- Fix: one admin install per patch. vsadmin-vc (RTM + KB947888) contains 1949 of 1959 of that patch's payloads byte-identical. The 10 it lacks are spare copies of localized MFC `.rc` files (`afxres.rc`, `afxribbon.rc`) stored under other keys; the image has these files from their own keys.
- vsadmin-vs (RTM + KB945140) contains every x86 compiler, linker and mspdb payload.
- All payloads in both patches are whole files, not deltas.
- In the files used here the two patches touch disjoint sets: KB945140 changes 13 (cl, c1, c1xx, c2, link, ml, nmake, the .config files, mspdb80, mspdbcore, mspdbsrv), KB947888 changes or adds 913. So the toolset is the VC image with the 13 KB945140 files laid over it. Placement comes entirely from the MSI tables via msiexec; nothing was picked by FL_ name.

**2. Windows SDK v6.0A.** Headers and libs are RTM: no SP1 patch targets that MSI. The SDK tools come from the Win32Tools MSI with KB946729 applied: RC.Exe 6.01.6723.01 (RTM was 6.00.5724.00), mt.exe 5.02.3790.2076 (RTM .2075), and RcDll.Dll changed too (its version resource reads oddly; the SHA256 is in the manifest). KB946733 (.NET tools) and KB950424 (wsatui.dll) do not apply.

**3. toolchain/msvc9: 3605 files, 481 MB, every one listed with version and SHA256 in MANIFEST.txt.** Versions:
- cl.exe, c1.dll, c1xx.dll, c2.dll: 15.00.30729.01
- link.exe, ml.exe, nmake.exe, mspdb80.dll, mspdbcore.dll, mspdbsrv.exe: 9.00.30729.01
- **lib.exe, cvtres.exe and msobj80.dll are 9.00.21022.08 (RTM).** That is correct, not a gap: neither SP1 patch carries a payload for any of them (checked in the extracted cab lists and in the MSP file itself). lib.exe only hands off to link.exe /lib, which is 30729.01.
- File counts: VC/bin 37 (x86 only, plus the 1033 UI DLLs), VC/include 227, VC/lib 58, atlmfc/include 981, atlmfc/lib 32, SDK/Include 1673, SDK/Lib 544, SDK/bin 53.

**4. Wrappers and CMake.**
- Wine drive O: maps to the repo root, so build paths have no spaces.
- The wrappers (tools/msvc9/cl, link, lib, rc, mt, ml) all call winewrap.py. It sets INCLUDE, LIB and WINEPATH, turns absolute Unix paths (including `/Fo…`, `/OUT:…` and `@rsp` files) into O:\ or Z:\, turns them back to Unix paths in the output so Ninja's dependency scan works, passes exit codes through, and starts mspdbsrv detached beforehand.
- Exit codes come through: 2 for a missing source file, 0 on success.
- cmake/msvc9-wine.cmake with CMake 4.2.3 and Ninja: the compiler is identified as "MSVC 15.0.30729.1". The probe project (one OBJECT library and one exe) builds, CMake's own link step runs on Linux, Ninja's dependency records hold Unix paths, and probe.exe runs under wine with exit code 0.
- STL headers, windows.h and atlbase.h (CComBSTR) compile. The link with /MD /FIXED /BASE:0x400000 /OPT:REF /OPT:ICF /INCREMENTAL:NO /MANIFEST:NO works. rc, ml, lib and mt work too, except that rc 6.1 rejects /nologo.

**5. PDB gate.** Without the stand-in, link /DEBUG fails with LNK1101 "incorrect MSPDB80.DLL version" and cl /Zi fails with C1902. Cause: wine reports "ntlm_auth was not found", then "RpcBindingSetAuthInfoExW unsupported AuthnSvc 10", so mspdb80 cannot talk to mspdbsrv.
- Fix: tools/msvc9/ntlm-fake/ntlm_auth answers `--version` with "Version 4.19.5" and is placed first on PATH. Logging showed wine only ever calls it with --version, so no NTLM exchange happens.
- Recipe (a), `/Zi /Fd` with mspdbsrv pre-started (`-start -spawn -shutdowntime -1`) then `link /DEBUG /PDB`: works.
- Recipe (b), `/Z7` then `link /DEBUG /PDB:x.pdb`: works.
- Validation with llvm-pdbutil 21 and with reccmp's cvdump.exe 14.00.23611 run under wine (`reccmp-cvdump -s -l -p`, exit 0): function records `S_GPROC32 [0001:00001A80] ProbeSum`, `[0001:00000810] ProbeFormat` and `main`, line records such as `probelib.cpp 0001:00001A80-00001ABF, 5 pairs`, and `S_COMPILE3` in each module.

**6. reccmp** is installed in toolchain/venv at commit a0f494c0fe4d7fb06e0ae4a7f65f74a628540aec (0.1.9), now pinned in tools/requirements.txt.
- Neither image crashed in its parser (reccmp's `detect_image`) or in its import, import-thunk, pointer, string and FuncInfo scans.
- probe.exe: debug image, PDB path found, 70 import slots, 31 import thunks, 375 pointers, 222 strings.
- PLAYER (bin/player_1023/RobloxPlayerBeta.exe): 0 relocations, 274,328 heuristic pointers, 91,176 string candidates, 1 import thunk, 1 FuncInfo record.
- **reccmp reads PLAYER's imports from the VMProtect loader's table** (2031 slots at 0xef1430–0xef346c in .vmp1), not from the linker's IAT at 0xaa1000–0xaa2eac that .text actually calls through. Calls to imported functions will be left unnamed unless something supplies those names.

## Files

- `tools/msvc9/assemble.sh`
- `tools/msvc9/pever.py`
- `tools/msvc9/winewrap.py`
- `tools/msvc9/cl`
- `tools/msvc9/link`
- `tools/msvc9/lib`
- `tools/msvc9/rc`
- `tools/msvc9/mt`
- `tools/msvc9/ml`
- `tools/msvc9/ntlm-fake/ntlm_auth`
- `tools/msvc9/reccmp_load_check.py`
- `tools/msvc9/probe/CMakeLists.txt`
- `tools/msvc9/probe/probe.cpp`
- `tools/msvc9/probe/probelib.cpp`
- `cmake/msvc9-wine.cmake`
- `tools/requirements.txt`
- `toolchain/msvc9/MANIFEST.txt (gitignored)`
- `toolchain/vsadmin-vc, toolchain/vsadmin-vs, toolchain/sdkadmin-hl, toolchain/sdkadmin-w32, toolchain/sdkadmin-w32-rtm (gitignored admin images)`
- `toolchain/logs/vs_admin_vc_only.log, vs_admin_vs_only.log, sdk_hl.log, sdk_w32.log, sdk_w32_rtm.log`

## Unresolved

- reccmp takes PLAYER's import table from the VMProtect loader's directory in .vmp1 (0xef1430-0xef346c), not the linker IAT at 0xaa1000-0xaa2eac that .text uses. Calls through the real IAT will be left unnamed unless reccmp CSV rows or a small reccmp patch supply those names. Not addressed here.
- reccmp's FuncInfo scan finds only 1 record in PLAYER. I did not check whether its VC9 EH-magic detection misses the rest.
- A full reccmp-reccmp comparison run (project yml, PDB-backed scoring) was not attempted; only image parsing and cvdump on the probe PDB were verified.
- toolchain/vsadmin-sp1 (the bad combined-patch image) and toolchain/vc9 (binaries picked by file name) are still on disk. Both are superseded and should not be used; delete them if the space is wanted.
- rc.exe 6.1 rejects /nologo. Harmless for CMake's default rc command, but a hand-written rc command must leave it out.
- No hotfix builds (.4056 or .4455) were considered. The toolset is plain SP1 .01, as the research notes recommend until a codegen diff points elsewhere.
- Both wine msiexec bugs (multi-patch admin installs dropping files, and the NTLM requirement for mspdbsrv RPC) are measured on wine 10.0 only.
