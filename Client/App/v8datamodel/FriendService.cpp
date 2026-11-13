/* App/v8datamodel/FriendService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/FriendService.h"

namespace RBX {
	const char* const sFriendService = "FriendService";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::FriendService);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x006af1b0
// FUNCTION: STUDIO 0x0080dfb0
template<>
EnumDesc<FriendService::FriendStatus>::EnumDesc()
	: EnumDescriptor("FriendStatus", typeid(FriendService::FriendStatus))
{
	addPair(FriendService::FRIEND_STATUS_UNKNOWN, "Unknown");
	addPair(FriendService::FRIEND_STATUS_NOT_FRIEND, "NotFriend");
	addPair(FriendService::FRIEND_STATUS_FRIEND, "Friend");
	addPair(FriendService::FRIEND_STATUS_FRIEND_REQUEST_SENT, "FriendRequestSent");
	addPair(FriendService::FRIEND_STATUS_FRIEND_REQUEST_RECEIVED, "FriendRequestReceived");
}

template<>
const Type& Type::getSingleton<FriendService::FriendStatus>()
{
	return EnumDesc<FriendService::FriendStatus>::singleton();
}

template<> EnumRegistrar<FriendService::FriendStatus> EnumRegistrar<FriendService::FriendStatus>::registrar(0);
template<> TypeRegistrar<FriendService::FriendStatus> TypeRegistrar<FriendService::FriendStatus>::registrar(0);

// FUNCTION: PLAYER 0x006af2f0
// FUNCTION: STUDIO 0x0080e0f0
template<>
EnumDesc<FriendService::FriendEventType>::EnumDesc()
	: EnumDescriptor("FriendRequestEvent", typeid(FriendService::FriendEventType))
{
	addPair(FriendService::ISSUE_REQUEST, "Issue");
	addPair(FriendService::REVOKE_REQUEST, "Revoke");
	addPair(FriendService::ACCEPT_REQUEST, "Accept");
	addPair(FriendService::DENY_REQUEST, "Deny");
}

template<>
const Type& Type::getSingleton<FriendService::FriendEventType>()
{
	return EnumDesc<FriendService::FriendEventType>::singleton();
}

template<> EnumRegistrar<FriendService::FriendEventType> EnumRegistrar<FriendService::FriendEventType>::registrar(0);
template<> TypeRegistrar<FriendService::FriendEventType> TypeRegistrar<FriendService::FriendEventType>::registrar(0);

} // namespace Reflection
} // namespace RBX
