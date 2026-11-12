/* V8DataModel/BindableEvent.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<BindableEvent, Instance> 0x347D08;
     sBindableEvent = "BindableEvent".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BindableEvent.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sBindableEvent;

	class BindableEvent
		: public DescribedCreatable<BindableEvent, Instance, sBindableEvent>
	{
	public:
		BindableEvent() {}
	};

} // namespace RBX
