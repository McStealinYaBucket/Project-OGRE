/* V8DataModel/PrismInstance.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<PrismInstance, PartInstance> 0x46764C; display literal "PrismPart";
     sPrismInstance = "PrismPart" (s-var VALUE is the Lua name; C++ name stays short).
   TODO: full decompile (geometry/param surface).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/PrismInstance.h */

#pragma once

#include "V8DataModel/PartInstance.h"

namespace RBX {

	extern const char* const sPrism;

	class PrismInstance
		: public DescribedNonCreatable<PrismInstance, PartInstance, sPrism>
	{
	public:
		PrismInstance() {}
	};

} // namespace RBX
