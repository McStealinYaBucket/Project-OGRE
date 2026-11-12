/* App/v8datamodel/GuiRoot.cpp — October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/GuiRoot.h"

namespace RBX {
	const char* const sGuiRoot = "GuiRoot";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::GuiRoot);
}
