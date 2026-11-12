/* V8DataModel/IntConstrainedValue.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED FactoryProduct<ConstrainedValue<int>, Instance> 0x4F01EC (Creator 0x4EF7DC; ConstrainedValue<T> template TODO);
     sIntConstrainedValue = "IntConstrainedValue".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/IntConstrainedValue.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sIntConstrainedValue;

	class IntConstrainedValue
		: public DescribedCreatable<IntConstrainedValue, Instance, sIntConstrainedValue>
	{
	public:
		IntConstrainedValue() {}
	};

} // namespace RBX
