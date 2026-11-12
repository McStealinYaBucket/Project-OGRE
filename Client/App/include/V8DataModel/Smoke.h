/* V8DataModel/Smoke.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Smoke, Instance>;
     sSmoke = "Smoke" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Smoke.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sSmoke;

	class Smoke
		: public DescribedCreatable<Smoke, Instance, sSmoke>
	{
	public:
		Smoke() {}
	};

} // namespace RBX
