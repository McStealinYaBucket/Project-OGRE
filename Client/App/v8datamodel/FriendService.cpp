/* App/v8datamodel/FriendService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/FriendService.h"
#include "Network/Players.h"
#include "Util/Http.h"
#include "RbxFormat.h"
#include <sstream>

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

	// FUNCTION: PLAYER 0x006b14e0
	// FUNCTION: STUDIO 0x008101e0
	void FriendService::friendEventReplicationChanged(int playerId, int otherPlayerId, FriendEventType eventType)
	{
		if (Network::Players* players = ServiceProvider::find<Network::Players>(this))
			players->friendEventFired(playerId, otherPlayerId, eventType);
	}

	// Not written yet (stores friendStatusTable[small][big] and replicates). STUB body so callers pair;
	// __declspec(noinline) is a stand-in until the real body lands (VC9 inlines a same-TU STUB).
	// STUB: PLAYER 0x006b1500
	// STUB: STUDIO 0x00810200
	__declspec(noinline) void FriendService::storeAndReplicateFriendStatus(int playerId, int otherPlayerId, FriendStatus status)
	{
		STUB(0x006b1500);
	}

	static FriendService::FriendStatus InvertFriendStatus(FriendService::FriendStatus status)
	{
		switch (status)
		{
		case FriendService::FRIEND_STATUS_FRIEND_REQUEST_SENT:
			return FriendService::FRIEND_STATUS_FRIEND_REQUEST_RECEIVED;
		case FriendService::FRIEND_STATUS_FRIEND_REQUEST_RECEIVED:
			return FriendService::FRIEND_STATUS_FRIEND_REQUEST_SENT;
		default:
			return status;
		}
	}

	// FUNCTION: PLAYER 0x006b2440
	// FUNCTION: STUDIO 0x00811140
	void FriendService::friendStatusReplicationChanged(int playerId, int otherPlayerId, FriendStatus status)
	{
		if (!Network::Players::backendProcessing(this, false))
			storeAndReplicateFriendStatus(playerId, otherPlayerId, status);

		if (Network::Players* players = ServiceProvider::find<Network::Players>(this))
		{
			players->friendStatusChanged(playerId, otherPlayerId, status);
			players->friendStatusChanged(otherPlayerId, playerId, InvertFriendStatus(status));
		}
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

	// Body not written (the async bulk-friends response handler). STUB body so callers pair.
	// STUB: PLAYER 0x006b2c80
	// STUB: STUDIO 0x00811980
	void FriendService::ProcessBulkFriendResponse(weak_ptr<FriendService> weakFriendService, int userId,
		std::set<int> userIdsRequested, std::string* response, std::exception* error)
	{
		STUB(0x006b2c80);
	}

	// FUNCTION: PLAYER 0x006b36a0
	// FUNCTION: STUDIO 0x008123a0
	void FriendService::playerAdded(int userId)
	{
		if (Network::Players::backendProcessing(this, false))
		{
			if (players.size() > 0)
			{
				if (!m_unk0x7c.empty() && userId >= 0)
				{
					bool nonGuests = false;
					std::ostringstream ostr;
					ostr << RBX::format(m_unk0x7c.c_str(), userId);
					for (std::set<int>::const_iterator iter = players.begin(), end = players.end(); iter != end; ++iter)
					{
						if (*iter >= 0)
						{
							ostr << "&otherUserIds=" << *iter;
							nonGuests = true;
						}
					}

					if (nonGuests)
						Http(ostr.str()).get(boost::bind(&ProcessBulkFriendResponse, weak_from(this), userId, players, _1, _2));
					else
						ProcessBulkFriendResponse(weak_from(this), userId, players, NULL, NULL);
				}
				else
				{
					ProcessBulkFriendResponse(weak_from(this), userId, players, NULL, NULL);
				}
			}
		}

		if (players.count(userId) == 0)
			players.insert(userId);
	}

} // namespace RBX
