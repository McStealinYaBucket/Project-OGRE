# probe-libs (workflow phase3-measure, wf_271afd88-e3a)

Every remaining engine library now has its flags settled, mostly by byte probes. I compiled 77 probe functions across 16 probe sets (PLAYER, STUDIO and Log.dll) with VC9. All of them are byte-identical under the winning flag set, and each flag is decided by the named functions that stop matching when that one flag is flipped. Re-run them with `python3 -P tools/probe_flags.py` (about 1 minute) and the census with `python3 -P tools/probe_libs.py`.

Five results are surprises:
1. **GfxBase is built `/Od`.** It is unoptimized, with `/Ob2`, `/GS-` and `/arch:SSE`. All 7 probes in each target match and all 7 are lost under `/O2`. The 2016 project leaves Optimization unset, which means Disabled. CMakeLists.txt currently says "unset = /O2", which is wrong.
2. **AppDraw and RbxG3D use `/Oy`, not the 2016 `/Oy-`.** RbxOgre and GfxAdapters also use `/Oy`, so every optimized engine library uses exactly the App set.
3. **The client shells are built differently from the engine.** The PLAYER shell (WindowsClient, which also compiles Win, ClientBase, ClientShared and boost program_options) and the STUDIO shell (RobloxStudio plus the same shared folders) use `/O2 /Oy /GS /fp:precise /GL` with **no `/arch`**: x87 only, `fnstsw` compares, zero SSE instructions. The 2016 projects say `/GS-` and `/arch:SSE2`, and both are wrong for 2013.
4. **`/GL` sits in the client shells, not the engine.** Two whole-range statistics in `tools/probe_libs.py` separate them. `regarg` is the share of functions taking arguments in non-standard registers: 12.9–19.9% in the shells, 0–1.9% in every engine library. `ltcg8` counts aligned frames in functions with no double-precision data: 166 + 30 in PLAYER's shell and 401 in STUDIO's, against 22 in all of App. The Rich header object counts match the project sizes: PLAYER's 40 LTCG objects line up with the 44 files the 2016 WindowsClient project compiles (33 of its own plus 11 program_options), and STUDIO's 127 match RobloxStudio's own `.cpp` and moc files.
5. **Log.dll is also `/GL`, `/GS`, and has no `/arch`.** Its Rich header has exactly 2 LTCG objects, one per source file (FastLog.cpp, FastLogStream.cpp).

For STUDIO, App and Network use the same flags as PLAYER: the earlier Lua (514/514) and RakNet (565/565) STUDIO matches decide it. Every STUDIO probe set also matches with the same set as its PLAYER twin.

**STL:** `_SECURE_SCL=0` is decided by the `Poly::getNumSurfaces` probe (PLAYER 0x00754610, STUDIO 0x008939c0), which stops matching under `_SECURE_SCL=1`. `std::vector` is 0x10 bytes. `std::string` is 0x1c bytes under either setting, as the StatsService constructor's member offsets show. `_invalid_parameter_noinfo` is not imported anywhere, so neither binary uses checked iterators.

**Other definitions:** DeviceCaps (DXUT) has NDEBUG undefined: its `assert()` calls survive as `_wassert`. The STUDIO shell has `QT_NO_DEBUG` undefined: `Q_ASSERT` survives in moc code.

I did not edit CMakeLists.txt or Client/. No IDA database was opened and ida/LOG.md was not touched; scratch work was under /tmp/claude-1000. One note for whoever applies this: because the shells are `/GL`, the client exe link needs `/LTCG`, and their non-trivial functions will only match after a whole-program link. The leaf probes still match from a plain `.obj` because LTCG leaves those bodies unchanged.

## Structured fields

