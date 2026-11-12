# Per-library compiler flags, 2026-11-12 (Phase 3 steps 2 and 3)

Reproduce: `python3 -P tools/probe_flags.py` (byte probes, `tools/flagprobes/*.cpp`, list in `tools/flagprobes/probes.txt`)
and `python3 -P tools/probe_libs.py` (census). "probe" = a VC9 compile is byte-identical (relocations masked) and
flipping that one flag loses the named function. "census" = whole-range statistic only.

Common to every row: VC9 SP1 15.00.30729.01, `/GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32`.

| Library (target) | Flags | Deciding functions |
|---|---|---|
| App, Network (PLAYER and STUDIO) | `/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast` | earlier runs: Lua 514/514 and RakNet 565-566 in both targets |
| Base (both) | `/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast` | /Oy, /O2: 0x009b5ba0, 0x009b3950 / /Ob2: 0x009bb4d0 (lost under /Ob1 and /Ob0) / /GS-: 0x009b1240, 0x009b0f50, 0x009b10e0 / /arch:SSE: 0x009b10e0 (no /arch), 0x009b98b0 and 0x009b10e0 (SSE2) / /fp:fast: 0x009b0f50 / /Oi: census (no strlen/strcmp import anywhere) |
| AppDraw (both) | `/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast` | 0x009c2900, 0x009c2990, 0x009c2a10 decide /Oy, /Ob, /arch (none and SSE2), /fp, /Oi, /O2 / /GS-: census only (0 cookies, 5 EH frames) / /Ob1 vs /Ob2 and /GF, /Gy-: unproven |
| GfxBase (both) | `/Od /Ob2 /Oi /Oy- /GS- /arch:SSE /fp:fast` | **/Od**: all 7 probes (0x009c5ed0 ...) / /Ob2 (or /Ob1, not /Ob0): STUDIO 0x00aa1ff0 / /GS-: 0x009d5720 / /arch:SSE: 0x009c7c10 / /Oy-, /fp, /Oi: no codegen effect under /Od |
| RbxOgre (both) | `/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast` | /Oy, /Ob: 0x0051f270 / /GS-: 0x00480bf0 / /arch:SSE: 0x00491700 (none), 0x0048e530 (SSE2) / /fp:fast: 0x00491700 / /Oi: census |
| GfxAdapters (both) | `/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast` | 0x0060b930 (/Ob2 over /Ob1, /Oy, SSE over none), 0x0060cd30 (/GS-) / SSE2 and /fp: census only |
| RbxG3D (both) | `/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast` | census only (Frustum ctor 0x009ee010: EH frame, no cookie, no ebp frame, SSE1 floats) |
| WindowsClient + Win + ClientBase + ClientShared (PLAYER shell) | `/O2 /Oi /Oy /GS /fp:precise /GL`, **no /arch** | /GS: 0x00413740 / /fp:precise: 0x0040bfa0 / no /arch: 0x00429ce0, 0x0040bfa0 / /Oy: all 3 / /GL: Rich 40 LTCG objects = this project's TUs (incl. program_options, 0x00478330) and census (regarg 12.9-19.9%, ltcg8 166+30) / /Ob: unproven |
| RobloxStudio + ClientShared + Win + ClientBase (STUDIO shell) | `/O2 /Ob2 /Oi /Oy /GS /fp:precise /GL`, **no /arch** | /Oi: 0x0050eca0 (inline strcmp) / /Ob>0: 0x0050ec50 / /Oy: 0x0050eca0, 0x0050ed00, 0x0043e8a0 / /GS: 0x0047eb90 / /fp:precise: 0x004780e0 / no /arch: 0x00486610 / /GL: Rich 127 LTCG objects, census (regarg 12.9%, ltcg8 401) / Qt `QT_NO_DEBUG` not defined (Q_ASSERT in 0x0050ec50) |
| DeviceCaps (PLAYER 0x009ad000-0x009b0e00, DXUT) | `/O2 /Oy /GS`, no /arch, **NDEBUG not defined** | census only: cookies 9 > EH 6, x87 fnstsw only, `_wassert` calls (0x009ad3a0) |
| Log.dll (bin/player, bin/studio) | `/O2 /Ob2 /Oi /Oy /GS /fp:? /GL`, **no /arch** | /Oy, /O2: 0x10002770 ... / /Ob>0 and no /arch: 0x10002940 / /GS: census (94 cookies over 86 EH frames) / /GL: Rich header has exactly 2 `Utc1500_LTCG_CPP` objects (FastLog.cpp, FastLogStream.cpp), regarg 24.5% / /fp: unproven |

STL (PLAYER 0x00754610 and STUDIO 0x008939c0, `RBX::Poly::getNumSurfaces`): `_SECURE_SCL=0` (lost under 1).
`sizeof(std::vector<T>)` = 0x10 (allocator byte padded, then `_Myfirst`/`_Mylast`/`_Myend` at +4/+8/+0xc), no
`_Myownedaux`; vector iterators are bare pointers (4 bytes). `sizeof(std::string)` = 0x1c (`_Bx` +4, `_Mysize` +0x14,
`_Myres` +0x18), the same under both settings; `RBX::Stats::StatsService::StatsService` 0x0048d510 places strings at
+0x64 and +0x9c with the next members at +0x80 and +0xb8. `_invalid_parameter_noinfo` is not imported by
PLAYER or STUDIO, so no TU in either binary uses checked iterators. VC9 has no `_Myproxy` (VS2010).
