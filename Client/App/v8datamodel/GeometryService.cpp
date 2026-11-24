/* App/v8datamodel/GeometryService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/GeometryService.h"

namespace RBX {
	const char* const sGeometryService = "Geometry";	// 2013 string (PLAYER doDeclare 0x0062e430 -> 0x00b79530 -> 0x00b45800)
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::GeometryService);
}
