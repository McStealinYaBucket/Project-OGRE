/* V8DataModel/DebuggerBreakpoint.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED FactoryProduct<Scripting::DebuggerBreakpoint, Instance> 0x3661EE (Creator 0x72E806; flattened from Scripting ns);
     sDebuggerBreakpoint = "DebuggerBreakpoint".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/DebuggerBreakpoint.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sDebuggerBreakpoint;

	class DebuggerBreakpoint
		: public DescribedCreatable<DebuggerBreakpoint, Instance, sDebuggerBreakpoint>
	{
	public:
		DebuggerBreakpoint() {}
	};

} // namespace RBX
