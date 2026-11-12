/* V8DataModel/Backpack.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Backpack, Hopper, sBackpack> 0x2783A4; 2016 shape (Backpack : Hopper) matches;
     sBackpack = "Backpack".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Backpack.h */

#pragma once

#include "V8DataModel/Hopper.h"

namespace RBX {

	extern const char* const sBackpack;

	class Backpack
		: public DescribedCreatable<Backpack, Hopper, sBackpack>
	{
	public:
		Backpack() {}
	};

} // namespace RBX
