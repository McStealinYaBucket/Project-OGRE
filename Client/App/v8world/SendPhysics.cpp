/* App/v8world/SendPhysics.cpp -- October 2013. Asserts at the 2013 lines 18, 24, 44, 53. */
#include "stdafx.h"
#include "V8World/SendPhysics.h"
#include "V8World/Assembly.h"
#include "V8World/Mechanism.h"
#include "V8World/Primitive.h"

namespace RBX {
	// FUNCTION: PLAYER 0x008e61b0
	// FUNCTION: STUDIO 0x00a044b0
	SendPhysics::SendPhysics()
	{}

	// FUNCTION: PLAYER 0x008e60f0
	// FUNCTION: STUDIO 0x00a043f0
	SendPhysics::~SendPhysics()
	{
		RBXASSERT(simJobs.empty());
	}
	// FUNCTION: PLAYER 0x008e5ec0
	// FUNCTION: STUDIO 0x00a041c0
	void SendPhysics::buildSimJob(SimJob* job) {
		if (job->getAssembly()) {
			RBXASSERT(!job->getConstAssembly()->getConstSimJob());
			job->getAssembly()->simJob = job;
		}

		simJobs.push_back(*job);
	}


	// FUNCTION: PLAYER 0x008e5f40
	// FUNCTION: STUDIO 0x00a04240
	void SendPhysics::destroySimJob(SimJob* job)
	{
		SimJob* transferTo = (simJobs.size() > 1)
										? nextSimJob(job)
										: NULL;

		SimJobTracker::transferTrackers(job, transferTo);
		simJobs.erase(simJobs.iterator_to(*job));

		if (job->getAssembly()) {
			RBXASSERT(job->getAssembly()->getSimJob() == job);
			job->getAssembly()->simJob = NULL;
		}
	}

	// STUB: PLAYER 0x008e5fe0
	// STUB: STUDIO 0x00a042e0
	void SendPhysics::onMovingAssemblyRootAdded(Assembly* a)
	{
		RBXASSERT(Mechanism::isMovingAssemblyRoot(a));
		if (!a->getSimJob())
		{
			SimJob* job = new SimJob(a);
			buildSimJob(job);

			assemblyPhysicsOnSignal(a->getAssemblyPrimitive());
		}
		a->getSimJob()->useCount++;
	}

	// STUB: PLAYER 0x008e60b0
	// STUB: STUDIO 0x00a043b0
	void SendPhysics::onMovingAssemblyRootRemoving(Assembly* a)
	{
		SimJob* job = a->getSimJob();


		if (--job->useCount == 0)
		{
			assemblyPhysicsOffSignal(a->getAssemblyPrimitive());

			destroySimJob(job);
			delete job;
		}
	}

} // namespace RBX
