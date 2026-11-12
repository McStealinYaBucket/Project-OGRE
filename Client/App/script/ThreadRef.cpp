/* App/script/ThreadRef.cpp -- October 2013 (thread/fiber refs).
   Evidence: LiveThreadRef C1/C2 0x1AE868/0x1AF68C + D1/D2 0x1AF7A2/
     0x1AF7AC + WeakThreadRef C1/C2 0x1AE368/0x1AE590 + D1/D2 0x1AE680/
     0x1AE68A + addToNode 0x1AE43E + addRef 0x1AE4A6 + reset 0x1AE708 +
     removeFromNode 0x1AE764 + removeRef 0x1AE872 + Node::create 0x1AE980
     + Node::eraseAllRefs 0x1AE8C6 + Node D1/D2 0x1AE938/0x1AE942 +
     WeakFunctionRef C1 0x1AEB2A + D1/D2 0x1AF45E/0x1AF468 + removeRef
     0x1AF4FA + ThreadRef C1 0x180860 + lua_tofunction 0x1AED68 +
     lua_pushfunction 0x1AED8E + dumpThreadRefCounts 0x1AEAC8
     (batches 445-453). Original file: Client/App/script/ThreadRef.cpp
     (assert paths ThreadRef.cpp lines 112/419; the 2016 carry this file
     replaces had the wrong eraseAllRefs/dtor shapes).
   Layout (binary-pinned): WeakThreadRef = {vptr +0, refs +4 (intrusive
     base, zeroed, managed by boost free fns), previous +8, next +12,
     liveThreadRef +16 (intrusive_ptr), node +20 (borrowed Node* from
     L-20)} = 24B; WeakFunctionRef adds functionId at +24;
     LiveThreadRef = {refs +0, L +4, threadId +8 (the REGISTRY ref of
     the pushed thread -- the header name is kept)} = 12B; Node =
     {refs +0, first +4} = 8B. The carried ThreadRef.h already matches
     (no header change).
   DELTAs: counting is MANUAL (RbxInterlockedIncrement/Decrement calls;
     see Base/include/rbx/Countable.h -- the 2016 auto-count would
     double-count); sync stays rbx::spin_mutex (binary uses tbb's --
     unobservable exclusion equivalent; the shim tbb type is untested);
     FFlag::AllowYieldInHooks gates global-state routing (BSS false);
     FastLog texts kept verbatim (WeakThreadRef channel + counts text);
     the lua_State-20 Node slot is read/written raw (the fork's
     RobloxExtraSpace owns it; Node::get has no binary symbol, so no
     accessor is assumed).
   Queued (yield era): WeakThreadRef operator==/!=/=, Node::get,
     Node/GenericFunction bridges + wiring (slots landed 252). */

#include "stdafx.h"

#include "script/ThreadRef.h"
#include "script/LuaSignal.h"
#include "script/LuaArguments.h"
#include "Util/FastFlags.h"
#include "Util/Object.h"
#include "V8DataModel/ScriptContext.h"
#include "lua/luabridge.h"
#include "lua/lua.hpp"
#include "luaconf.h"
#include "util/standardout.h"

#include <boost/bind.hpp>

using namespace RBX;
using namespace RBX::Lua;

LOGVARIABLE(WeakThreadRef, 0)
LOGVARIABLE(ThreadRefCounts, 0)
LOGVARIABLE(LuaBridge, 0)

WeakThreadRef::Mutex WeakThreadRef::sync;

// Binary-passing helper: Countable<T>::count is rbx::atomic<int>, a
// single-{long} standard-layout member, so its address is the long the
// out-of-line interlocked helpers take (the binary passes it directly).
template<class Counted>
static long volatile* countAddr()
{
	return reinterpret_cast<long volatile*>(&Diagnostics::Countable<Counted>::count);
}

// mac: 0x1AF68C (278B, batch451): thread + non-null assert (ThreadRef.cpp:
// 419) + pushthread/registry-ref (global-routed iff AllowYieldInHooks).
// (C1 0x1AE868 forwards here; one definition emits both.)
// STUB: PLAYER 0x008713a0
// STUB: STUDIO 0x00827820
Lua::detail::LiveThreadRef::LiveThreadRef(lua_State* thread)
	: L(thread)
{
	if (FLog::Asserts != 0 && thread == NULL)
	{
		if (RBX::_internal::_debugHook == 0
			|| RBX::_internal::_debugHook("L",
				"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/ThreadRef.cpp", 419) == 0)
		{
			ReleaseAssert(FLog::Asserts,
				"L file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/ThreadRef.cpp line: 419");
		}
	}
	lua_pushthread(thread);
	if (FFlag::AllowYieldInHooks != 0)
	{
		lua_State* global = ScriptContext::getGlobalState(thread);
		lua_xmove(thread, global, 1);
		threadId = luaL_ref(global, LUA_REGISTRYINDEX);
	}
	else
		threadId = luaL_ref(thread, LUA_REGISTRYINDEX);
}

// mac: 0x1AF7AC (138B, batch448): unref (global-routed iff flag) + count--.
Lua::detail::LiveThreadRef::~LiveThreadRef()
{
	lua_State* state = L;
	if (FFlag::AllowYieldInHooks != 0)
		state = ScriptContext::getGlobalState(L);
	luaL_unref(state, LUA_REGISTRYINDEX, threadId);
}

// mac: 0x1AE368 (213B, batch448): node from L-20 + link + live ref.
// STUB: PLAYER 0x008723d0
// STUB: STUDIO 0x008288c0
WeakThreadRef::WeakThreadRef(lua_State* thread)
	: previous(NULL)
	, next(NULL)
	, node(*reinterpret_cast<Node**>(reinterpret_cast<char*>(thread) - 20))
{
	Mutex::scoped_lock lock(sync);
	addToNode();
	addRef(thread);
}

