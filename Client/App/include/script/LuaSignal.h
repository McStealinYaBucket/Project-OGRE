/* script/LuaSignal.h -- October 2013 (first import, section 243).
   Evidence: Bridge<connection> registerClass 0x171A9C + on_index
     0x17F06E + on_newindex 0x17F180 + on_gc 0x171C26 + on_eq 0x171C66 +
     on_tostring x2 (0x171CD4/0x1AB686) + wrappers 0x1980D0/0x198130 +
     pushNewObject 0x17F394 + className "RBXScriptConnection" (batch443
     rodata); SignalConnectionBridge::disconnect 0x17F148;
     connection::disconnect 0x14A06A + connected 0x14A0CC + operator==
     0x14A134 + operator!= 0x14A1CC + operator= 0x14A264 (batch444);
     Bridge<EventInstance> registerClass 0x171F8E + on_index 0x17E872 +
     on_newindex 0x17F008 + on_gc 0x172118 + on_eq 0x172168 + on_tostring
     x2 (0x1721D6/0x17F1E6) + wrappers 0x19800E/0x19806E + operator==
     0x172942 + pushNewObject 0x1788D6 + className "RBXScriptSignal"
     (batches 436-444). EventBridge::connect 0x17EA64 + wait 0x17ED78
     banked for the yield/slot era.
   EventInstance layout (binary-pinned): {const EventDescriptor* event
     (+0; +12 is the Descriptor name), boost::weak_ptr<Instance> (+4)}
     = 12B. No C1/C2 DB-wide -- the struct stays an aggregate.
   EventInstance on_index/wrappers/registerClass wiring +
     EventBridge::connect/wait landed in section 253.
   Original file: Client/App/script/LuaSignal.cpp (.h companion;
   2013 path TBD). */

#pragma once

#include "lua/luabridge.h"
#include "rbx/signal.h"
#include "script/ThreadRef.h"

#include <boost/weak_ptr.hpp>

namespace RBX {
	class Instance;
	namespace Reflection { class EventDescriptor; }

	namespace Lua {

		class SignalConnectionBridge : public Bridge<rbx::signals::connection>
		{
			friend class Bridge<rbx::signals::connection>;
		public:
			static int disconnect(lua_State* L);
		};

		// Event closures (section 253): pushed by EventInstance::on_index.
		class EventBridge
		{
		public:
			static int connect(lua_State* L);
			static int wait(lua_State* L);
		};

