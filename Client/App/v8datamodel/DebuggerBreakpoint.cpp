/* App/v8datamodel/DebuggerBreakpoint.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/DebuggerBreakpoint.h"

namespace RBX {
	const char* const sDebuggerBreakpoint = "DebuggerBreakpoint";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::DebuggerBreakpoint);
}
