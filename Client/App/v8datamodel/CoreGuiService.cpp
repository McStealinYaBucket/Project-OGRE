/* App/v8datamodel/CoreGuiService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/CoreGuiService.h"

namespace RBX {
	const char* const sCoreGuiService = "CoreGui";  // 0.125 metadata (was "CoreGuiService")
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::CoreGuiService);
}

namespace RBX {

	// mac: 0x45E19E: create ScreenGui + name "RobloxGui" + robloxLock + parent.
	// STUB: PLAYER 0x00708d80
	// STUB: STUDIO 0x00795a60
	void CoreGuiService::createRobloxScreenGui()
	{
		robloxGui = Creatable<Instance>::create<ScreenGui>();
		robloxGui->setName("RobloxGui");
		robloxGui->setRobloxLocked(true);
		robloxGui->setParent(this);
	}

} // namespace RBX
