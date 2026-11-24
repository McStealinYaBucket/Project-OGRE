/* App/v8datamodel/StatsService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile (stat items era). */

#include "stdafx.h"

#include "V8DataModel/StatsService.h"

namespace RBX {
namespace Stats {
	const char* const sStats = "Stats";	// 2013 string (PLAYER doDeclare 0x00480f00 -> 0x00b509d8 -> 0x00b019b4)
}
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Stats::StatsService);
}
