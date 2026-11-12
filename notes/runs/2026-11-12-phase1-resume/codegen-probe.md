# codegen-probe (workflow phase1-resume, 2026-11-12)

Yes, the probe is finished. Compiled with VC9 SP1 (15.00.30729.01) through tools/msvc9/cl, all 52 probed Lua functions in PLAYER come out byte-identical after masking. The flags that do it are `/O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_LIB /D_RELEASE /TP`. **Two of the 2016 App.vcxproj Release flags are wrong for 2013: the binary needs /Oy (no frame pointer), not /Oy-, and /arch:SSE, not /arch:SSE2.** The other hypothesis flags held. A wider scan matched 298 of 376 functions (12 bytes or more) from 17 Lua TUs to a PLAYER function. Reproduce with `python3 -P tools/msvc9/lua_probe.py` (prints "52/52 identical").

**How the functions were found (PLAYER Oct 23).** Nothing in PLAYER was named, so each function was located by its strings, its callees and its place in the source file, then checked against its iOS 10-29 symbol:
- **lobject.c** sits at 0x8a78c0-0x8a7f00. Anchors: luaO_str2d at 0x8a79a0 imports strtod, strtoul and isspace, as on iOS; luaO_pushvfstring at 0x8a7aa0 uses "(null)"; luaO_chunkid at 0x8a7db0 uses `[string "`.
- **lapi.c** sits at 0x873950-0x874d80. Anchors: lua_replace at 0x873c00 uses "no calling environment"; lua_typename at 0x873d10 uses "no value" and the type-name table at 0xbb09f4.
- **ltable.c** sits at 0x98be80-0x98cc70. Anchors: findindex at 0x98bf50 uses "invalid key to 'next'"; luaH_set at 0x98c610 uses "table index is NaN" and "table index is nil".
- **lmem.c:** luaM_toobig at 0x98d9e0 uses "memory allocation error: block too big".
- **lcode.c:** luaK_checkstack at 0x9a6a20 uses "function or expression too complex"; luaK_code at 0x9a6e90 uses "code size overflow".
- **Cross-check:** across all 52 functions, every relocation in our objects resolves to exactly one PLAYER address. Every string relocation lands on the string found independently by text search. luaO_nilobject is at 0xb84c50.
- **Data checks:** the log_2 table at 0xb84c60 reads 0,1,2,2,3,3,3,3,4…. The type names read nil, boolean, userdata, number, string, table, function, userdata, thread, proto, upval, which is stock order. So 2013 has no enum or field shuffling.

| Function | PLAYER | Evidence | Size (bytes) | Identical? | Under the 2016 hypothesis (/Oy-, /arch:SSE2) |
|---|---|---|---|---|---|
| luaO_int2fb | 0x008a78c0 | iOS __Z11luaO_int2fbj, leaf; first in the lobject run; constants 0x10 and 8 | 46 | Yes | push ebp/mov ebp frame added (48 B) |
| luaO_fb2int | 0x008a78f0 | iOS leaf; (x>>3)&31 and (x&7)+8 shift | 24 | Yes | frame added (27 B) |
| luaO_log2 | 0x008a7910 | reads the log_2 table at 0xb84c60 | 39 | Yes | frame added |
| luaO_rawequalObj | 0x008a7940 | switch on type 0..3; called by luaH_get and lua_rawequal | 69 + jump table | Yes | /arch:SSE2 gives movsd/ucomisd where the target has fld/fucomip; with no /arch it gives fcomp/fnstsw |
| luaO_str2d | 0x008a79a0 | strtod, strtoul and isspace imports | 164 | Yes | frame added |
| lua_gettop | 0x00873b00 | lapi order; (top-base)>>4; 98 callers | 14 | Yes | frame added |
| lua_settop | 0x00873b10 | lapi order, stock body | 75 | Yes | frame added |
| lua_type | 0x00873cf0 | calls index2adr at 0x873950; compares with luaO_nilobject; returns -1 | 28 | Yes | frame added |
| lua_typename | 0x00873d10 | "no value"; luaT_typenames | 23 | Yes | frame added |
| luaH_getnum | 0x0098c480 | called by luaH_get; nilobject | 155 | Yes | /arch:SSE2 changes the double compare |
| luaH_getstr | 0x0098c520 | called by luaH_get | 56 | Yes | frame added |
| luaH_get | 0x0098c560 | calls luaH_getnum, luaH_getstr, mainposition and luaO_rawequalObj (same callee set as iOS) | 165 | Yes | SSE2 differs |
| luaS_newlstr | 0x0098cdd0 | called by pushstr (lobject); calls newlstr at 0x98cd30, which calls luaM_toobig, luaM_realloc_, memcpy and luaS_resize | 229 | Yes, with the xmmintrin fix below | 228 B: static newlstr takes L in ebx |
| luaZ_fill | 0x0098d320 | lzio run; luaZ_openspace (0x98d480) calls luaM_toobig and luaM_realloc_ as on iOS | 60 | Yes | frame added |
| luaK_checkstack | 0x009a6a20 | "function or expression too complex" | 56 | Yes | frame added |
| luaK_code | 0x009a6e90 | "code size overflow" twice; calls patchlistaux and luaM_growaux_ | 155 | Yes, once the 2016 wrapper is removed | different argument order and loads |

