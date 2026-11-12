/* App/v8world/SimJobTracker.cpp -- October 2013 (round-robin send cursor).
   Bodies: setSimJob 0x59A8B6 / tracking 0x59A810 / getSimJob 0x59A9F4 /
     stopTracking 0x59A6D2 (batch779) / transferTrackers 0x59AAE6
     (batch778) / containedBy (inline in the binary -- landed here).
   fastRemoveShort<SimJobTracker*> 0x59AD96 LANDED batch780 in its
     binary home include/Util/StlExtra.h (find + asserts :47/:48/:49 +
     swap-with-last + resize(size-1) + removed-index return); the
     file-local stand-in below is RETIRED -- stopTracking now calls
     the real RBX::fastRemoveShort.
   2013 line numbers from the assert strings (SimJob.cpp).
   Self-audit: fidelity ~99% (all five body shapes + the exact assert
     strings transcribed; fastRemoveShort internals home-landed in
     StlExtra.h batch780), function ~80% (Network-era driving pending
     -- header note). */

#include "stdafx.h"

#include "V8World/SimJobTracker.h"
#include "V8World/SimJob.h"

#include "Util/StlExtra.h"

#include <algorithm>

namespace RBX {

	// mac: 0x59A8B6 (batch779).
	// STUB: PLAYER 0x008227b0
	// STUB: STUDIO 0x00916780
	SimJobTracker* SimJobTracker::setSimJob(SimJob* m)
	{
		stopTracking();
		if (m != NULL)
		{
			RBXASSERT(!this->containedBy(m));       // :66
			m->trackers.push_back(this);            // 0x59AF28 vector call.
			RBXASSERT(this->containedBy(m));        // :70
			simJob = m;
		}
		return this;   // binary NULL arm returns stale eax (unconsumed).
	}

	// mac: 0x59A810 (batch779).
	// STUB: PLAYER 0x00822270
	// STUB: STUDIO 0x00916240
	bool SimJobTracker::tracking()
	{
		if (simJob != NULL)
		{
			RBXASSERT(this->containedBy(simJob));   // :53
			return true;
		}
		return false;
	}

	// mac: 0x59A9F4 (batch779).
	// STUB: PLAYER 0x008222e0
	// STUB: STUDIO 0x009162b0
	SimJob* SimJobTracker::getSimJob()
	{
		RBXASSERT(simJob);                      // :78
		RBXASSERT(this->containedBy(simJob));   // :79
		return simJob;
	}

	// mac: 0x59A6D2 (batch779).
	// STUB: PLAYER 0x008226d0
	// STUB: STUDIO 0x009166a0
	void SimJobTracker::stopTracking()
	{
		if (simJob != NULL)
		{
			RBXASSERT(this->containedBy(simJob));       // :40
			fastRemoveShort(simJob->trackers, this);    // 0x59AD96.
			RBXASSERT(!this->containedBy(simJob));      // :44
			simJob = NULL;
		}
	}

	// mac: 0x59AAE6 (batch778): back-pop loop; every setSimJob stops the
	// tracker first, which removes it from from -- the vector shrinks
	// each lap, so begin != end closes.
	// STUB: PLAYER 0x008228c0
	// STUB: STUDIO 0x00916890
	void SimJobTracker::transferTrackers(SimJob* from, SimJob* to)
	{
		RBXASSERT(from);        // :85
		RBXASSERT(from != to);  // :86
		while (from->trackers.begin() != from->trackers.end())
		{
			SimJobTracker* tracker = from->trackers.back();
			tracker->setSimJob(to);
		}
	}

	// Inline in the binary (assert strings cite it); landed out-of-line.
	bool SimJobTracker::containedBy(SimJob* m)
	{
		if (m == NULL)
			return false;
		return std::find(m->trackers.begin(), m->trackers.end(), this) != m->trackers.end();
	}

} // namespace RBX
