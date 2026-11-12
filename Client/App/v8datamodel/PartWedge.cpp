/* App/v8datamodel/PartWedge.cpp — October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/PartWedge.h"

namespace RBX {
namespace PART {
	const char* const sWedge = "WedgePart";
}
}

using namespace RBX;
using namespace RBX::PART;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::PART::Wedge);
}
