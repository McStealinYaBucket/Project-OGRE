/* App/v8datamodel/DebugSettings.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/DebugSettings.h"

namespace RBX {
	const char* const sDebugSettings = "DebugSettings";
	// 2016-carry ("?" default, written by the build stamp at startup).
	std::string DebugSettings::robloxVersion = "?";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::DebugSettings);
}
