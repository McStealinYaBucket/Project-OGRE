/* V8DataModel/ManualWeld.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<ManualWeld, ManualSurfaceJointInstance> 0x415BE4; C++-only s-var (metadata has Weld — mapping TODO);
     sManualWeld = "ManualWeld".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ManualWeld.h */

#pragma once

#include "V8DataModel/ManualSurfaceJointInstance.h"

namespace RBX {

	extern const char* const sManualWeld;

	class ManualWeld
		: public DescribedCreatable<ManualWeld, ManualSurfaceJointInstance, sManualWeld>
	{
	public:
		ManualWeld() {}
	};

} // namespace RBX
