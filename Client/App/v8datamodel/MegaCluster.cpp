/* App/v8datamodel/MegaCluster.cpp — October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/MegaCluster.h"

namespace RBX {
	const char* const sMegaCluster = "Terrain";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::MegaClusterInstance);
}
