/* V8DataModel/PlayerHUD.h — October 2013 STUB (minimal, unblocks DataModel).
   Evidence: Creatable<Instance>::create<PlayerHUD> stored at +2912 in DataModel::
     initializeContents 0x2D3336; "PlayerHUD" string in both 2013 PEs. No March 2016
     counterpart (removed by 2016) — Service base inferred (setAndLockParent'ed to
     the DataModel, which only accepts Services).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/PlayerHUD.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sPlayerHUD;

	class PlayerHUD
		: public DescribedCreatable<PlayerHUD, Instance, sPlayerHUD>
		, public Service
	{
	public:
		PlayerHUD() {}
	};

} // namespace RBX
