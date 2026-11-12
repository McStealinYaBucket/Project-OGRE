/* V8DataModel/DebuggerManager.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED NonFactoryProduct<Scripting::DebuggerManager, Instance> 0x732FAC (flattened from Scripting ns);
     sDebuggerManager = "DebuggerManager".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/DebuggerManager.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sDebuggerManager;

	class DebuggerManager
		: public DescribedNonCreatable<DebuggerManager, Instance, sDebuggerManager>
	{
	public:
		DebuggerManager() {}
		static boost::shared_ptr<DebuggerManager> singleton();
	};

} // namespace RBX
