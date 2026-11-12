/* App/v8datamodel/GuiLayerCollector.cpp — October 2013 (ctor shell).
   Evidence: GuiLayerCollector C2 0x6E5C74.
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/GuiLayerCollector.h"

namespace RBX {
	const char* const sLayerCollector = "LayerCollector";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::GuiLayerCollector);
}

namespace RBX {

	typedef DescribedNonCreatable<GuiLayerCollector, GuiBase2d, sLayerCollector, Reflection::ClassDescriptor::PERSISTENT> GuiLayerCollectorSuper;

	// mac: 0x6E5C74: GuiBase2d(name); pre-size the 2x11 layer grid.
	// STUB: PLAYER 0x008b4430
	// STUB: STUDIO 0x00988c30
	GuiLayerCollector::GuiLayerCollector(const char* name)
		: GuiLayerCollectorSuper(name)
	{
		for (int queue = 0; queue < GUIQUEUE_COUNT; ++queue)
			guiLayers[queue].resize(maxZIndex() + 1);
	}

} // namespace RBX
