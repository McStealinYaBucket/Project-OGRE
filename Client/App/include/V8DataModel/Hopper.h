/* V8DataModel/Hopper.h -- October 2013 STUB (minimal shell).
   Evidence: Hopper C2 0x3EB8DC; parent of Backpack (FactoryProduct<Backpack, Hopper> 0x2783A4) and of StarterPackService/LegacyHopperService (NonFactory); 0.125 metadata class;
     sHopper = "Hopper".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Hopper.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sHopper;

	class Hopper
		: public DescribedNonCreatable<Hopper, Instance, sHopper>
	{
	public:
		Hopper() {}
	};

} // namespace RBX
