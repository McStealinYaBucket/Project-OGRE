/* V8DataModel/Flag.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Flag, Tool> 0x37BACC;
     sFlag = "Flag".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Flag.h */

#pragma once

#include "V8DataModel/Tool.h"

namespace RBX {

	extern const char* const sFlag;

	class Flag
		: public DescribedCreatable<Flag, Tool, sFlag>
	{
	public:
		Flag() {}
	};

} // namespace RBX
