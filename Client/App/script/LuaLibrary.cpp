/* script/LuaLibrary.cpp -- October 2013 (first import, section 241).
   Evidence: see include/script/LuaLibrary.h header block.
   2013-vs-2016 DELTAs (all body-proven): length-aware pushlstring forms
   everywhere a library name crosses the stack (2016 uses pushstring);
   FLog::Asserts + debugHook gates carry line numbers from the 2013 file
   (getApi :44/:49/:50, on_index :96/:101/:102, push :239, find :194,
   saveLibraryResult :146/:171) with the binary's build-tree path literal;
   RBXASSERT_BALLANCED_LUA_STACK / _DEBUG-only top checks are dropped
   (dev-only, unresolvable here -- loud per policy).
   pushNewObject pointer-form note (0x631826 + 0x1648FC, batches 424/431):
   the binary passes a pointer and the callee copy-constructs the payload
   into the userdata (inlined moves for PODs, real copy ctor for string).
   The by-value pushNewObject(L, item) used below takes the identical
   path (copy ctor into fresh userdata + same metatable) -- byte-identical
   results, so no overload is added.
   Original file: Client/App/script/LuaLibrary.cpp. */

#include "stdafx.h"
#include "decomp.h"

#include "script/LuaLibrary.h"
#include "lua/lua.hpp"
#include "util/utilities.h"
#include "util/exception.h"
#include "util/standardout.h"
#include "RbxFormat.h"
#include "Util/Object.h"
#include "V8DataModel/ScriptContext.h"
#include "V8DataModel/ContentProvider.h"
#include "V8DataModel/LuaSettings.h"
#include "V8DataModel/Script.h"
#include "V8DataModel/BaseScript.h"
#include "security/SecurityContext.h"

#include <list>
#include <string>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <stdexcept>

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>

namespace RBX {

	// mac: 0x62E3DA (31B, batch434): plain name copy (carried 2016 shape).
	// At RBX scope (explicit specializations live with their template).
	template<>
	std::string StringConverter<Lua::Library>::convertToString(const Lua::Library& value)
	{
		return value.getLibraryName();
	}

namespace Lua {

	template<>
	// GLOBAL: PLAYER 0x00d8f33c
	// GLOBAL: STUDIO 0x00f2badc
	// Bridge<RBX::Lua::Library,1>::className
	const char* Bridge<Library, true>::className = "RbxLibrary";

	// mac: 0x197F4C (96B, batch434): luaL_checklstring wrapper verbatim.
	// FUNCTION: PLAYER 0x00671e20
	// FUNCTION: STUDIO 0x007d9d80
	template<>
	int Bridge<Library, true>::on_index(lua_State* L)
	{
		const char* name = luaL_checklstring(L, 2, NULL);
		return on_index(getObject(L, 1), name, L);
	}

	// Line-numbered assert gates shared by the LuaLibrary.cpp bodies (see
	// the SingletonBridge::push precedent in include/lua/luabridge.h;
	// ReleaseAssert is log-and-continue in this build). The binary threads
	// the 2013 build-tree path literal through every message (kept
	// verbatim); the line number is the only varying part.
	static const char* kLuaLibraryPath =
		"/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaLibrary.cpp";

	static void libraryNilGate(lua_State* L, int line)
	{
		if (FLog::Asserts != 0
			&& lua_type(L, -1) == LUA_TNIL
			&& (RBX::_internal::_debugHook == 0
				|| RBX::_internal::_debugHook("!lua_isnil( L, -1 )", kLuaLibraryPath, line) == 0))
		{
			char msg[256];
			_snprintf(msg, sizeof(msg),
				"!lua_isnil( L, -1 ) file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaLibrary.cpp line: %d", line);
			msg[sizeof(msg) - 1] = 0;
			ReleaseAssert(FLog::Asserts, msg);
		}
	}

