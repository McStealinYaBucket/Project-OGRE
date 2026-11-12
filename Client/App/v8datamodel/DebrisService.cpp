/* App/v8datamodel/DebrisService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/DebrisService.h"

namespace RBX {
	const char* const sDebrisService = "Debris";  // 0.125 metadata + 2016 value (was "DebrisService")
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::DebrisService);
}