```json
{
 "libraries": [
  {
   "library": "App",
   "target": "PLAYER",
   "flags": "/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_LIB /D_RELEASE",
   "proofs": "Carried over from earlier runs (Lua probe 514/514): luaO_rawequalObj 0x008a7940 and luaH_getnum 0x0098c480 decide /arch:SSE; math_deg 0x008ad760 decides /fp:fast; luaX_next decides /GS-; lua_gettop 0x00873b00 decides /Oy. _SECURE_SCL=0 decided by RBX::Poly::getNumSurfaces 0x00754610 (this run).",
   "address_range": "0x00613530-0x009ad000",
   "census": "19,302 functions. Plain ebp frames 72/10,296. 17 functions read the stack cookie against 8,987 EH frames (unresolved). 14,801 SSE1, 0 SSE2, 51 P6 x87 compares. ltcg8 22, regarg 0.5%."
  },
  {
   "library": "App",
   "target": "STUDIO",
   "flags": "same as PLAYER",
   "proofs": "Lua 514/514 identical in STUDIO with the App flags (earlier lib-lua run). _SECURE_SCL=0 decided by Poly::getNumSurfaces 0x008939c0 (lost under =1).",
   "address_range": "0x006cb000-0x00a62000",
   "census": "18,963 functions; plain frames 74/10,144; 1 cookie vs 8,800 EH frames; 14,817 SSE1, 0 SSE2; regarg 0.3%."
  },
  {
   "library": "Network (+RakNet 4.036)",
   "target": "PLAYER",
   "flags": "/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_LIB /D_MBCS",
   "proofs": "Carried over from the RakNet run (566/567): ReadFloat16 0x005c3910 decides /fp:fast; SystemAddress::operator= 0x005710d0 decides /arch:SSE over SSE2; /Oy, /GS- and /Ob2 are each decided by hundreds of functions.",
   "address_range": "0x0051f6b0-0x006091f8",
   "census": "4,665 functions; plain frames 19/2,327; 0 cookies vs 2,330 EH frames; 401 SSE1, 0 SSE2; regarg 1.1%."
  },
  {
   "library": "Network",
   "target": "STUDIO",
   "flags": "same as PLAYER",
   "proofs": "RakNet 565/565 identical in STUDIO with the same flags (earlier run).",
   "address_range": "0x005b7700-0x0069f000",
   "census": "4,547 functions; 0 cookies vs 2,248 EH frames; 0 SSE2; regarg 0.2%."
  },
  {
   "library": "Base",
   "target": "PLAYER",
   "flags": "/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_LIB",
   "proofs": "Probe 8/8 identical. /Oy: 7 functions lost under /Oy- (e.g. format 0x009b5ba0, ExclusiveArbiter::arbiterName 0x009b3950). /Ob2: Time::nowFastSec 0x009bb4d0 lost under /Ob1 (3 lost under /Ob0). /GS-: c_time::localtime 0x009b1240, Log::formatMem 0x009b0f50, Log::formatTime 0x009b10e0. /arch:SSE: Log::formatTime 0x009b10e0 lost with no /arch; Job::computeStandardError 0x009b98b0 and 0x009b10e0 lost under SSE2. /fp:fast: Log::formatMem 0x009b0f50. /O2: 6 lost under /O1. /Oi: unproven by probe (census: no strlen or strcmp import anywhere). Debugable::doCrash 0x009b1a00 keeps its `#pragma optimize(\"\", off)` frame.",
   "address_range": "0x009b0e00-0x009bd680",
   "census": "274 functions; plain frames 4/164; 0 cookies vs 108 EH frames; 0 SSE1/SSE2, 6 P6 compares; regarg 0.0%."
  },
  {
   "library": "Base",
   "target": "STUDIO",
   "flags": "same as PLAYER",
   "proofs": "Probe 8/8 at 0xa72300, 0xa70870, 0xa68af0, 0xa68dd0, 0xa6ba80, 0xa68330, 0xa68040, 0xa681d0. The same flips lose the same functions (GS: 0xa68330, 0xa68040, 0xa681d0; SSE2: 0xa70870, 0xa681d0; no /arch: 0xa681d0; fp: 0xa68040).",
   "address_range": "0x00a6a000-0x00a77000",
   "census": "238 functions; 4 cookies vs 88 EH frames; regarg 0.4%."
  },
  {
   "library": "AppDraw",
   "target": "PLAYER",
   "flags": "/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_LIB /D_RELEASE (the 2016 /GF /Gy- are unproven)",
   "proofs": "Probe 4/4 identical: CircleRadialNormal::eval 0x009c2900, evalTangent 0x009c2990, evalNormal 0x009c2a10, evalBinormal 0x009c2a90. /Oy (2016 says /Oy-): all 4 lost. /Ob: all 4 lost under /Ob0 (/Ob1 vs /Ob2 not decided). No /arch: 4 lost; SSE2: 3 lost. /fp:fast, /Oi and /O2: 3 lost each. /GS-: census only.",
   "address_range": "0x009c1000-0x009c5e10",
   "census": "34 functions; 0 cookies vs 5 EH frames; 647 SSE1, 0 SSE2."
  },
  {
   "library": "AppDraw",
   "target": "STUDIO",
   "flags": "same as PLAYER",
   "proofs": "Probe 4/4 at 0xa7a0e0, 0xa7a170, 0xa7a1f0, 0xa7a270; identical flip pattern.",
   "address_range": "0x00a79000-0x00a7c000",
   "census": "21 functions; 0 cookies vs 4 EH frames."
  },
  {
   "library": "GfxBase",
   "target": "PLAYER",
   "flags": "/Od /Ob2 /Oi /Oy- /GS- /arch:SSE /fp:fast /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_LIB",
   "proofs": "Probe 7/7. /Od: all 7 lost under /O2 (CRenderSettings::setAutoQualityLevel 0x009c5ed0, Adorn::preSubmitPass 0x009d55f0, AdornBillboarder::getCamera 0x009d5650, IAdornable::shouldRenderSetDirty 0x009ca750, AdornBillboarder::getRenderCaps 0x009d5b10, ViewBase::render 0x009c7c10, AdornBillboarder::drawFont2DImpl 0x009d5720). /GS-: 0x009d5720. /arch:SSE: 0x009c7c10 (lost with no /arch and under SSE2). /Ob2 or /Ob1 (not /Ob0): the inlined runtime_error ctor, STUDIO 0x00aa1ff0. /Oy-, /fp and /Oi have no codegen effect under /Od (the 2016 values are kept).",
   "address_range": "0x009c5e10-0x009d5b70",
   "census": "214 functions, 100% ebp frames, the /Od spill pattern everywhere; 0 cookies vs 56 EH frames."
  },
  {
   "library": "GfxBase",
   "target": "STUDIO",
   "flags": "same as PLAYER",
   "proofs": "Probe 7/7 at 0x6c22a0, 0xaa1ec0, 0xaa1f20, 0x6c6c80, 0xaa23e0, 0x6c3fb0, 0xaa1ff0. /Ob0 loses 0xaa1ff0, /GS loses 0xaa1ff0, SSE2 or no /arch lose 0x6c3fb0, /O2 loses all.",
   "address_range": "0x006c2000-0x006c8000 plus 0x00a99000-0x00aa2400 (FileMeshData, AdornBillboarder)",
   "census": "125 functions, 100% ebp frames."
  },
  {
   "library": "RbxOgre",
   "target": "PLAYER",
   "flags": "/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_LIB",
   "proofs": "Probe 9/9: istreamDataStream read/seek/eof/close/skip/tell 0x0051f270-0x0051f650, Name::doDeclare<sDecal> 0x00480bf0, AdornRbxGfx::getViewport 0x00491700, FrameRateManagerStatsItem::update 0x0048e530. /Oy: 7 lost. /Ob: 3 lost under /Ob0. /GS-: 0x00480bf0. No /arch: 0x00491700. SSE2: 0x0048e530. /fp:fast: 0x00491700. /O2: all lost under /O1. /Oi: census.",
   "address_range": "0x00480700-0x0051f6b0",
   "census": "2,029 functions; plain frames 10/1,246; 0 cookies vs 762 EH frames; 6,571 SSE1, 0 SSE2; regarg 0.7%."
  },
  {
   "library": "RbxOgre",
   "target": "STUDIO",
   "flags": "same as PLAYER",
   "proofs": "Probe 7/7 at 0x5b7570-0x5b7950 and 0x5210b0. /GS- and SSE2: census (the doDeclare and update probes are COMDATs that a Studio-shell TU contributed in STUDIO).",
   "address_range": "0x00514000-0x005b7700",
   "census": "1,933 functions; 0 cookies vs 685 EH frames; 0 SSE2; regarg 0.8%."
  },
  {
   "library": "GfxAdapters",
   "target": "PLAYER",
   "flags": "/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_LIB",
   "proofs": "Probe 2/2, using boost 1.53 unordered_set<GfxPart*> instances this library's TU emitted (no source exists for this Ogre-era library). The table swap 0x0060b930 decides /Ob2 over /Ob1, /Oy, SSE over no /arch, and /O2. The table_impl copy ctor 0x0060cd30 decides /GS-. SSE2 vs SSE and /fp: unproven (census: 0 SSE2).",
   "address_range": "0x00609330-0x00612550",
   "census": "147 functions; plain frames 0/79; 0 cookies vs 68 EH frames; 46 SSE1, 0 SSE2; regarg 0.0%."
  },
  {
   "library": "GfxAdapters",
   "target": "STUDIO",
   "flags": "same as PLAYER",
   "proofs": "Probe 2/2 at 0x6a2530 and 0x6a35e0; same flip pattern.",
   "address_range": "0x006a3000-0x006a9000",
   "census": "82 functions; 0 cookies vs 48 EH frames."
  },
  {
   "library": "RbxG3D",
   "target": "PLAYER",
   "flags": "/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_LIB (the 2016 /Oy- is refuted by census)",
   "proofs": "Unproven by probe; census only. The Frustum ctor 0x009ee010 (the library's only function in PLAYER) has an EH frame without a cookie (/GS-), uses ebp as a general register (/Oy), and uses 70 SSE1 instructions with no SSE2. /Ob, /fp and SSE vs SSE2 are unproven.",
   "address_range": "0x009ee000-0x009ee700",
   "census": "Frustum.cpp only. 0x009ee700-0x009eed6e is an unrelated rdtsc/SEH CPU-timing library."
  },
  {
   "library": "WindowsClient (+Win, ClientBase, ClientShared, boost program_options)",
   "target": "PLAYER",
   "flags": "/O2 /Ob2 /Oi /Oy /GS /fp:precise /GL (no /arch) /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_WINDOWS; the exe links with /LTCG",
   "proofs": "Probe 3/3 (unnamed Windows-only code): 0x00429ce0 (double->unsigned clamp), 0x0040bfa0 (float scale, then clamp), Name::doDeclare<sScript> 0x00413740. /GS: 0x00413740. /fp:precise: 0x0040bfa0. No /arch: 0x00429ce0 and 0x0040bfa0 lost under /arch:SSE. /Oy: all 3. /O2: all 3. /GL: census plus the Rich header (40 LTCG objects \u2248 this project's TU list, which includes program_options, already proven /GL at 0x00478330). /Ob and /Oi: unproven.",
   "address_range": "0x00401000-0x00480700 (own objects 0x401000-0x46c000, /GL code tail 0x46c000-0x480700)",
   "census": "2,596 + 458 functions; plain frames 5/1,517; 1,018 cookies vs 1,004 EH frames; 0 SSE1/SSE2, 36 fnstsw; ltcg8 166 + 30; regarg 12.9% and 19.9%."
  },
  {
   "library": "RobloxStudio (+ClientShared, Win, ClientBase)",
   "target": "STUDIO",
   "flags": "/O2 /Ob2 /Oi /Oy /GS /fp:precise /GL (no /arch) /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32 plus the Qt defines, without QT_NO_DEBUG; the exe links with /LTCG",
   "proofs": "Probe 4/4: moc UpdateUIManager::metaObject 0x0050ec50, qt_metacast 0x0050eca0, qt_metacall 0x0050ed00, and FrameRateManagerStatsItem::update 0x0043e8a0. /Oi: 0x0050eca0 (inline strcmp). /Ob>0: 0x0050ec50. /Oy: 3 lost. /O2: all 4. Shell code at Studio addresses: /GS 0x0047eb90; /fp:precise 0x004780e0; no /arch 0x00486610 and 0x004780e0. /GL: census plus Rich (127 LTCG objects). QT_NO_DEBUG undefined: Q_ASSERT/qt_assert in 0x0050ec50.",
   "address_range": "0x00401000-0x00514000",
   "census": "5,244 functions; plain frames 8/2,657; 2,456 cookies vs 2,450 EH frames; 0 SSE, 69 fnstsw; ltcg8 401; regarg 12.9%."
  },
  {
   "library": "ClientShared",
   "target": "STUDIO",
   "flags": "same as RobloxStudio (compiled inside the RobloxStudio project; 2016 vcxproj lists ..\\ClientShared\\DataModelSerialize.cpp)",
   "proofs": "Region census only (DataModelSerialize.cpp at 0x00411ac0 sits in the /GS, /GL, no-/arch shell range); no dedicated probe.",
   "address_range": "inside 0x00401000-0x00514000"
  },
  {
   "library": "DeviceCaps (DXUT)",
   "target": "PLAYER",
   "flags": "/O2 /Oy /GS (no /arch), NDEBUG not defined; /Ob, /Oi, /fp unproven",
   "proofs": "Census only. The asserts survive as _wassert calls (0x009ad3a0, 0x009ada80), so NDEBUG is undefined. CompareFunction 0x009af740 is DXUT's SortModesCallback (not probed).",
   "address_range": "0x009ad000-0x009b0e00",
   "census": "52 functions; plain frames 1/46; 9 cookies vs 6 EH frames; 0 SSE, 3 fnstsw."
  },
  {
   "library": "Log.dll",
   "target": "PLAYER",
   "flags": "/O2 /Ob2 /Oi /Oy /GS /GL (no /arch) /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32; /fp unproven; link /LTCG",
   "proofs": "Probe 7/7 on exports: FastLog(3 args) 0x10002770, FastLog(5 args) 0x10002790, Init 0x10002920, SetExternalLogFunc 0x10002930, FastLogF 0x10002940, GetFastLogCounter 0x10002e70, NowFast 0x10003370. /Oy: 6 lost. No /arch: FastLogF lost under /arch:SSE. /Ob>0: FastLogF. /O2: 4 lost under /O1. /GS: census (94 cookies vs 86 EH frames). /GL: the Rich header has exactly 2 Utc1500_LTCG_CPP objects (FastLog.cpp, FastLogStream.cpp); regarg 24.5%. /Oi: inline strlen in FastLogS 0x10002b30 (not probed).",
   "address_range": "bin/player/Log.dll 0x10001000-0x10010000 (the bin/studio copy differs in SHA but has the same Rich header)",
   "census": "290 functions; plain frames 1/196; 94 cookies vs 86 EH frames; 0 SSE, 3 fnstsw; ltcg8 22; regarg 24.5%."
  }
 ],
 "stl": "_SECURE_SCL=0. Decided by probe: RBX::Poly::getNumSurfaces() const (PLAYER 0x00754610, STUDIO 0x008939c0) materialises &faces at mesh+0x20 (`add eax,0x20`) and reads _Mylast/_Myfirst at +0x28/+0x24, i.e. vector+8/+4. It is identical under _SECURE_SCL=0 and lost under =1, where VC9 puts the pointers at +0xc/+0x10. Containers (asserted in tools/flagprobes/STL.cpp):\n- sizeof(std::vector<T>) = 0x10: the empty allocator padded to 4 bytes, then _Myfirst/_Mylast/_Myend at +4/+8/+0xc. Under =1 it is 0x18.\n- There is no _Myownedaux (VC9's checked-STL auxiliary pointer). _Myproxy does not exist in VC9; it arrived with VS2010.\n- Vector iterators are bare 4-byte pointers (8 bytes under =1).\n- sizeof(std::string) = 0x1c, identical under either setting: allocator, _Bx[16] at +4, _Mysize at +0x14, _Myres at +0x18. The c_str pattern `cmp [s+0x18],0x10; jb; mov eax,[s+4]` appears at 0x009b29f2. The StatsService ctor 0x0048d510 constructs strings at +0x64 and +0x9c through the imported MSVCP90 default ctor, and the next members sit at +0x80 and +0xb8. Strings are constructed through MSVCP90 imports, so string layout is fixed by the DLL.\n- Neither PLAYER nor STUDIO imports _invalid_parameter_noinfo, so no translation unit in either binary was built with checked iterators.\n\nI did not find a single constructor that initialises both a std::vector and a std::string in one function. The vector proof comes from the getNumSurfaces probe, and the string proof from the StatsService stores.",
 "files": [
  "tools/probe_flags.py",
  "tools/probe_libs.py",
  "tools/flagprobes/probes.txt",
  "tools/flagprobes/GfxBase.cpp",
  "tools/flagprobes/Base.cpp",
  "tools/flagprobes/AppDraw.cpp",
  "tools/flagprobes/RbxOgre.cpp",
  "tools/flagprobes/GfxAdapters.cpp",
  "tools/flagprobes/WindowsClient.cpp",
  "tools/flagprobes/RobloxStudio.cpp",
  "tools/flagprobes/Log.cpp",
  "tools/flagprobes/STL.cpp",
  "notes/flags-proof.md"
 ],
 "unresolved": [
  "/GL in the client shells and Log.dll is decided by the Rich header object counts plus the regarg and ltcg8 census columns, not by a byte probe. A probe would need a whole-program /LTCG link of a test executable; the leaf probes match from plain .obj files because LTCG leaves those bodies unchanged.",
  "WindowsClient /Ob and /Oi: no probe decides them (/Ob0 and /Oi- still give 3/3). /Ob2 /Oi is the 2016 hypothesis; the census (no strlen or strcmp import) supports /Oi.",
  "/fp for GfxAdapters and Log.dll, SSE2 vs SSE for GfxAdapters and RbxG3D, and every RbxG3D and DeviceCaps flag are census-only or unproven. DeviceCaps (DXUT) has no source to probe.",
  "/GF and /Gy- (the 2016 values for AppDraw, RbxG3D and GfxBase) cannot be decided from function bytes; they need a string-pooling or COMDAT-folding test.",
  "PLAYER's App range has 17 functions that read the stack cookie (STUDIO has 1), although App is /GS-. They are probably /GS boost static-library code inside the range that is not yet in lib-resolved, but I did not trace them.",
  "The 2016 CMakeLists comments are wrong in places: GfxBase's 'Optimization unset = /O2' (the bytes show /Od), /Oy- for Base, AppDraw, RbxG3D and G3D (the bytes show /Oy), and the client shells' /GS- and /arch:SSE2 (the bytes show /GS and no /arch). Fixing them is left to the Link agent, which owns CMakeLists.txt.",
  "WindowsClient probes 0x00429ce0, 0x0040bfa0 and 0x00413740 are unnamed (Windows-only code has no iOS or Mac symbols); I cannot tell whether they sit in a WindowsClient, Win or ClientBase file. All three are compiled in one project, so the flags are the same.",
  "STUDIO's RbxOgre /GS- and SSE2 are census-only. The PLAYER doDeclare and update probes are COMDATs that a Studio-shell TU contributed in STUDIO, so they test the shell's flags there instead.",
  "Log.dll: only bin/player/Log.dll was probed. bin/studio/Log.dll has a different SHA256 but the same Rich header, so the same flags are presumed."
 ]
}
```

