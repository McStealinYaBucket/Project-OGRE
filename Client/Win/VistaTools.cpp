/* Win/VistaTools.cpp -- October 2013 client shell (PLAYER and STUDIO compile it; path: see
   VistaTools.h). Only IsVistaPlus is written: RobloxCrashReporter's ctor (LogManager.cpp) calls it. */

#include <windows.h>

#include "VistaTools.h"

// True on Windows NT 6.0 (Vista) and later. The zero-initialised OSVERSIONINFO is a memset of the
// 0x90 bytes after dwOSVersionInfoSize, which is then set to 0x94.
// FUNCTION: PLAYER 0x0046a2e0
// FUNCTION: STUDIO 0x0048d690
bool IsVistaPlus()
{
	OSVERSIONINFO osver = {0};
	osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (::GetVersionEx(&osver) && osver.dwPlatformId == VER_PLATFORM_WIN32_NT && osver.dwMajorVersion >= 6)
		return true;

	return false;
}
