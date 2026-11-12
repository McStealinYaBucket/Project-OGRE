/* V8DataModel/HeartbeatTask.h — October 2013 STUB (scheduler job shell).
   Evidence: RunService::start 0x236688 news HeartbeatTask +
     HeartbeatTask(shared_ptr<RunService>) 0x23AC84... (0x23A76A).
   TODO: full decompile (step body = heartbeat processing).
   Original file: (2013 path TODO). */

#pragma once

#include "rbx/TaskScheduler.Job.h"

namespace RBX {

	class RunService;

	class HeartbeatTask : public TaskScheduler::Job
	{
		shared_ptr<RunService> runService;
	public:
		HeartbeatTask(shared_ptr<RunService> runService);  // 0x23A76A
	private:
		Time::Interval sleepTime(const TaskScheduler::Job::Stats& stats);
		TaskScheduler::Job::Error error(const TaskScheduler::Job::Stats& stats);
		double getPriorityFactor();
		TaskScheduler::StepResult step(const TaskScheduler::Job::Stats& stats);
	};

} // namespace RBX
