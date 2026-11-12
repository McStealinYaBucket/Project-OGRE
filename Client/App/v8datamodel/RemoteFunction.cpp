/* App/v8datamodel/RemoteFunction.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/RemoteFunction.h"

namespace RBX {
	const char* const sRemoteFunction = "RemoteFunction";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::RemoteFunction);
}
