/* lua/lua.hpp -- October 2013 reconstruction.
   The Lua C API in one include, plus RBX::Lua's small helpers.
   iOS 10-29 has RBX::Lua::safe_lua_tostring, throwable_lua_tostring,
   lua_tofloat and protect_metatable, each (lua_State*, int).
   Dropped as 2016-only: lua_checkstring_secure and lua_resetstack (no iOS
   symbol; LuaAtomicClasses.cpp and LuaInstanceBridge.cpp note the 2013 code
   paths that do without them). ScopedPopper and ScopedState are inline-only
   (no symbol either way) and nothing in this tree uses them; kept for the
   prior tree's API. */
#pragma once

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

namespace RBX {
namespace Lua {

	extern const char* safe_lua_tostring(lua_State* L, int idx);
	extern const char* throwable_lua_tostring(lua_State* L, int idx);
	extern float lua_tofloat(lua_State* L, int idx);
	extern void protect_metatable(lua_State* L, int idx);

	// std::string push, length taken from the string (embedded NULs survive)
	inline void lua_pushstring(lua_State* L, const std::string& str)
	{
		lua_pushlstring(L, str.c_str(), str.size());
	}

	// Pops n slots when the scope ends; n can be adjusted on the way.
	class ScopedPopper
	{
		int n;
		lua_State* const L;

	public:
		ScopedPopper(lua_State* state, int count) : L(state), n(count) {}
		~ScopedPopper() { lua_pop(L, n); }

		ScopedPopper& operator+=(int k) { n += k; return *this; }
		ScopedPopper& operator-=(int k) { n -= k; return *this; }
	};

	// A fresh lua_State that closes itself.
	class ScopedState
	{
		lua_State* const L;

	public:
		ScopedState() : L(luaL_newstate()) {}
		~ScopedState() { lua_close(L); }

		operator lua_State*() { return L; }
	};

} // namespace Lua
} // namespace RBX
