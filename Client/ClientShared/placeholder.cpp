// Placeholder so the ClientShared OBJECT library has a source to build (STUDIO only). Delete it once
// the library has real sources (and drop it from CMakeLists.txt).
//
// Shell template stand-in -- NOT original code, carries no marker. STUDIO's /GL shell instantiates
// Stats::Item::Item itself (its FrameRateManagerStatsItem, 0x0043e8a0, sits beside it): the copy all
// callers reach is the shell's LTCG one at STUDIO 0x0043e560 (/GS cookie, x87), not App's. The shell is
// linked first, so instantiating the inline ctor here makes our link resolve it the same way (see
// Client/WindowsClient/placeholder.cpp). Delete it when FrameRateManagerStatsItem is written.

#include "V8DataModel/StatsItem.h"

#include <new>

extern "C" void ogreStudioShellStandInUse(void* raw)
{
	new (raw) RBX::Stats::Item();
}

#pragma comment(linker, "/include:_ogreStudioShellStandInUse")
