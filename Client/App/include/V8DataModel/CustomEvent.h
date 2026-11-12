/* V8DataModel/CustomEvent.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<CustomEvent, Instance> 0x34C33C (receiver side has no symbols — deferred);
     sCustomEvent = "CustomEvent".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/CustomEvent.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sCustomEvent;

	class CustomEvent
		: public DescribedCreatable<CustomEvent, Instance, sCustomEvent>
	{
	public:
		CustomEvent() {}
	};

} // namespace RBX
