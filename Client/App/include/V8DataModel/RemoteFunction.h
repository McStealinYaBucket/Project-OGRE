/* V8DataModel/RemoteFunction.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<RemoteFunction, Instance> 0x748C88;
     sRemoteFunction = "RemoteFunction".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/RemoteFunction.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sRemoteFunction;

	class RemoteFunction
		: public DescribedCreatable<RemoteFunction, Instance, sRemoteFunction>
	{
	public:
		RemoteFunction() {}
	};

} // namespace RBX
