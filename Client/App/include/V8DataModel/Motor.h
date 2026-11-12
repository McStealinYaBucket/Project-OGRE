/* V8DataModel/Motor.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Motor, JointInstance> 0x4151B6;
     sMotor = "Motor".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Motor.h */

#pragma once

#include "V8DataModel/JointInstance.h"

namespace RBX {

	extern const char* const sMotor;

	class Motor
		: public DescribedCreatable<Motor, JointInstance, sMotor>
	{
	public:
		Motor() {}
	};

} // namespace RBX