	static void libraryTableGate(lua_State* L, int line)
	{
		if (FLog::Asserts != 0
			&& lua_type(L, -1) != LUA_TTABLE
			&& (RBX::_internal::_debugHook == 0
				|| RBX::_internal::_debugHook("lua_istable( L, -1 )", kLuaLibraryPath, line) == 0))
		{
			char msg[256];
			_snprintf(msg, sizeof(msg),
				"lua_istable( L, -1 ) file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaLibrary.cpp line: %d", line);
			msg[sizeof(msg) - 1] = 0;
			ReleaseAssert(FLog::Asserts, msg);
		}
	}

	// mac: 0x171D12 (394B, batch433): index/newindex/gc/eq/tostring + readonly.
	// FUNCTION: PLAYER 0x00880e60
	// FUNCTION: STUDIO 0x009e54e0
	template<>
	void Bridge<Library, true>::registerClass(lua_State* L,
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

	// sub_62E62B (881B, batch434): GetApi -- the library table's string
	// keys as a 1-based array.
	int LibraryBridge::getApi(lua_State* L)
	{
		std::list<std::string> api;
		{
			const Library& self = Bridge<Library>::getObject(L, 1);
			lua_pushlightuserdata(L, (void*)&registerLibraryTable);
			lua_rawget(L, LUA_REGISTRYINDEX);
			libraryNilGate(L, 44);
			{
				const std::string& s = self.getLibraryName();
				lua_pushlstring(L, s.c_str(), s.size());
			}
			lua_gettable(L, -2);
			libraryNilGate(L, 49);
			libraryTableGate(L, 50);
			int tableIndex = lua_gettop(L);
			lua_pushnil(L);
			while (lua_next(L, tableIndex) != 0)
			{
				if (lua_isstring(L, -2))
				{
					std::string key = safe_lua_tostring(L, -2);
					if (!key.empty())
						api.push_back(key);
				}
				lua_pop(L, 1);
			}
			lua_pop(L, 2);
		}
		lua_createtable(L, (int)api.size(), 0);
		int i = 1;
		for (std::list<std::string>::iterator iter = api.begin(); iter != api.end(); ++iter, ++i)
		{
			lua_pushnumber(L, i);
			lua_pushlstring(L, iter->c_str(), iter->size());
			lua_settable(L, -3);
		}
		return 1;
	}

	// mac: 0x62E9EC (102B, batch433): always throws, with the name.
	template<>
	void Bridge<Library, true>::on_newindex(Library& object,
		const char* name, lua_State* L)
	{
		throw RBX::runtime_error("%s cannot be assigned to", name);
	}

	// mac: 0x62EE82 (430B, batch428): string-keyed singleton push (the table
	// key is the library NAME, not the pointer -- unlike SharedPtrBridge).
	// STUB: PLAYER 0x00889630
	// STUB: STUDIO 0x009e5ed0
	void LibraryBridge::push(lua_State* L, const Library& item)
	{
		lua_pushlightuserdata(L, (void*)&push);
		lua_rawget(L, LUA_REGISTRYINDEX);
		libraryNilGate(L, 239);
		{
			const std::string& s = item.getLibraryName();
			lua_pushlstring(L, s.c_str(), s.size());
		}
		lua_rawget(L, -2);
		if (lua_type(L, -1) == LUA_TNIL)
		{
			lua_settop(L, -2);
			pushNewObject(L, item);
			{
				const std::string& s = item.getLibraryName();
				lua_pushlstring(L, s.c_str(), s.size());
			}
			lua_pushvalue(L, -2);
			lua_rawset(L, -4);
		}
		lua_remove(L, -2);
	}


	// mac: 0x171E9C (58B, batch433): checkudata + destroy. The binary destroys
	// the std::string member directly (~string on the payload address);
	// ~Library is the identical implicit dtor (single string member), so
	// the spelled form matches the construction path (copy-built Library).
	template<>
	int Bridge<Library, true>::on_gc(lua_State* L)
	{
		getObject(L, 1).~Library();
		return 0;
	}

	// mac: 0x171ED6 (115B, batch433): string compare (NOT pointer identity).
	template<>
	int Bridge<Library, true>::on_eq(lua_State* L)
	{
		lua_pushboolean(L, getObject(L, 1) == getObject(L, 2));
		return 1;
	}

	// mac: 0x171F4A (67B, batch433): checkudata + forward.
	template<>
	int Bridge<Library, true>::on_tostring(lua_State* L)
	{
		return on_tostring(getObject(L, 1), L);
	}

	// mac: 0x1729EE (170B, batch433): converter + pushlstring.
	template<>
	int Bridge<Library, true>::on_tostring(const Library& object,
		lua_State* L)
	{
		std::string s = RBX::StringConverter<Library>::convertToString(object);
		lua_pushlstring(L, s.c_str(), s.size());
		return 1;
	}

	// mac: 0x62F17A (84B, batch428): push-table + result-table.
	// FUNCTION: PLAYER 0x00889730
	// FUNCTION: STUDIO 0x009e5fd0
	void LibraryBridge::registerClassLibrary(lua_State* L)
	{
		lua_pushlightuserdata(L, (void*)&push);
		lua_newtable(L);
		lua_rawset(L, LUA_REGISTRYINDEX);
		registerLibraryTable(L);
	}

	// mac: 0x62EA52 (1071B, batch428): validate the loader result (exactly 1
	// table with a Help function) and file it under both registry tables.
	// STUB: PLAYER 0x00889a90
	// STUB: STUDIO 0x009e6330
	void LibraryBridge::saveLibraryResult(lua_State* L, int results, std::string libraryName)
	{
		std::string exception = "";
		if (results == 1)
		{
			if (lua_type(L, -1) == LUA_TTABLE)
			{
				lua_pushstring(L, "Help");
				lua_rawget(L, -2);
				if (lua_type(L, -1) != LUA_TFUNCTION)
					exception = "Libraries must contain a \"Help\" function";
				lua_settop(L, -2);
			}
			else
				exception = "Libraries should return exactly 1 table";
		}
		else
			exception = "Libraries should return exactly 1 result, and shouldn't wait";
		lua_pushlightuserdata(L, (void*)&registerLibraryTable);
		lua_rawget(L, LUA_REGISTRYINDEX);
		libraryNilGate(L, 146);
		if (!exception.empty())
		{
			lua_pushlstring(L, libraryName.c_str(), libraryName.size());
			lua_pushlstring(L, exception.c_str(), exception.size());
		}
		else
		{
			lua_pushstring(L, libraryName.c_str());
			lua_pushvalue(L, -3);
		}
		lua_rawset(L, -3);
		lua_settop(L, -2);
		if (!exception.empty())
		{
			lua_pushlightuserdata(L, (void*)&push);
			lua_rawget(L, LUA_REGISTRYINDEX);
			libraryNilGate(L, 171);
			lua_pushlstring(L, libraryName.c_str(), libraryName.size());
			lua_pushlstring(L, exception.c_str(), exception.size());
			lua_rawset(L, -3);
			lua_settop(L, -2);
		}
		else
		{
			push(L, Library(libraryName));
			lua_settop(L, -2);
		}
	}

	// mac: 0x62F030 (330B, batch428): 0 = absent, 2 = stored error string
	// (pushed nil + message), 1 = the library userdata.
	// STUB: PLAYER 0x00889d60
	// STUB: STUDIO 0x009e6600
	int LibraryBridge::find(lua_State* L, const std::string& libraryName)
	{
		lua_pushlightuserdata(L, (void*)&push);
		lua_rawget(L, LUA_REGISTRYINDEX);
		libraryNilGate(L, 194);
		lua_pushlstring(L, libraryName.c_str(), libraryName.size());
		lua_rawget(L, -2);
		if (lua_type(L, -1) == LUA_TNIL)
		{
			lua_settop(L, -3);
			return 0;
		}
		if (lua_isstring(L, -1))
		{
			lua_pushnil(L);
			lua_insert(L, lua_gettop(L) - 1);
			lua_remove(L, -3);
			return 2;
		}
		lua_remove(L, -2);
		return 1;
	}

	// sub_62E99C (80B, batch429): the library-result table.
	void LibraryBridge::registerLibraryTable(lua_State* L)
	{
		lua_pushlightuserdata(L, (void*)&registerLibraryTable);
		lua_newtable(L);
		lua_rawset(L, LUA_REGISTRYINDEX);
	}

	// mac: 0x62E3FA (561B, batch433): GetApi closure / Name string / registry-
	// table member lookup.
	// STUB: PLAYER 0x0088c6a0
	// STUB: STUDIO 0x009e9240
	template<>
	int Bridge<Library, true>::on_index(const Library& object,
		const char* name, lua_State* L)
	{
		if (strcmp(name, "GetApi") == 0)
		{
			lua_pushcfunction(L, LibraryBridge::getApi);
			return 1;
		}
		if (strcmp(name, "Name") == 0)
		{
			const std::string& s = object.getLibraryName();
			lua_pushlstring(L, s.c_str(), s.size());
			return 1;
		}
		lua_pushlightuserdata(L, (void*)&LibraryBridge::registerLibraryTable);
		lua_rawget(L, LUA_REGISTRYINDEX);
		libraryNilGate(L, 96);
		{
			const std::string& s = object.getLibraryName();
			lua_pushlstring(L, s.c_str(), s.size());
		}
		lua_gettable(L, -2);
		libraryNilGate(L, 101);
		libraryTableGate(L, 102);
		lua_pushstring(L, name);
		lua_gettable(L, -2);
		lua_remove(L, -2);
		lua_remove(L, -2);
		return 1;
	}

	// VS2012 odr-use holders (openState names the registerClass at 0x18B7EF).
	int (*volatile g_bridgeLibraryIndex)(lua_State*) =
		(int (*)(lua_State*))&Bridge<Library>::on_index;

} // namespace Lua

