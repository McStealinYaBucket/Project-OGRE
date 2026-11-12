/* V8DataModel/Snap.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Snap, JointInstance> 0x3670CE;
     sSnap = "Snap".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Snap.h */

#pragma once

#include "V8DataModel/JointInstance.h"

namespace RBX {

	extern const char* const sSnap;

	class Snap
		: public DescribedCreatable<Snap, JointInstance, sSnap>
	{
	public:
		Snap() {}
	};

} // namespace RBX
