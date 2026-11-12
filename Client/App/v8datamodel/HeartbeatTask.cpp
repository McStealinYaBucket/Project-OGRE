/* App/v8datamodel/HeartbeatTask.cpp — October 2013 STUB (shell only).
   TODO: full decompile (constructor wiring + heartbeat step). */

#include "stdafx.h"

#include "V8DataModel/HeartbeatTask.h"
#include "V8DataModel/RunService.h"

namespace RBX {

	// STUB: PLAYER 0x0072d040
	// STUB: STUDIO 0x0079acc0
	HeartbeatTask::HeartbeatTask(shared_ptr<RunService> runService)
		: TaskScheduler::Job("Heartbeat", shared_ptr<TaskScheduler::Arbiter>(), Time::Interval(0))
		, runService(runService)
	{
	}

	Time::Interval HeartbeatTask::sleepTime(const TaskScheduler::Job::Stats& stats)
	{
		(void)stats;
		return Time::Interval::max();
	}

	TaskScheduler::Job::Error HeartbeatTask::error(const TaskScheduler::Job::Stats& stats)
	{
		(void)stats;
		return Error();
	}

	double HeartbeatTask::getPriorityFactor()
	{
		return 1.0;
	}

	TaskScheduler::StepResult HeartbeatTask::step(const TaskScheduler::Job::Stats& stats)
	{
		(void)stats;
		return TaskScheduler::Done;
	}

} // namespace RBX