Also identical with the final flags: luaO_log2's neighbours pushstr (0x8a7a50) and index2adr (0x873950); lua_xmove, lua_setlevel, lua_atpanic, lua_rawequal, lua_pushnil, lua_replace, lua_tonumber, lua_tointeger, lua_pushnumber and lua_pushinteger; hashnum, mainposition, findindex, luaH_next, computesizes, setnodevector, unbound_search, luaH_getn and luaH_set; luaS_resize and newlstr; luaZ_lookahead, luaZ_init, luaZ_read and luaZ_openspace; luaK_reserveregs, luaK_codeABC, luaK_codeABx, luaK_jump, luaK_ret, luaK_nil, constfolding, fixjump and patchlistaux. For the remaining lcode, lgc, llex, lparser and lundump functions found in the wider scan, see `tools/msvc9/lua_probe.py`.

**One flag at a time** (out of 52 functions; everything else held at the final flags):

| Change | Identical | What breaks |
|---|---|---|
| /Oy- | 3/52 | frame pointer everywhere |
| /arch:SSE2 | 41/52 | the 11 functions that use doubles get SSE2 instructions |
| no /arch | 45/52 | x87 compares use fcomp/fnstsw instead of fucomip |
| /Ob1 | 41/52 | inlining |
| /O1, or /O2 /Os | 6/52 | size optimisation |
| /Oi- | 45/52 | intrinsics |
| /fp:precise | 52/52 here | the wider 17-TU scan separates them: math_deg (0x8ad760) is fmul by 57.29577951 (constant at 0xb85ae0) only under /fp:fast, and math_random also matches only under fast |
| /GS | 52/52 here | the wider scan has 8 functions (luaX_next, luaY_parser, body and others) that match only under /GS-, and none the other way |
| /Gy | 52/52 | no codegen effect |

`tools/arch_census.py` checks /arch across the whole executable, not just Lua. PLAYER `.text` (0x401000-0x9f0000) has **0** scalar-double SSE2 instructions, 24,013 scalar SSE1 and 88 P6 x87 compares. STUDIO has 1 SSE2 instruction against 23,967 SSE1. So /arch:SSE was the build setting for the engine as a whole.

