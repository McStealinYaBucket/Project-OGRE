/* script/ExitHandlers.h -- October 2013 reconstruction.
   What a script thread does when it finishes. The handler signatures come
   from iOS 10-29: RBX::Lua::Continuations::Continuations(Scripts::Continuations
   const&), onSuccessHandler(lua_State*, boost::function<void
   (shared_ptr<Reflection::Tuple const>)>) and onErrorHandler(lua_State*,
   boost::function<void (char const*, char const*, shared_ptr<BaseScript>,
   int)>). Member order (success first, then error) is what ScriptContext.cpp
   builds on. */
#pragma once

#include "boost/function.hpp"
#include "reflection/Type.h"

struct lua_State;

namespace RBX {

class BaseScript;

namespace Scripts {

	typedef boost::function<void(shared_ptr<const Reflection::Tuple>)> SuccessHandler;
	typedef boost::function<void(const char* message, const char* callStack,
	                             shared_ptr<BaseScript> script, int line)> ErrorHandler;

	// Caller-facing pair of completion callbacks.
	struct Continuations
	{
		SuccessHandler successHandler;
		ErrorHandler errorHandler;

		bool empty() const { return successHandler.empty() && errorHandler.empty(); }
	};

} // namespace Scripts

namespace Lua {

	// The same pair bound to the finishing thread; ScriptContext::resume fires one.
	class Continuations
	{
	public:
		Continuations() {}
		Continuations(const Scripts::Continuations& handlers);

		boost::function<void(lua_State*)> success; // thread returned normally
		boost::function<void(lua_State*)> error;   // thread raised an error

	private:
		static void onSuccessHandler(lua_State* L, Scripts::SuccessHandler handler);
		static void onErrorHandler(lua_State* L, Scripts::ErrorHandler handler);
	};

} // namespace Lua

} // namespace RBX
