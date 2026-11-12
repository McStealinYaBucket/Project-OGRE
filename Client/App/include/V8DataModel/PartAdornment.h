/* V8DataModel/PartAdornment.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<PartAdornment, GuiBase3d> 0x260E74; C++-only (no metadata);
     sPartAdornment = "PartAdornment".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/PartAdornment.h */

#pragma once

#include "V8DataModel/GuiBase3d.h"

namespace RBX {

	extern const char* const sPartAdornment;

	class PartAdornment
		: public DescribedNonCreatable<PartAdornment, GuiBase3d, sPartAdornment>
	{
	public:
		PartAdornment() {}
	};

} // namespace RBX