	// ---- LibraryService (section 241; binary home script/LuaLibrary.cpp:
	// the 2013 build's asserts at :315/:420 carry this file's path) ----

	namespace
	{
		// LuaLibrary.cpp:420 gate (contentReady's response guard): the
		// binary calls debugHook first, then ReleaseAssert with the
		// prebuilt literal (no FLog pre-check, per 0x62F94E).
		void libraryResponseGate(const std::string* response)
		{
			if (response != NULL)
				return;
			if (RBX::_internal::_debugHook == 0
				|| RBX::_internal::_debugHook("response", Lua::kLuaLibraryPath, 420) == 0)
				ReleaseAssert(FLog::Asserts,
					"response file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaLibrary.cpp line: 420");
		}

		// LuaLibrary.cpp:315 gate (RBXASSERT(results > 0) in resumeThread):
		// mirror of this file's nil-gate idiom.
		void libraryResultsGate(int results)
		{
			if (FLog::Asserts != 0 && results <= 0
				&& (RBX::_internal::_debugHook == 0
					|| RBX::_internal::_debugHook("results > 0", Lua::kLuaLibraryPath, 315) == 0))
			{
				char msg[256];
				_snprintf(msg, sizeof(msg),
					"results > 0 file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/script/LuaLibrary.cpp line: %d", 315);
				msg[sizeof(msg) - 1] = 0;
				ReleaseAssert(FLog::Asserts, msg);
			}
		}
	}

