/* script/LuaInstanceBridge.h -- October 2013 (Instance bridge head,
   section 258a).
   Evidence: Bridge<shared_ptr<Instance>, false> registerClass 0x1718CE
     (334B: 3-arg index/newindex/gc/tostring + readonly, NO __eq) +
     on_gc 0x171A1C + on_tostring x2 (0x171A5E/0x178706, pushes the
     instance name) + wrappers 0x1A282C/0x1A288C + pushNewObject 0x1A22A4
     (generic-equivalent: 8B shared copy + registry metatable, no new
     overload) + SharedPtrBridge<Instance>::push 0x178748 (weak-table
     reuse shape + lua_isnil assert :179) + className "Object" (0xB250D9,
     landed section 227) (batches 514-516); ObjectBridge::newInstance
     0x173774 + lockInstance 0x173982 (pushes true) + unlockInstance
     0x1739A6 (returns 0) + registerInstanceClassLibrary 0x199134
     ({"new","Lock","Unlock"} table, batch518).
   Queued (258b/c): obj-form on_index 0x1739AE (7.4KB) + on_newindex
     0x1768A4 (7.8KB) + wrappers + openState 0x18B85F wiring (addresses
     taken in-TU -- nothing linkable until the bodies land) + newInstance
     (Limits::Counter + fw+22) + registerInstanceClassLibrary (takes
     newInstance's address).
   Landed since (258b-d): all of the above. Method era (2026-10-17,
     batches 735-737): callMember pair 0x17587C/0x175B4A + the on_index
     method-closure arms (yield first, then function; closure keyed by
     the descriptor address in the environment table; asserts at
     lines 371/399) -- YieldFunctionStateObject's first real callers.
     Still queued: the Parent +476-guard audit.
   Original file: Client/App/script/LuaInstanceBridge.cpp (.h companion;
   2013 path TBD). */

#pragma once

#include "lua/luabridge.h"
#include "V8Tree/Instance.h"

namespace RBX { namespace Lua {

	class ObjectBridge
	{
	public:
		static int newInstance(lua_State* L);
		static int lockInstance(lua_State* L);
		static int unlockInstance(lua_State* L);
		static int callMemberFunction(lua_State* L);
		static int callMemberYieldFunction(lua_State* L);
		static void registerInstanceClassLibrary(lua_State* L);
	};

	template<> void Bridge<boost::shared_ptr<Instance>, false>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<boost::shared_ptr<Instance>, false>::on_index(
		const boost::shared_ptr<Instance>& object,
		const char* name, lua_State* L);
	template<> void Bridge<boost::shared_ptr<Instance>, false>::on_newindex(
		boost::shared_ptr<Instance>& object,
		const char* name, lua_State* L);
	template<> int Bridge<boost::shared_ptr<Instance>, false>::on_index(lua_State* L);
	template<> int Bridge<boost::shared_ptr<Instance>, false>::on_newindex(lua_State* L);
	template<> int Bridge<boost::shared_ptr<Instance>, false>::on_gc(lua_State* L);
	template<> int Bridge<boost::shared_ptr<Instance>, false>::on_tostring(lua_State* L);
	template<> int Bridge<boost::shared_ptr<Instance>, false>::on_tostring(
		const boost::shared_ptr<Instance>& object, lua_State* L);

} }
