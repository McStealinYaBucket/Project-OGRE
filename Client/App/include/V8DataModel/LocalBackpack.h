/* V8DataModel/LocalBackpack.h — October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<LocalBackpack> + shared_from<LocalBackpack> stored
     at +2928 in DataModel::initializeContents 0x2D3336; "LocalBackpack" string in both
     2013 PEs. No March 2016 counterpart (2016 has Backpack/Hopper instead) — Service
     base inferred (parents to the DataModel via the ServiceProvider path).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/LocalBackpack.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sLocalBackpack;

	class LocalBackpack
		: public DescribedCreatable<LocalBackpack, Instance, sLocalBackpack>
		, public Service
	{
	public:
		LocalBackpack() {}
	};

} // namespace RBX
