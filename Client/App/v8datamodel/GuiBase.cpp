/* App/v8datamodel/GuiBase.cpp — October 2013 (ctor shell).
   Evidence: GuiBase C2 0x3949CA (Instance(name, NULL) + members).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/GuiBase.h"

namespace RBX {
	const char* const sGuiBase = "GuiBase";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::GuiBase);
}

namespace RBX {

	// mac: 0x3949CA: Instance(name, NULL); +25/+26/+27 = -1; +28/+29 = 0.
	GuiBase::GuiBase(const char* name)
		: DescribedNonCreatable<GuiBase, Instance, sGuiBase, Reflection::ClassDescriptor::PERSISTENT>(name, (FWInstance*)NULL)
		, unknown25(-1)
		, unknown26(-1)
		, unknown27(-1)
		, unknown28(0)
		, unknown29(0)
	{
	}

} // namespace RBX
