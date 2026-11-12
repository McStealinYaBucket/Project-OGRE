# flirt-sigs (workflow phase1-resume, 2026-11-12)

The VC9 SP1 FLIRT signatures are built and measured, and `tools/ida/apply_flirt.py` is ready for a later stage to run on the canonical databases. Every measurement used scratch copies. I did not open, change or snapshot any `ida/*.i64`, so I did not add to `ida/LOG.md`.

**Sigs built** (`tools/ida/build_flirt.py`, which runs pcf and sigmake from `~/ida-pro-9.4/tools/flair`). Two builds in a row gave byte-identical sigs:
- `vc9sp1-msvcrt.sig` (129 patterns): the static part of `msvcrt.lib`. 1720 of its 1806 members are import stubs, which pcf skips.
- `vc9sp1-msvcprt.sig` (8 patterns).
- `vc9sp1-comsuppw.sig` (26 patterns). I chose `comsuppw` over `comsupp`: the two are byte-identical and differ only in how `wchar_t` is mangled, and both targets import native-`wchar_t` names (`basic_string@_W…` from MSVCP90).
- `vc9sp1-atls.sig` (1286 patterns).

**Left out, on measurement:**
- `delayimp.lib`: neither target has a delay-import directory (data directory 13 is 0).
- `uuid`, `strmiids`, `dxguid`: data only; pcf finds no code.
- All 28 SDK v6.0A static libs that contain code: 26 hit nothing in PLAYER or STUDIO. The only hits were false positives on trivial Roblox accessors: `clfsmgmt` `CClfsLogCcb::GetManagementContext` at PLAYER `0x004b0330`, and `format.lib` COR_ILMETHOD setters at PLAYER `0x005c30c0`/`0x00745a20` and STUDIO `0x0065e3d0`/`0x008a8950`.

**Collisions, each settled in the build script** (the build fails on any collision group its decision table does not cover):
- `___setargv` over `___wsetargv`, because both targets import `__getmainargs` and `_acmdln`, not the wide versions.
- `_WinMainCRTStartup` over the other three startups, because both targets are subsystem 2 (GUI) and ANSI.
- All 14 atls groups are excluded. Each is a byte-identical alias (`CSimpleArray<G/K/…>` members, `AtlAdd`/`AtlMultiply<I/K>`, `CA2WEX`/`CW2AEX` destructors, `__AllocStdCallThunk` vs `ATL::__AllocStdCallThunk`, `CComBSTR` vs `stringdispid` destructors), and nothing in either binary picks one.
- One pattern is dropped before sigmake: atls's `??0CTraceSnapshot@@QAE@XZ`, a bare 24-byte zero-fill. It matched PLAYER `0x0042d480`, which is really a Roblox-compiled copy of `CComCriticalSection::CComCriticalSection` (no `mov edi,edi` hotpatch prefix, memset inlined), called from `??0_ATL_BASE_MODULE70`.

**What `apply_flirt.py` does:**
1. Applies the four sigs in the order msvcrt, msvcprt, comsuppw, atls.
2. **Referenced-name check.** FLIRT ignores a module's `^off name` references when it applies, so a hit is undone when a direct call/jmp or an import call at such an offset reaches a named function or import with a different name. Dummy names, `unknown_libname` and import thunks do not count against a hit. The check also allows for IDA's spellings of C import names (one leading underscore dropped, `@N` removed, `_N` added).
3. **Duplicate check.** If one name lands on several hits in the same segment, all of them are undone, since they cannot be told apart.
4. **Data names, then a second pass.** Accepted hits pass their referenced global names to the unnamed globals they use: `___security_cookie`, vftables, SEH tables. Functions still unnamed are then tried again. PLAYER has no load-config directory, so the cookie was never named, and `@__security_check_cookie@4` (PLAYER `0x009ef8ec`) only matches after this step. Its sig carries `REF ___security_cookie`.

Every hit, with its evidence, goes to `<db dir>/logs/<TARGET>-flirt.tsv`. With the `probe` argument the script also tries each sig on functions that are already named, and checks itself on a control sample: 0 of 237 (PLAYER) and 0 of 211 (STUDIO) unmatched functions probed as matches.

