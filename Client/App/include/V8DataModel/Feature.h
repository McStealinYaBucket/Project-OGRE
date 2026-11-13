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
		enum TopBottom
		{
			TOP = 0,
			CENTER_TB = 1,
			BOTTOM = 2,
		};

		enum InOut
		{
			EDGE = 0,
			INSET = 1,
			CENTER_IO = 2,
		};

		enum LeftRight
		{
			LEFT = 0,
			CENTER_LR = 1,
			RIGHT = 2,
		};

		Feature() {}
	};

} // namespace RBX
