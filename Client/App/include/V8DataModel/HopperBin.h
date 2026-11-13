/* V8DataModel/HopperBin.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<HopperBin, BackpackItem> 0x3EE9CA;
     sHopperBin = "HopperBin" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/HopperBin.h */

#pragma once

#include "V8DataModel/BackpackItem.h"

namespace RBX {

	extern const char* const sHopperBin;

	class HopperBin
		: public DescribedCreatable<HopperBin, BackpackItem, sHopperBin>
	{
	public:
		enum BinType
		{
			SCRIPT_BIN = 0,
			GAME_TOOL = 1,
			GRAB_TOOL = 2,
			CLONE_TOOL = 3,
			HAMMER_TOOL = 4,
		};

		HopperBin() {}
	};

} // namespace RBX
