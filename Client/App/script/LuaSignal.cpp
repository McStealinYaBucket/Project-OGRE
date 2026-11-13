/* script/LuaSignal.cpp -- October 2013 (first import, section 243).
   Evidence: see include/script/LuaSignal.h header block.
   pushNewObject note (0x17F394 + 0x1788D6): the binary passes a pointer
   and copies payload bytes with manual ref bumps; the by-value
   pushNewObject(L, x) used below copy-constructs through the same
   weak-ptr machinery (copy ctor bumps identically) -- byte-identical
   results, so no overload is added (same finding as the 0x1648FC/
   0x631826 analysis in section 241).
   EventInstance on_gc note (0x172118): destroys only the weak_ptr member
   (+4/+8), leaving the event pointer alone. Spelled as instance.reset()
   (releases the weak ref identically); the userdata block itself is
   Lua-owned past __gc either way.
   EventInstance on_index x2 + wrappers + registerClass body +
     EventBridge::connect/wait landed in section 253.
   Original file: Client/App/script/LuaSignal.cpp. */

#include "stdafx.h"

#include "script/LuaSignal.h"
#include "Reflection/Event.h"
#include "V8Tree/Instance.h"
#include "Util/Name.h"
#include "util/exception.h"
#include "security/SecurityContext.h"
#include "util/standardout.h"
#include "RbxAssert.h"
#include "luaconf.h"

#include <cstring>

namespace RBX { namespace Lua {

	template<>
	// GLOBAL: PLAYER 0x00d8ee3c
	// GLOBAL: STUDIO 0x00f2b634
	// Bridge<rbx::signals::connection,1>::className
	const char* Bridge<rbx::signals::connection, true>::className = "RBXScriptConnection";

	// mac: 0x1980D0 (96B-pattern): luaL_checklstring wrapper verbatim.
	// FUNCTION: PLAYER 0x00671640
	// FUNCTION: STUDIO 0x007d95a0
	template<>
	int Bridge<rbx::signals::connection, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}

