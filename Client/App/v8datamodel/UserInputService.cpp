/* App/v8datamodel/UserInputService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/UserInputService.h"

namespace RBX {
	const char* const sUserInputService = "UserInputService";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::UserInputService);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x006a45d0
// FUNCTION: STUDIO 0x00840650
template<>
EnumDesc<UserInputService::SwipeDirection>::EnumDesc()
	: EnumDescriptor("SwipeDirection", typeid(UserInputService::SwipeDirection))
{
	addPair(UserInputService::DIRECTION_RIGHT, "Right");
	addPair(UserInputService::DIRECTION_LEFT, "Left");
	addPair(UserInputService::DIRECTION_UP, "Up");
	addPair(UserInputService::DIRECTION_DOWN, "Down");
}

template<>
const Type& Type::getSingleton<UserInputService::SwipeDirection>()
{
	return EnumDesc<UserInputService::SwipeDirection>::singleton();
}

template<> EnumRegistrar<UserInputService::SwipeDirection> EnumRegistrar<UserInputService::SwipeDirection>::registrar(0);
template<> TypeRegistrar<UserInputService::SwipeDirection> TypeRegistrar<UserInputService::SwipeDirection>::registrar(0);

} // namespace Reflection
} // namespace RBX
