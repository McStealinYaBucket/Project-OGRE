/* V8DataModel/SelectionLasso.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<SelectionLasso, GuiBase3d> 0x47ED60; C++-only (no metadata);
     sSelectionLasso = "SelectionLasso".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/SelectionLasso.h */

#pragma once

#include "V8DataModel/GuiBase3d.h"

namespace RBX {

	extern const char* const sSelectionLasso;

	class SelectionLasso
		: public DescribedNonCreatable<SelectionLasso, GuiBase3d, sSelectionLasso>
	{
	public:
		SelectionLasso() {}
	};

} // namespace RBX
