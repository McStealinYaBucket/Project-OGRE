/* App/v8datamodel/PersonalServerService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/PersonalServerService.h"

namespace RBX {
	const char* const sPersonalServerService = "PersonalServerService";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::PersonalServerService);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x007a71d0
// FUNCTION: STUDIO 0x008c3790
template<>
EnumDesc<PersonalServerService::PrivilegeType>::EnumDesc()
	: EnumDescriptor("PrivilegeType", typeid(PersonalServerService::PrivilegeType))
{
	addPair(PersonalServerService::PRIVILEGE_OWNER, "Owner");
	addPair(PersonalServerService::PRIVILEGE_ADMIN, "Admin");
	addPair(PersonalServerService::PRIVILEGE_MEMBER, "Member");
	addPair(PersonalServerService::PRIVILEGE_VISITOR, "Visitor");
	addPair(PersonalServerService::PRIVILEGE_BANNED, "Banned");
}

template<>
const Type& Type::getSingleton<PersonalServerService::PrivilegeType>()
{
	return EnumDesc<PersonalServerService::PrivilegeType>::singleton();
}

template<> EnumRegistrar<PersonalServerService::PrivilegeType> EnumRegistrar<PersonalServerService::PrivilegeType>::registrar(0);
template<> TypeRegistrar<PersonalServerService::PrivilegeType> TypeRegistrar<PersonalServerService::PrivilegeType>::registrar(0);

} // namespace Reflection
} // namespace RBX
