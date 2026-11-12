/* V8DataModel/Weld.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Weld, JointInstance> 0x415E64 (distinct from ManualWeld);
     sWeld = "Weld".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Weld.h */

#pragma once

#include "V8DataModel/JointInstance.h"

namespace RBX {

	extern const char* const sWeld;

	class Weld
		: public DescribedCreatable<Weld, JointInstance, sWeld>
	{
	public:
		Weld() {}
	};

} // namespace RBX