	// mac: 0x631CB2: lock the parked thread, find the saved results, resume.
	// STUB: PLAYER 0x0088ace0
	// STUB: STUDIO 0x009e7610
	void LibraryService::LibraryStateObject::resumeThread(boost::shared_ptr<LibraryStateObject> state)
	{
		Lua::ThreadRef thread = state->thread->lock();
		if (!thread.get())
			return;
		int results = Lua::LibraryBridge::find(thread.get(), state->library);
		libraryResultsGate(results);
		ScriptContext::getContext(thread.get())->resume(thread, results);
	}

	// mac: 0x631A3E: report + resume with the failure. The binary resumes with
	// 2 values; push nil + message onto the parked thread first (same
	// shape as on_ypcall_failure's push-then-resume).
	// mac: 0x631A3E (batch717): snapshot the parked top (0x631AB4), report the
	// failure (printf + printCallStack 0x631ADD), push nil + message
	// (pushlstring via data/size) and resume with 2 results, then restore
	// the top (lua_settop 0x631BE4).  Also reached from the request-script
	// failure continuation.
	void LibraryService::LibraryStateObject::resumeThreadWithException(
		boost::shared_ptr<LibraryStateObject> state, std::string exception)
	{
		Lua::ThreadRef thread = state->thread->lock();
		if (!thread.get())
			return;
		int savedTop = lua_gettop(thread.get());
		StandardOut::singleton()->printf(MESSAGE_ERROR, "%s", exception.c_str());
		ScriptContext::printCallStack(thread.get(), NULL);
		lua_pushnil(thread.get());
		lua_pushlstring(thread.get(), exception.data(), exception.size());
		ScriptContext::getContext(thread.get())->resume(thread, 2);
		lua_settop(thread.get(), savedTop);
	}

