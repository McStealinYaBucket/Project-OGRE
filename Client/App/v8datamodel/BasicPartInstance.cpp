/* App/v8datamodel/BasicPartInstance.cpp — October 2013 STUB (sName + empty reflection).
   TODO: full decompile (legacy part-type surface). */

#include "stdafx.h"

#include "V8DataModel/BasicPartInstance.h"

namespace RBX {
	const char* const sBasicPart = "Part";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::BasicPartInstance);
}
