/* App/v8datamodel/Settings.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/Settings.h"

namespace RBX {
	const char* const sSettings = "Settings";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Settings);
}

namespace RBX {
	// The base DescribedNonCreatable<Settings, ServiceProvider, sSettings> ctor stays out of line
	// (PLAYER 0x00657cf0).
	// FUNCTION: PLAYER 0x00657f60
	// FUNCTION: STUDIO 0x00740700
	Settings::Settings(const std::string& settingsFile)
		: settingsFile(settingsFile)
		, settingsErased(false)
	{
	}
}
