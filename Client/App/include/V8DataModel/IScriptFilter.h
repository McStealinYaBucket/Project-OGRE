/* V8DataModel/IScriptFilter.h -- October 2013 (script-run filter + RuntimeScriptService).
   Evidence (batches 730-771):
     - BaseScript::computeNewWorkspace 0x1827A2 (batch740:443-475) and BaseScript::preloadCode
       0x18287E (batch740:478-504 / batch768:109-137) walk the script's ancestor chain with
       __dynamic_cast(ancestor, typeinfo Instance, typeinfo IScriptFilter, -1) and call
       scriptShouldRun(script) on the first hit.
     - ScriptContext::scriptShouldRun 0x1952EA (public; implemented in script/ScriptContext.cpp):
       return script != 0 && fastDynamicCast<CoreScript>(script) != 0.
     - RuntimeScriptService: ctor 0x1848EC (zeroes pendingScripts/runningScripts/isRunning,
       batch770:1132-1161), onServiceProvider 0x196AC4 (old -> disconnect new ->
       ServiceProvider::create<RunService> + runTransitionSignal.connect; batch770:502-630),
       onRunTransition adapter 0x19ABCC (tail-calls onRunState(newState); batch770:634-666),
       onRunState 0x196B9C (isRunning toggle: moving/stop swapped sets -> ScriptContext
       addScript/removeScript; batch770:668-830), runScript 0x19705A (batch770:640-735),
       releaseScript 0x19745C (batch770:955-1082). Binary layout: runTransitionConnection@+96,
       pendingScripts@+100, runningScripts@+124, isRunning byte@+148 (24-byte GCC sets).
       Implementation lives entirely in script/ScriptContext.cpp (extern sRuntimeScriptService
       + RBX_REGISTER_CLASS there).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/IScriptFilter.h */

#pragma once

#include <set>

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "Util/RunTransition.h"
#include "rbx/signal.h"

namespace RBX {

	extern const char* const sRuntimeScriptService;

	class BaseScript;

	// Consulted when a BaseScript joins the tree: computeNewWorkspace walks ancestors
	// and asks the first IScriptFilter found whether the script may run.
	// novtable in 2013: PLAYER has no IScriptFilter vftable, and the ScriptContext ctor never
	// stores one at its subobject (+0x60).
	class RBXInterface IScriptFilter
	{
		friend class BaseScript;
	protected:
		// mac: 0x1952EA (ScriptContext implements): script != 0 && fastDynamicCast<CoreScript>(script) != 0
		virtual bool scriptShouldRun(BaseScript* script) = 0;
	};

	// Service tracking which DataModel scripts should be running; forwards run/stop
	// to the DataModel's ScriptContext (implementation in script/ScriptContext.cpp).
	class RuntimeScriptService
		: public DescribedNonCreatable<RuntimeScriptService, Instance, sRuntimeScriptService, Reflection::ClassDescriptor::INTERNAL_LOCAL>
		, public Service
	{
	public:
		RuntimeScriptService();

		// mac: 0x19705A: CoreScript -> addScript immediately; otherwise
		// (Players::backendProcessing || isRunning) -> runningScripts + addScript,
		// else pendingScripts.
		void runScript(BaseScript* script);

		// mac: 0x19745C (7 asserts, batch770:955-1082 / batch771)
		void releaseScript(BaseScript* script);

		/*override*/ virtual void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider);

	private:
		// mac: 0x19ABCC: bridge { onRunState(event.newState); }
		void onRunTransition(RunTransition event);

		// mac: 0x196B9C: isRunning toggle moves the pending/running sets into the
		// ScriptContext via addScript/removeScript.
		void onRunState(RunState state);

		rbx::signals::scoped_connection runTransitionConnection;
		std::set<BaseScript*> pendingScripts;
		std::set<BaseScript*> runningScripts;
		bool isRunning;
	};

} // namespace RBX
