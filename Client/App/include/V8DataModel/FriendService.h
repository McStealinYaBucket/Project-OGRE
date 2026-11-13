/* V8DataModel/FriendService.h -- October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<FriendService> from DataModel::initializeContents
     0x2D3336 (created in initializeContents.); "FriendService" string in both 2013 PEs.
   Functionality + Service base carried from March 2016 conventions.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/FriendService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sFriendService;

	class FriendService
		: public DescribedNonCreatable<FriendService, Instance, sFriendService, Reflection::ClassDescriptor::RUNTIME_LOCAL>
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

		FriendService() {}
	};

} // namespace RBX
