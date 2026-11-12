/* V8DataModel/MotorFeature.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<MotorFeature, Feature> 0x371DA4;
     sMotorFeature = "MotorFeature".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/MotorFeature.h */

#pragma once

#include "V8DataModel/Feature.h"

namespace RBX {

	extern const char* const sMotorFeature;

	class MotorFeature
		: public DescribedCreatable<MotorFeature, Feature, sMotorFeature>
	{
	public:
		MotorFeature() {}
	};

} // namespace RBX
