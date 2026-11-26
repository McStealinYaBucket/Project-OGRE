/* V8DataModel/UserInputService.h -- October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<UserInputService> from DataModel::initializeContents
     0x2D3336 (created in initializeContents.); "UserInputService" string in both 2013 PEs.
   Functionality + Service base carried from March 2016 conventions.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/UserInputService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sUserInputService;

	class UserInputService
		: public DescribedCreatable<UserInputService, Instance, sUserInputService>
		, public Service
	{
	public:
		enum SwipeDirection
		{
			DIRECTION_RIGHT = 0,
			DIRECTION_LEFT = 1,
			DIRECTION_UP = 2,
			DIRECTION_DOWN = 3,
		};

		UserInputService() {}

		// iOS name and type; stores the bool at +0x80 (PLAYER 0x0069ff10).
		void setProcessToolCommandsSeparately(bool value);
	};

} // namespace RBX
