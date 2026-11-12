/* App/v8datamodel/ServerStorage.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/ServerStorage.h"

namespace RBX {
	const char* const sServerStorage = "ServerStorage";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::ServerStorage);
}
