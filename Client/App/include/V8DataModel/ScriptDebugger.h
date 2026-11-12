/* V8DataModel/ScriptDebugger.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED FactoryProduct<Scripting::ScriptDebugger, Instance> 0x178F80 (flattened from Scripting ns);
     sScriptDebugger = "ScriptDebugger".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ScriptDebugger.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sScriptDebugger;

	class ScriptDebugger
		: public DescribedCreatable<ScriptDebugger, Instance, sScriptDebugger>
	{
	public:
		ScriptDebugger() {}
	};

} // namespace RBX
