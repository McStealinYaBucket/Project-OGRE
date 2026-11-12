/* App/v8datamodel/SpawnerService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/SpawnerService.h"

namespace RBX {
	const char* const sSpawnerService = "SpawnerService";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::SpawnerService);
}

namespace RBX {

// mac: 0x2CF1D0 calls SpawnerService::ClearContents (capital C per binary). TODO: body.
void SpawnerService::ClearContents()
{
}

} // namespace RBX
