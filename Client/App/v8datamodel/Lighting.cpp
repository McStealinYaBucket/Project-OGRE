/* App/v8datamodel/Lighting.cpp — October 2013. Child hooks from the PLAYER bytes; reflection,
   ctor and the sky/fog/time-of-day members not written yet. */

#include "stdafx.h"

#include "V8DataModel/Lighting.h"
#include "V8DataModel/Sky.h"

DECOMP_SIZE_ASSERT(RBX::Lighting, 0x1f0);

namespace RBX {
	const char* const sLighting = "Lighting";

	// FUNCTION: PLAYER 0x00706920
	// STUB: STUDIO 0x007d2750
	void Lighting::onChildChanged(Instance* instance, const PropertyChanged& event)
	{
		Super::onChildChanged(instance, event);
		if (sky.get() == instance)
			lightingChangedSignal(true);
	}

	// FUNCTION: PLAYER 0x007074c0
	// FUNCTION: STUDIO 0x007d3270
	bool Lighting::askAddChild(const Instance* instance) const
	{
		return Instance::fastDynamicCast<Sky>(instance) != NULL;
	}

	// onChildRemoving 0x007068f0 (sky.reset()) and onChildAdded 0x00707510 (sky = shared_from(sky))
	// are kept out: their shared_ptr<Sky>::reset / shared_from<Sky> instantiations join our linker's
	// folded template bodies and cost other functions their call pairing (measured 2026-11-13).
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Lighting);
}
