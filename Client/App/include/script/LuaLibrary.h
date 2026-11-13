/* script/LuaLibrary.h -- October 2013 (first import, section 241).
   Evidence: Bridge<Library> registerClass 0x171D12 + on_index(obj)
     0x62E3FA + on_newindex 0x62E9EC + on_gc 0x171E9C + on_eq 0x171ED6 +
     on_tostring x2 (0x171F4A/0x1729EE) + wrappers 0x197F4C/0x197FAC +
     className "RbxLibrary" (batch432 rodata); LibraryBridge::push
     0x62EE82 + find 0x62F030 + saveLibraryResult 0x62EA52 +
     registerClassLibrary 0x62F17A + registerLibraryTable (sub_62E99C);
     pushNewObject<Library> 0x631826; getApi (sub_62E62B);
     StringConverter<Library> 0x62E3DA (batches 426-434). 2016
     App/include/script/LuaLibrary.h carried for the skeleton with
     binary-pinned DELTAs (pushlstring length-aware forms; line-numbered
     assert gates; see the .cpp).
   Original file: Client/App/script/LuaLibrary.cpp (.h companion). */

#pragma once

#include "lua/luabridge.h"

#include "Util/AsyncHttpQueue.h"
#include "Util/ProtectedString.h"
#include "script/ThreadRef.h"

#include <string>
#include <list>
#include <map>
#include <set>
#include <boost/unordered_set.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

namespace RBX { namespace Lua {

	class Library
	{
		std::string libraryName;
	public:
		Library(std::string libraryName)
			:libraryName(libraryName)
		{};

		const std::string& getLibraryName() const { return libraryName; }
		bool operator==(const Library& other) const
		{
			return this->libraryName == other.libraryName;
		}
	};

	class LibraryBridge : public Bridge<Library>
	{
		friend class Bridge<Library>;
	public:
		static void registerClassLibrary(lua_State* L);
		static int find(lua_State* L, const std::string& libraryName);
		static void push(lua_State* L, const Library& item);
		static void saveLibraryResult(lua_State* L, int results, std::string libraryName);
	private:
		static void registerLibraryTable(lua_State* L);
		static int getApi(lua_State* L);
	};

	// Explicit-specialization declarations for the Library instantiation
	// (definitions in script/LuaLibrary.cpp; same convention as
	// LuaAtomicClasses.h).
	template<> void Bridge<Library>::registerClass(lua_State* L,
		int (*on_indexFn)(lua_State*), int (*on_newindexFn)(lua_State*));
	template<> int Bridge<Library>::on_index(const Library& object,
		const char* name, lua_State* L);
	template<> void Bridge<Library>::on_newindex(Library& object,
		const char* name, lua_State* L);
	template<> int Bridge<Library>::on_index(lua_State* L);
	template<> int Bridge<Library>::on_gc(lua_State* L);
	template<> int Bridge<Library>::on_eq(lua_State* L);
	template<> int Bridge<Library>::on_tostring(lua_State* L);
	template<> int Bridge<Library>::on_tostring(const Library& object,
		lua_State* L);

} // namespace Lua

	class Script;
	class ScriptContext;

	// RBX::LibraryService (0x90 bytes; binary home script/LuaLibrary.cpp
	// -- the 2013 asserts at :315/:420 carry this file's path; ctor via
	// ScriptContext 0x18F8EA).
	// 2013 layout (MSVC offsets; the old comments here were Mac offsets): markedLibraries is
	// a boost::unordered_set<std::string> (iOS 10-29 ctor/dtor callees are
	// boost::unordered::detail::table<set<..., std::string, boost::hash<std::string>, ...>>),
	// not 2016's std::set, so resumeCallbacks sits at 0x5c (queueResumeThread PLAYER
	// 0x0088dd71 reads [esi+0x60] after add esi,0x5c).
	// SIZE 0x68
	class LibraryService : boost::noncopyable
	{
	public:
		// { url +0x00, name +0x04, force +0x08 } (x86 COW string layout;
		// map-node access at +0x18 = name, batches 649/706).
		struct LibraryDefinition
		{
			std::string url;
			std::string name;
			bool force;

			LibraryDefinition() : force(false) {}
		};

