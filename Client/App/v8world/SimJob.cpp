/* App/v8world/SimJob.cpp -- October 2013 (per-moving-root network job, and the SimJobTracker
   round-robin send cursor: 2013 has no SimJobTracker.cpp, the tracker's asserts cite SimJob.cpp
   :40-:86, the job's :100-:107).
   Bodies: SimJob C1 0x59ABCC tail -> C2 0x59ABD6 and D1 0x59AC8E tail ->
     D2 0x59AC98 (batch778). (The C1/D1 pairs collapse into the single
     landed bodies below -- same construction net.)
   SimJobTracker: setSimJob 0x59A8B6 / tracking 0x59A810 / getSimJob 0x59A9F4 /
     stopTracking 0x59A6D2 (batch779) / transferTrackers 0x59AAE6
     (batch778) / containedBy (inline in the binary -- landed here).
     stopTracking calls RBX::fastRemoveShort (include/Util/StlExtra.h, 0x59AD96).
   Functions are in PLAYER (and STUDIO) address order; the 2013 source order by assert line is
     stopTracking, tracking, setSimJob, getSimJob, transferTrackers, SimJob, ~SimJob.
   Self-audit: fidelity 100%, function ~80% (see the header of
     V8World/SimJob.h). */

#include "stdafx.h"

#include "V8World/SimJob.h"
#include "V8World/SimJobTracker.h"
#include "V8World/Assembly.h"

#include "Util/StlExtra.h"

#include <algorithm>

namespace RBX {

	// Inline in the binary (assert strings cite it); landed out-of-line.
	bool SimJobTracker::containedBy(SimJob* m)
	{
		if (m == NULL)
			return false;
		return std::find(m->trackers.begin(), m->trackers.end(), this) != m->trackers.end();
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

	// mac: 0x59ABD6 (batch778): vector self-constructs empty (binary
	// zeroizes the begin/end/cap words at +8/+0xC/+0x10 -- same net)
	// and the :100 assert.
	// STUB: PLAYER 0x00822390
	// STUB: STUDIO 0x00916360
	SimJob::SimJob(Assembly* assembly)
		: trackers()
		, assembly(assembly)
		, useCount(0)
	{
		RBXASSERT(assembly->getConstSimJob() == NULL);   // :100
	}

	// mac: 0x59AC98 (batch778): :106 + :107 asserts; no member deletes
	// (the vector frees itself; the job MEMORY is deleted by
	// SendPhysics::onMovingAssemblyRootRemoving after destroySimJob).
	// Non-virtual -- the 0x59A16C teardown is ~SimJob + operator delete.
	// STUB: PLAYER 0x00822430
	// STUB: STUDIO 0x00916400
	SimJob::~SimJob()
	{
		RBXASSERT(useCount == 0);                        // :106
		RBXASSERT(assembly->getConstSimJob() == NULL);   // :107
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

} // namespace RBX