// mac: 0x1AE590 (192B, batch448): shares the live ref, re-links under the
// copied node scope (no addRef -- no new live thread is created).
WeakThreadRef::WeakThreadRef(const WeakThreadRef& other)
	: previous(NULL)
	, next(NULL)
	, liveThreadRef(other.liveThreadRef)
	, node(other.node)
{
	Mutex::scoped_lock lock(sync);
	addToNode();
}

// Behavior-derived operator= (no binary symbol DB-wide -- batch506
// sweep: the only WeakThreadRef symbols are C1/C2/D1/D2/addToNode/
// addRef/reset/removeFromNode/removeRef/Node fns -- so byte-verify is
// impossible; unlink+copy+link is the only assignment semantics under
// which the slot-refresh path is memory-safe).
WeakThreadRef& WeakThreadRef::operator=(const WeakThreadRef& other)
{
	if (this != &other)
	{
		Mutex::scoped_lock lock(sync);
		removeFromNode();
		liveThreadRef = other.liveThreadRef;
		node = other.node;
		addToNode();
	}
	return *this;
}

// mac: 0x1AE68A (126B, batch448): unlink + release live + count--.
// STUB: PLAYER 0x00871c40
// STUB: STUDIO 0x008280c0
WeakThreadRef::~WeakThreadRef()
{
	reset();
	liveThreadRef.reset();
}

// mac: 0x1AE43E (104B, batch450): head-insert into node->first (+20 gate).
// STUB: PLAYER 0x00871200
// STUB: STUDIO 0x00827670
void WeakThreadRef::addToNode()
{
	Node* self = node;
	if (self != NULL)
	{
		if (FLog::WeakThreadRef != 0)
		{
			FLog::FastLog(FLog::WeakThreadRef,
				"WeakThreadRef::addToNode() for node %p",
				reinterpret_cast<const void*>(self));
		}
		WeakThreadRef* oldFirst = self->first;
		if (oldFirst != NULL)
		{
			next = oldFirst;
			oldFirst->previous = this;
		}
		else
			next = NULL;
		previous = NULL;
		self->first = this;
	}
}

// mac: 0x1AE4A6 (224B, batch450): asserts no live ref yet (ThreadRef.cpp:
// 112), then news a LiveThreadRef for a non-null thread.
// STUB: PLAYER 0x00871d60
// STUB: STUDIO 0x008281e0
void WeakThreadRef::addRef(lua_State* L)
{
	if (FLog::Asserts != 0 && liveThreadRef.get() != NULL)
	{
		if (RBX::_internal::_debugHook == 0
			|| RBX::_internal::_debugHook("!liveThreadRef",
				"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/ThreadRef.cpp", 112) == 0)
		{
			ReleaseAssert(FLog::Asserts,
				"!liveThreadRef file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/ThreadRef.cpp line: 112");
		}
	}
	if (L != NULL)
	{
		liveThreadRef = new Lua::detail::LiveThreadRef(L);
		if (FLog::WeakThreadRef != 0)
		{
			FLog::FastLog(FLog::WeakThreadRef,
				"WeakThreadRef::addRef() for node %p",
				reinterpret_cast<const void*>(node));
		}
	}
}

// mac: 0x1AE708 (92B, batch450): unlink + virtual removeRef under the lock.
void WeakThreadRef::reset()
{
	Mutex::scoped_lock lock(sync);
	removeFromNode();
	removeRef();
}

// mac: 0x1AE764 (127B, batch450): doubly-linked unlink + head fixup.
// STUB: PLAYER 0x00871260
// STUB: STUDIO 0x008276d0
void WeakThreadRef::removeFromNode()
{
	Node* self = node;
	if (self != NULL)
	{
		if (FLog::WeakThreadRef != 0)
		{
			FLog::FastLog(FLog::WeakThreadRef,
				"WeakThreadRef::removeFromNode() for node %p",
				reinterpret_cast<const void*>(self));
		}
		if (next != NULL)
			next->previous = previous;
		if (previous != NULL)
			previous->next = next;
		if (self->first == this)
			self->first = next;
		next = NULL;
		previous = NULL;
		node = NULL;
	}
}

// mac: 0x1AE872 (84B, batch450): releases the live ref (logged).
// FUNCTION: PLAYER 0x00871e50
// FUNCTION: STUDIO 0x008282d0
void WeakThreadRef::removeRef()
{
	if (FLog::WeakThreadRef != 0)
	{
		FLog::FastLog(FLog::WeakThreadRef,
			"WeakThreadRef::removeRef() for node %p",
			reinterpret_cast<const void*>(node));
	}
	liveThreadRef.reset();
}

// mac: 0x1AE980 (124B, batch450): owns a fresh Node into the L-20 slot.
// STUB: PLAYER 0x00872480
// STUB: STUDIO 0x00828970
boost::intrusive_ptr<WeakThreadRef::Node> WeakThreadRef::Node::create(lua_State* thread)
{
	boost::intrusive_ptr<Node> created(new Node());
	*reinterpret_cast<boost::intrusive_ptr<Node>*>(reinterpret_cast<char*>(thread) - 20) = created;
	if (FLog::WeakThreadRef != 0)
	{
		FLog::FastLog(FLog::WeakThreadRef,
			"WeakThreadRef::Node::create node %p",
			reinterpret_cast<const void*>(created.get()));
	}
	return created;
}

