/* V8DataModel/ExtrudedPartInstance.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<ExtrudedPartInstance, PartInstance> 0x344628; display literal "TrussPart" (replaces 0-byte stub);
     sExtrudedPartInstance = "TrussPart" (s-var VALUE is the Lua name; C++ name stays short).
   TODO: full decompile (geometry/param surface).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ExtrudedPartInstance.h */

#pragma once

#include "V8DataModel/PartInstance.h"

namespace RBX {

	extern const char* const sExtrudedPart;

	class ExtrudedPartInstance
		: public DescribedCreatable<ExtrudedPartInstance, PartInstance, sExtrudedPart>
	{
	public:
		ExtrudedPartInstance() {}
	};

} // namespace RBX
