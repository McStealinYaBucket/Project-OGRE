/* App/v8datamodel/StarterGuiService.cpp — October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/StarterGuiService.h"

namespace RBX {
	const char* const sStarterGuiService = "StarterGui";  // 0.125 metadata (was "StarterGuiService")
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::StarterGuiService);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x00709b20
// FUNCTION: STUDIO 0x007962b0
template<>
EnumDesc<StarterGuiService::CoreGuiType>::EnumDesc()
	: EnumDescriptor("CoreGuiType", typeid(StarterGuiService::CoreGuiType))
{
	addPair(StarterGuiService::COREGUI_PLAYERLISTGUI, "PlayerList");
	addPair(StarterGuiService::COREGUI_HEALTHGUI, "Health");
	addPair(StarterGuiService::COREGUI_BACKPACKGUI, "Backpack");
	addPair(StarterGuiService::COREGUI_CHATGUI, "Chat");
	addPair(StarterGuiService::COREGUI_ALL, "All");
}

template<>
const Type& Type::getSingleton<StarterGuiService::CoreGuiType>()
{
	return EnumDesc<StarterGuiService::CoreGuiType>::singleton();
}

template<> EnumRegistrar<StarterGuiService::CoreGuiType> EnumRegistrar<StarterGuiService::CoreGuiType>::registrar(0);
template<> TypeRegistrar<StarterGuiService::CoreGuiType> TypeRegistrar<StarterGuiService::CoreGuiType>::registrar(0);

} // namespace Reflection
} // namespace RBX
