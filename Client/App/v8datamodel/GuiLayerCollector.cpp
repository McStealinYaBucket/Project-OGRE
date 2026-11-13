/* App/v8datamodel/GuiLayerCollector.cpp -- October 2013. The asserts record this path
   (GuiLayerCollector.cpp:159, "!guiResponse.getTarget()"). */

#include "stdafx.h"

#include "V8DataModel/GuiLayerCollector.h"
#include "decomp.h"

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

DECOMP_SIZE_ASSERT(RBX::GuiLayerCollector, 0xb4);

namespace RBX {

	typedef DescribedNonCreatable<GuiLayerCollector, GuiBase2d, sLayerCollector, Reflection::ClassDescriptor::PERSISTENT> GuiLayerCollectorSuper;

	// The original also calls IAdornable's out-of-line dtor (PLAYER 0x009ca5c0), which GfxBase's
	// IAdornable does not declare yet.
	// STUB: PLAYER 0x008b4350
	// STUB: STUDIO 0x00988b50
	GuiLayerCollector::~GuiLayerCollector()
	{
		FASTLOG2(FLog::GuiTargetLifetime, "GuiLayerCollector destroyed: %p, GuiTarget pointer: %p", this, static_cast<GuiTarget*>(this));
	}

	// FUNCTION: PLAYER 0x008b4430
	// FUNCTION: STUDIO 0x00988c30
	GuiLayerCollector::GuiLayerCollector(const char* name)
		: GuiLayerCollectorSuper(name)
	{
		FASTLOG2(FLog::GuiTargetLifetime, "GuiLayerCollector created: %p, GuiTarget pointer: %p", this, static_cast<GuiTarget*>(this));
		for (int queue = 0; queue < GUIQUEUE_COUNT; ++queue)
			for (int z = minZIndex(); z <= maxZIndex(); ++z)
				guiLayers[queue].push_back(GuiVector());
	}

} // namespace RBX