// mac: 0x1AE8C6 (114B, batch451): virtual-clear every ref, then clears first.
// (Replaces the 2016 carry, which dropped the per-ref virtual removeRef
// calls and zeroed ref->node instead of the +20 cache.)
// STUB: PLAYER 0x00871990
// STUB: STUDIO 0x00827e10
void WeakThreadRef::Node::eraseAllRefs()
{
	Mutex::scoped_lock lock(sync);
	for (WeakThreadRef* ref = first; ref != NULL; ref = ref->next)
	{
		ref->removeRef();
		ref->node = NULL;
	}
	first = NULL;
}

// mac: 0x1AE942 (61B, batch452): logs, then eraseAllRefs (tail call).
// FUNCTION: PLAYER 0x00871a10
// FUNCTION: STUDIO 0x00827e90
WeakThreadRef::Node::~Node()
{
	if (FLog::WeakThreadRef != 0)
	{
		FLog::FastLog(FLog::WeakThreadRef,
			"WeakThreadRef::Node::~Node(), node = %p",
			reinterpret_cast<const void*>(this));
	}
	eraseAllRefs();
}

// mac: 0x1AEB2A (166B, batch448): base + vptr + function registry ref
// (global-routed iff AllowYieldInHooks).
WeakFunctionRef::WeakFunctionRef(lua_State* thread, int index)
	: WeakThreadRef(thread)
	, functionId(0)
{
	lua_pushvalue(thread, index);
	if (FFlag::AllowYieldInHooks != 0)
	{
		lua_State* global = ScriptContext::getGlobalState(thread);
		lua_xmove(thread, global, 1);
		functionId = luaL_ref(global, LUA_REGISTRYINDEX);
	}
	else
		functionId = luaL_ref(thread, LUA_REGISTRYINDEX);
}

// mac: 0x1AF5A4 (232B, batch454): base copy (shares the live ref) + the
// function VALUE re-registered under a fresh ref (ids are never shared).
// Fetch/store states are the copier's own live thread (identical to the
// source's by construction -- the live ref is shared).
WeakFunctionRef::WeakFunctionRef(const WeakFunctionRef& other)
	: WeakThreadRef(other)
	, functionId(0)
{
	// Gate + states via the public threadDangerous() (liveThreadRef is
	// private to the base; the values are identical -- shared live ref).
	if (other.threadDangerous() != NULL)
	{
		lua_State* mine = threadDangerous();
		if (FFlag::AllowYieldInHooks != 0)
		{
			lua_State* global = ScriptContext::getGlobalState(mine);
			lua_rawgeti(global, LUA_REGISTRYINDEX, other.functionId);
			functionId = luaL_ref(global, LUA_REGISTRYINDEX);
		}
		else
		{
			lua_rawgeti(mine, LUA_REGISTRYINDEX, other.functionId);
			functionId = luaL_ref(mine, LUA_REGISTRYINDEX);
		}
	}
}

// mac: 0x1AF468 (145B, batch452): vptr + unref function (iff live) + base.
// (No lock() here -- the binary reads the live thread straight through;
// locking would perturb the diagnostics counts.)
// STUB: PLAYER 0x00871e90
// STUB: STUDIO 0x00828310
WeakFunctionRef::~WeakFunctionRef()
{
	lua_State* mine = threadDangerous();
	if (functionId != 0 && mine != NULL)
	{
		lua_State* state = mine;
		if (FFlag::AllowYieldInHooks != 0)
			state = ScriptContext::getGlobalState(state);
		luaL_unref(state, LUA_REGISTRYINDEX, functionId);
	}
}

// mac: 0x1AF4FA (159B, batch451): unrefs the functionId (iff live thread),
// then the base removeRef.
// STUB: PLAYER 0x00871f30
// STUB: STUDIO 0x008283b0
void WeakFunctionRef::removeRef()
{
	lua_State* mine = threadDangerous();
	if (functionId != 0 && mine != NULL)
	{
		if (FLog::WeakThreadRef != 0)
		{
			FLog::FastLog(FLog::WeakThreadRef,
				" WeakFunctionRef::removeRef() for node %p",
				reinterpret_cast<const void*>(node));
		}
		lua_State* state = mine;
		if (FFlag::AllowYieldInHooks != 0)
			state = ScriptContext::getGlobalState(state);
		luaL_unref(state, LUA_REGISTRYINDEX, functionId);
		functionId = 0;
	}
	WeakThreadRef::removeRef();
}

// mac: 0x180860 (122B, batch446): count++ + owning LiveThreadRef.
// (No out-of-line dtor in the binary; the implicit member teardown
// stands. The manual count has no matching decrement evidenced --
// ThreadRef's count is cumulative, not live; noted, not invented.)
// STUB: PLAYER 0x00673910
// STUB: STUDIO 0x007db670
ThreadRef::ThreadRef(lua_State* thread)
	: liveThreadRef(new Lua::detail::LiveThreadRef(thread))
{
}

// mac: 0x1AED68 (38B, batch451): placement construction into the return slot.
WeakFunctionRef RBX::Lua::lua_tofunction(lua_State* L, int index)
{
	return WeakFunctionRef(L, index);
}

// mac: 0x1AED8E (116B, batch451): registry fetch (global-routed iff flag).
void RBX::Lua::lua_pushfunction(lua_State* L, const WeakFunctionRef& function)
{
	if (FFlag::AllowYieldInHooks == 0)
		lua_rawgeti(L, LUA_REGISTRYINDEX, function.functionId);
	else
	{
		lua_State* global = ScriptContext::getGlobalState(L);
		lua_rawgeti(global, LUA_REGISTRYINDEX, function.functionId);
		lua_xmove(global, L, 1);
	}
}

// mac: 0x180AD0 (74B, batch446): null connection + linked thread.
WaitScriptSlot::WaitScriptSlot(lua_State* thread)
	: thread(thread)
{
}

