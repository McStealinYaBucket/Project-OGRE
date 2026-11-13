/* App/v8datamodel/GuiBase3d.cpp -- October 2013. No 2013 assert names this file; the 2016 path. */

#include "stdafx.h"

#include "decomp.h"

#include "V8DataModel/GuiBase3d.h"

namespace RBX {
	const char* const sGuiBase3d = "GuiBase3d";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::GuiBase3d);
}

DECOMP_SIZE_ASSERT(RBX::GuiBase3d, 0x88);

namespace RBX {

	// FUNCTION: PLAYER 0x00953230
	// FUNCTION: STUDIO 0x00987010
	GuiBase3d::GuiBase3d(const char* name)
		: Super(name)
		, color(BrickColor::brick_23)
		, transparency(0.0f)
		, visible(true)
	{
	}

} // namespace RBX