		// 0xC bytes { thread +0, library +4, force +8 } (ctor 0x636DE0;
		// WeakThreadRef heap 0x18).
		class LibraryStateObject : boost::noncopyable
		{
			// issueDelayedLibraryRequest (LibraryService) reads these
			// parked-thread fields; nested->enclosing access is not
			// reciprocal in C++.
			friend class LibraryService;
		public:
			LibraryStateObject(lua_State* thread, const std::string& library, bool force)
				: thread(new Lua::WeakThreadRef(thread))
				, library(library)
				, force(force)
			{
			}

			static void resumeThread(boost::shared_ptr<LibraryStateObject> state);
			static void resumeThreadWithException(boost::shared_ptr<LibraryStateObject> state, std::string exception);
			static void justResume(boost::shared_ptr<LibraryStateObject> state, int results);

		private:
			boost::intrusive_ptr<Lua::WeakThreadRef> thread;	// +0x00
			std::string library;								// +0x04
			bool force;											// +0x08
		};

		explicit LibraryService(ScriptContext* context);
		~LibraryService();

		// mac: 0x630BA0: "Rbx"-prefixed names require WritePlayer; duplicate
		// registrations with a changed url/force throw.
		void registerLibrary(const std::string& name, const std::string& url, bool force);
		// mac: 0x63056C: development libraries (script sources). Key = the name
		// the caller passes (mangled (std::string const&, shared_ptr<Script>),
		// batch706); the ScriptContext wrapper forwards its first argument.
		void registerDevelopmentLibrary(const std::string& name, boost::shared_ptr<Script> script);

		// mac: 0x630560: the client calls this once the library set is final
		// (enables the delayed-queue pump in onHeartbeat).
		void markLibrariesLoaded() { librariesRegistered = true; }

		// Lua entry points (0x631664 / 0x630F1E / 0x630EB6).
		int requestLibrary(lua_State* L, const std::string& library, bool force);
		int tryRequestLibrary(lua_State* L, const std::string& library, bool force);
		int checkForLoadedLibrary(lua_State* L, const std::string& library);

		// Heartbeat pump: drains the delayed queue + resume callbacks.
		void onHeartbeat();

		// contentReady arms (0x62F920 remote / 0x62FE44 local).
		void contentReady(const std::string& library, const std::string& url,
			AsyncHttpQueue::RequestResult result, const std::string* response);
		void contentReadyLocal(const std::string& library, const ProtectedString& code);

	private:
		// mac: 0x62F89C: weak-locks the context and forwards the fetch result.
		static void ContentReadyHelper(boost::weak_ptr<ScriptContext> context,
			std::string library, std::string url,
			AsyncHttpQueue::RequestResult result, std::istream* stream,
			boost::shared_ptr<const std::string> response);
		// Heartbeat pump for one delayed request.
		void issueDelayedLibraryRequest(boost::shared_ptr<LibraryStateObject> state);
		// mac: 0x62F36C / 0x62F51C: push resume binders onto resumeCallbacks.
		void queueExceptionThread(boost::shared_ptr<LibraryStateObject> state, const std::string& exception);
		void queueResumeThread(boost::shared_ptr<LibraryStateObject> state);
		// mac: 0x62F600: fail every parked thread of one library.
		void resumeAllThreadsWithException(const std::string& library);
		// mac: 0x6306BA: admin-script substitution (BaseScript::adminScriptsPath).
		bool loadLocalLibrary(const std::string& library);
		// sub_630557: invoke every queued callback.
		static void invokeCallbacks(std::list<boost::function<void()> >& callbacks);

		ScriptContext* context;											// 0x00
		bool librariesRegistered;										// 0x04
		std::list<boost::shared_ptr<LibraryStateObject> > delayedQueue;	// 0x08
		std::map<std::string, LibraryDefinition> definitions;			// 0x14
		std::map<std::string, boost::shared_ptr<Script> > developmentLibraries;	// 0x20
		std::map<std::string, std::string> libraryExceptionMap;			// 0x2c
		std::map<std::string, std::list<boost::shared_ptr<LibraryStateObject> > > suspendedThreads;	// 0x38
		boost::unordered_set<std::string> markedLibraries;				// 0x44
		std::list<boost::function<void()> > resumeCallbacks;			// 0x5c  queueResumeThread 0x0088dd77
	};

} // namespace RBX
