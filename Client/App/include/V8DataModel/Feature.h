/* V8DataModel/Feature.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<Feature, Instance> 0x3718A4;
     sFeature = "Feature".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Feature.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sFeature;

	class Feature
		: public DescribedNonCreatable<Feature, Instance, sFeature>
	{
	public:
		Feature() {}
	};

} // namespace RBX
