/* App/v8datamodel/SleepingJob.cpp -- October 2013, from the 2013 bytes (PLAYER 0x0072b000..0x0072b2d0).
   2013 differs from 2016: wake uses fetch_and_store, and both wake and error depend on
   FFlag::LastWakeTimeSleepingJobError. */

#include "stdafx.h"

#include "V8DataModel/SleepingJob.h"
#include "rbx/TaskScheduler.h"

// GLOBAL: PLAYER 0x00df4e48
// GLOBAL: STUDIO 0x00f8d5b8
FASTFLAGVARIABLE(LastWakeTimeSleepingJobError, false)

namespace RBX {

	// FUNCTION: PLAYER 0x0072b000
	// FUNCTION: STUDIO 0x00829ff0
	SleepingJob::SleepingJob(const char* name, TaskType taskType, bool isPerPlayer,
		shared_ptr<DataModelArbiter> arbiter, Time::Interval stepBudget, double desiredFps)
		: DataModelJob(name, taskType, isPerPlayer, arbiter, stepBudget)
		, desiredFps(desiredFps)
	{
		isAwake = false;
	}

	// FUNCTION: PLAYER 0x0072b0e0
	// FUNCTION: STUDIO 0x0082a0d0
	void SleepingJob::sleep()
	{
		isAwake = false;
	}

	// FUNCTION: PLAYER 0x0072b0f0
	// FUNCTION: STUDIO 0x0082a0e0
	Time::Interval SleepingJob::sleepTime(const Stats&)
	{
		return isAwake ? Time::Interval(0) : Time::Interval::max();
	}

	// FUNCTION: PLAYER 0x0072b140
	// FUNCTION: STUDIO 0x0082a130
	TaskScheduler::Job::Error SleepingJob::error(const Stats& stats)
	{
		if (!isAwake)
			return Error();

		if (FFlag::LastWakeTimeSleepingJobError)
		{
			Stats fakedStats = stats;
			Time::Interval timeSinceAwoke = Time::now<Time::Fast>() - lastWakeTime;
			if (timeSinceAwoke < fakedStats.timespanSinceLastStep)
				fakedStats.timespanSinceLastStep = timeSinceAwoke;
			return computeStandardError(fakedStats, desiredFps);
		}
		return computeStandardError(stats, desiredFps);
	}

	// FUNCTION: PLAYER 0x0072b220
	// FUNCTION: STUDIO 0x0082a210
	void SleepingJob::wake()
	{
		if (!isAwake.fetch_and_store(true))
		{
			if (FFlag::LastWakeTimeSleepingJobError)
				lastWakeTime = Time::now<Time::Fast>();
			TaskScheduler::singleton().reschedule(shared_from_this());
		}
	}

} // namespace RBX