	// mac: 0x63276A: resume with the n results the caller already pushed onto
	// the parked thread (tryRequestLibrary's pushnil+message).
	void LibraryService::LibraryStateObject::justResume(boost::shared_ptr<LibraryStateObject> state, int results)
	{
		Lua::ThreadRef thread = state->thread->lock();
		if (!thread.get())
			return;
		ScriptContext::getContext(thread.get())->resume(thread, results);
	}

	LibraryService::LibraryService(ScriptContext* context)
		: context(context)
		, librariesRegistered(false)
	{
	}

	LibraryService::~LibraryService()
	{
	}

	// mac: 0x630EB6: parked-result lookup (exception map first, then Bridge).
	// STUB: PLAYER 0x0088c920
	// STUB: STUDIO 0x009e94c0
	int LibraryService::checkForLoadedLibrary(lua_State* L, const std::string& library)
	{
		std::map<std::string, std::string>::iterator found = libraryExceptionMap.find(library);
		if (found != libraryExceptionMap.end())
		{
			lua_pushnil(L);
			lua_pushstring(L, found->second.c_str());
			return 2;
		}
		return Lua::LibraryBridge::find(L, library);
	}

	// mac: 0x63056C: development-library validation + store. The name comes from
	// the caller (mangled (std::string const&, shared_ptr<Script>) -- batch706
	// xref 0x189CD1 from ScriptContext::registerDevelopmentLibrary).
	// FUNCTION: PLAYER 0x0088cce0
	// STUB: STUDIO 0x009e9800
	void LibraryService::registerDevelopmentLibrary(const std::string& name, boost::shared_ptr<Script> script)
	{
		if (definitions.find(name) != definitions.end())
			throw RBX::runtime_error("Library %s already registered to an assetId", name.c_str());
		if (developmentLibraries.find(name) != developmentLibraries.end())
			throw RBX::runtime_error("Library %s already registered to a script", name.c_str());
		if (name.find("Rbx") != std::string::npos)
			throw RBX::runtime_error("You can't register Development libraries that contain the letters 'Rbx'");
		developmentLibraries[name] = script;
	}

	// mac: 0x631664: immediate attempt for force requests; otherwise park on
	// the delayed queue until markLibrariesLoaded + heartbeat.
	int LibraryService::requestLibrary(lua_State* L, const std::string& library, bool force)
	{
		if (force)
		{
			int results = tryRequestLibrary(L, library, force);
			if (results != 0)
				return results;
			RobloxExtraSpace::get(L)->yieldCaptured = 1;
			boost::shared_ptr<LibraryStateObject> state(
				new LibraryStateObject(L, library, force));
			suspendedThreads[library].push_back(state);
		}
		else
		{
			RobloxExtraSpace::get(L)->yieldCaptured = 1;
			boost::shared_ptr<LibraryStateObject> state(
				new LibraryStateObject(L, library, force));
			delayedQueue.push_back(state);
		}
		return lua_yield(L, 0);
	}

