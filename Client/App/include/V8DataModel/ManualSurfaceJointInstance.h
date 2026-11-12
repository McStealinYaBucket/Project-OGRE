/* V8DataModel/ManualSurfaceJointInstance.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<ManualSurfaceJointInstance, JointInstance> 0x367002; C++-only (no metadata);
     sManualSurfaceJointInstance = "ManualSurfaceJointInstance".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ManualSurfaceJointInstance.h */

#pragma once

#include "V8DataModel/JointInstance.h"

namespace RBX {

	extern const char* const sManualSurfaceJointInstance;

	class ManualSurfaceJointInstance
		: public DescribedCreatable<ManualSurfaceJointInstance, JointInstance, sManualSurfaceJointInstance>
	{
	public:
		ManualSurfaceJointInstance() {}
	};

} // namespace RBX
