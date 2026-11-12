/* V8DataModel/ArcHandles.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<ArcHandles, HandlesBase> 0x273DBC;
     sArcHandles = "ArcHandles".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ArcHandles.h */

#pragma once

#include "V8DataModel/HandlesBase.h"

namespace RBX {

	extern const char* const sArcHandles;

	class ArcHandles
		: public DescribedCreatable<ArcHandles, HandlesBase, sArcHandles>
	{
	public:
		ArcHandles() {}
	};

} // namespace RBX
