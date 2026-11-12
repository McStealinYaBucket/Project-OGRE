#pragma once

#include "rbx/intrusive_ptr_target.h"
#include "boost/intrusive_ptr.hpp"
#include "rbx/boost.hpp"
#include "rbx/threadsafe.h"
#include "reflection/type.h"
#include "reflection/Event.h"
#include "rbx/signal.h"

struct lua_State;

using boost::shared_ptr;

LOGGROUP(WeakThreadRef)

namespace RBX {

	class ScriptContext;

	namespace Lua {

		void dumpThreadRefCounts();

	// Used internally
		namespace detail {
			class LiveThreadRef 
				: public rbx::quick_intrusive_ptr_target<LiveThreadRef>
				, public Diagnostics::Countable<LiveThreadRef>
				, boost::noncopyable
			{
				lua_State* L;
				int threadId;
				friend class WeakThreadRef;
				friend class ThreadRef;
			public:
				// Do not create this. It is an internal class
				LiveThreadRef (lua_State* thread);
				~LiveThreadRef();
				bool empty() const {
					return L == NULL;
				}
				lua_State* thread() const {
					return L;
				}

			};
		}

	// You get this by calling lock() on WeakThreadRef
	class ThreadRef
		: public Diagnostics::Countable<ThreadRef>
	{
		boost::intrusive_ptr<detail::LiveThreadRef> liveThreadRef;
		friend class WeakThreadRef;
		ThreadRef (detail::LiveThreadRef* liveThreadRef):liveThreadRef(liveThreadRef) {}
	public:
		ThreadRef() {}
		// mac: 0x180860 (122B, batch446): count++ + owning LiveThreadRef.
		// Defined out-of-line in script/ThreadRef.cpp (the inline carry
		// missed the manual count++).
		ThreadRef(lua_State* thread);
		lua_State* get() const {
			return liveThreadRef ? liveThreadRef->thread() : NULL;
		}
		operator lua_State*() const
		{
			return get();
		}
		bool empty() const {
			return liveThreadRef && liveThreadRef->thread() != NULL;
		}
	};

	// Registers a weak reference to a thread, ensuring that it isn't collected (sometimes)
	class WeakThreadRef
		: public rbx::quick_intrusive_ptr_target<WeakThreadRef>
		, boost::noncopyable
		, public Diagnostics::Countable<WeakThreadRef>
	{
		// TODO: boost::mutex would be safer
		typedef rbx::spin_mutex Mutex;
		static Mutex sync;
	public:
		class Node
			: public rbx::quick_intrusive_ptr_target<Node>
			, boost::noncopyable
		{
			friend class WeakThreadRef;
			WeakThreadRef* first;
		public:
			Node():first(0) {}
			~Node();
			static boost::intrusive_ptr<Node> create(lua_State* thread);
			static Node* get(lua_State* thread);

			// Clear all refs to thread and its children
			void eraseAllRefs();

			template<class Func>
			void forEachRefs(Func func)
			{
				for (WeakThreadRef* ref = first; ref!=NULL; ref = ref->next)
				{
					func(ref->lock());
				}
			}
		};
		friend class Node;
	private:
		WeakThreadRef* previous;
		WeakThreadRef* next;
		boost::intrusive_ptr<detail::LiveThreadRef> liveThreadRef;
		void addRef(lua_State* L);
		void addToNode();
		void removeFromNode();
	protected:
		Node* node;
		virtual void removeRef();
		lua_State* thread() const {
			return threadDangerous();
		}
	public:
		WeakThreadRef():node(0), previous(0), next(0) {}
		WeakThreadRef(lua_State* thread);
		WeakThreadRef(const WeakThreadRef& other);
		WeakThreadRef& operator=(const WeakThreadRef& other);
		virtual ~WeakThreadRef();

		bool operator==(const WeakThreadRef& other) const;
		bool operator!=(const WeakThreadRef& other) const;
		void reset();
		bool empty() const {
			return liveThreadRef ? liveThreadRef->thread()==0 : true;
		}
		ThreadRef lock()
		{
			return ThreadRef(liveThreadRef.get());
		}
		lua_State* threadDangerous() const {
			return liveThreadRef ? liveThreadRef->thread() : NULL;
		}

	};

