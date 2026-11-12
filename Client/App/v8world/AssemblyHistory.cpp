/* App/v8world/AssemblyHistory.cpp -- October 2013 (history wake).
   Body: wakeUp (batch261: *(+24) = 15). See the header shell note. */

#include "stdafx.h"

#include "V8World/AssemblyHistory.h"

namespace RBX {

	// Batch261.
	void AssemblyHistory::wakeUp()
	{
		wakeCounter = 15;
	}

} // namespace RBX
