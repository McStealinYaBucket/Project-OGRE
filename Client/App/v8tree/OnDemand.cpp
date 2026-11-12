/* App/v8tree/OnDemand.cpp -- October 2013 (OnDemandPartInstance ctor).
   See V8Tree/OnDemand.h for evidence. C2 0x442DBC behaviors carried:
   back-ptrs +52/+60, self +184, frameDirty init 1, compute bytes zeroed,
   cached CF default. +188 global + gaps skipped (see header). */

#include "stdafx.h"

#include "V8Tree/OnDemand.h"

namespace RBX {

	OnDemandPartInstance::OnDemandPartInstance(PartInstance* owner)
		: touchTransmitter(NULL)
		, touchedSlotCount(0)
		, ownerA(owner)
		, ownerB(owner)
		, frameDirty(true)
		, selfRef(this)
		, streamRemoving(false)
	{
		for (int i = 0; i < 16; ++i)
			computePropState[i] = 0;
		for (int i = 0; i < 8; ++i)
			reserved188[i] = 0;
	}

} // namespace RBX
