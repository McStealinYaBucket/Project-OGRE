/* V8DataModel/SelectionPointLasso.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<SelectionPointLasso, SelectionLasso> 0x47EBB4;
     sSelectionPointLasso = "SelectionPointLasso".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/SelectionPointLasso.h */

#pragma once

#include "V8DataModel/SelectionLasso.h"

namespace RBX {

	extern const char* const sSelectionPointLasso;

	class SelectionPointLasso
		: public DescribedCreatable<SelectionPointLasso, SelectionLasso, sSelectionPointLasso>
	{
	public:
		SelectionPointLasso() {}
	};

} // namespace RBX
