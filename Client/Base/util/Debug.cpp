/* Copyright 2003-2005 ROBLOX Corporation, All Rights Reserved */

/* Base/util/Debug.cpp — vendored from March 2016 reference (tiny, version-neutral:
   ReleaseAssert + Debugable crash plumbing). */

#include "rbx/Debug.h"
#include "RbxAssert.h"
#include "RbxFormat.h"
#include <algorithm>
#include <typeinfo>

const int CRASHONASSERT = 255;

// FUNCTION: PLAYER 0x009b19c0
// FUNCTION: STUDIO 0x00a68ab0
void ReleaseAssert(int channel, const char* msg) {
	if(channel == CRASHONASSERT)
		RBXCRASH(msg);
	else
		FLog::FastLog(channel, msg, 0);
}

namespace RBX
{
	// overload this in the debugger to pass by the crash
	volatile bool Debugable::doCrashEnabled = true;

	void Debugable::doCrash()
	{
		if(doCrashEnabled)
		{
			DebugBreak();
		}
	}

	#pragma optimize ("", off )

	void Debugable::doCrash(const char* message)
	{
#if defined(RBX_PLATFORM_DURANGO)
        OutputDebugStringA("ASSERTION FAILED: ");
        OutputDebugStringA(message);
        OutputDebugStringA("\n");
        if (!IsDebuggerPresent())
            return;
#endif
		if (doCrashEnabled)
		{
			DebugBreak();
		}
	}

	#pragma optimize ("", on )

	// STUB: PLAYER 0x009b1a70
	// STUB: STUDIO 0x00a68b40
	void Debugable::dump(std::ostream& stream)
	{
		stream << typeid(*this).name();
	}

} // rbx namespace
