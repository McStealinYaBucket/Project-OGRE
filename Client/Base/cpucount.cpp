/* Base/cpucount.cpp — October 2013 (RbxTotalUsableCoreCount home).
   Evidence: link debt from TaskScheduler TUs; 0x144E89 (Mac side uses
     MacSystemUtil core counts); March 2016 Base/cpucount.cpp (Windows:
     std::thread::hardware_concurrency with defaultValue fallback).
   Carried from 2016 verbatim (same function, same fallback contract). */

#include "CPUCount.h"

#if defined (_WIN32)
#   include <windows.h>	// OGRE vc9: was <thread>, which VC9 does not have
#endif

unsigned int RbxTotalUsableCoreCount(unsigned int defaultValue)
{
#ifdef _WIN32
	// OGRE vc9: was std::thread::hardware_concurrency() (C++11), which reads dwNumberOfProcessors.
	// PLAYER 0x009bd190 (called with 1 by TaskScheduler::setThreadCount 0x009b7ec0) is different:
	// it ignores defaultValue, returns a count from the Intel-style CPUCount at 0x009bceb0, and
	// falls back to GetSystemInfo().dwNumberOfProcessors only when CPUCount returns status 8.
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	unsigned n = info.dwNumberOfProcessors;
	return n ? n : defaultValue;
#else
	return defaultValue;
#endif
}
