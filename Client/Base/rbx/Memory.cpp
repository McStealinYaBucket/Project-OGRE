/* Copyright 2003-2005 ROBLOX Corporation, All Rights Reserved */

// Base/rbx/Memory.cpp — vendored from March 2016 reference (pool statics +
// malloc/free/realloc only).
// TODO: global operator new/delete overrides (2016 has them below these in the
// same file; whether Oct 2013 overrides global new is unverified — decide at
// final-link stage).

// This prevent inclusion of winsock.h in windows.h, which prevents windows redifinition errors
// Look at winsock2.h for details, winsock2.h is #included from boost.hpp & other places.
#ifdef _WIN32
#define _WINSOCKAPI_
#endif

#include "rbx/Memory.h"


bool RBX::roblox_allocator::crashOnAllocationFailure = true;

namespace RBX
{
    std::vector<size_t*> poolAvailabilityList;
    std::vector<releaseFunc> poolReleaseMemoryFuncList;
    std::vector<size_t*> poolAllocationList;

	char* roblox_allocator::malloc(const size_type size)
	{
		char* result = (char*)std::malloc(size > 0 ? size : 1);
		if (!result && size && crashOnAllocationFailure)
			RBXCRASH();	// We want a nice fat crash here so that the process quits and we can log it
		return result;
	}
	void roblox_allocator::free(char * const block)
	{
		return std::free(block);
	}
	char* roblox_allocator::realloc(char *ptr, size_t nsize)
	{
		char* result = (char*)std::realloc(ptr, nsize);
		if (!result && nsize && crashOnAllocationFailure)
			RBXCRASH();	// We want a nice fat crash here so that the process quits and we can log it
		return result;
	}
}
