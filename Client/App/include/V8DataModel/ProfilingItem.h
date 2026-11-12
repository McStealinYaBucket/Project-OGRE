/* V8DataModel/ProfilingItem.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED NonFactoryProduct<ProfilingItem, Stats::Item> 0x4A5138 (base flattened to StatsItem);
     sProfilingItem = "ProfilingItem".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ProfilingItem.h */

#pragma once

#include "V8DataModel/StatsItem.h"

namespace RBX {

	extern const char* const sProfilingItem;

	class ProfilingItem
		: public DescribedNonCreatable<ProfilingItem, StatsItem, sProfilingItem>
	{
	public:
		ProfilingItem() {}
	};

} // namespace RBX
