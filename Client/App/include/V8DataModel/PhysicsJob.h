/* V8DataModel/PhysicsJob.h — October 2013 STUB (scheduler job shell).
   Evidence: RunService::start 0x236688 news PhysicsJob + PhysicsJob(shared_ptr
     <DataModel>) 0x23AC84. True hierarchy/TU unknown (likely v8world);
     stubbed as a TaskScheduler::Job here so start() links.
   TODO: full decompile (step body = physics stepping).
   Original file: (2013 path TODO). */

#pragma once

#include "rbx/TaskScheduler.Job.h"

namespace RBX {

	class DataModel;

	class PhysicsJob : public TaskScheduler::Job
	{
		shared_ptr<DataModel> dataModel;
	public:
		PhysicsJob(shared_ptr<DataModel> dataModel);  // 0x23AC84
	private:
		Time::Interval sleepTime(const TaskScheduler::Job::Stats& stats);
		TaskScheduler::Job::Error error(const TaskScheduler::Job::Stats& stats);
		double getPriorityFactor();
		TaskScheduler::StepResult step(const TaskScheduler::Job::Stats& stats);
	};

} // namespace RBX
