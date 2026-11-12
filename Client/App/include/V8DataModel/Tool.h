/* V8DataModel/Tool.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Tool, BackpackItem> 0x4DAB1C;
     sTool = "Tool" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Tool.h */

#pragma once

#include "V8DataModel/BackpackItem.h"

namespace RBX {

	extern const char* const sTool;

	class Tool
		: public DescribedCreatable<Tool, BackpackItem, sTool>
	{
	public:
		Tool() {}
	};

} // namespace RBX
