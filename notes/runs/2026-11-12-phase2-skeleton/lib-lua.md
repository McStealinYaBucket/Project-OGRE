# lib-lua (workflow phase2-skeleton, wf_e3afb0d0-6b4)

The rebuilt 2013 Lua source compiles byte-identical to every Lua function it locates in PLAYER: 514 located, 514 identical, with string literals checked as well as the masked bytes. STUDIO gives the same result (514/514), so both source lines carry the same Lua.

**Source.** I started from lua.org lua-5.1.4.tar.gz; its sha256 b038e225…af3a matches the checksum lua.org publishes. Compiled as C++ with the proven App flags, the untouched source gives 358 of 501 located functions identical. Fifteen small patches, each justified by PLAYER bytes in its header, bring that to 514/514.

**The xmmintrin effect.** The real luaconf.h includes rbx/Debug.h (for RBXASSERT). That pulls in Base/include/RbxPlatform.h, which on WIN32 includes <windows.h> and <intrin.h>, and VC9's intrin.h includes emmintrin.h and xmmintrin.h. Using <intrin.h> or <xmmintrin.h> alone gives the same bytes. The assert sites are confirmed in PLAYER:
- luaconf.h:852 is RBXASSERT(node) in RobloxExtraSpace(RobloxExtraSpace*) at 0x00874f40.
- luaconf.h:862 is RBXASSERT(shared->threadCount >= 0) in ~RobloxExtraSpace at 0x008766a0.

**What 2013 Roblox changed, proven by bytes:**
- **01 intrin.h:** makes the SSE intrinsics header visible, which fixes the static-function register conventions (patch 01-intrin).
- **Table.readonly (02, 03, 07):** the byte sits between flags and lsizenode (sizeof(Table) = 0x24; luaH_new 0x0098c3c0). lua_setreadonly is at 0x00874470. The readonly check is only in lua_rawset 0x008745f0, with "Attempt to modify a readonly table", and in luaV_settable 0x0098a3b0, which calls luaG_runerror with an empty string. lua_rawseti and lua_setmetatable have no check in 2013.
- **luaconf compat switches (04, 05, 06, 13, 15):** LUA_COMPAT_MOD, LSTR, VARARG, GFIND and OPENLIB are all undefined. iOS names the function luaI_openlib.
- **Udata.may_gc (08):** a bool at +0x14. luaS_newudata sets it, newproxy clears it and rejects anything but nil/true ("newproxy only supports the arguments nil and true"), and GCTM skips __gc when it is clear.
- **lua_exception (09):** luaD_throw throws it. The FuncInfo of luaD_rawrunprotected catches lua_exception& and std::exception const&, and the second is turned into luaG_runerror(L, "%s", e.what()). 2016 catches RBX::base_exception instead.
- **RobloxExtraSpace (10):** it is 44 bytes, not 2016's 40, because identity:5 and yieldCaptured:1 are in separate units. lua_resume clears yieldCaptured with `and byte [L-0x10], 0xfe`.
- **FFlag LuaEnableBytecode (11):** registered at 0x00a82e60, default false. f_parser only uses luaU_undump when the flag is set.
- **getfuncname (12):** names "index" for OP_GETTABLE/OP_SELF and "newindex" for OP_SETTABLE.
- **Loaders (14):** luaL_loadprotected 0x00876140 and protReader 0x00875f10 follow the iOS names. loadstring now goes through fromTrustedSource, the hasher is a boost::scoped_ptr, and luaL_loadbuffer is gone from PLAYER.

**Not present in 2013** (the stock 5.1.4 bodies are identical): match-depth limits in lstrlib, tail-call disabling in the parser, the LUA_MASKERROR hook, lua_chk_ptr_rblx checks, collectgarbage restricted to "count", the resume isFirstCall check, LUA_IDSIZE 256, ckey or opcode encoding, and RBX::base_exception.

**The 78 functions the earlier probe could not match** fall into three groups:
- **Modified:** the bodies listed above.
- **Inlined everywhere:** small statics such as getjump, freereg, isnumeral, getcurrenv and the luai_userstate* wrappers. Our .obj also emits an unreferenced copy, which the linker drops.
- **Removed or dead-stripped:** ldblib, liolib, loslib, loadlib and linit are absent, and so are their strings (for example "unable to generate a unique filename"). Stock externs with no caller (lua_equal, lua_cpcall, lua_get/setallocf, lua_tocfunction, luaL_loadbuffer, luaL_loadstring, luaL_typerror, luaL_optnumber, luaL_gsub) are also absent. PLAYER cannot tell "removed from source" apart from "unreferenced and stripped by the linker" for those externs.

**Location.** PLAYER functions were located by unique masked-body search, calls from identical callers, luaL_Reg tables, unique-string xrefs, .obj order and iOS names, with checks that callees and string text agree. 10 of the iOS-transferred Lua names in ida/exports/PLAYER-names.csv are off by one function, for example 0x00875050 is RobloxExtraSpace::constructChild, not lua_newthread. The other functions with no iOS name were located from the bytes.

No canonical .i64 was opened or changed. A 13-row group per target holds RBX inline code that sits inside the lapi/lstate spans; it is flagged library=rbx-in-lua-tu in the CSV.

## Structured fields

