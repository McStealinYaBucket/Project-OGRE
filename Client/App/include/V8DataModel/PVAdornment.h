/* V8DataModel/PVAdornment.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<PVAdornment, GuiBase3d> 0x260708; C++-only (no metadata);
     sPVAdornment = "PVAdornment".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/PVAdornment.h */

#pragma once

#include "V8DataModel/GuiBase3d.h"

namespace RBX {

	extern const char* const sPVAdornment;

	class PVAdornment
		: public DescribedNonCreatable<PVAdornment, GuiBase3d, sPVAdornment>
	{
	public:
		PVAdornment() {}
	};

} // namespace RBX
