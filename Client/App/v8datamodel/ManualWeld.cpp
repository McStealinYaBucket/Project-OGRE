/* App/v8datamodel/ManualWeld.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/ManualWeld.h"

namespace RBX {
	const char* const sManualWeld = "ManualWeld";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::ManualWeld);
}
