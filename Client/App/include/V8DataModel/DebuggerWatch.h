/* V8DataModel/DebuggerWatch.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED FactoryProduct<Scripting::DebuggerWatch, Instance> 0x19B438 (Creator 0x19B708; flattened from Scripting ns);
     sDebuggerWatch = "DebuggerWatch".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/DebuggerWatch.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sDebuggerWatch;

	class DebuggerWatch
		: public DescribedCreatable<DebuggerWatch, Instance, sDebuggerWatch>
	{
	public:
		DebuggerWatch() {}
	};

} // namespace RBX