	// mac: 0x630BA0: optional permission gate, duplicate-conflict throw,
	// optional printf (LuaSettings +0x78), then store under the normalized
	// key ("Rbx" tail when present; def.name keeps the original spelling
	// only when adminScriptsPath is live).
	// STUB: PLAYER 0x0088cfb0
	// STUB: STUDIO 0x009e9ad0
	void LibraryService::registerLibrary(const std::string& name, const std::string& url, bool force)
	{
		if (name.find("Rbx") == 0)
			Security::Context::current().requirePermission(Security::WritePlayer);

		std::map<std::string, LibraryDefinition>::iterator found = definitions.find(name);
		if (found != definitions.end()
			&& (found->second.url != url || found->second.force != force))
			throw RBX::runtime_error("Library %s already registered to a different assetId", name.c_str());

		if (LuaSettings::singleton().getUnknown120())
			// DELTA: binary vararg trace shows two string pointers here
			// (format 'RegisterLibrary %d "%s"' @0xB79195); log-only,
			// no behavioral impact.
			StandardOut::singleton()->printf(MESSAGE_INFO,
				"RegisterLibrary %d \"%s\"", url.c_str(), name.c_str());

		std::string normalized = name;
		size_t pos = normalized.find("Rbx");
		if (pos != std::string::npos)
			normalized = name.substr(pos);
		definitions[normalized].url = url;
		definitions[normalized].force = force;
		if (!BaseScript::adminScriptsPath.empty())
			definitions[normalized].name = name;
	}

	// mac: 0x62F51C: queue a plain resume binder.
	// FUNCTION: PLAYER 0x0088dd00
	// FUNCTION: STUDIO 0x009ea820
	void LibraryService::queueResumeThread(boost::shared_ptr<LibraryStateObject> state)
	{
		resumeCallbacks.push_back(boost::bind(&LibraryStateObject::resumeThread, state));
	}

	// mac: 0x62F600: fail every parked thread of one library. The message is
	// read back from libraryExceptionMap (call sites assign just before;
	// a miss yields the empty message -- binary-identical default).
	// STUB: PLAYER 0x0088e620
	// STUB: STUDIO 0x009eb140
	void LibraryService::resumeAllThreadsWithException(const std::string& library)
	{
		// BINARY DELTA: the binary head-tests a global byte at 0xEAD06C
		// before the lookup (unresolved import-table pointer; both arms
		// converge on this map read + empty default).
		std::string exception;
		std::map<std::string, std::string>::iterator found = libraryExceptionMap.find(library);
		if (found != libraryExceptionMap.end())
			exception = found->second;

		std::list<boost::shared_ptr<LibraryStateObject> >& threads = suspendedThreads[library];
		for (std::list<boost::shared_ptr<LibraryStateObject> >::iterator it = threads.begin();
			it != threads.end(); ++it)
			queueExceptionThread(*it, exception);
		suspendedThreads.erase(library);
	}

	// mac: 0x62FE44: local/dev source completion (normalized key throughout).
	// DELTA: takes ProtectedString instead of the 2016 flyweight type
	// (unobservable -- same source payload).
	// STUB: PLAYER 0x0088e760
	// STUB: STUDIO 0x009eb280
	void LibraryService::contentReadyLocal(const std::string& library, const ProtectedString& code)
	{
		std::string normalized = library;
		size_t pos = normalized.find("Rbx");
		if (pos != std::string::npos)
			normalized = library.substr(pos);

		if (markedLibraries.find(normalized) != markedLibraries.end())
			return;
		markedLibraries.insert(normalized);

		try
		{
			context->addLibrary(normalized, code);

			std::list<boost::shared_ptr<LibraryStateObject> >& threads = suspendedThreads[normalized];
			std::for_each(threads.begin(), threads.end(),
				boost::bind(&LibraryService::queueResumeThread, this, _1));
			suspendedThreads.erase(normalized);
		}
		catch (std::exception& e)
		{
			libraryExceptionMap[normalized].assign(
				std::string("Library failed during loading: ") + e.what());
			resumeAllThreadsWithException(normalized);
		}
	}

