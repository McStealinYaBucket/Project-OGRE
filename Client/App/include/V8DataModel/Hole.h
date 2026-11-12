/* V8DataModel/Hole.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Hole, Feature> 0x3665D6;
     sHole = "Hole".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Hole.h */

#pragma once

#include "V8DataModel/Feature.h"

namespace RBX {

	extern const char* const sHole;

	class Hole
		: public DescribedCreatable<Hole, Feature, sHole>
	{
	public:
		Hole() {}
	};

} // namespace RBX
