/* V8DataModel/SelectionPartLasso.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<SelectionPartLasso, SelectionLasso> (Creator 0x47E86A);
     sSelectionPartLasso = "SelectionPartLasso".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/SelectionPartLasso.h */

#pragma once

#include "V8DataModel/SelectionLasso.h"

namespace RBX {

	extern const char* const sSelectionPartLasso;

	class SelectionPartLasso
		: public DescribedCreatable<SelectionPartLasso, SelectionLasso, sSelectionPartLasso>
	{
	public:
		SelectionPartLasso() {}
	};

} // namespace RBX
