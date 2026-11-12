/* App/v8datamodel/PersonalServerService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/PersonalServerService.h"

namespace RBX {
	const char* const sPersonalServerService = "PersonalServerService";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::PersonalServerService);
}
