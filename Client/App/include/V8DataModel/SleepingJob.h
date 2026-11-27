/* V8DataModel/SleepingJob.h -- October 2013.
   RTTI .?AVSleepingJob@RBX@@, vftable PLAYER 0x00b4d754 / STUDIO 0x00c178f4 (7 slots: a folded
   deleting dtor, sleepTime, error, DataModelJob::getPriorityFactor, DataModelJob::step,
   getDesiredConcurrencyCount, _purecall for stepDataModelJob). No 2013 __FILE__ names the file; the
   path follows the 2016 tree.

   Layout from the ctor (PLAYER 0x0072b000, ret 0x24) and wake (0x0072b220):
     +0x218 isAwake       tbb::atomic<bool>: wake's `lock xchg byte` is TBB 2.1's fetchstore1, sleep()
                          is a plain byte store. 2016 has rbx::atomic<int>.
     +0x220 desiredFps
     +0x228 lastWakeTime  only written when FFlag::LastWakeTimeSleepingJobError is set (registered in
                          SleepingJob.obj, PLAYER 0x00a72540).
   The shell's UserInputSleepingJob (RTTI, SIZE 0x260) derives it. */

#pragma once

#include "V8DataModel/DataModelJob.h"
#include "rbx/rbxtime.h"

#include <tbb/atomic.h>

namespace RBX {

	// VTABLE: PLAYER 0x00b4d754
	// VTABLE: STUDIO 0x00c178f4
	// SIZE 0x230
	class SleepingJob : public DataModelJob
	{
	private:
		tbb::atomic<bool> isAwake;	// 0x218
		const double desiredFps;	// 0x220
		Time lastWakeTime;			// 0x228

	public:
		SleepingJob(const char* name, TaskType taskType, bool isPerPlayer,
			shared_ptr<DataModelArbiter> arbiter, Time::Interval stepBudget, double desiredFps);

		void wake();
		void sleep();

		/*override*/ virtual Time::Interval sleepTime(const Stats& stats);	// vtable+0x04
		/*override*/ virtual Error error(const Stats& stats);				// vtable+0x08
	};

} // namespace RBX
