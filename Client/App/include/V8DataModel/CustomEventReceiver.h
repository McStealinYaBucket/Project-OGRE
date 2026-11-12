/* V8DataModel/CustomEventReceiver.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<CustomEventReceiver, Instance> 0x34D38A (Creator 0x671C44);
     sCustomEventReceiver = "CustomEventReceiver".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/CustomEventReceiver.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sCustomEventReceiver;

	class CustomEventReceiver
		: public DescribedCreatable<CustomEventReceiver, Instance, sCustomEventReceiver>
	{
	public:
		CustomEventReceiver() {}
	};

} // namespace RBX