```json
{
 "files": [
  "tools/lib/lua_match.py",
  "ida/exports/PLAYER-lib-lua.csv",
  "staging/lua-5.1.4-2013/upstream",
  "staging/lua-5.1.4-2013/patches/01-intrin.patch",
  "staging/lua-5.1.4-2013/patches/02-table-readonly.patch",
  "staging/lua-5.1.4-2013/patches/03-setreadonly.patch",
  "staging/lua-5.1.4-2013/patches/04-no-compat-mod.patch",
  "staging/lua-5.1.4-2013/patches/05-no-compat-lstr.patch",
  "staging/lua-5.1.4-2013/patches/06-no-compat-vararg.patch",
  "staging/lua-5.1.4-2013/patches/07-settable-readonly.patch",
  "staging/lua-5.1.4-2013/patches/08-udata-may-gc.patch",
  "staging/lua-5.1.4-2013/patches/09-lua-exception.patch",
  "staging/lua-5.1.4-2013/patches/10-robloxextraspace.patch",
  "staging/lua-5.1.4-2013/patches/11-luaenablebytecode.patch",
  "staging/lua-5.1.4-2013/patches/12-getfuncname-index.patch",
  "staging/lua-5.1.4-2013/patches/13-no-compat-gfind.patch",
  "staging/lua-5.1.4-2013/patches/14-loadprotected.patch",
  "staging/lua-5.1.4-2013/patches/15-no-compat-openlib.patch",
  "3rdparty/_downloads/lua-5.1.4/lua-5.1.4.tar.gz"
 ],
 "csv": "ida/exports/PLAYER-lib-lua.csv",
 "matched_identical": 514,
 "located_not_identical": 0,
 "flags": "Flags: /TP /O2 /Ob2 /Oi /Oy /GS- /arch:SSE /fp:fast /GR /EHsc /MD /D_SECURE_SCL=0 /DNDEBUG /DWIN32 /D_LIB /D_RELEASE (plus /W0 /D_CRT_SECURE_NO_WARNINGS, which do not change codegen, and /I boost_1_53_0 for boost/scoped_ptr.hpp). This is the App flag set: all 514 PLAYER (and 514 STUDIO) Lua functions are byte-identical under it.\n\nWhich function proves which flag:\n- /Oy: no ebp frame in any of the 514, for example lua_gettop 0x00873b00.\n- /arch:SSE, not SSE2: the x87 fld/fucomip compares in luaO_rawequalObj 0x008a7940 and luaH_getnum 0x0098c480.\n- /fp:fast: math_deg 0x008ad760.\n- /GS-: luaX_next and llex 0x0098e7c0, which have no cookie.\n- /EHsc (C++ EH; asynchronous /EHa not confirmed or ruled out separately): the try/catch FuncInfo in luaD_rawrunprotected 0x008aadd0 and the placement-new EH states in lua_newstate 0x00876a00, RobloxExtraSpace::constructChild 0x00875050, luaL_loadprotected 0x00876140, protReader 0x00875f10 and luaB_loadstring 0x008b0750.\n- /MD: the msvcp90 std::basic_string dllimports (string ctors/dtor/operator!= through IAT 0x00aa12f4..0x00aa1458) in luaL_loadprotected, protReader and luaB_loadstring.\n- /TP (compiled as C++): every symbol is C++-decorated, for example ?lua_gettop@@YAHPAUlua_State@@@Z.\n- /Gy (implied by /O2): one COMDAT per function, so the linker keeps .obj order, which the order step relies on.\n\nInclude-set effect: <intrin.h> must be visible, which the real code gets through rbx/Debug.h -> RbxPlatform.h. Without it, static functions take their first argument in ebx: newlstr 0x0098cd30, fixjump 0x009a6740, patchlistaux 0x009a6910 and about 90 more.",
 "unresolved": [
  "The RBX code compiled into the Lua TUs is not reproduced: RobloxExtraSpace ctor/dtor (0x00874f40, 0x00876970, 0x008766a0), RBX::Intrusive::Set insert/remove/iterator (0x00874cf0, 0x00874d80, 0x008764d0, 0x00876580) and the boost shared_ptr<Shared> helpers (0x008765d0, 0x008765f0, 0x00876680, 0x008767c0, 0x00876910) are listed as library=rbx-in-lua-tu rows with no .obj counterpart. Patch 10 uses a stand-in RobloxExtraSpace (correct 44-byte layout, ctor/dtor only declared) and patch 14 uses stand-in RBX declarations (RbxProtectedString.h); both give identical Lua code, but the real class bodies need the RBX Base/App headers (Intrusive/Set.h, ThreadRef, ProtectedString, MD5Hasher).",
  "FastLog.h in patch 11 is a minimal stand-in. 2013 FASTFLAGVARIABLE has no PlaceFilter_ string (none exists in PLAYER), unlike the FastLog.h in the existing App tree, which is newer than 2013 for this macro.",
  "For dead externs (lua_equal, lua_cpcall, lua_getallocf/setallocf, lua_tocfunction, luaL_loadbuffer, luaL_loadstring, luaL_typerror, luaL_optnumber, luaL_gsub, luaL_addstring) PLAYER cannot tell removal from source apart from /OPT:REF dead-stripping. They are absent in both PLAYER and STUDIO.",
  "ldblib, liolib, loslib, loadlib and linit have no body in PLAYER or STUDIO and their library strings are absent. That fits removal, but whether the .c files were still compiled (for example luaopen_debug left as a stub that was later stripped) cannot be determined.",
  "/EHsc versus /EHa and /GR are only consistent with the bytes, not separately proven. No Lua function has RTTI use or asynchronous-EH-specific code.",
  "10 iOS-transferred Lua names in ida/exports/PLAYER-names.csv are wrong (off by one function). The canonical databases were not touched; the CSV rows carry the corrected names: 0x00875050 constructChild, 0x008a8ab0 addinfo, 0x008a8be0 auxgetinfo, 0x0098bf50 findindex, 0x0098c6d0 unbound_search, 0x0098cd30 newlstr, 0x0098dea0 trydecpoint, 0x0098f130 indexupvalue, 0x00990040 prefixexp, 0x009903a0 simpleexp."
 ]
}
```

