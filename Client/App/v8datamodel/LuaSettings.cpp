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
// LuaSettings registers in FactoryRegistration.cpp (2013: its classDescriptor copy 0x00788570).
}

namespace RBX {

// Out of line in 2013 (it follows the item ctor 0x0085e220 in LuaSettings.obj), which is why
// that ctor pushes the "LuaSettings" literal.
// FUNCTION: PLAYER 0x0085e360
// FUNCTION: STUDIO 0x00948980
LuaSettings::LuaSettings()
	: gcPause(200)
	, gcStepMul(200)
	, unknown104(0.03)
	, unknown112(2)
	, unknown116(0)
	, unknown120(false)
	, waitingThreadsBudget(0.1f)
{
	Instance::setName("Lua");
}

} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::LuaSettings, 0x88);