	// Standard luaL_checklstring wrappers (0x19800E/0x19806E banked).
	// FUNCTION: PLAYER 0x006717c0
	// FUNCTION: STUDIO 0x007d9720
	template<>
	int Bridge<EventInstance, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}

	// mac: 0x17F1E6 (204B, batch441): "Signal " + the descriptor's name.
	// FUNCTION: PLAYER 0x0087d470
	// FUNCTION: STUDIO 0x009e1af0
	template<>
	int Bridge<EventInstance, true>::on_tostring(const EventInstance& object,
		lua_State* L)
	{
		std::string s = "Signal ";
		s += object.event->name->str;
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	// mac: 0x17F180 (102B, batch442): always throws, with the name.
	template<>
	void Bridge<rbx::signals::connection, true>::on_newindex(rbx::signals::connection& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x17F148 (56B, batch444): method-call form of disconnect.
	// FUNCTION: PLAYER 0x0087d550
	// FUNCTION: STUDIO 0x009e1bd0
	int SignalConnectionBridge::disconnect(lua_State* L)
	{
		Bridge<rbx::signals::connection>::getObject(L, 1).disconnect();
		return 0;
	}


	// mac: 0x171C26 (64B, batch442): releases the slot weak ref (the member
	// destructor takes the identical path).
	template<>
	int Bridge<rbx::signals::connection, true>::on_gc(lua_State* L)
	{
		getObject(L, 1).~connection();
		return 0;
	}

	// mac: 0x171C66 (110B, batch442): connection::operator==.
	template<>
	int Bridge<rbx::signals::connection, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x17F06E (217B, batch442): disconnect closure / connected bool.
	// FUNCTION: PLAYER 0x0087d7b0
	// FUNCTION: STUDIO 0x009e1e30
	template<>
	int Bridge<rbx::signals::connection, true>::on_index(const rbx::signals::connection& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "disconnect") == 0)
		{
			lua_pushcfunction(L, SignalConnectionBridge::disconnect);
			return 1;
		}
		if (strcmp(name, "connected") == 0)
		{
			lua_pushboolean(L, object.connected());
			return 1;
		}
		throw RBX::runtime_error("%s is not a valid member", name);
	}
	static bool g_allowSignalConnect = false; // see the note at its old place below
	// mac: 0x17EA64 (788B, batch437): wrap the Lua fn at index 2, connect it
	// through the descriptor, push the connection. Shared-ownership is
	// RAII here; the binary's manual addref/release dance is observably
	// identical. The +12 descriptor virtual is read as isScriptable()
	// (the only true-defaulted gate that admits connections); the flag
	// is g_allowSignalConnect (BSS-false proceeds; audit queued).
	// STUB: PLAYER 0x0087e930
	// STUB: STUDIO 0x009e2fb0
	int EventBridge::connect(lua_State* L)
	{
		const EventInstance& instance =
			Bridge<EventInstance>::getObject(L, 1);
		boost::shared_ptr<Instance> object = instance.instance.lock();
		if (object.get() != NULL
			&& instance.event->isScriptable()
			&& !g_allowSignalConnect)
		{
			if (instance.event->security != 0)
			{
				RBX::Security::Context::current().requirePermission(
					instance.event->security, NULL);
			}
			FunctionScriptSlot slot(L, 2);
			shared_ptr<Reflection::GenericSlotWrapper> wrapper =
				Reflection::GenericSlotWrapper::create(slot);
			rbx::signals::connection result =
				instance.event->connectGeneric(object.get(), wrapper);
			// The live slot is the WRAPPER's copy (vptr + slot at +4):
			// it owns its connection for self-disconnect. (The stack
			// slot dies at scope end.)
			static_cast<Reflection::TGenericSlotWrapper<FunctionScriptSlot>*>(
				wrapper.get())->slot.connection = result;
			Bridge<rbx::signals::connection>::pushNewObject(L, result);
			return 1;
		}
		rbx::signals::connection empty;
		Bridge<rbx::signals::connection>::pushNewObject(L, empty);
		return 1;
	}

	template<>
	// GLOBAL: PLAYER 0x00d8ee38
	// GLOBAL: STUDIO 0x00f2b630
	// Bridge<RBX::Lua::EventInstance,1>::className
	const char* Bridge<EventInstance, true>::className = "RBXScriptSignal";

	// mac: 0x172942 (157B, batch441): same event pointer + same live instance.
	// Promotion MUST NOT throw: the binary promotes with the non-throwing
	// acquire (null weak_ptrs promote to null and compare false). The
	// 1-arg shared_ptr(weak_ptr) ctor throws bad_weak_ptr on empty --
	// lock() takes the identical non-throwing path.
	bool EventInstance::operator==(const EventInstance& other) const
	{
		if (event != other.event)
			return false;
		boost::shared_ptr<Instance> a = instance.lock();
		if (a.get() == NULL)
			return false;
		boost::shared_ptr<Instance> b = other.instance.lock();
		return b.get() != NULL && a == b;
	}

	// mac: 0x172118 (80B, batch441): releases the instance weak ref only.
	template<>
	int Bridge<EventInstance, true>::on_gc(lua_State* L)
	{
		getObject(L, 1).instance.reset();
		return 0;
	}

	// mac: 0x172168 (110B, batch441): EventInstance::operator==.
	template<>
	int Bridge<EventInstance, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x17ED78 (655B, batch437): one-shot wait slot + yield. The
	// yieldCaptured tripwire mirrors queueWaiter (path LuaSignalBridge.cpp
	// line 276, inferred-carry from the :29 precedent).
	// STUB: PLAYER 0x0087ebd0
	// STUB: STUDIO 0x009e3250
	int EventBridge::wait(lua_State* L)
	{
		const EventInstance& instance =
			Bridge<EventInstance>::getObject(L, 1);
		boost::shared_ptr<Instance> object = instance.instance.lock();
		if (object.get() != NULL
			&& instance.event->isScriptable())
		{
			if (instance.event->security != 0)
			{
				RBX::Security::Context::current().requirePermission(
					instance.event->security, NULL);
			}
			WaitScriptSlot slot(L);
			shared_ptr<Reflection::GenericSlotWrapper> wrapper =
				Reflection::GenericSlotWrapper::create(slot);
			rbx::signals::connection keepAlive =
				instance.event->connectGeneric(object.get(), wrapper);
			static_cast<Reflection::TGenericSlotWrapper<WaitScriptSlot>*>(
				wrapper.get())->slot.connection = keepAlive;
		}
		if (FLog::Asserts != 0 && RobloxExtraSpace::get(L)->yieldCaptured)
		{
			if (RBX::_internal::_debugHook == 0
				|| RBX::_internal::_debugHook("!RobloxExtraSpace::get(L)->yieldCaptured",
					"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaSignalBridge.cpp", 276) == 0)
			{
				ReleaseAssert(FLog::Asserts,
					"!RobloxExtraSpace::get(L)->yieldCaptured file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaSignalBridge.cpp line: 276");
			}
		}
		RobloxExtraSpace::get(L)->yieldCaptured = true;
		return lua_yield(L, 0);
	}

	// mac: 0x17E872 (498B, batch437): connect/connectFirst/connectLast/wait
	// closures (+ deprecation paths); disconnect + others throw.
	// FUNCTION: PLAYER 0x0087ee40
	// FUNCTION: STUDIO 0x009e34c0
	template<>
	int Bridge<EventInstance, true>::on_index(const EventInstance& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "connect") == 0)
		{
			lua_pushcfunction(L, EventBridge::connect);
			return 1;
		}
		if (strcmp(name, "connectFirst") == 0)
		{
			RBX::Security::Context::current().requirePermission(
				RBX::Security::RobloxPlace, NULL);
			StandardOut::singleton()->print(MESSAGE_WARNING,
				"connectFirst is deprecated");
			lua_pushcfunction(L, EventBridge::connect);
			return 1;
		}
		if (strcmp(name, "connectLast") == 0)
		{
			RBX::Security::Context::current().requirePermission(
				RBX::Security::RobloxPlace, "connectLast");
			StandardOut::singleton()->print(MESSAGE_WARNING,
				"connectLast is deprecated");
			lua_pushcfunction(L, EventBridge::connect);
			return 1;
		}
		if (strcmp(name, "wait") == 0)
		{
			lua_pushcfunction(L, EventBridge::wait);
			return 1;
		}
		if (strcmp(name, "disconnect") == 0)
			throw RBX::runtime_error(
				"Event:disconnect() has been deprecated. Use connection object returned by connect()");
		throw RBX::runtime_error("%s is not a valid member", name);
	}

	// mac: 0x17F008 (102B, batch441): always throws, with the name.
	template<>
	void Bridge<EventInstance, true>::on_newindex(EventInstance& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// Connect-gate flag (connect 0x17EB01-0x17EB0D, batch512): the binary
	// reads a BYTE from a thread-indexed table (index = [[[L-0x28]+12]+4],
	// base unresolvable statically) -- NOT a plain bool. Modeled as a
	// BSS-false bool (any index reads 0 in an all-zero table, so false
	// proceeds identically); the table's identity stays queued with the
	// flag-ident audit. No setter is banked DB-wide.
	// (g_allowSignalConnect is defined above EventBridge::connect, its first user.)

	// mac: 0x171CD4 (62B, batch442): checkudata + forward.
	// FUNCTION: PLAYER 0x0087f9d0
	// FUNCTION: STUDIO 0x009e4050
	template<>
	int Bridge<rbx::signals::connection, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x1721D6 (62B, batch441): checkudata + forward.
	// FUNCTION: PLAYER 0x0087fae0
	// FUNCTION: STUDIO 0x009e4160
	template<>
	int Bridge<EventInstance, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x171A9C (394B, batch442): index/newindex/gc/eq/tostring + readonly.
	// FUNCTION: PLAYER 0x00881190
	// FUNCTION: STUDIO 0x009e5810
	template<>
	void Bridge<rbx::signals::connection, true>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*))
	{
		luaL_newmetatable(L, className);
		protect_metatable(L, -1);
		lua_pushstring(L, "__index");
		lua_pushcfunction(L, on_indexFn);
		lua_settable(L, -3);
		lua_pushstring(L, "__newindex");
		lua_pushcfunction(L, on_newindexFn);
		lua_settable(L, -3);
		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, on_gc);
		lua_settable(L, -3);
		lua_pushstring(L, "__eq");
		lua_pushcfunction(L, on_eq);
		lua_settable(L, -3);
		lua_pushstring(L, "__tostring");
		lua_pushcfunction(L, on_tostring);
		lua_settable(L, -3);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	template<>
	int Bridge<EventInstance, true>::on_newindex(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		on_newindex(getObject(L, 1), name, L);
		return 0;
	}

	// mac: 0x171F8E (394B, batch444): index/newindex/gc/eq/tostring + readonly.
	// FUNCTION: PLAYER 0x00881510
	// FUNCTION: STUDIO 0x009e5b90
	template<>
	void Bridge<EventInstance, true>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*))
	{
		luaL_newmetatable(L, className);
		protect_metatable(L, -1);
		lua_pushstring(L, "__index");
		lua_pushcfunction(L, on_indexFn);
		lua_settable(L, -3);
		lua_pushstring(L, "__newindex");
		lua_pushcfunction(L, on_newindexFn);
		lua_settable(L, -3);
		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, on_gc);
		lua_settable(L, -3);
		lua_pushstring(L, "__eq");
		lua_pushcfunction(L, on_eq);
		lua_settable(L, -3);
		lua_pushstring(L, "__tostring");
		lua_pushcfunction(L, on_tostring);
		lua_settable(L, -3);
		lua_setreadonly(L, -1, 1);
		lua_pop(L, 1);
	}

	// mac: 0x1AB686 (51B, batch442): fixed "Connection" literal.
	// Defined in ScriptEvent.cpp (PLAYER 0x008a8f40): 2013 calls it out of line from
	// on_tostring(lua_State*) 0x0087f9e5.
	template<>
	int Bridge<rbx::signals::connection, true>::on_tostring(const rbx::signals::connection& object,
		lua_State* L);





	// VS2012 odr-use holders (openState names connection's at 0x18B897;
	// EventInstance's queue with the yield era).
	int (*volatile g_bridgeConnectionIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<rbx::signals::connection>::on_index;
	int (*volatile g_bridgeEventIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<EventInstance>::on_index;
	int (*volatile g_bridgeEventNewIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<EventInstance>::on_newindex;

} }