	// mac: 0x62F36C: queue a resume-with-exception binder.
	void LibraryService::queueExceptionThread(boost::shared_ptr<LibraryStateObject> state, const std::string& exception)
	{
		resumeCallbacks.push_back(
			boost::bind(&LibraryStateObject::resumeThreadWithException, state, exception));
	}

	// mac: 0x62F920: remote fetch completion (raw library key throughout).
	// STUB: PLAYER 0x0088e990
	// STUB: STUDIO 0x009eb4b0
	void LibraryService::contentReady(const std::string& library, const std::string& url,
		AsyncHttpQueue::RequestResult result, const std::string* response)
	{
		if (result != AsyncHttpQueue::Succeeded)
		{
			libraryExceptionMap[library].assign("Library failed to load", 22);
			resumeAllThreadsWithException(library);
			return;
		}

		libraryResponseGate(response);

		if (markedLibraries.find(library) != markedLibraries.end())
			return;
		markedLibraries.insert(library);

		try
		{
			const char* body = response->c_str();
			if (!url.empty())
				body = ContentProvider::verifyRequestedScriptSignature(body, url, true);
			context->addLibrary(library, ProtectedString::fromTrustedSource(std::string(body)));

			std::list<boost::shared_ptr<LibraryStateObject> >& threads = suspendedThreads[library];
			std::for_each(threads.begin(), threads.end(),
				boost::bind(&LibraryService::queueResumeThread, this, _1));
			suspendedThreads.erase(library);
		}
		catch (std::runtime_error&)
		{
			libraryExceptionMap[library].assign("Library failed signature check", 30);
			resumeAllThreadsWithException(library);
		}
		catch (std::exception& e)
		{
			libraryExceptionMap[library].assign(
				std::string("Library failed during loading: ") + e.what());
			resumeAllThreadsWithException(library);
		}
	}

	// mac: 0x62F89C: weak-lock the context and forward the fetch completion
	// (binary passes response.px; no libraryService null check).
	// STUB: PLAYER 0x0088edf0
	// STUB: STUDIO 0x009eb910
	void LibraryService::ContentReadyHelper(boost::weak_ptr<ScriptContext> context,
		std::string library, std::string url,
		AsyncHttpQueue::RequestResult result, std::istream* stream,
		boost::shared_ptr<const std::string> response)
	{
		(void)stream;	// binary never forwards the istream
		if (boost::shared_ptr<ScriptContext> strong = context.lock())
			strong->libraryService->contentReady(library, url, result, response.get());
	}

	// Heartbeat pump for one delayed request: relock, retry; a live result
	// resumes inline, otherwise the thread parks under the library key.
	void LibraryService::issueDelayedLibraryRequest(boost::shared_ptr<LibraryStateObject> state)
	{
		Lua::ThreadRef thread = state->thread->lock();
		if (!thread.get())
			return;
		int results = tryRequestLibrary(thread.get(), state->library, state->force);
		if (results != 0)
			resumeCallbacks.push_back(
				boost::bind(&LibraryStateObject::justResume, state, results));
		else
			suspendedThreads[state->library].push_back(state);
	}

