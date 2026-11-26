/* App/v8datamodel/FriendService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/FriendService.h"
#include "Network/Players.h"

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

#include "decomp.h"

namespace RBX {

	// FUNCTION: PLAYER 0x006afb40
	// FUNCTION: STUDIO 0x0080e860
	void FriendService::playerRemoving(int userId)
	{
		if (Network::Players::backendProcessing(this, false))
		{
			FriendRequestSet::iterator iter = friendRequestSet.begin();
			FriendRequestSet::iterator end = friendRequestSet.end();
			while (iter != end)
			{
				if (iter->first == userId || iter->second == userId)
					friendRequestSet.erase(iter++);
				else
					++iter;
			}
		}

		FriendStatusMap::iterator iter = friendStatusTable.begin();
		FriendStatusMap::iterator end = friendStatusTable.end();
		while (iter != end)
		{
			if (iter->first == userId)
				friendStatusTable.erase(iter++);
			else
			{
				if (iter->first < userId)
				{
					FriendStatusInternalMap::iterator f = iter->second.find(userId);
					if (f != iter->second.end())
						iter->second.erase(f);
				}
				++iter;
			}
		}

		players.erase(userId);
	}

	// Not written yet: find<Network::Players>(this), then a Players call (PLAYER 0x00566790) that
	// the Network headers do not declare yet. STUB body so the bind target pairs.
	// STUB: PLAYER 0x006b14e0
	// STUB: STUDIO 0x008101e0
	void FriendService::friendEventReplicationChanged(int playerId, int otherPlayerId, FriendEventType eventType)
	{
		STUB(0x006b14e0);
	}

	// Not written yet: backendProcessing gate, a store (0x006b1500), then Players calls (0x0055f070).
	// STUB: PLAYER 0x006b2440
	// STUB: STUDIO 0x00811140
	void FriendService::friendStatusReplicationChanged(int playerId, int otherPlayerId, FriendStatus status)
	{
		STUB(0x006b2440);
	}

	// FUNCTION: PLAYER 0x006b2780
	// FUNCTION: STUDIO 0x00811480
	FriendService::FriendService()
		: enable(false)
	{
		setName("FriendService");
	}

	// FUNCTION: PLAYER 0x006b2b50
	// FUNCTION: STUDIO 0x00811850
	void FriendService::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
	{
		if (friendStatusConnection.connected())
			friendStatusConnection.disconnect();
		if (friendEventConnection.connected())
			friendEventConnection.disconnect();

		if (newProvider)
		{
			friendStatusConnection = friendStatusReplicatingSignal.connect(
				boost::bind(&FriendService::friendStatusReplicationChanged, this, _1, _2, _3));
			friendEventConnection = friendEventReplicatingSignal.connect(
				boost::bind(&FriendService::friendEventReplicationChanged, this, _1, _2, _3));
		}
	}

	// Not written yet (773 bytes: builds the friend-status request URL). STUB body so callers pair.
	// STUB: PLAYER 0x006b36a0
	// STUB: STUDIO 0x008123a0
	void FriendService::playerAdded(int userId)
	{
		STUB(0x006b36a0);
	}

} // namespace RBX
