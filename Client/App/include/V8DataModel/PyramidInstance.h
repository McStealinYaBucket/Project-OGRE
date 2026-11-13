/* V8DataModel/PyramidInstance.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<PyramidInstance, PartInstance> 0x469BFC; display literal "PyramidPart";
     sPyramidInstance = "PyramidPart" (s-var VALUE is the Lua name; C++ name stays short).
   TODO: full decompile (geometry/param surface).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/PyramidInstance.h */

#pragma once

#include "V8DataModel/PartInstance.h"

namespace RBX {

	extern const char* const sPyramid;

	class PyramidInstance
		: public DescribedNonCreatable<PyramidInstance, PartInstance, sPyramid>
	{
	public:
		enum NumSidesEnum
		{
			sides3 = 3,
			sides4 = 4,
			sides5 = 5,
			sides6 = 6,
			sides8 = 8,
			sides10 = 10,
			sides20 = 20,
		};

		PyramidInstance() {}
	};

} // namespace RBX
