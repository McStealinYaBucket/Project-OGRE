/* App/v8datamodel/PhysicsJob.cpp — October 2013 STUB (shell only).
   TODO: full decompile (constructor wiring + physics step). */

#include "stdafx.h"

#include "V8DataModel/PhysicsJob.h"
#include "V8DataModel/DataModel.h"

namespace RBX {

	// STUB: PLAYER 0x0072b5e0
	// STUB: STUDIO 0x00798fe0
	PhysicsJob::PhysicsJob(shared_ptr<DataModel> dataModel)
		: TaskScheduler::Job("Physics", shared_ptr<TaskScheduler::Arbiter>(), Time::Interval(0))
		, dataModel(dataModel)
	{
	}

	Time::Interval PhysicsJob::sleepTime(const TaskScheduler::Job::Stats& stats)
	{
		(void)stats;
		return Time::Interval::max();
	}

	TaskScheduler::Job::Error PhysicsJob::error(const TaskScheduler::Job::Stats& stats)
	{
		(void)stats;
		return Error();
	}

	double PhysicsJob::getPriorityFactor()
	{
		return 1.0;
	}

	TaskScheduler::StepResult PhysicsJob::step(const TaskScheduler::Job::Stats& stats)
	{
		(void)stats;
		return TaskScheduler::Done;
	}

} // namespace RBX
