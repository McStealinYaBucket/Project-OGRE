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

// Registered right after OverlayDataModel's ConnectToGame flags (initializer PLAYER 0x00a6a1f4).
FASTFLAGVARIABLE(OverlayDataModelEnabled, false)	// PLAYER 0x00de98d0

#include "decomp.h"

namespace RBX {

	// Not written yet (weak_ptr store at +0xe58). STUB body so callers pair.
	// STUB: PLAYER 0x00652f00
	// STUB: STUDIO 0x007cba50
	void OverlayDataModel::FUN_00652f00(weak_ptr<DataModel> dataModel)
	{
		STUB(0x00652f00);
	}

	// Not written yet (FASTLOG, Creatable::create<OverlayDataModel>, doDataModelSetup). STUB body so
	// callers pair.
	// STUB: PLAYER 0x00693310
	// STUB: STUDIO 0x00850680
	shared_ptr<OverlayDataModel> OverlayDataModel::createOverlayDataModel(bool flag, Verb* lockVerb)
	{
		STUB(0x00693310);
		return shared_ptr<OverlayDataModel>();
	}

} // namespace RBX
