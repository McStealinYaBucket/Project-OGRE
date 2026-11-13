/* App/v8datamodel/SocialService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/SocialService.h"

namespace RBX {
	const char* const sSocialService = "SocialService";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::SocialService);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x007a5450
// FUNCTION: STUDIO 0x008b9b70
template<>
EnumDesc<SocialService::StuffType>::EnumDesc()
	: EnumDescriptor("Stuff", typeid(SocialService::StuffType))
{
	addPair(SocialService::HEADS_STUFF, "Heads");
	addPair(SocialService::FACES_STUFF, "Faces");
	addPair(SocialService::HATS_STUFF, "Hats");
	addPair(SocialService::T_SHIRTS_STUFF, "TShirts");
	addPair(SocialService::SHIRTS_STUFF, "Shirts");
	addPair(SocialService::PANTS_STUFF, "Pants");
	addPair(SocialService::GEARS_STUFF, "Gears");
	addPair(SocialService::TORSOS_STUFF, "Torsos");
	addPair(SocialService::LEFT_ARMS_STUFF, "LeftArms");
	addPair(SocialService::RIGHT_ARMS_STUFF, "RightArms");
	addPair(SocialService::LEFT_LEGS_STUFF, "LeftLegs");
	addPair(SocialService::RIGHT_LEGS_STUFF, "RightLegs");
	addPair(SocialService::BODIES_STUFF, "Bodies");
	addPair(SocialService::COSTUMES_STUFF, "Costumes");
}

template<>
const Type& Type::getSingleton<SocialService::StuffType>()
{
	return EnumDesc<SocialService::StuffType>::singleton();
}

template<> EnumRegistrar<SocialService::StuffType> EnumRegistrar<SocialService::StuffType>::registrar(0);
template<> TypeRegistrar<SocialService::StuffType> TypeRegistrar<SocialService::StuffType>::registrar(0);

} // namespace Reflection
} // namespace RBX