**Where the differences came from the source, not the flags.** The "2013" tree's Lua is the 2016 hardened fork. The probe needed four scratch-only changes; each one is tied to PLAYER bytes:
1. **RBX_STUDIO_BUILD, stubbed luaconf includes:** this turns off LUAVM_SECURE, the shuffled fields, LuaSecureDouble, the lua_unlock text-address check and the string-hash check. PLAYER's lua_settop and lua_xmove have no hook, its struct offsets are stock (top +8, base +0xc, l_G +0x10, nCcalls +0x34), and its doubles are read plainly.
2. **2016 `LuaVMValue<T>` wrapper removed** from lobject.h and lstate.h. Without this, luaH_next differs only in SIB operand order and luaK_code in argument scheduling.
3. **2016 `ckey` field removed** from global_State. PLAYER reads panic at G+0x58; the tree puts it at +0x5c (lua_atpanic).
4. **`<xmmintrin.h>` included.** Without it, VC9 /arch:SSE passes the first argument of static functions in ebx (newlstr, luaS_newlstr, fixjump and patchlistaux differ). The real luaconf.h pulls in RBX headers that presumably bring in xmmintrin (2016's LuaVM.h includes emmintrin), and that stops ebx being used. I did not trace which 2013 header does it.

The 78 functions that did not match in the wider scan were not examined one by one. They include functions confirmed removed (lua_equal, lua_cpcall, lua_getallocf, lua_setallocf), small statics inlined in PLAYER (getjump, freereg, isnumeral) and 2016-modified bodies (luaO_pushvfstring, lua_tolstringsecure, much of lparser).

**Codegen facts proven by these matches:**
- Without /GL, VC9 already gives static functions custom register calling conventions: pushstr takes L in esi and the string in edx; index2adr takes ecx and eax. The Lua code is not LTCG: lua_gettop (14 B) is called, not inlined, from 98 sites, and luaM_toobig (15 B) from 23.
- Loop-alignment padding (`lea ecx,[ecx]`, `mov edi,edi`) and jump tables placed inside the function both reproduce.
- IDA's function size stops before an in-function jump table, so `tools/probe_compare.py` compares up to the .obj length.

**Tools written.** `tools/probe_compare.py` extracts a function from a COFF .obj, masks every relocated byte on both sides, prints where each masked symbol lands in PLAYER, and shows a masked unified diff. `tools/msvc9/lua_probe.py` rebuilds the scratch tree with the four changes and scores the 52 functions; switches cover each change, and --flags sets the compiler flags. `tools/arch_census.py` is the /arch census above.

No canonical database was changed; I worked on a scratch copy of ida/PLAYER.i64 at /tmp/claude-1000/ogre-18Ms, closed it without saving and deleted it. ida/LOG.md was not touched.

## Files

- `tools/probe_compare.py`
- `tools/msvc9/lua_probe.py`
- `tools/arch_census.py`

## Unresolved

- The tools/msvc9/winewrap.py wrapper hangs when wineserver is not already running. My first cl call sat for over 300 s with cl.exe defunct, because the wineserver it spawns inherits subprocess.run's stdout pipe. Once a wineserver was up, compiles took about 0.2 s. Starting `wineserver -p` detached beforehand, or having the wrapper do so, should fix it; I did not edit the wrapper.
- Which 2013 RBX header supplies xmmintrin (or another __m128 / align(16) declaration) and so stops ebx being used for arguments is inferred, not traced: <xmmintrin.h> reproduces the effect, and a bare align(16) struct declaration does not. Each App TU will need the real include set for its static-function calling conventions to match.
- The Lua source the probe compiles is the 2016 fork with four probe changes (RBX_STUDIO_BUILD, LuaVMValue removed, ckey removed, a stub luaconf). The real 2013 Lua source still has to be written back. lua_probe.py records the differences it measured, but 78 of the 376 scanned functions were not explained one by one (removed APIs, inlined statics, 2016-modified bodies such as luaO_pushvfstring and much of lparser/llex/lundump).
- /fp:fast and /GS- are proven by only a few functions each (math_deg and math_random; 8 /GS- functions in llex and lparser). The 52-function set alone cannot separate them.
- /EHsc versus /EHa, /GR, /MD and /Zi produced no difference in the functions probed. They need functions with EH state or RTTI to decide, so the probe neither confirms nor contradicts them.
- STUDIO's census shows 1 scalar-SSE2 instruction. I did not locate it; it is probably library code or a disassembly artefact.
- The probe was not run on STUDIO or PLAYER_1015. Their Lua addresses are unmapped.
