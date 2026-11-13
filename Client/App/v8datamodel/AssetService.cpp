/* App/v8datamodel/AssetService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/AssetService.h"

namespace RBX {
	const char* const sAssetService = "AssetService";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::AssetService);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x00849670
// FUNCTION: STUDIO 0x009837e0
template<>
EnumDesc<AssetService::AccessType>::EnumDesc()
	: EnumDescriptor("AccessType", typeid(AssetService::AccessType))
{
	addPair(AssetService::ME, "Me");
	addPair(AssetService::FRIENDS, "Friends");
	addPair(AssetService::EVERYONE, "Everyone");
	addPair(AssetService::INVITEONLY, "InviteOnly");
}

template<>
const Type& Type::getSingleton<AssetService::AccessType>()
{
	return EnumDesc<AssetService::AccessType>::singleton();
}

template<> EnumRegistrar<AssetService::AccessType> EnumRegistrar<AssetService::AccessType>::registrar(0);
template<> TypeRegistrar<AssetService::AccessType> TypeRegistrar<AssetService::AccessType>::registrar(0);

} // namespace Reflection
} // namespace RBX