		template<> void Bridge<rbx::signals::connection>::registerClass(lua_State* L,
			int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
		template<> int Bridge<rbx::signals::connection>::on_index(const rbx::signals::connection& object,
			const char* name, lua_State* L);
		template<> void Bridge<rbx::signals::connection>::on_newindex(rbx::signals::connection& object,
			const char* name, lua_State* L);
		template<> int Bridge<rbx::signals::connection>::on_index(lua_State* L);
		template<> int Bridge<rbx::signals::connection>::on_newindex(lua_State* L);
		template<> int Bridge<rbx::signals::connection>::on_gc(lua_State* L);
		template<> int Bridge<rbx::signals::connection>::on_eq(lua_State* L);
		template<> int Bridge<rbx::signals::connection>::on_tostring(lua_State* L);
		template<> int Bridge<rbx::signals::connection>::on_tostring(const rbx::signals::connection& object,
			lua_State* L);

		// 12B event handle (see header block). Aggregate on purpose.
		class EventInstance
		{
		public:
			const Reflection::EventDescriptor* event;
			boost::weak_ptr<Instance> instance;
			bool operator==(const EventInstance& other) const;
		};

		template<> void Bridge<EventInstance>::registerClass(lua_State* L,
			int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
		template<> int Bridge<EventInstance>::on_index(const EventInstance& object,
			const char* name, lua_State* L);
		template<> void Bridge<EventInstance>::on_newindex(EventInstance& object,
			const char* name, lua_State* L);
		template<> int Bridge<EventInstance>::on_index(lua_State* L);
		template<> int Bridge<EventInstance>::on_newindex(lua_State* L);
		template<> int Bridge<EventInstance>::on_gc(lua_State* L);
		template<> int Bridge<EventInstance>::on_eq(lua_State* L);
		template<> int Bridge<EventInstance>::on_tostring(lua_State* L);
		template<> int Bridge<EventInstance>::on_tostring(const EventInstance& object,
			lua_State* L);

		// Explicit-specialization declarations for the Node bridge
		// (definitions in script/ThreadRef.cpp).
		template<> void Bridge<boost::intrusive_ptr<WeakThreadRef::Node>>::registerClass(lua_State* L,
			int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
		template<> int Bridge<boost::intrusive_ptr<WeakThreadRef::Node>>::on_index(
			const boost::intrusive_ptr<WeakThreadRef::Node>& object,
			const char* name, lua_State* L);
		template<> void Bridge<boost::intrusive_ptr<WeakThreadRef::Node>>::on_newindex(
			boost::intrusive_ptr<WeakThreadRef::Node>& object,
			const char* name, lua_State* L);
		template<> int Bridge<boost::intrusive_ptr<WeakThreadRef::Node>>::on_index(lua_State* L);
		template<> int Bridge<boost::intrusive_ptr<WeakThreadRef::Node>>::on_newindex(lua_State* L);
		template<> int Bridge<boost::intrusive_ptr<WeakThreadRef::Node>>::on_gc(lua_State* L);
		template<> int Bridge<boost::intrusive_ptr<WeakThreadRef::Node>>::on_eq(lua_State* L);
		template<> int Bridge<boost::intrusive_ptr<WeakThreadRef::Node>>::on_tostring(lua_State* L);
		template<> int Bridge<boost::intrusive_ptr<WeakThreadRef::Node>>::on_tostring(
			const boost::intrusive_ptr<WeakThreadRef::Node>& object, lua_State* L);

		// Explicit-specialization declarations for the sync GenericFunction
		// bridge (definitions in script/ThreadRef.cpp; section 248). The
		// async twin queues behind scheduleResume (yield era).
		template<> void Bridge<boost::shared_ptr<GenericFunction>>::registerClass(lua_State* L,
			int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
		template<> int Bridge<boost::shared_ptr<GenericFunction>>::on_index(
			const boost::shared_ptr<GenericFunction>& object,
			const char* name, lua_State* L);
		template<> void Bridge<boost::shared_ptr<GenericFunction>>::on_newindex(
			boost::shared_ptr<GenericFunction>& object,
			const char* name, lua_State* L);
		template<> int Bridge<boost::shared_ptr<GenericFunction>>::on_index(lua_State* L);
		template<> int Bridge<boost::shared_ptr<GenericFunction>>::on_newindex(lua_State* L);
		template<> int Bridge<boost::shared_ptr<GenericFunction>>::on_gc(lua_State* L);
		template<> int Bridge<boost::shared_ptr<GenericFunction>>::on_eq(lua_State* L);
		template<> int Bridge<boost::shared_ptr<GenericFunction>>::on_tostring(lua_State* L);
		template<> int Bridge<boost::shared_ptr<GenericFunction>>::on_tostring(
			const boost::shared_ptr<GenericFunction>& object, lua_State* L);

		// Explicit-specialization declarations for the async
		// GenericAsyncFunction bridge (definitions in script/ThreadRef.cpp;
		// section 251).
		template<> void Bridge<boost::shared_ptr<GenericAsyncFunction>>::registerClass(lua_State* L,
			int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
		template<> int Bridge<boost::shared_ptr<GenericAsyncFunction>>::on_index(
			const boost::shared_ptr<GenericAsyncFunction>& object,
			const char* name, lua_State* L);
		template<> void Bridge<boost::shared_ptr<GenericAsyncFunction>>::on_newindex(
			boost::shared_ptr<GenericAsyncFunction>& object,
			const char* name, lua_State* L);
		template<> int Bridge<boost::shared_ptr<GenericAsyncFunction>>::on_index(lua_State* L);
		template<> int Bridge<boost::shared_ptr<GenericAsyncFunction>>::on_newindex(lua_State* L);
		template<> int Bridge<boost::shared_ptr<GenericAsyncFunction>>::on_gc(lua_State* L);
		template<> int Bridge<boost::shared_ptr<GenericAsyncFunction>>::on_eq(lua_State* L);
		template<> int Bridge<boost::shared_ptr<GenericAsyncFunction>>::on_tostring(lua_State* L);
		template<> int Bridge<boost::shared_ptr<GenericAsyncFunction>>::on_tostring(
			const boost::shared_ptr<GenericAsyncFunction>& object, lua_State* L);

	}
}
