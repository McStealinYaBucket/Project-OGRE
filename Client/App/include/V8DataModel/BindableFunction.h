/* V8DataModel/BindableFunction.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<BindableFunction, Instance> 0x348D64;
     sBindableFunction = "BindableFunction".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BindableFunction.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sBindableFunction;

	class BindableFunction
		: public DescribedCreatable<BindableFunction, Instance, sBindableFunction>
	{
	public:
		BindableFunction() {}
	};

} // namespace RBX
