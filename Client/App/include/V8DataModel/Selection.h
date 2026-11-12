/* V8DataModel/Selection.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<Selection, Instance> 0x368E7E; abstract (no creator by design);
     sSelection = "Selection".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Selection.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sSelection;

	class Selection
		: public DescribedNonCreatable<Selection, Instance, sSelection>
	{
	public:
		Selection() {}
	};

} // namespace RBX
