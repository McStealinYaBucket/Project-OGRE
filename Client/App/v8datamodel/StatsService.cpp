/* App/v8datamodel/StatsService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile (stat items era). */

#include "stdafx.h"

#include "V8DataModel/StatsService.h"

namespace RBX {
namespace Stats {
	const char* const sStatsService = "StatsService";
}
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Stats::StatsService);
}
