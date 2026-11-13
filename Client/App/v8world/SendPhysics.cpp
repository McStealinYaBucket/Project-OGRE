/* App/v8world/SendPhysics.cpp -- October 2013 (network-send queue holder).
   Bodies: C2 0x599C1C / D2 0x599D6E (batch778) / buildSimJob 0x599F42 /
     destroySimJob 0x599FD4 (batch777) / onMovingAssemblyRootAdded
     0x59A096 / onMovingAssemblyRootRemoving 0x59A16C (batch777) /
     nextSimJob 0x59A1D6 (batch778; asserted body lives in the 2013
     HEADER -- include/V8World/SendPhysics.h:41 cite) / setTrackerSimJob
     0x8D6FCC (batch779). 2013 line numbers from the assert strings.
   DELTAs (loud): nextSimJob advances with std::find where the binary
     reads the O(1) intrusive node link (plus a port-belt assert);
     setTrackerSimJob's tbb::internal::AtomicBackoff pause -> naked spin
     (rbx::spin_mutex::lock precedent, rbx/threadsafe.h; tbb not landed);
     the 1-byte cmpxchg lock is ours as the same-slot 4-byte rbx::atomic.
   Self-audit: fidelity ~92%, function ~55% (see the header). */
#include "stdafx.h"
#include "V8World/SendPhysics.h"
#include "V8World/SimJob.h"
#include "V8World/SimJobTracker.h"
#include "V8World/Assembly.h"
#include "V8World/Mechanism.h"
#include <algorithm>
namespace RBX {
	// mac: 0x599C1C (batch778): stores count 0, the empty-list anchor (self-pointing), the CV zero-state, the lock byte 0, then provisions the two signal heads (SAFE_HEAP-mutexed statics in the binary, +0x1C before +0x20). Our member ctors produce the same state in the same order.
	SendPhysics::SendPhysics()
		: count(0)
		, simJobs()
		, validator()
		, sendLock(0)
		, signalAdded()
		, signalRemoving()
	{
	}
	// mac: 0x599D6E (batch778): assert "simJobs.empty()" SendPhysics.cpp:18, then member teardown (signal +0x20 disconnect+release, +0x1C ditto, CV word asserts, the list-clear walk -- dead for the empty list the assert guarantees). No job deletes in either build.
	// STUB: PLAYER 0x008e60f0
	// STUB: STUDIO 0x00a043f0
	SendPhysics::~SendPhysics()
	{
		RBXASSERT(simJobs.empty());   // :18
		count = 0;                    // the binary resets +0 after the walk.
	}

	// mac: 0x59A096 (batch777). Assert text spells the param 'a'.
	// STUB: PLAYER 0x008e5fe0
	// STUB: STUDIO 0x00a042e0
	SimJob* SendPhysics::onMovingAssemblyRootAdded(Assembly* a)
	{
		RBXASSERT(Mechanism::isMovingAssemblyRoot(a));   // :53

		SimJob* job = a->getSimJob();                    // binary: [a+0x64].
		if (job == NULL)
		{
			job = new SimJob(a);                         // new(0x1C).
			buildSimJob(job);
			signalAdded(a->getAssemblyPrimitive());
		}
		++job->useCount;                                 // [job+0x18].
		return job;
	}

	// mac: 0x59A16C (batch777). No asserts (unguarded +0x64 read).
	// STUB: PLAYER 0x008e60b0
	// STUB: STUDIO 0x00a043b0
	void SendPhysics::onMovingAssemblyRootRemoving(Assembly* a)
	{
		SimJob* job = a->getSimJob();
		if (job->useCount-- == 1)                        // binary: postfix --.
		{
			signalRemoving(a->getAssemblyPrimitive());
			destroySimJob(job);
			delete job;   // binary: ~SimJob + operator delete (the null
			              // guard there is the compiler's own).
		}
	}

	// mac: 0x599F42 (batch777).
	// STUB: PLAYER 0x008e5ec0
	// STUB: STUDIO 0x00a041c0
	void SendPhysics::buildSimJob(SimJob* job)
	{
		if (job->getAssembly() != NULL)
		{
			RBXASSERT(!job->getConstAssembly()->getConstSimJob());   // :24
			job->getAssembly()->simJob = job;                        // [asm+0x64] = job.
		}
		simJobs.push_back(job);   // binary: last-field swap + old-last rewire
		                          // + first keeps the head == push_back.
		++count;
	}

	// mac: 0x599FD4 (batch777).
	// STUB: PLAYER 0x008e5f40
	// STUB: STUDIO 0x00a04240
	void SendPhysics::destroySimJob(SimJob* job)
	{
		SimJob* next = NULL;
		if (count >= 2)
			next = nextSimJob(job);
		SimJobTracker::transferTrackers(job, next);
		simJobs.remove(job);   // binary unlinks only when job->next != 0
		                       // (links-zero = the double-destroy break);
		                       // remove() no-ops the absent job too, and
		                       // the --count below runs in both builds.
		--count;
		// (binary zeroes the job's intrusive links here -- ours own no
		// member links, link-covered.)

		if (job->getAssembly() != NULL)
		{
			RBXASSERT(job->getAssembly()->getSimJob() == job);   // :44
			job->getAssembly()->simJob = NULL;                   // [asm+0x64] = 0.
		}
	}

	// mac: 0x8D6FCC (batch779). The binary spins on `lock cmpxchg [this+0x18],
	// 1` with tbb::internal::AtomicBackoff::pause on the failed arm,
	// then SimJobTracker::setSimJob, then the plain unlock store.
	// STUB: PLAYER 0x005e39c0
	// STUB: STUDIO 0x0067b680
	SimJobTracker* SendPhysics::setTrackerSimJob(SimJobTracker& tracker, SimJob* job) const
	{
		if (sendLock.compare_and_swap(1, 0) != 0)
		{
			do
			{
				// (backoff DELTA -- naked spin, rbx::spin_mutex::lock
				// precedent in Base/include/rbx/threadsafe.h.)
			}
			while (sendLock.compare_and_swap(1, 0) != 0);
		}
		SimJobTracker* result = tracker.setSimJob(job);
		sendLock = 0;   // binary: plain byte-store unlock (not CAS'd).
		return result;
	}

} // namespace RBX
