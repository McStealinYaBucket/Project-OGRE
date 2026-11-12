#pragma once

#include "Lua.hpp"

#include "util/exception.h"
#include "util/utilities.h"
#include "rbx/Debug.h"
#include "RbxAssert.h"
#include "rbx/atomic.h"

namespace RBX { namespace Lua {

void newweaktable(lua_State *L, const char *mode);

template<class C, int (C::*Func)(lua_State*)>
int memberFunctionProxy(lua_State* thread)
{
	C* c = reinterpret_cast<C*>(lua_touserdata(thread, lua_upvalueindex(1)));
	return (c->*Func)(thread);
}

template<class C, int (C::*Func)(lua_State*)>
void pushMemberFunction(lua_State* L, C* c)
{
	lua_pushlightuserdata(L, (void*)c);
	lua_pushcclosure(L, &memberFunctionProxy<C, Func>, 1);
}

// TODO: Use traits pattern rather than bool __eq
template<class Class, bool __eq = true>
class Bridge
{
public:
	// 0 arg constructor
	static Class* pushNewObject(lua_State *L)
	{
		Class* data = (Class*)lua_newuserdata(L, sizeof(Class));
		new(data) Class;
		luaL_getmetatable(L, className);
		lua_setmetatable(L, -2);
		return data;
	}
	// 1 arg constructor
	// STUB: PLAYER 0x00888b10
	// STUB: STUDIO 0x00928580
	template<typename Param1>
	static Class* pushNewObject(lua_State *L, Param1 param1)
	{
		Class* data = (Class*)lua_newuserdata(L, sizeof(Class));
		new(data) Class(param1);
		luaL_getmetatable(L, className);
		lua_setmetatable(L, -2);
		return data;
	}
	// 2 arg constructor
	template<typename Param1, typename Param2>
	static Class* pushNewObject(lua_State *L, Param1 param1, Param2 param2)
	{
		Class* data = (Class*)lua_newuserdata(L, sizeof(Class));
		new(data) Class(param1, param2);
		luaL_getmetatable(L, className);
		lua_setmetatable(L, -2);
		return data;
	}
	
	// Throws an exception if index doesn't hold the right type
	static Class& getObject(lua_State *L, unsigned int index) {
		void *ud = luaL_checkudata(L, index, className);
		return *reinterpret_cast<Class*>(ud);
	}

	// Returns false if index doesn't hold the right type (leaving value unchanged)
	// STUB: PLAYER 0x008a5b40
	// STUB: STUDIO 0x00929a50
	template<typename V>
	static bool getValue(lua_State *L, unsigned int index, V& value) {

		// A re-implementation of luaL_checkudata that doesn't throw an exception
		void *p = lua_touserdata(L, index);
		if (p != NULL) {  /* value is a userdata? */
			if (lua_getmetatable(L, index)) {  /* does it have a metatable? */
				lua_getfield(L, LUA_REGISTRYINDEX, className);  /* get correct metatable */
				if (lua_rawequal(L, -1, -2)) {  /* does it have the correct mt? */
					lua_pop(L, 2);  /* remove both metatables */
					value = *reinterpret_cast<Class*>(p);
					return true;
				}
				else
					lua_pop(L, 2);  /* remove both metatables */
			}
		}
		return false;
	}

    static void registerClass (lua_State *L);

	// 2013 3-arg form (Bridge<G3D::Vector3,true>::registerClass 0x16D168,
	// batch347; same shape for Vector2 per 0x16D41C): the caller passes the
	// type's on_index/on_newindex lua_CFunctions explicitly. The 2016
	// 1-arg-only header API does NOT match the 2013 binary -- both forms
	// are declared; each Bridge instantiation defines the one the binary
	// shows it defining.
	static void registerClass(lua_State* L, int (*on_indexFn)(lua_State*),
		int (*on_newindexFn)(lua_State*));

/// 2013 DELTA vs the carried 2016 header: NO protected section. The 2016
/// header hides on_index/on_newindex/etc. behind `#ifdef _WIN32
/// protected:`, but 2013's openState (a non-friend, non-member) names
/// Bridge<G3D::Vector3>::on_index/on_newindex directly as registerClass
/// args (0x18B92E/0x18B932 arg loads, batch320) -- they must be public.
/// Binary evidence wins: everything stays public.

	// The following members must be implemented by client:
	static int on_index(const Class& object, const char* name, lua_State *L);
	static void on_newindex(Class& object, const char* name, lua_State *L);

	// This member may be specialized when StringConverter has no implementation for Class:
	static int on_tostring(const Class& object, lua_State *L);
	
	static const char* className;

	static int on_gc(lua_State *L) {
		getObject(L, 1).~Class();
		return 0;
	}

	static int on_tostring(lua_State *L) {
		return on_tostring(getObject(L, 1), L);
	}

	static int on_newindex(lua_State *L) {
		const char* name = lua_checkstring_secure(L, 2);
		on_newindex(getObject(L, 1), name, L);
		return 0;
	}

	static int on_index(lua_State *L) {
		const char* name = lua_checkstring_secure(L, 2);
		return on_index(getObject(L, 1), name, L);
	}