	// LibraryService heartbeat: drain the delayed queue once registered,
	// then invoke the queued resume callbacks (binary iterates the
	// original list then clears; the swap keeps identical order +
	// reentrancy safety -- issueDelayedLibraryRequest never touches
	// delayedQueue).
	void LibraryService::onHeartbeat()
	{
		if (librariesRegistered)
		{
			std::list<boost::shared_ptr<LibraryStateObject> > queue;
			queue.swap(delayedQueue);
			for (std::list<boost::shared_ptr<LibraryStateObject> >::iterator it = queue.begin();
				it != queue.end(); ++it)
				issueDelayedLibraryRequest(*it);
		}
		if (!resumeCallbacks.empty())
		{
			std::list<boost::function<void()> > callbacks;
			callbacks.swap(resumeCallbacks);
			invokeCallbacks(callbacks);
		}
	}

	// sub_630557: invoke every queued callback.
	void LibraryService::invokeCallbacks(std::list<boost::function<void()> >& callbacks)
	{
		for (std::list<boost::function<void()> >::iterator it = callbacks.begin();
			it != callbacks.end(); ++it)
			(*it)();
	}

	// mac: 0x6306BA: admin-script substitution root lookup + load.
	// STUB: PLAYER 0x0088ef10
	// STUB: STUDIO 0x009eba30
	bool LibraryService::loadLocalLibrary(const std::string& library)
	{
		boost::filesystem::path path = boost::filesystem::initial_path();
		path /= BaseScript::adminScriptsPath;
		path /= library + ".lua";

		boost::filesystem::file_status status = boost::filesystem::status(path);
		if (status.type() <= boost::filesystem::file_not_found)
		{
			StandardOut::singleton()->printf(MESSAGE_WARNING,
				"Missing LuaLibrary substitution: \"%s\"", path.string().c_str());
			return false;
		}

		StandardOut::singleton()->printf(MESSAGE_INFO,
			"Substituting LuaLibrary \"%s\" with \"%s\"",
			library.c_str(), path.string().c_str());

		std::ifstream stream(path.string().c_str());
		std::string content((std::istreambuf_iterator<char>(stream)),
			std::istreambuf_iterator<char>());
		contentReadyLocal(library, ProtectedString::fromTrustedSource(content));
		return true;
	}

	// mac: 0x630F1E: single-shot attempt; 0 means "still in flight" (the
	// caller parks), otherwise the Lua result count.
	// STUB: PLAYER 0x0088f240
	// STUB: STUDIO 0x009ebd60
	int LibraryService::tryRequestLibrary(lua_State* L, const std::string& library, bool force)
	{
		if (definitions.find(library) != definitions.end()
			&& definitions[library].force && !force)
		{
			lua_pushnil(L);
			std::string message = RBX::format(
				"Library %s is restricted to Roblox developers", library.c_str());
			lua_pushstring(L, message.c_str());
			return 2;
		}

		int results = checkForLoadedLibrary(L, library);
		if (results != 0)
			return results;

		if (definitions.find(library) == definitions.end()
			&& developmentLibraries.find(library) == developmentLibraries.end())
		{
			lua_pushnil(L);
			std::string message = RBX::format("Unknown library %s", library.c_str());
			lua_pushstring(L, message.c_str());
			return 2;
		}

		if (developmentLibraries.find(library) != developmentLibraries.end())
		{
			contentReadyLocal(library, developmentLibraries[library]->getEmbeddedCodeSafe());
			return checkForLoadedLibrary(L, library);
		}

		if (!BaseScript::adminScriptsPath.empty())
		{
			if (loadLocalLibrary(definitions[library].name))
				return checkForLoadedLibrary(L, library);
		}

		ContentProvider* provider = ServiceProvider::create<ContentProvider>(context);
		ContentId contentId(RBX::format("rbxassetid://%s",
			definitions[library].url.c_str()));
		provider->getContent(contentId, ContentProvider::PRIORITY_SCRIPT,
			boost::bind(&LibraryService::ContentReadyHelper, weak_from(context),
				library, definitions[library].url, _1, _2, _3),
			AsyncHttpQueue::AsyncWrite);
		return checkForLoadedLibrary(L, library);
	}

} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::LibraryService, 0x68);
