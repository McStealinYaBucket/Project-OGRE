/* App/v8datamodel/OverlayDataModel.cpp -- October 2013 (shell +
   registrar; body queued with the overlay era). */

#include "stdafx.h"

#include "V8DataModel/OverlayDataModel.h"

namespace RBX {
	const char* const sOverlayDataModel = "OverlayDataModel";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::OverlayDataModel);
}
