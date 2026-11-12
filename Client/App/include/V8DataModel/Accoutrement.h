/* V8DataModel/Accoutrement.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Accoutrement, Instance> 0x25F286;
     sAccoutrement = "Accoutrement".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Accoutrement.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sAccoutrement;

	class Accoutrement
		: public DescribedCreatable<Accoutrement, Instance, sAccoutrement>
	{
	public:
		Accoutrement() {}
	};

} // namespace RBX
