/* App/v8datamodel/GuiLabel.cpp — October 2013 (ctor shell).
   Evidence: GuiLabel C2 0x3AEE42.
   TODO: full decompile. */

#include "stdafx.h"

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

namespace RBX {

	typedef DescribedNonCreatable<GuiLabel, GuiObject, sGuiLabel, Reflection::ClassDescriptor::PERSISTENT> GuiLabelSuper;

	// mac: 0x3AEE42: GuiObject(name, false).
	GuiLabel::GuiLabel(const char* name)
		: GuiLabelSuper(name, false)
	{
	}

} // namespace RBX
