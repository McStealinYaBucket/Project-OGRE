/* V8DataModel/Hat.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Hat, Accoutrement> 0x25D74C;
     sHat = "Hat".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Hat.h */

#pragma once

#include "V8DataModel/Accoutrement.h"

namespace RBX {

	extern const char* const sHat;

	class Hat
		: public DescribedCreatable<Hat, Accoutrement, sHat>
	{
	public:
		Hat() {}
	};

} // namespace RBX
