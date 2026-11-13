/* App/v8datamodel/GuiBase.cpp -- October 2013.
   No 2013 assert names this file; the 2016 path. In PLAYER its .obj holds Name::doDeclare<sGuiBase>
   0x007bc370, the Described ctor 0x007bc440 and the GuiBase ctor 0x007bc4b0. */

#include "stdafx.h"

#include "decomp.h"

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

DECOMP_SIZE_ASSERT(RBX::GuiBase, 0x7c);

namespace RBX {

	// FUNCTION: PLAYER 0x007bc4b0
	// FUNCTION: STUDIO 0x008d8ed0
	GuiBase::GuiBase(const char* name)
		: Super(name)
		, m_unk0x78(0.0f)
	{
	}

} // namespace RBX
