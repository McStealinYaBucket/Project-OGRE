/* App/v8datamodel/SkateboardPlatform.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/SkateboardPlatform.h"

namespace RBX {
	const char* const sSkateboardPlatform = "SkateboardPlatform";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::SkateboardPlatform);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x00933490
// FUNCTION: STUDIO 0x0095b700
template<>
EnumDesc<SkateboardPlatform::MoveState>::EnumDesc()
	: EnumDescriptor("MoveState", typeid(SkateboardPlatform::MoveState))
{
	addPair(SkateboardPlatform::STOPPED, "Stopped");
	addPair(SkateboardPlatform::COASTING, "Coasting");
	addPair(SkateboardPlatform::PUSHING, "Pushing");
	addPair(SkateboardPlatform::STOPPING, "Stopping");
	addPair(SkateboardPlatform::AIR_FREE, "AirFree");
}

template<>
const Type& Type::getSingleton<SkateboardPlatform::MoveState>()
{
	return EnumDesc<SkateboardPlatform::MoveState>::singleton();
}

template<> EnumRegistrar<SkateboardPlatform::MoveState> EnumRegistrar<SkateboardPlatform::MoveState>::registrar(0);
template<> TypeRegistrar<SkateboardPlatform::MoveState> TypeRegistrar<SkateboardPlatform::MoveState>::registrar(0);

} // namespace Reflection
} // namespace RBX
