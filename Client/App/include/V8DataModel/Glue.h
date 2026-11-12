/* V8DataModel/Glue.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Glue, JointInstance> 0x367326;
     sGlue = "Glue".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Glue.h */

#pragma once

#include "V8DataModel/JointInstance.h"

namespace RBX {

	extern const char* const sGlue;

	class Glue
		: public DescribedCreatable<Glue, JointInstance, sGlue>
	{
	public:
		Glue() {}
	};

} // namespace RBX
