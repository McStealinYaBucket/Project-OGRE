/* App/v8world/SimJob.cpp -- October 2013: SimJob and SimJobTracker (2013 has no SimJobTracker.cpp:
   the tracker's asserts cite this file, lines 40-86, the job's 100-107). Source order follows the
   assert lines; VC9 emits the job's ctor/dtor before stopTracking (PLAYER 0x00822390 vs 0x008226d0).
   containedBy is inlined everywhere (no out-of-line copy survives). */

#include "stdafx.h"

#include "V8World/SimJob.h"
#include "V8World/Primitive.h"
#include "V8World/Assembly.h"
#include "Util/StlExtra.h"

namespace RBX {
	// FUNCTION: PLAYER 0x00822250 FOLDED
	// FUNCTION: STUDIO 0x00916220 FOLDED
	const SimJob* SimJob::getConstSimJobFromPrimitive(const Primitive* primitive)
	{
		if (primitive) {
			if (const Assembly* assembly = primitive->getConstAssembly()) {
				if (const SimJob* answer = assembly->getConstSimJob()) {
					return answer;
				}
			}
		}
		return NULL;
	}
	// FUNCTION: PLAYER 0x00822250 FOLDED
	// FUNCTION: STUDIO 0x00916220 FOLDED
	SimJob* SimJob::getSimJobFromPrimitive(Primitive* primitive)
	{ return const_cast<SimJob*>(getConstSimJobFromPrimitive(primitive)); }

	bool SimJobTracker::containedBy(SimJob* s) {
		return (std::find(s->trackers.begin(), s->trackers.end(), this) != s->trackers.end());
	}
	// FUNCTION: PLAYER 0x008226d0
	// FUNCTION: STUDIO 0x009166a0
	void SimJobTracker::stopTracking()
	{
		if (simJob) {
			RBXASSERT(this->containedBy(simJob));

			fastRemoveShort<SimJobTracker*>(simJob->trackers, this);

			RBXASSERT(!this->containedBy(simJob));
			simJob = NULL;
		}
	}
	// FUNCTION: PLAYER 0x00822270
	// FUNCTION: STUDIO 0x00916240
	bool SimJobTracker::tracking()
	{
		if (simJob) {
			RBXASSERT(this->containedBy(simJob));
			return true;
		}
		else {
			return false;
		}
	}
	// FUNCTION: PLAYER 0x008227b0
	// FUNCTION: STUDIO 0x00916780
	void SimJobTracker::setSimJob(SimJob* m)
	{
		stopTracking();
		if (m) {
			RBXASSERT(!this->containedBy(m));

			m->trackers.push_back(this);

			RBXASSERT(this->containedBy(m));
			simJob = m;
		}
	}
	// FUNCTION: PLAYER 0x008222e0
	// FUNCTION: STUDIO 0x009162b0
	SimJob* SimJobTracker::getSimJob()
	{
		RBXASSERT(simJob);
		RBXASSERT(this->containedBy(simJob));
		return simJob;
	}
	// FUNCTION: PLAYER 0x008228c0
	// FUNCTION: STUDIO 0x00916890
	void SimJobTracker::transferTrackers(SimJob* from, SimJob* to) {
		RBXASSERT(from);
		RBXASSERT(from != to);

		while (!from->trackers.empty()) {
			SimJobTracker* transfer = from->trackers.back();

			transfer->setSimJob(to);
		}
	}
	// FUNCTION: PLAYER 0x00822390
	// FUNCTION: STUDIO 0x00916360
	SimJob::SimJob(Assembly* _assembly)
		: assembly(_assembly)
		, useCount(0)
	{
		RBXASSERT(assembly->getConstSimJob() == NULL);
	}
	// FUNCTION: PLAYER 0x00822430
	// FUNCTION: STUDIO 0x00916400
	SimJob::~SimJob()
	{
		RBXASSERT(useCount == 0);
		RBXASSERT(assembly->getConstSimJob() == NULL);
	}

} // namespace RBX
