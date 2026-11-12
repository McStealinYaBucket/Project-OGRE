/* V8DataModel/Sky.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Sky, Instance>;
     sSky = "Sky" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Sky.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sSky;

	class Sky
		: public DescribedCreatable<Sky, Instance, sSky>
	{
	public:
		Sky() {}
	};

} // namespace RBX