	// A function that takes any number of arguments and returns a tuple
	typedef boost::function<shared_ptr<const Reflection::Tuple>(shared_ptr<const Reflection::Tuple>)> GenericFunction;
	
	class IAsyncResult
	{
	public:
		// This may throw
		virtual boost::shared_ptr<const Reflection::Tuple> getValue() = 0;
		virtual ~IAsyncResult() {}
	};
	// A function that takes any number of arguments and returns the result through a callback
	typedef boost::function<void(shared_ptr<const Reflection::Tuple>, boost::function<void(IAsyncResult*)>)> GenericAsyncFunction;

	class WeakFunctionRef : public WeakThreadRef
	{
	private:
		int functionId;			
		typedef WeakThreadRef Super;
	public:
		WeakFunctionRef():functionId(0) {}
		
		WeakFunctionRef(lua_State* thread, int index);	// Constructs a FunctionRef from the Lua stack
		
		virtual ~WeakFunctionRef();
		
		// Copy:
		WeakFunctionRef(const WeakFunctionRef& other);
		WeakFunctionRef& operator=(const WeakFunctionRef& other);

		// Query:
		bool operator==(const WeakFunctionRef& other) const;
		bool operator!=(const WeakFunctionRef& other) const;

		friend WeakFunctionRef lua_tofunction(lua_State* L);
		friend void lua_pushfunction(lua_State* L, const WeakFunctionRef& function);
	protected:
		virtual void removeRef();
	};

	// Operations with Lua
	WeakFunctionRef lua_tofunction(lua_State* L, int index);
	void lua_pushfunction(lua_State* L, const WeakFunctionRef& function);
	void lua_pushfunction(lua_State* L, shared_ptr<GenericFunction> function);
	void lua_pushfunction(lua_State* L, shared_ptr<GenericAsyncFunction> function);

	// One-shot wait slot (0x180AD0; binary layout {connection +0,
	// thread +4} = 28B). operator() queues behind resume/LuaArguments.
	class WaitScriptSlot
	{
	public:
		rbx::signals::connection connection;
		WeakThreadRef thread;
		WaitScriptSlot(lua_State* thread);
		void operator()(const Reflection::EventArguments& arguments);
	};

	// Firing slot (0x180B1C; binary layout {connection +0, context +4,
	// function +8 (WeakFunctionRef 28B), cachedSlotThread +36
	// (WeakThreadRef 24B), reentrancy +60} = 64B). The cached thread is
	// refreshed after clean runs, reset on yield/error (see operator()).
	// C1's cached init (0x180B1C, batch512) stores [D2C55E]+8 at +36
	// with +44/+48/+52/+56/+60 zeroed (+40 untouched) after the
	// diagnostic count++ -- i.e. the 24B cached slot arrives with null
	// links/live/node and a nonzero first word, which no mapped path
	// reads pre-overwrite (reset/assign/dtor/Asserts all tolerate it,
	// and the default ctor leaves exactly the observable state), so it
	// is default-constructed here. [D2C55E]'s identity stays queued
	// (relocated-ref audit; the flat read is unmapped).
	class FunctionScriptSlot
	{
	public:
		rbx::signals::connection connection;
		ScriptContext* context;
		WeakFunctionRef function;
		WeakThreadRef cachedSlotThread;
		int reentrancy;
		FunctionScriptSlot(lua_State* thread, int index);
		void operator()(const Reflection::EventArguments& arguments);
	};

	// Callback invoker (0x175E1A; LuaInstanceBridge.cpp -- defined in
	// script/ThreadRef.cpp pending that era). Invokes a Lua function with
	// Tuple args on the cached thread (or a fresh child) and returns the
	// results as a Tuple; throws on dead fn / error / yield.
	shared_ptr<Reflection::Tuple> callCallback(WeakFunctionRef function,
		shared_ptr<const Reflection::Tuple> args,
		boost::intrusive_ptr<WeakThreadRef> cachedThread);

} }