**PLAYER (scratch copy):**
- 97 functions accepted: msvcrt 42 in `.text` plus 15 in `.vmp0`, msvcprt 6, comsuppw 7, atls 27. Four of them are `unknown_libname`.
- 26 globals named.
- 3 rejected:
  - `0x009ef047` was matched as `??_G_Fac_node`, but it calls `??1CWin32Heap`, so it is really `??_GCWin32Heap`.
  - `0x00e32dd9` and `0x00e33bde`, two `??_G_Fac_node` hits in `.vmp0` whose callees are unnamed, so they cannot be verified.
- `.vmp0` (`0x00e32000`–`0x00e3a000`) holds a second copy of the CRT, starting with `__DllMainCRTStartup@12`, so a separate CRT-linked image appears to sit inside VMProtect's section. Those hits are marked `seg=.vmp0`.
- 10 already-named functions probe as VC9 matches, all under the same names. One of them, `??4_Init_locks` at `0x009ef59a`, is confirmed by PLAYER's own export table: the export directory near `0x00d018f0` lists RVA `0x5ef59a` under that name.

**STUDIO (scratch copy):**
- 26 accepted: atls 23, msvcrt 2, msvcprt 1. 5 globals named.
- 1 rejected: `0x00a7cf4b`, the same `CWin32Heap` case.
- comsuppw: 0 hits, consistent with STUDIO having no `_com_error` RTTI.
- IDA's `vc32rtf` had already named STUDIO's CRT. 45 named functions probe as VC9 matches, all under the same names.

**False-positive checks:** no accepted hit is a vftable slot of a non-library RTTI class or references an assert/`__FILE__` string, in either target.

**Spot check:** about 45 PLAYER hits read by disassembly, all consistent:
- Vftable stores agree with the names (`CWin32Heap` → `off_BC3540`, `CAtlStringMgr` → `off_BC3554`, `_com_error` → `off_BC35B4`).
- Call graphs agree (`__ftol2_sse` has 133 callers, `__allmul` 44, `__get_sse2_info` is called by `___sse2_available_init`, `__ValidateImageBase` by `__IsNonwritableInCurrentImage`).
- The startup chain runs `_WinMainCRTStartup` → `___security_init_cookie` / `___tmainCRTStartup`.

The scratch database copies are deleted. The per-hit tables are in `ida/logs/PLAYER-flirt-scratch-measure.tsv` and `ida/logs/STUDIO-flirt-scratch-measure.tsv`.

## Files

- `tools/ida/build_flirt.py`
- `tools/ida/apply_flirt.py`
- `toolchain/sig/vc9sp1-msvcrt.sig (+ .pat/.exc/.log, gitignored, rebuilt by build_flirt.py)`
- `toolchain/sig/vc9sp1-msvcprt.sig (+ .pat/.log)`
- `toolchain/sig/vc9sp1-comsuppw.sig (+ .pat/.log)`
- `toolchain/sig/vc9sp1-atls.sig (+ .pat/.exc/.log)`
- `ida/logs/PLAYER-flirt-scratch-measure.tsv`
- `ida/logs/STUDIO-flirt-scratch-measure.tsv`

## Unresolved

- Not applied to the canonical databases. Next stage: snapshot ida/PLAYER.i64 / STUDIO.i64 (and PLAYER_1015), run tools/ida/apply_flirt.py (command in its docstring), then add an ida/LOG.md entry.
- The 14 excluded atls collision groups (byte-identical aliases) stay unnamed. In PLAYER this leaves AtlAdd/AtlMultiply (sub_445900, sub_4309B0, unknown_libname_3) and the CSimpleArray members (unknown_libname_1/4/5) without names.
- Roblox-compiled COMDAT copies of ATL inlines are not covered by atls.lib: PLAYER 0x0042d480 is CComCriticalSection::CComCriticalSection and 0x0041ba30 looks like ATL::AtlThrowImpl (called from AtlThrowLastWin32 at 0x0041b040), but both are unnamed. They need a sig built from our own /O2 compiles without /hotpatch.
- PLAYER .vmp0 holds a second CRT, apparently a separate CRT-linked image (contains __DllMainCRTStartup@12). 15 hits there are accepted and marked seg=.vmp0. What that image is was not examined.
- The two ??_G_Fac_node hits in .vmp0 (0x00e32dd9, 0x00e33bde) are rejected as unverifiable; what they really are is unknown.
- PLAYER_1015 was not measured.
- FLAIR signatures for the third-party libraries (boost, G3D, Lua, RakNet, D3DX 13.10) are out of scope and not built.
