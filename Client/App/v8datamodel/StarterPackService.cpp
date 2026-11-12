/* App/v8datamodel/StarterPackService.cpp — October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/StarterPackService.h"

namespace RBX {
	const char* const sStarterPackService = "StarterPack";  // 0.125 metadata (was "StarterPackService")
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::StarterPackService);
}