	static int on_eq(lua_State *L) {
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}
};

// This class hides parent on purpose
template<class Class>
class SharedPtrBridge : protected Bridge<boost::shared_ptr<Class>, false>
{
public:
    static void registerClass (lua_State *L)
	{
        Bridge<boost::shared_ptr<Class>, false>::registerClass(L);
	}

	static void registerClassLibrary (lua_State *L) {
		// Declare the UserData re-use table
		lua_pushlightuserdata(L, (void*)&push);
		newweaktable(L, "v");
        lua_rawset(L, LUA_REGISTRYINDEX);
	}

	// STUB: PLAYER 0x006730e0
	// STUB: STUDIO 0x007dafa0
	static void push(lua_State *L, boost::shared_ptr<Class> instance)
	{
		if (instance==NULL)
			lua_pushnil(L);
		else
		{
#ifdef _DEBUG
			int i = lua_gettop(L);
#endif
			// Matt Campbell at Serotek Corporation had a great idea. Create only
			// a single instance of the userdata and re-use it. This way the userdata
			// can be used in table keys, and we don't need an __eq operator
			// Also see http://lua-users.org/lists/lua-l/2004-07/msg00391.html

			lua_pushlightuserdata(L, (void*)&push ); /* Registry mapping for weak table. Key is arbitrary. */
			lua_rawget(L, LUA_REGISTRYINDEX);                             // Stack:   t
			RBXASSERT(!lua_isnil( L, -1 ));  // Did you forget to call registerClassLibrary??

			// Now the top of the stack is our lookup table
			// See if we already have a UserData for this instance
			lua_pushlightuserdata (L, (void*)instance.get());							// Stack:   t, i
			lua_rawget (L, -2);															// Stack:   t, t[i]

			if( lua_isnil( L, -1 ) ) {													// Stack:   t, nil or I
				lua_pop (L, 1);															// Stack:   t
				Bridge<boost::shared_ptr<Class>, false>::pushNewObject(L, instance);	// Stack:   t, I
				/* store the value for later use */
				lua_pushlightuserdata (L, (void*)instance.get());						// Stack:   t, I, i
				lua_pushvalue (L, -2);													// Stack:   t, I, i, I
				lua_rawset (L, -4);														// Stack:   t, I
			}
			lua_remove (L, -2);															// Stack:   I
#ifdef _DEBUG
			RBXASSERT(lua_gettop(L) == i + 1);
#endif
		}
	}

	static boost::shared_ptr<Class> getPtr(lua_State *L, unsigned int index)
	{
		if (lua_isnil(L, index))
			return boost::shared_ptr<Class>();
		else
			return RBX::Lua::Bridge<boost::shared_ptr<Class>, false>::getObject(L, index);
	}

	template<typename V>
	static bool getPtr(lua_State *L, unsigned int index, V& value) {
		if (lua_isnil(L, index))
		{
			value = boost::shared_ptr<Class>();
			return true;
		}
		else
			return Bridge<boost::shared_ptr<Class>, false>::getValue(L, index, value);
	}

};

// This class hides Bridge on purpose. 2013 DELTA vs the carried 2016
// header (which hardcodes false): the bool is a template parameter --
// the binary instantiates SingletonBridge<T, true> three times
// (AllEnumDescriptors/EnumDescriptor/Item; batch416) and the push +
// registerClassLibrary bodies below are transcribed from those
// instantiations (0x1A23E8/0x1A2574/0x1A2700 + 0x198FF8/0x199048/0x199098,
// batch417). pushNewObject goes through Bridge<T, __eq> (same className).
template<class T, bool __eq>
class SingletonBridge : protected Bridge<T, __eq>
{
public:
	static void registerClass (lua_State *L) {
		Bridge<T, __eq>::registerClass(L);
	}

	static void registerClassLibrary (lua_State *L) {
		// Declare the UserData re-use table (NOT weak -- plain
		// createtable(0,0), unlike SharedPtrBridge's "v" table).
		lua_pushlightuserdata(L, (void*)&push);
		lua_newtable(L);
		lua_rawset(L, LUA_REGISTRYINDEX);
	}

	static void push(lua_State *L, T item)
	{
		lua_pushlightuserdata(L, (void*)&push);
		lua_rawget(L, LUA_REGISTRYINDEX);
		// mac: 0x1A2734-gate (FLog::Asserts + debugHook consult the missing
		// table; ReleaseAssert is log-and-continue in this build).
		if (FLog::Asserts != 0
			&& lua_type(L, -1) == LUA_TNIL
			&& (RBX::_internal::_debugHook == 0
				|| RBX::_internal::_debugHook("!lua_isnil( L, -1 )", "include/lua/luabridge.h", 250) == 0))
		{
			ReleaseAssert(FLog::Asserts, "!lua_isnil( L, -1 ) file: include/lua/luabridge.h line: 250");
		}
		lua_pushlightuserdata(L, (void*)item);
		lua_rawget(L, -2);
		if (lua_type(L, -1) == LUA_TNIL) {
			lua_settop(L, -2);
			Bridge<T, __eq>::pushNewObject(L, item);
			lua_pushlightuserdata(L, (void*)item);
			lua_pushvalue(L, -2);
			lua_rawset(L, -4);
		}
		lua_remove(L, -2);
	}
};

} }



