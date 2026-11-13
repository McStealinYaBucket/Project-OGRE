/* App/v8datamodel/TextLabel.cpp -- October 2013. No 2013 assert names this file; the 2016 path. */

#include "stdafx.h"
#include "decomp.h"

#include "V8DataModel/TextLabel.h"
#include "Util/BrickColor.h"

namespace RBX {
	const char* const sTextLabel = "TextLabel";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::TextLabel);
}

DECOMP_SIZE_ASSERT(RBX::GuiTextMixin, 0x54);
DECOMP_SIZE_ASSERT(RBX::TextLabel, 0x270);

namespace RBX {

	// FUNCTION: PLAYER 0x006c1c10
	// FUNCTION: STUDIO 0x00854b20
	TextLabel::TextLabel()
		: Super("TextLabel")
		, GuiTextMixin("Label", BrickColor(BrickColor::brick_26).color3())
	{
		setGuiQueue(GUIQUEUE_TEXT);
	}

} // namespace RBX
