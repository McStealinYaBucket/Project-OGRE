/* V8DataModel/HandlesBase.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<HandlesBase, PartAdornment> 0x3E9770; C++-only (no metadata);
     sHandlesBase = "HandlesBase".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/HandlesBase.h */

#pragma once

#include "V8DataModel/PartAdornment.h"

namespace RBX {

	extern const char* const sHandlesBase;

	class HandlesBase
		: public DescribedNonCreatable<HandlesBase, PartAdornment, sHandlesBase>
	{
	public:
		HandlesBase() {}
	};

} // namespace RBX
