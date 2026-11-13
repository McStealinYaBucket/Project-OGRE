/* V8World/SendPhysics.h -- October 2013 (network-send queue holder).
   Evidence (batch776/777/778/779): C2 0x599C1C / D2 0x599D6E /
     buildSimJob 0x599F42 / destroySimJob 0x599FD4 /
     onMovingAssemblyRootAdded 0x59A096 / onMovingAssemblyRootRemoving
     0x59A16C / nextSimJob 0x59A1D6 / setTrackerSimJob 0x8D6FCC (const
     member) / reportSimJobs<JobSender> 0x8D6E4A (member template; the
     binary's only instantiation source is Network's
     RoundRobinPhysicsSender::JobSender). World news the member at +188
     with new(0x24) exactly.
   Members: int count (+0); the binary's simJobs list threads SimJob
     intrusive {next@job+0, prev@job+4} with the anchor {count@+0,
     first@+4, last@+8} (empty = first == last == &anchor at this+4 --
     GCC std::list anchor algebra, so our std::list<SimJob*> models
     membership/order identically; the node links live in our heap
     nodes instead of job+0/job+4). ConcurrencyValidator +0xC (12B).
     Send-lock +0x18 (binary: one `lock cmpxchg` BYTE -- ours is the
     landed rbx::atomic, same 4-byte slot, 1-vs-4 width unobservable).
     Two signal_with_args<1, void(Primitive*)> embeds +0x1C/+0x20 (fired
     on job build / on the destroy path; member names unrecovered --
     TODO). sizeof(SendPhysics) = 0x24.
   Ownership: jobs are new'd in onMovingAssemblyRootAdded and deleted
     in onMovingAssemblyRootRemoving (dtor does NOT delete jobs -- D2
     asserts the list empty at SendPhysics.cpp:18).
   Self-audit: fidelity ~92% (every decoded body transcribed; DELTAs:
     nextSimJob advances via std::find over the list where the binary
     reads the O(1) node link + a port-belt contained-assert; the tbb
     backoff inside setTrackerSimJob -> naked spin per the in-tree
     rbx::spin_mutex::lock precedent (tbb is not landed, cf.
     rbx/threadsafe.h "we dont have tbb anymore"); signal member NAMES
     unrecovered); function ~55% for the whole send phase (the class is
     shape-complete and driven by the landed SimulateStage flows, but
     +0x1C/+0x20 consumers, RoundRobinPhysicsSender and the World
     send-phase network flow all queue with the Network era).
   Original file: Client/App/v8world/SendPhysics.h (path inferred). */

#pragma once

#include <algorithm>
#include <list>
#include "rbx/signal.h"
#include "rbx/atomic.h"
#include "Util/ConcurrencyValidator.h"

namespace RBX {

	class Assembly;
	class Primitive;
	class SimJob;
	class SimJobTracker;

	class SendPhysics
	{
	public:
		SendPhysics();                  // 0x599C1C (C1 0x599C12 tails it).
		~SendPhysics();                 // 0x599D6E (D1 0x599D64 tails it).

		// mac: 0x59A096: builds the moving root's SimJob (one per root
		// assembly) and ++useCounts it; the +0x1C signal fires ONLY on
		// the just-built arm, with the assembly's primitive. Assert
		// "Mechanism::isMovingAssemblyRoot(a)" SendPhysics.cpp:53.
		// Binary tail returns the job; nothing in the binary consumes
		// it (port keeps the pointer return).
		SimJob* onMovingAssemblyRootAdded(Assembly* a);
		// mac: 0x59A16C: --useCount; when it was 1, the +0x20 signal fires,
		// destroySimJob unlinks/untracks, then ~SimJob + delete. No
		// asserts in the binary (unguarded +100 read).
		void onMovingAssemblyRootRemoving(Assembly* a);

