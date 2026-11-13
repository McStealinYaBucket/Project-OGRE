/* App/v8datamodel/GuiLabel.cpp — October 2013 (ctor shell).
   Evidence: GuiLabel C2 0x3AEE42.
   TODO: full decompile. */

#include "stdafx.h"
#include "decomp.h"

#include "V8DataModel/GuiLabel.h"

namespace RBX {
	const char* const sGuiLabel = "GuiLabel";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::GuiLabel);
}

DECOMP_SIZE_ASSERT(RBX::GuiLabel, 0x21c);

namespace RBX {

	typedef DescribedNonCreatable<GuiLabel, GuiObject, sGuiLabel, Reflection::ClassDescriptor::PERSISTENT> GuiLabelSuper;

	// FUNCTION: PLAYER 0x007cb640
	// FUNCTION: STUDIO 0x008e80f0
	GuiLabel::GuiLabel(const char* name)
		: GuiLabelSuper(name, false)
	{
	}

} // namespace RBX
