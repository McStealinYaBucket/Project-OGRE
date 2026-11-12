/* V8DataModel/SelectionBox.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<SelectionBox, PVAdornment> 0x366902;
     sSelectionBox = "SelectionBox".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/SelectionBox.h */

#pragma once

#include "V8DataModel/PVAdornment.h"

namespace RBX {

	extern const char* const sSelectionBox;

	class SelectionBox
		: public DescribedCreatable<SelectionBox, PVAdornment, sSelectionBox>
	{
	public:
		SelectionBox() {}
	};

} // namespace RBX
