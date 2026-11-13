/* V8DataModel/Handles.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Handles, HandlesBase> 0x3E48C8;
     sHandles = "Handles".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Handles.h */

#pragma once

#include "V8DataModel/HandlesBase.h"

namespace RBX {

	extern const char* const sHandles;

	class Handles
		: public DescribedCreatable<Handles, HandlesBase, sHandles>
	{
	public:
		enum VisualStyle
		{
			RESIZE_HANDLES = 0,
			MOVEMENT_HANDLES = 1,
		};

		Handles() {}
	};

} // namespace RBX
