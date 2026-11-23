/* App/v8datamodel/LuaSettings.cpp -- October 2013 (Lua GC tuning item).
   See include/V8DataModel/LuaSettings.h for evidence. */

#include "stdafx.h"

#include "V8DataModel/LuaSettings.h"
#include "decomp.h"

namespace RBX {
	const char* const sLuaSettings = "LuaSettings";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::LuaSettings);
}

DECOMP_SIZE_ASSERT(RBX::LuaSettings, 0x88);
