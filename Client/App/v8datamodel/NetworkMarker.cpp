/* App/v8datamodel/NetworkMarker.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/NetworkMarker.h"

namespace RBX {
	const char* const sMarker = "Marker";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::NetworkMarker);
}
