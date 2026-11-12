/* V8DataModel/DoubleConstrainedValue.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED FactoryProduct<ConstrainedValue<double>, Instance> 0x4EF330 (Creator 0x4EE8F0; ConstrainedValue<T> template TODO);
     sDoubleConstrainedValue = "DoubleConstrainedValue".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/DoubleConstrainedValue.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sDoubleConstrainedValue;

	class DoubleConstrainedValue
		: public DescribedCreatable<DoubleConstrainedValue, Instance, sDoubleConstrainedValue>
	{
	public:
		DoubleConstrainedValue() {}
	};

} // namespace RBX
