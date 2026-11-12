/* App/include/script/LuaProfiler.h -- October 2013 (TENTH ERA: real-bodies
   landing, 2026-10-18; supersedes the C1 shell + hook-era placeholders).
   Binary shape (batches 772/773/774/775):
   - The class is GLOBAL-namespace `LuaProfiler` (mangles __ZN11LuaProfiler...;
     statics instance @0xFA6420, stringCache @0xFA6424), NOT RBX::Lua:: as the
     C1 shell spelled it. The RBX::Lua:: wrapper is retired on the mangle
     evidence; no other consumer exists besides v8datamodel\ScriptContext.cpp
     (hook dispatch + resume), both updated in the same landing.
   - Methods: ctor 0x1A8172 (assert "instance == NULL" @B2DC75, binary
     ScriptContext.cpp line 321), dtor 0x1A80D4 ("LUAEND" @B2DBE9 + assert
     "instance == this" @B2DBF0, binary line 356), hookCall 0x197D28,
     static getResumePosition 0x1A8416, StringCache::getBegin 0x1A8502,
     StringCache::getCall 0x1A8608, Function::operator< 0x1A8C0A. The
     bodies are defined in v8datamodel\ScriptContext.cpp -- the ctor/dtor
     asserts are compiled in that TU (part-file string @B2BEC3), so the
     method definitions live there too.
   - ROSTER DELTAS retired with this landing: the binary has NO 1-arg
     hookCall, NO hookReturn, NO hookLine and NO int-returning
     getResumePosition(-- the C1/hook-era placeholder members are deleted).
     Consumer map (slot xrefs, batch773): hook dispatch calls ONLY the
     2-arg hookCall (slot 959A70 <- ScriptContext::hook @18B37F); the ctor
     and dtor are called only from ScriptContext::resume @192D02 (C2 slot
     959A76 <- 192F52, D2 slot 959A7C <- 192F79/1930DA); getBegin and
     getResumePosition are ctor-only; getCall is hookCall + the ctor frame
     walk. The hook's RETURN event logs "LUARET" directly from the hook
     body (hook-era landing, batch744), which is why no hookReturn member
     exists.
   - StringCache binary layout: std::map<std::string,std::string> at +0
     (the getBegin cache) and std::map<Function,std::string> at +24 (the
     getCall cache; operator[] targets this+24, batch773 1A8636-63F); the
     dtor erases (this+0, *(void**)this+8) then (this+24, *(void**)this+32)
     @1979D0 (batch772 decompile). Function key = 16 bytes / 4 dwords
     {name, namewhat, source, linedefined} (batch772/773). The static is
     constructed empty by InitFunc_123 @1AA4F0 with both maps' __cxa_atexit
     teardown registration (batch774 decompile) -- reproduced exactly by
     defining the static in the one TU and letting the compiler emit the
     init/teardown pair.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/include/script/LuaProfiler.h */

#pragma once

#include <map>
#include <string>

struct lua_State;
struct lua_Debug;

class LuaProfiler
{
public:
	// Nested per-binary StringCache (__ZN11LuaProfiler11StringCache...).
	// Member order: the string-keyed map at binary +0, the Function-keyed
	// map at binary +24.
	class StringCache
	{
	public:
		// 16-byte key {name, namewhat, source, linedefined} -- the four
		// lua_Debug fields getCall receives: binary reads ar+4/ar+8/
		// ar+0x10/ar+0x1C (batch773 hookCall asm 197D52-77 and the ctor
		// frame walk 1A82D7-82F3) = lua_Debug::name / namewhat / source /
		// linedefined in the standard 5.1 layout that the ported
		// Lua-5.1.4\src\lua.h carries.
		struct Function
		{
			const char* name;
			const char* namewhat;
			const char* source;
			int line;

			// mac: 0x1A8C0A (batch775 asm 1A8C0A-8C3C): ascending UNSIGNED
			// pointer order on the first three dwords (setb at 1A8C29);
			// SIGNED on line (setl at 1A8C37).
			bool operator<(const Function& rhs) const;
		};

		// mac: 0x1A8502: cached = beginMap[position]; on empty cached =
		// "LUABEG" (@B2DD34) + position; returns the cached buffer.
		const char* getBegin(const std::string& position);

		// mac: 0x1A8608: the Function-keyed cache. op = "get " (@B2DD0D) when
		// namewhat=="index" (@B2DD07), "set " (@B2DD1B) when
		// namewhat=="newindex" (@B2DD12), else "" (also for
		// namewhat==NULL, batch773 1A8653-868B); text =
		// RBX::format("LUACALL%s(%d): %s%s" @B2DD20, source?source:"?",
		// line, op, name?name:"?") -- vararg order restored from the
		// batch773 push order (1A86A0-D2); returns the cached buffer.
		const char* getCall(const char* name, const char* namewhat,
			const char* source, int line);

		// Defined out-of-line (empty; the binary's StringCache Cx/Dx
		// symbols carry only the map setup/teardown, D2 @1979D0).
		StringCache();
		~StringCache();

	private:
		std::map<std::string, std::string> beginMap;	// binary +0
		std::map<Function, std::string> callMap;		// binary +24
	};

	// Zero-init statics; definitions live in v8datamodel\ScriptContext.cpp
	// (one TU -- the ctor/dtor asserts are compiled there per
	// the part-file string @B2BEC3; binary statics 0xFA6420 / 0xFA6424).
	static LuaProfiler* instance;				// 0xFA6420
	static StringCache stringCache;				// 0xFA6424

	// mac: 0x1A8172. The sole consumer is ScriptContext::resume (C2 slot 959A76
	// <- 192F52, batch774): gate = FLog::LuaProfiler on + no nested
	// construction, then resume runs INSIDE this scope so the hook events
	// see instance != NULL.
	LuaProfiler(lua_State* L, int level);

	// mac: 0x1A80D4: "LUAEND" + "instance == this" (binary line 356) +
	// instance = NULL. Scope exit of the resume construction block.
	~LuaProfiler();

	// mac: 0x197D28, 2-arg member (mangle __ZN11LuaProfiler8hookCallEP9lua_State
	// P9lua_Debug; call shape ScriptContext::hook -> slot 959A70 @18B37F).
	// No 1-arg hookCall / hookReturn / hookLine members exist in the
	// binary roster (batch772a); the RETURN event logs "LUARET" straight
	// from the hook body.
	void hookCall(lua_State* L, lua_Debug* ar);

	// mac: 0x1A8416. STATIC member: the ctor calls it with an (out, L, level)
	// stack and no this (batch773 1A81FF-820C). Reads byte L+6, dword
	// L+8 and a 12-byte record stride against the 2013 Lua fork's
	// private thread structures (offsets verbatim; loud DELTA-lite note
	// in the definition -- no App-side header names those fields).
	static std::string getResumePosition(lua_State* L, int level);

private:
	LuaProfiler(const LuaProfiler&);
	LuaProfiler& operator=(const LuaProfiler&);
};
