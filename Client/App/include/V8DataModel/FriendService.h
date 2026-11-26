/* V8DataModel/FriendService.h -- October 2013. Layout (0x14c) proven by the ctor PLAYER 0x006b2780,
   the implicit dtor 0x006b2970, onServiceProvider 0x006b2b50 and playerRemoving 0x006afb40 (all 100%).
   Evidence: ServiceProvider::create<FriendService> from DataModel::initializeContents
     0x2D3336 (created in initializeContents.); "FriendService" string in both 2013 PEs.
   Functionality + Service base carried from March 2016 conventions.
   Still STUB: playerAdded, friendEventReplicationChanged, friendStatusReplicationChanged; the rest of
   the 2016 API (getFriendStatus, issue/reject requests, getFriendsOnline) is not written.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/FriendService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "rbx/signal.h"
#include <map>
#include <set>
#include <string>

namespace RBX {

	extern const char* const sFriendService;

	// SIZE 0x14c
	class FriendService
		: public DescribedCreatable<FriendService, Instance, sFriendService, Reflection::ClassDescriptor::INTERNAL>
		, public Service
	{
	public:
		enum FriendEventType
		{
			ISSUE_REQUEST = 0,
			REVOKE_REQUEST = 1,
			ACCEPT_REQUEST = 2,
			DENY_REQUEST = 3,
		};

		enum FriendStatus
		{
			FRIEND_STATUS_UNKNOWN = 0,
			FRIEND_STATUS_NOT_FRIEND = 1,
			FRIEND_STATUS_FRIEND = 2,
			FRIEND_STATUS_FRIEND_REQUEST_SENT = 3,
			FRIEND_STATUS_FRIEND_REQUEST_RECEIVED = 4,
		};

		// Out of line (PLAYER 0x006b2780, new(0x14c) in Creatable::create 0x00421820).
		FriendService();

		// Players' onChildAdded/onChildRemoving call these with the player's userId; both start
		// with Players::backendProcessing(this, false) (PLAYER 0x006b36a0 / 0x006afb40).
		void playerAdded(int userId);
		void playerRemoving(int userId);

		// Layout from the 2013 ctor 0x006b2780 (EH states in brackets). Names are 2016's where the
		// 2013 node shapes agree (map node isnil at +0x1d/+0x15/+0x11 fixes each value type).
		rbx::remote_signal<void(int, int, FriendEventType)> friendEventReplicatingSignal;	// 0x64 [1-2]
		rbx::remote_signal<void(int, int, FriendStatus)> friendStatusReplicatingSignal;		// 0x6c [3-4]

	protected:
		// Instance (FriendService vftable 0x00b3976c slot 0x20). 2013 calls no Super.
		/*override*/ void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider);

	private:
		void friendEventReplicationChanged(int playerId, int otherPlayerId, FriendEventType eventType);
		void friendStatusReplicationChanged(int playerId, int otherPlayerId, FriendStatus status);

		rbx::signals::scoped_connection friendStatusConnection;	// 0x74 [5]
		rbx::signals::scoped_connection friendEventConnection;	// 0x78 [6]
		std::string m_unk0x7c;			// 0x7c [7]  six URL strings; which is which is unproven
		std::string m_unk0x98;			// 0x98 [8]
		std::string m_unk0xb4;			// 0xb4 [9]
		std::string m_unk0xd0;			// 0xd0 [10]
		std::string m_unk0xec;			// 0xec [11]
		std::string m_unk0x108;			// 0x108 [12]

		// friendStatusTable[smallUserId][bigUserId] (playerRemoving 0x006afb40 only searches the
		// inner map when key < userId).
		typedef std::map<int, FriendStatus> FriendStatusInternalMap;
		typedef std::map<int, FriendStatusInternalMap> FriendStatusMap;
		FriendStatusMap friendStatusTable;							// 0x124 [13]
		typedef std::set<std::pair<int, int> > FriendRequestSet;
		FriendRequestSet friendRequestSet;							// 0x130 [14]
		std::set<int> players;										// 0x13c [15]
		bool enable;												// 0x148 false

		// The implicit destructor and its deleting destructor (the vftable 0x00b39744 thunk 0x006b2960 jumps
		// here). The names export puts this D0's name on 0x006aaf50, a base-class D0 that calls ~Instance.
		// SYNTHETIC: PLAYER 0x006b2970
		// SYNTHETIC: STUDIO 0x00811670
		// RBX::FriendService::~FriendService

		// SYNTHETIC: PLAYER 0x006b2b30
		// SYNTHETIC: STUDIO 0x00811830
		// RBX::FriendService::`scalar deleting destructor'
	};

} // namespace RBX
