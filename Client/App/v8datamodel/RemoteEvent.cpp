/* App/v8datamodel/RemoteEvent.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/RemoteEvent.h"

namespace RBX {
	const char* const sRemoteEvent = "RemoteEvent";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::RemoteEvent);
}
