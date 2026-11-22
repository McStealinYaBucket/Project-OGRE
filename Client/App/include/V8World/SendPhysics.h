/* V8World/SendPhysics.h -- October 2013. Layout (ctor PLAYER 0x008e61b0, World news 0x3c) as in 2016; nextSimJob's assert cites SendPhysics.h:41. */
#pragma once
#include "V8World/SimJob.h"
#include "rbx/signal.h"
#include "rbx/Debug.h"
#include "Util/ConcurrencyValidator.h"
#include <tbb/spin_mutex.h>

namespace RBX {
	class Assembly;
	class Primitive;

	// SIZE 0x3c
	class SendPhysics
	{
	private:
		SimJobList simJobs;                         // 0x00

		ConcurrencyValidator concurrencyValidator;  // 0x0c

		void buildSimJob(SimJob* job);
		void destroySimJob(SimJob* job);
		mutable tbb::spin_mutex changeTrackerMutex; // 0x30

		// STUB: PLAYER 0x005e39c0
		// STUB: STUDIO 0x0067b680
		void setTrackerSimJob(SimJobTracker& tracker, SimJob* simJob) const
		{
			tbb::spin_mutex::scoped_lock lock(changeTrackerMutex);
			tracker.setSimJob(simJob);
		}

	public:
		rbx::signals::signal_with_args<1, void(Primitive*)> assemblyPhysicsOnSignal;   // 0x34
		rbx::signals::signal_with_args<1, void(Primitive*)> assemblyPhysicsOffSignal;  // 0x38

		// FUNCTION: PLAYER 0x005e3bd0
		// FUNCTION: STUDIO 0x0067b6e0
		SimJob* nextSimJob(SimJob* current)
		{
			RBXASSERT(!simJobs.empty());
			SimJobList::iterator iter = simJobs.iterator_to(*current);
			++iter;
			return (iter == simJobs.end()) ? &simJobs.front() : &*iter;
		}

		// STUB: PLAYER 0x005e3ee0
		// STUB: STUDIO 0x0067bbb0
		template<class Callback>
		int reportSimJobs(Callback& callback, SimJobTracker& tracker, const SimJob* ignore, int numToReport = -1)
		{
			int reported = 0;
			if (simJobs.empty()) {
				return 0;
			}
			if (!tracker.tracking()) {
				setTrackerSimJob(tracker, &simJobs.front());
			}
			SimJob* simJob = tracker.getSimJob();
			int num = numToReport;
			if (num == -1)
				num = simJobs.size();
			while (reported < num) {
				SimJob* current = simJob;
				++reported;
				simJob = nextSimJob(simJob);
				if (current != ignore) {
					if (!callback.report(*current)) {
						break;
					}
				}
			}
			setTrackerSimJob(tracker, simJob);
			return reported;
		}

		SendPhysics();

		~SendPhysics();

		int getNumSimJobs() { return simJobs.size(); }

		void onMovingAssemblyRootAdded(Assembly* assembly);
		void onMovingAssemblyRootRemoving(Assembly* assembly);
	};

} // namespace RBX