// mac: 0x17F85E (385B, batch446): one-shot wait -- disconnect, push the event
// arguments onto the waiter, resume it with their count, clear its stack.
// ThreadRef pins + net-zero count pairs stand in for the manual
// addref/release dance (observably identical).
void WaitScriptSlot::operator()(const Reflection::EventArguments& arguments)
{
	connection.disconnect();
	Lua::ThreadRef reacquired = thread.lock();
	lua_State* live = reacquired.get();
	if (live != NULL)
	{
		int pushed = 0;
		for (Reflection::ValueArray::const_iterator iter = arguments.begin();
			iter != arguments.end(); ++iter)
		{
			pushed += Lua::LuaArguments::push(*iter, live);
		}
		ScriptContext* context = RobloxExtraSpace::get(live)->context();
		Lua::ThreadRef pin(reacquired);
		context->resume(pin, pushed);
		lua_settop(live, 0);
	}
}

// mac: 0x171174 (394B, batch456): index/newindex/gc/eq/tostring + readonly.
// FUNCTION: PLAYER 0x00881270
// FUNCTION: STUDIO 0x009e58f0
template<>
void Bridge<boost::intrusive_ptr<WeakThreadRef::Node>, true>::registerClass(lua_State* L,
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

// mac: 0x1AE9FC (102B, batch457): Nodes expose no members.
template<>
int Bridge<boost::intrusive_ptr<WeakThreadRef::Node>, true>::on_index(
	const boost::intrusive_ptr<WeakThreadRef::Node>& object,
	const char* name, lua_State* L)
{
	throw RBX::runtime_error("%s is not a valid member", name);
}

// mac: 0x1AEA62 (102B, batch457): always throws, with the name.
template<>
void Bridge<boost::intrusive_ptr<WeakThreadRef::Node>, true>::on_newindex(
	boost::intrusive_ptr<WeakThreadRef::Node>& object,
	const char* name, lua_State* L)
{
	throw RBX::runtime_error("%s cannot be assigned to", name);
}

// mac: 0x198DB2 (96B, batch456): luaL_checklstring wrapper verbatim.
template<>
int Bridge<boost::intrusive_ptr<WeakThreadRef::Node>, true>::on_index(lua_State* L)
{
	const char* name = luaL_checklstring(L, 2, NULL);
	return on_index(getObject(L, 1), name, L);
}

// mac: 0x198E12 (98B, batch456): same verbatim.
template<>
int Bridge<boost::intrusive_ptr<WeakThreadRef::Node>, true>::on_newindex(lua_State* L)
{
	const char* name = luaL_checklstring(L, 2, NULL);
	on_newindex(getObject(L, 1), name, L);
	return 0;
}

// mac: 0x1712FE (64B, batch456): releases the intrusive ref.
template<>
int Bridge<boost::intrusive_ptr<WeakThreadRef::Node>, true>::on_gc(lua_State* L)
{
	getObject(L, 1).reset();
	return 0;
}

// mac: 0x17133E (105B, batch456): stored-pointer identity.
template<>
int Bridge<boost::intrusive_ptr<WeakThreadRef::Node>, true>::on_eq(lua_State* L)
{
	lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
	return 1;
}

// mac: 0x1713A8 (62B, batch456): checkudata + forward.
// STUB: PLAYER 0x0087fa40
// STUB: STUDIO 0x009e40c0
template<>
int Bridge<boost::intrusive_ptr<WeakThreadRef::Node>, true>::on_tostring(lua_State* L)
{
	return on_tostring(getObject(L, 1), L);
}

// mac: 0x1AB6BA (51B, batch457): fixed "WeakThreadRef" literal (13 chars --
// NOT "WeakThreadRef::Node" like the className).
// FUNCTION: PLAYER 0x008a8f60
// FUNCTION: STUDIO 0x009ecb80
template<>
int Bridge<boost::intrusive_ptr<WeakThreadRef::Node>, true>::on_tostring(
	const boost::intrusive_ptr<WeakThreadRef::Node>& object, lua_State* L)
{
	lua_pushlstring(L, "WeakThreadRef", 13);
	return 1;
}

template<>
// GLOBAL: PLAYER 0x00d8e508
// GLOBAL: STUDIO 0x00ee1b20
// RBX::Lua::Bridge<boost::intrusive_ptr<RBX::Lua::WeakThreadRef::Node>,1>::className
const char* Bridge<boost::intrusive_ptr<WeakThreadRef::Node>, true>::className =
	"WeakThreadRef::Node";

// VS2012 odr-use holders (openState names these at 0x18BA73).
int (*volatile g_bridgeNodeIndex)(lua_State*) =
	(int (*)(lua_State*))&Bridge<boost::intrusive_ptr<WeakThreadRef::Node>>::on_index;
int (*volatile g_bridgeNodeNewIndex)(lua_State*) =
	(int (*)(lua_State*))&Bridge<boost::intrusive_ptr<WeakThreadRef::Node>>::on_newindex;

// mac: 0x180B1C (193B, batch446): null connection + context + function +
// null cached thread + zero reentrancy + diagnostic count.
FunctionScriptSlot::FunctionScriptSlot(lua_State* thread, int index)
	: context(RobloxExtraSpace::get(thread)->context())
	, function(thread, index)
	, reentrancy(0)
{
	RBX::RbxInterlockedIncrement(countAddr<FunctionScriptSlot>());
}

// mac: 0x17FFF2 (2157B, batch446): reentrancy-guarded fire -- push the
// function + event arguments onto an exec thread (cached spare if
// first-level, else fresh) and resume it. Yield/error resets the cache
// (first-level only); clean runs refresh it; errors disconnect.
// ThreadRef pins + net-zero count pairs stand in for the manual
// addref/release dance. All Asserts are log-continue (never branch).
// STUB: PLAYER 0x0087dee0
// STUB: STUDIO 0x009e2560
void FunctionScriptSlot::operator()(const Reflection::EventArguments& arguments)
{
	int depth = reentrancy++;
	if (depth + 1 >= 7)
	{
		StandardOut::singleton()->print(MESSAGE_ERROR,
			"maximum event re-entrancy depth exceeded");
		--reentrancy;
		return;
	}
	Lua::ThreadRef fnPin = function.lock();
	lua_State* fnThread = fnPin.get();
	if (fnThread == NULL)
	{
		if (FLog::Asserts != 0)
		{
			ReleaseAssert(FLog::Asserts,
				"cachedSlotThread.empty() file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaSignalBridge.cpp line: 173");
		}
		connection.disconnect();
		--reentrancy;
		return;
	}
	if (FLog::LuaBridge != 0)
	{
		FLog::FastLog(FLog::LuaBridge, "Executing lua from signal, execution depth: %u",
			reinterpret_cast<const void*>((size_t)reentrancy));
	}
	Lua::ThreadRef execPin;
	bool firstLevel = (depth <= 0);
	if (firstLevel)
		execPin = cachedSlotThread.lock();
	lua_State* execThread = execPin.get();
	if (execThread == NULL)
	{
		lua_State* fresh = lua_newthread(fnThread);
		// Fresh-child L-20 slot (same rule as delay/spawn): zero it --
		// the refresh below wraps it in a WeakThreadRef immediately.
		*(void**)((char*)fresh - 20) = NULL;
		execPin = Lua::ThreadRef(fresh);
		execThread = fresh;
	}
	if (FLog::Asserts != 0 && lua_type(fnThread, -1) != LUA_TTHREAD)
	{
		ReleaseAssert(FLog::Asserts,
			"lua_isthread(functionThread, -1) file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaSignalBridge.cpp line: 131");
	}
	lua_settop(fnThread, -2);
	int pushed = 0;
	Lua::lua_pushfunction(execThread, function);
	for (Reflection::ValueArray::const_iterator iter = arguments.begin();
		iter != arguments.end(); ++iter)
	{
		pushed += Lua::LuaArguments::push(*iter, execThread);
	}
	Lua::ThreadRef callPin(execPin);
	int status = context->resume(callPin, pushed);
	if ((status == 1 || status == 2) && depth <= 0)
		cachedSlotThread.reset();
	if (status == 2)
	{
		StandardOut::singleton()->print(MESSAGE_WARNING,
			"Disconnected event because of exception");
		connection.disconnect();
	}
	if (fnThread != NULL && status == 0 && depth <= 0)
	{
		Lua::WeakThreadRef refresh(execThread);
		cachedSlotThread = refresh;
	}
	if (FLog::Asserts != 0)
	{
		Lua::ThreadRef spareCheck = cachedSlotThread.lock();
		if (spareCheck.get() == NULL)
		{
			ReleaseAssert(FLog::Asserts,
				"slotThread file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaSignalBridge.cpp line: 186");
		}
	}
	lua_settop(execThread, 0);
	--reentrancy;
}

// mac: 0x175E1A (2228B, batches 489-495): invoke a Lua callback with Tuple
// args, collecting the results into a fresh Tuple. File (assert paths):
// Client/App/script/LuaInstanceBridge.cpp (lives here pending that era;
// same precedent as the slot operators carrying LuaSignalBridge.cpp
// literals). Returns the results; throws "No callback function"
// (0xB2A85C) on a dead function, rethrows the Lua error on status 2,
// and throws "Callbacks cannot yield" (0xB2AAD9) on status 1.
// The cached thread (intrusive, may be dead but must be non-null -- the
// binary derefs it for reset/refresh) is reused when live, else a fresh
// child is created, trimmed, and (on success) stored back. Pins +
// net-zero count pairs stand in for the manual addref/release dance;
// all Asserts are log-continue (never branch).
// STUB: PLAYER 0x0087b940
// STUB: STUDIO 0x009dffc0
shared_ptr<Reflection::Tuple> RBX::Lua::callCallback(WeakFunctionRef function,
	shared_ptr<const Reflection::Tuple> args,
	boost::intrusive_ptr<WeakThreadRef> cachedThread)
{
	ThreadRef fnPin = function.lock();
	lua_State* fnThread = fnPin.get();
	if (fnThread == NULL)
		throw RBX::runtime_error("No callback function");
	ThreadRef execPin = (cachedThread.get() != NULL)
		? cachedThread->lock()
		: ThreadRef();
	lua_State* execThread = execPin.get();
	bool fresh = false;
	int top = 0;
	if (execThread == NULL)
	{
		top = lua_gettop(fnThread);
		lua_State* newborn = lua_newthread(fnThread);
		// Fresh-child L-20 slot (same rule as delay/spawn/slots): the
		// success path below wraps the exec thread in a WeakThreadRef,
		// which reads it -- zero it here.
		*(void**)((char*)newborn - 20) = NULL;
		execPin = ThreadRef(newborn);
		execThread = newborn;
		if (FLog::Asserts != 0 && lua_type(fnThread, -1) != LUA_TTHREAD)
		{
			if (RBX::_internal::_debugHook == 0
				|| RBX::_internal::_debugHook("lua_isthread(functionThread, -1)",
					"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaInstanceBridge.cpp", 659) == 0)
			{
				ReleaseAssert(FLog::Asserts,
					"lua_isthread(functionThread, -1) file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaInstanceBridge.cpp line: 659");
			}
		}
		while (lua_gettop(fnThread) > top + 1)
			lua_remove(fnThread, -2);
		fresh = true;
		if (FLog::Asserts != 0 && lua_type(fnThread, -1) != LUA_TTHREAD)
		{
			if (RBX::_internal::_debugHook == 0
				|| RBX::_internal::_debugHook("lua_isthread(functionThread, -1)",
					"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaInstanceBridge.cpp", 666) == 0)
			{
				ReleaseAssert(FLog::Asserts,
					"lua_isthread(functionThread, -1) file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaInstanceBridge.cpp line: 666");
			}
		}
		if (FLog::Asserts != 0 && top + 1 != lua_gettop(fnThread))
		{
			if (RBX::_internal::_debugHook == 0
				|| RBX::_internal::_debugHook("top+1 == lua_gettop(functionThread)",
					"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaInstanceBridge.cpp", 667) == 0)
			{
				ReleaseAssert(FLog::Asserts,
					"top+1 == lua_gettop(functionThread) file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaInstanceBridge.cpp line: 667");
			}
		}
	}
	lua_pushfunction(fnThread, function);
	lua_xmove(fnThread, execThread, 1);
	int execTop = lua_gettop(execThread);
	int pushed = 0;
	for (Reflection::ValueArray::const_iterator iter = args->values.begin();
		iter != args->values.end(); ++iter)
	{
		pushed += LuaArguments::push(*iter, execThread);
	}
	// DELTA (0x176169-0x1761D8, cleared 0x17622B-0x176238): the
	// LuaDebugger block -- findDebugger(L) + debugger+0xDC marker set
	// around resume + clear -- queues behind the Scripting::
	// DebuggerManager era (findDebugger 0x726550 unlanded; RBX::Scripting::
	// DebuggerManager does not exist in-tree). The gate global
	// (off_EAD220) is read ONLY here (sole-user proof, batch492), and
	// FFlag::LuaDebugger defaults false (BSS-zero, FastFlags.cpp), so the
	// block is dead in this build; green paths never execute it.
	ScriptContext* context = RobloxExtraSpace::get(execThread)->context();
	ThreadRef callPin(execPin);
	int status = context->resume(callPin, pushed);
	if (fresh)
	{
		if (FLog::Asserts != 0 && lua_type(fnThread, -1) != LUA_TTHREAD)
		{
			if (RBX::_internal::_debugHook == 0
				|| RBX::_internal::_debugHook("lua_isthread(functionThread, -1)",
					"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaInstanceBridge.cpp", 702) == 0)
			{
				ReleaseAssert(FLog::Asserts,
					"lua_isthread(functionThread, -1) file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaInstanceBridge.cpp line: 702");
			}
		}
		lua_settop(fnThread, -2);
	}
	if (status == 2)
	{
		cachedThread->reset();
		const char* message = safe_lua_tostring(execThread, -1);
		lua_settop(execThread, 0);
		throw RBX::runtime_error("%s", message);
	}
	if (status == 1)
	{
		cachedThread->reset();
		lua_settop(execThread, 0);
		throw RBX::runtime_error("Callbacks cannot yield");
	}
	if (fresh)
	{
		WeakThreadRef refresh(execThread);
		*cachedThread = refresh;
	}
	int resultTop = lua_gettop(execThread);
	shared_ptr<Reflection::Tuple> result(
		new Reflection::Tuple(resultTop - execTop + 1));
	for (size_t i = 0; i < result->values.size(); ++i)
		LuaArguments::get(execThread, (int)i + 1, result->at(i), true);
	lua_settop(execThread, 0);
	return result;
}

// getSingleton spec for the sync-function Variant arm (section 248).
// Home follows 2016 (ThreadRef.cpp); name 2016-carried (the binary's
// TType D1/D0 at 0x1B0026/0x1B01F4 prove the spec, not the name).
template<>
const Reflection::Type& Reflection::Type::getSingleton<shared_ptr<GenericFunction> >()
{
	static Reflection::TType<shared_ptr<GenericFunction> > type("GenericFunction");
	return type;
}

// Pre-main touch (lockdown prescription, 246-postmortem): withVariantValue
// and test construction would otherwise build this post-lockdown.
template<> Reflection::TypeRegistrar<shared_ptr<GenericFunction> >
	Reflection::TypeRegistrar<shared_ptr<GenericFunction> >::registrar(0);

// mac: 0x1713E6 (394B, batch467): index/newindex/gc/eq/tostring + readonly
// (same metatable set as the Node bridge).
// FUNCTION: PLAYER 0x00881350
// FUNCTION: STUDIO 0x009e59d0
template<>
void Bridge<shared_ptr<GenericFunction>, true>::registerClass(lua_State* L,
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

// mac: 0x1AEBD0 (102B, batch468): functions expose no members.
template<>
int Bridge<shared_ptr<GenericFunction>, true>::on_index(
	const shared_ptr<GenericFunction>& object,
	const char* name, lua_State* L)
{
	throw RBX::runtime_error("%s is not a valid member", name);
}

// mac: 0x1AEC36 (102B, batch468): always throws, with the name.
template<>
void Bridge<shared_ptr<GenericFunction>, true>::on_newindex(
	shared_ptr<GenericFunction>& object,
	const char* name, lua_State* L)
{
	throw RBX::runtime_error("%s cannot be assigned to", name);
}

// mac: 0x198E74 (standard wrapper, batch467): luaL_checklstring verbatim.
template<>
int Bridge<shared_ptr<GenericFunction>, true>::on_index(lua_State* L)
{
	const char* name = luaL_checklstring(L, 2, NULL);
	return on_index(getObject(L, 1), name, L);
}

// mac: 0x198ED4 (standard wrapper, batch467): same verbatim.
template<>
int Bridge<shared_ptr<GenericFunction>, true>::on_newindex(lua_State* L)
{
	const char* name = luaL_checklstring(L, 2, NULL);
	on_newindex(getObject(L, 1), name, L);
	return 0;
}

// mac: 0x171570 (65B, batch467): releases the userdata's shared copy
// (spelled .reset() per the Node-bridge precedent; the bare release is
// unobservable once the userdata is garbage).
template<>
int Bridge<shared_ptr<GenericFunction>, true>::on_gc(lua_State* L)
{
	getObject(L, 1).reset();
	return 0;
}

// mac: 0x1715B2 (105B, batch467): stored-pointer identity.
template<>
int Bridge<shared_ptr<GenericFunction>, true>::on_eq(lua_State* L)
{
	lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
	return 1;
}

// mac: 0x17161C (62B, batch467): checkudata + forward.
// STUB: PLAYER 0x0087faa0
// STUB: STUDIO 0x009e4120
template<>
int Bridge<shared_ptr<GenericFunction>, true>::on_tostring(lua_State* L)
{
	return on_tostring(getObject(L, 1), L);
}

// mac: 0x1AB6EE (51B, batch468): fixed "GenericFunction" literal (15 chars).
// FUNCTION: PLAYER 0x008a8f80
// FUNCTION: STUDIO 0x009ecba0
template<>
int Bridge<shared_ptr<GenericFunction>, true>::on_tostring(
	const shared_ptr<GenericFunction>& object, lua_State* L)
{
	lua_pushlstring(L, "GenericFunction", 15);
	return 1;
}

template<>
// GLOBAL: PLAYER 0x00d8e50c
// GLOBAL: STUDIO 0x00ee1b24
// RBX::Lua::Bridge<boost::shared_ptr<boost::function<boost::shared_ptr<RBX::Reflection::Tuple const> __cdecl(boost::shared_ptr<RBX::Reflection::Tuple const>)> >,1>::className
const char* Bridge<shared_ptr<GenericFunction>, true>::className =
	"GenericFunction";

// mac: 0x1AEE8E (343B, batch459): upvalue function + arg Tuple in, result
// Tuple out (pushed element-wise, count returned; null result pushes
// nothing). Shared ownership is RAII here; the binary's manual
// addref/release dance is observably identical.
// STUB: PLAYER 0x008732d0
// STUB: STUDIO 0x008298e0
static int callGenericFunctionBridge(lua_State* L)
{
	shared_ptr<GenericFunction> f(
		Bridge<shared_ptr<GenericFunction>>::getObject(L, lua_upvalueindex(1)));
	shared_ptr<Reflection::Tuple> args = LuaArguments::getValues(L);
	shared_ptr<const Reflection::Tuple> result = (*f)(args);
	if (result)
		return LuaArguments::pushTuple(*result, L);
	return 0;
}

// mac: 0x1AEE02 (140B, batch458): shared-copy userdata + 1-upvalue closure
// (exception-safe via the by-value parameter's lifetime).
void RBX::Lua::lua_pushfunction(lua_State* L, shared_ptr<GenericFunction> function)
{
	Bridge<shared_ptr<GenericFunction>>::pushNewObject(L, function);
	lua_pushcclosure(L, callGenericFunctionBridge, 1);
}

// VS2012 odr-use holders (openState names these at 0x18BA8F).
int (*volatile g_bridgeGenericFunctionIndex)(lua_State*) =
	(int (*)(lua_State*))&Bridge<shared_ptr<GenericFunction>>::on_index;
int (*volatile g_bridgeGenericFunctionNewIndex)(lua_State*) =
	(int (*)(lua_State*))&Bridge<shared_ptr<GenericFunction>>::on_newindex;

// Async result continuation (2016 ThreadRef.cpp shape; bound by the
// async closure below). scheduleResume landed in section 250.
static void onAsyncResult(ThreadRef thread, weak_ptr<ScriptContext> context,
	Lua::IAsyncResult* result)
{
	shared_ptr<const Reflection::Tuple> value;
	try
	{
		value = result->getValue();
	}
	catch (RBX::base_exception& e)
	{
		RBX::StandardOut::singleton()->printf(RBX::MESSAGE_WARNING,
			"Exception caught from async call. %s", e.what());
		return;
	}
	if (shared_ptr<ScriptContext> lockedContext = context.lock())
		lockedContext->scheduleResume(thread, value);
}

// getSingleton spec for the async-function Variant arm (section 251).
// Home follows 2016 (ThreadRef.cpp); name 2016-carried (the binary's
// TType D1/D0 at 0x1B003A/0x1B010E prove the spec).
template<>
const Reflection::Type& Reflection::Type::getSingleton<shared_ptr<GenericAsyncFunction> >()
{
	static Reflection::TType<shared_ptr<GenericAsyncFunction> > type("GenericAsyncFunction");
	return type;
}

// Pre-main touch (lockdown prescription).
template<> Reflection::TypeRegistrar<shared_ptr<GenericAsyncFunction> >
	Reflection::TypeRegistrar<shared_ptr<GenericAsyncFunction> >::registrar(0);

// mac: 0x17165A (394B, batch467): same metatable set.
// FUNCTION: PLAYER 0x00881430
// FUNCTION: STUDIO 0x009e5ab0
template<>
void Bridge<shared_ptr<GenericAsyncFunction>, true>::registerClass(lua_State* L,
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

// mac: 0x1AEC9C (102B, batch468): no members.
template<>
int Bridge<shared_ptr<GenericAsyncFunction>, true>::on_index(
	const shared_ptr<GenericAsyncFunction>& object,
	const char* name, lua_State* L)
{
	throw RBX::runtime_error("%s is not a valid member", name);
}

// mac: 0x1AED02 (102B, batch468): always throws.
template<>
void Bridge<shared_ptr<GenericAsyncFunction>, true>::on_newindex(
	shared_ptr<GenericAsyncFunction>& object,
	const char* name, lua_State* L)
{
	throw RBX::runtime_error("%s cannot be assigned to", name);
}

// mac: 0x198F36 (standard wrapper, batch467).
template<>
int Bridge<shared_ptr<GenericAsyncFunction>, true>::on_index(lua_State* L)
{
	const char* name = luaL_checklstring(L, 2, NULL);
	return on_index(getObject(L, 1), name, L);
}

// mac: 0x198F96 (standard wrapper, batch467).
template<>
int Bridge<shared_ptr<GenericAsyncFunction>, true>::on_newindex(lua_State* L)
{
	const char* name = luaL_checklstring(L, 2, NULL);
	on_newindex(getObject(L, 1), name, L);
	return 0;
}

// mac: 0x1717E4 (batch467): releases the shared copy.
template<>
int Bridge<shared_ptr<GenericAsyncFunction>, true>::on_gc(lua_State* L)
{
	getObject(L, 1).reset();
	return 0;
}

// mac: 0x171826 (batch467): stored-pointer identity.
template<>
int Bridge<shared_ptr<GenericAsyncFunction>, true>::on_eq(lua_State* L)
{
	lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
	return 1;
}

// mac: 0x171890 (batch467): checkudata + forward.
// STUB: PLAYER 0x0087fac0
// STUB: STUDIO 0x009e4140
template<>
int Bridge<shared_ptr<GenericAsyncFunction>, true>::on_tostring(lua_State* L)
{
	return on_tostring(getObject(L, 1), L);
}

// mac: 0x1AB722 (51B, batch468): "GenericAsyncFunction" (20 chars).
// FUNCTION: PLAYER 0x008a8fa0
// FUNCTION: STUDIO 0x009ecbc0
template<>
int Bridge<shared_ptr<GenericAsyncFunction>, true>::on_tostring(
	const shared_ptr<GenericAsyncFunction>& object, lua_State* L)
{
	lua_pushlstring(L, "GenericAsyncFunction", 20);
	return 1;
}

template<>
// GLOBAL: PLAYER 0x00d8e510
// GLOBAL: STUDIO 0x00ee1b28
// RBX::Lua::Bridge<boost::shared_ptr<boost::function<void __cdecl(boost::shared_ptr<RBX::Reflection::Tuple const>,boost::function<void __cdecl(RBX::Lua::IAsyncResult *)>)> >,1>::className
const char* Bridge<shared_ptr<GenericAsyncFunction>, true>::className =
	"GenericAsyncFunction";

// mac: 0x1AF071 (956B, batch459): upvalue function + arg Tuple in, bind the
// result continuation, yield. The yieldCaptured tripwire mirrors the
// binary (ThreadRef.cpp:338); the Asserts gate matches the binary's
// FLog::Asserts structure.
// STUB: PLAYER 0x008736a0
// STUB: STUDIO 0x00829cb0
static int callGenericAsyncFunctionBridge(lua_State* L)
{
	shared_ptr<GenericAsyncFunction> f(
		Bridge<shared_ptr<GenericAsyncFunction>>::getObject(L, lua_upvalueindex(1)));
	shared_ptr<Reflection::Tuple> args = LuaArguments::getValues(L);
	ThreadRef thread(L);
	(*f)(args, boost::bind(&onAsyncResult, thread,
		RBX::weak_from(RobloxExtraSpace::get(L)->context()), boost::arg<1>()));
	if (FLog::Asserts != 0 && RobloxExtraSpace::get(L)->yieldCaptured)
	{
		if (RBX::_internal::_debugHook == 0
			|| RBX::_internal::_debugHook("!RobloxExtraSpace::get(L)->yieldCaptured",
				"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/ThreadRef.cpp", 338) == 0)
		{
			ReleaseAssert(FLog::Asserts,
				"!RobloxExtraSpace::get(L)->yieldCaptured file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/ThreadRef.cpp line: 338");
		}
	}
	RobloxExtraSpace::get(L)->yieldCaptured = true;
	return lua_yield(L, 0);
}

// mac: 0x1AEFE5 (140B, batch458): shared-copy userdata + 1-upvalue closure.
// FUNCTION: PLAYER 0x008738a0
// FUNCTION: STUDIO 0x00829eb0
void RBX::Lua::lua_pushfunction(lua_State* L, shared_ptr<GenericAsyncFunction> function)
{
	Bridge<shared_ptr<GenericAsyncFunction>>::pushNewObject(L, function);
	lua_pushcclosure(L, callGenericAsyncFunctionBridge, 1);
}

// VS2012 odr-use holders (openState names these at 0x18BAAB).
int (*volatile g_bridgeGenericAsyncFunctionIndex)(lua_State*) =
	(int (*)(lua_State*))&Bridge<shared_ptr<GenericAsyncFunction>>::on_index;
int (*volatile g_bridgeGenericAsyncFunctionNewIndex)(lua_State*) =
	(int (*)(lua_State*))&Bridge<shared_ptr<GenericAsyncFunction>>::on_newindex;

// mac: 0x1AEAC8 (88B, batch451): diagnostic counts (replaces the empty carry).
// FUNCTION: PLAYER 0x00871370
// FUNCTION: STUDIO 0x008277f0
void RBX::Lua::dumpThreadRefCounts()
{
	if (FLog::ThreadRefCounts != 0)
	{
		FLog::FastLog(FLog::ThreadRefCounts,
			"ThreadRef=%i WeakThreadRef=%i detail::LiveThreadRef=%i",
			reinterpret_cast<const void*>(Diagnostics::Countable<ThreadRef>::getCount()),
			reinterpret_cast<const void*>(Diagnostics::Countable<WeakThreadRef>::getCount()),
			reinterpret_cast<const void*>(Diagnostics::Countable<Lua::detail::LiveThreadRef>::getCount()));
	}
}
