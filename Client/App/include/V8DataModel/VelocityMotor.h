/* V8DataModel/VelocityMotor.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<VelocityMotor, JointInstance> 0x371970;
     sVelocityMotor = "VelocityMotor".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/VelocityMotor.h */

#pragma once

#include "V8DataModel/JointInstance.h"

namespace RBX {

	extern const char* const sVelocityMotor;

	class VelocityMotor
		: public DescribedCreatable<VelocityMotor, JointInstance, sVelocityMotor>
	{
	public:
		VelocityMotor() {}
	};

} // namespace RBX