		// mac: 0x59A1D6: next job cyclically; after the last comes the
		// first (binary: when a2->next == &anchor it reads the
		// anchor's first). Assert "!simJobs.empty()" is the 2013
		// INCLUDE-file citation (SendPhysics.h:41): defined here as in 2013
		// (the prior tree had it out of line in SendPhysics.cpp).
		// STUB: PLAYER 0x005e3bd0
		// STUB: STUDIO 0x0067b6e0
		SimJob* nextSimJob(SimJob* after)
		{
			RBXASSERT(!simJobs.empty());   // :41

			std::list<SimJob*>::const_iterator it = std::find(
				simJobs.begin(), simJobs.end(), after);
			RBXASSERT(it != simJobs.end());   // port belt: membership is the
			                                  // destroy/build invariant; the
			                                  // binary keeps no belt for it
			                                  // (it reads the raw node link).

			++it;
			if (it == simJobs.end())
				it = simJobs.begin();   // after the last comes the first
			return *it;
		}

		// mac: 0x8D6FCC (const member): charge the +0x18 CAS spin -- the
		// binary backoffs via tbb::internal::AtomicBackoff::pause,
		// ours spins naked (in-tree precedent: rbx::spin_mutex::lock)
		// -- then tracker.setSimJob(job), then plain-store unlock.
		SimJobTracker* setTrackerSimJob(SimJobTracker& tracker, SimJob* job) const;

		// mac: 0x8D6E4A -- member template (instantiated in the binary only
		// by Network::RoundRobinPhysicsSender::JobSender; never
		// instantiated in-tree until the Network era lands, so no
		// out-of-line body for it exists here).
		// STUB: PLAYER 0x005e3ee0
		// STUB: STUDIO 0x0067bbb0
		template<class JobSender>
		int reportSimJobs(JobSender& sender, SimJobTracker& tracker, const SimJob* exclude, int maxJobCount)
		{
			if (simJobs.empty())
				return 0;
			if (!tracker.tracking())
				setTrackerSimJob(tracker, simJobs.front());
			SimJob* job = tracker.getSimJob();
			int maxJobs = (maxJobCount == -1) ? count : maxJobCount;
			SimJob* parked = job;
			int sent = 0;
			if (maxJobs > 0)
			{
				for (int i = 1; ; ++i)
				{
					parked = nextSimJob(job);
					if (job != exclude && !sender.report(job))
						break;
					sent = i;
					if (i >= maxJobs)
						break;
					job = parked;
				}
			}
			setTrackerSimJob(tracker, parked);
			return sent;
		}

	private:
		// mac: 0x599F42: chains the backpointer (explicit assert ;24 --
		// SendPhysics.cpp), then the anchor-append (last-field swap --
		// push_back shape) + ++count.
		void buildSimJob(SimJob* job);
		// mac: 0x599FD4: computes the successor (only when >= 2 jobs),
		// transferTrackers to it, unlink, --count, unchain the
		// assembly (assert "job->getAssembly()->getSimJob() == job" :44).
		void destroySimJob(SimJob* job);

	private:
		int count;                                  // +0 (C2 zeroizes; kept
		                                            // synced -- reportSimJobs'
		                                            // -1 arm reads it in the
		                                            // binary).

		// Assert-shaped name (":18 simJobs.empty()" / ":41
		// !simJobs.empty()"). MODEL-RELATIVE mechanics DELTA: binary
		// nodes are the jobs themselves {next@job+0, prev@job+4};
		// membership/order identical, links live in list heap nodes.
		std::list<SimJob*> simJobs;                 // +4 (anchor first@+4, last@+8).

		ConcurrencyValidator validator;             // +0xC (12B; C2 zero-state =
		// writing 0 / "" writeLocation / reading 0 -- our ctor equivalents).

		mutable rbx::atomic<int> sendLock;          // +0x18 (binary 1-byte
		// cmpxchg; TODO name). mutable: setTrackerSimJob lands const.

		// Names TODO (no string/name evidence). Construction order
		// +0x1C then +0x20 (C2 provisions in that order; D2 unwinds
		// them in the reverse, which the member dtors reproduce).
		rbx::signals::signal_with_args<1, void(Primitive*)> signalAdded;      // +0x1C.
		rbx::signals::signal_with_args<1, void(Primitive*)> signalRemoving;   // +0x20.
	};

} // namespace RBX
