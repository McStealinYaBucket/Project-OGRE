/* V8DataModel/StudioTool.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<StudioTool, Instance> 0x4AE1A8;
     sStudioTool = "StudioTool".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/StudioTool.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sStudioTool;

	class StudioTool
		: public DescribedNonCreatable<StudioTool, Instance, sStudioTool>
	{
	public:
		StudioTool() {}
	};

} // namespace RBX
