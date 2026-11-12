/* V8DataModel/SurfaceSelection.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<SurfaceSelection, PartAdornment> 0x366832 (Creator 0x4B9176); sSurfaceSelection="SurfaceSelection";
     sSurfaceSelection = "SurfaceSelection".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/SurfaceSelection.h */

#pragma once

#include "V8DataModel/PartAdornment.h"

namespace RBX {

	extern const char* const sSurfaceSelection;

	class SurfaceSelection
		: public DescribedCreatable<SurfaceSelection, PartAdornment, sSurfaceSelection>
	{
	public:
		SurfaceSelection() {}
	};

} // namespace RBX
