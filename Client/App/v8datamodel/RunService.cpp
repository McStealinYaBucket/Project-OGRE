/* App/v8datamodel/RunService.cpp -- October 2013. PhysicsJob and HeartbeatTask live in this .obj
   (PhysicsJob 0x0072b5e0.., HeartbeatTask 0x0072d040.., between RunService's own functions), as
   classes local to the .cpp (only RunService::start creates them); both are defined before
   stopTasks, which needs them complete for the typed members. stopTasks, onServiceProvider,
   start and the ctor from the PLAYER bytes (0x0072bb60..0x0072efd0); reflection and the stepping
   bodies not written yet.
   Both jobs read TaskScheduler::Job::Stats as 2013 lays it out: timespanSinceLastStep at +0 and
   timespanOfLastStep at +8 (Stats ctor 0x009b98d0, ret 0xc, no timeNow). */

#include "stdafx.h"

#include "V8DataModel/RunService.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/DataModelJob.h"
#include "V8Kernel/Constants.h"
#include "rbx/TaskScheduler.h"
#include "decomp.h"

namespace RBX {
	const char* const sRunService = "RunService";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::RunService);
}

namespace RBX {

	// GLOBAL: PLAYER 0x00df4e68
	bool RunService::parallelPhysicsUserEnabled;

	// Its 2013 .obj is not known; defined here so DataModel.cpp's assert lines stay put.
	// GLOBAL: PLAYER 0x00d314cc
	bool DataModel::throttleAt30Fps = true;

	// 2013 layout (ctor PLAYER 0x0072b5e0): DataModelJob 0x218, stepsPerSec @0x218 (the ctor
	// caches Constants::uiStepsPerSec(), 2016 calls it each time), dataModel @0x220. No
	// circular_buffer of step records, no lastTime yet.
	// VTABLE: PLAYER 0x00b4d870
	// SIZE 0x228
	class PhysicsJob : public DataModelJob
	{
	public:
		double stepsPerSec;					// 0x218
		weak_ptr<DataModel> const dataModel;	// 0x220

		// FUNCTION: PLAYER 0x0072b5e0
		// FUNCTION: STUDIO 0x00798fe0
		PhysicsJob(shared_ptr<DataModel> dataModel)
			: DataModelJob("Physics", DataModelJob::Physics, false, dataModel, Time::Interval(0.01))
			, stepsPerSec(Constants::uiStepsPerSec())
			, dataModel(dataModel)
		{
		}

		// FUNCTION: PLAYER 0x0072b700
		// FUNCTION: STUDIO 0x00799100
		virtual Time::Interval sleepTime(const Stats& stats)
		{
			return computeStandardSleepTime(stats, DataModel::throttleAt30Fps ? stepsPerSec : 1000.0);
		}

		// FUNCTION: PLAYER 0x0072b740
		// FUNCTION: STUDIO 0x00799140
		virtual Job::Error error(const Stats& stats)
		{
			Job::Error result;
			result.error = stepsPerSec * stats.timespanSinceLastStep.seconds();
			return result;
		}

		// FUNCTION: PLAYER 0x0072b760
		// FUNCTION: STUDIO 0x00799160
		virtual int getDesiredConcurrencyCount() const
		{
			return RunService::parallelPhysicsUserEnabled ? 1024 : 1;
		}

		// SYNTHETIC: PLAYER 0x0072b780
		// SYNTHETIC: STUDIO 0x00799180
		// RBX::PhysicsJob::~PhysicsJob

		// SYNTHETIC: PLAYER 0x0072b7f0
		// SYNTHETIC: STUDIO 0x007991f0
		// RBX::PhysicsJob::`scalar deleting destructor'

		// FUNCTION: PLAYER 0x0072b880
		virtual TaskScheduler::StepResult stepDataModelJob(const Stats& stats)
		{
			shared_ptr<DataModel> d(dataModel.lock());
			if (!d)
				return TaskScheduler::Done;

			FASTLOG1(FLog::DataModelJobs, "Physics Job start, data model: %p", d.get());
			DataModel::scoped_write_request request(d.get());
			// Read into locals first: the original loads timespanSinceLastStep before timespanOfLastStep.
			double dt = stats.timespanSinceLastStep.seconds();
			double dutyDt = stats.timespanOfLastStep.seconds();
			d->physicsStep(Constants::uiDt(), dt, dutyDt, allotedConcurrency);
			FASTLOG1(FLog::DataModelJobs, "Physics Job finish, data model: %p", d.get());
			return TaskScheduler::Stepped;
		}
	};

	// 2013 layout (ctor PLAYER 0x0072d040): DataModelJob 0x218, runService @0x218, dataModel
	// @0x220 (both weak), fps @0x228 (30.0). No lastHeartbeatStamp, maxStepsPerCycle or step
	// records yet. HeartbeatTask::error has no body of its own in PLAYER.
	// VTABLE: PLAYER 0x00b4d8d8
	// SIZE 0x230
	class HeartbeatTask : public DataModelJob
	{
	public:
		weak_ptr<RunService> const runService;	// 0x218
		weak_ptr<DataModel> const dataModel;	// 0x220
		double fps;								// 0x228

		HeartbeatTask(shared_ptr<RunService> runService);
		virtual Time::Interval sleepTime(const Stats& stats);

		virtual Job::Error error(const Stats& stats)
		{
			return computeStandardError(stats, fps);
		}

		virtual TaskScheduler::StepResult stepDataModelJob(const Stats& stats);
	};

	// FUNCTION: PLAYER 0x0072bb60
	// FUNCTION: STUDIO 0x00799670
	void RunService::stopTasks()
	{
		if (physicsJob)
		{
			TaskScheduler::singleton().remove(physicsJob);
			physicsJob.reset();	// remove the reference to avoid risk of memory leak
		}
		if (heartbeatTask)
		{
			TaskScheduler::singleton().remove(heartbeatTask);
			heartbeatTask.reset();
		}
	}

	// 2013: no Super call survives (Instance::onServiceProvider is empty and inline).
	// FUNCTION: PLAYER 0x0072bc30
	// FUNCTION: STUDIO 0x00799740
	void RunService::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
	{
		if (oldProvider)
			stopTasks();
		Super::onServiceProvider(oldProvider, newProvider);
	}

	// FUNCTION: PLAYER 0x0072d040
	// FUNCTION: STUDIO 0x0079acc0
	HeartbeatTask::HeartbeatTask(shared_ptr<RunService> runService)
		: DataModelJob("Heartbeat", DataModelJob::Write, false, shared_from_dynamic_cast<DataModel>(runService->getParent()), Time::Interval(0.003))
		, runService(runService)
		, dataModel(shared_from_dynamic_cast<DataModel>(runService->getParent()))
		, fps(30)
	{
	}

	// FUNCTION: PLAYER 0x0072d220
	// FUNCTION: STUDIO 0x0079aea0
	Time::Interval HeartbeatTask::sleepTime(const Stats& stats)
	{
		return computeStandardSleepTime(stats, fps);
	}

	// SYNTHETIC: PLAYER 0x0072d250
	// SYNTHETIC: STUDIO 0x0079aed0
	// RBX::HeartbeatTask::~HeartbeatTask

	// SYNTHETIC: PLAYER 0x0072d2e0
	// SYNTHETIC: STUDIO 0x0079af60
	// RBX::HeartbeatTask::`scalar deleting destructor'

	// mac: 0x236688: create PhysicsJob (parent DataModel) + HeartbeatTask (self),
	// scheduler.add both. Byte-identical to 0x0072d300 in a probe compile; PLAYER scores 99.4%
	// because our linker folds shared_from<RunService> into shared_from<GuiItem> and the call
	// pairs with the GuiItem copy (STUDIO pairs it and is at 100%).
	// STUB: PLAYER 0x0072d300
	// FUNCTION: STUDIO 0x0079af80
	void RunService::start()
	{
		physicsJob = shared_ptr<PhysicsJob>(new PhysicsJob(shared_from_dynamic_cast<DataModel>(getParent())));
		heartbeatTask = shared_ptr<HeartbeatTask>(new HeartbeatTask(shared_from(this)));
		TaskScheduler::singleton().add(physicsJob);
		TaskScheduler::singleton().add(heartbeatTask);
	}

	// 2013 has no FASTLOG here (2016 logs the step and the budget).
	// STUB: PLAYER 0x0072da30
	// STUB: STUDIO 0x0079b6b0
	void RunService::raiseHeartbeat(double wallStep, const Time::Interval& stepBudget)
	{
		totalWallTime += wallStep;

		double gameStep = totalGameTime - totalGameTimeAtLastHeartbeat;

		heartbeatSignal(Heartbeat(totalWallTime, wallStep, totalGameTime, gameStep, Time::nowFast() + stepBudget));

		scriptHeartbeatSignal(wallStep);

		totalGameTimeAtLastHeartbeat = totalGameTime;
	}

	// 2013 has no cyclic-executive branch: the heartbeat is raised once per step.
	// STUB: PLAYER 0x0072dc30
	// STUB: STUDIO 0x0079b850
	TaskScheduler::StepResult HeartbeatTask::stepDataModelJob(const Stats& stats)
	{
		shared_ptr<DataModel> d(dataModel.lock());
		if (!d)
			return TaskScheduler::Done;
		shared_ptr<RunService> r(runService.lock());
		if (!r)
			return TaskScheduler::Done;

		FASTLOG1(FLog::DataModelJobs, "Heartbeat start, data model: %p", d.get());
		DataModel::scoped_write_request request(d.get());

		r->raiseHeartbeat(stats.timespanSinceLastStep.seconds(), stepBudget);

		FASTLOG1(FLog::DataModelJobs, "Heartbeat finish, data model: %p", d.get());
		return TaskScheduler::Stepped;
	}

	// FUNCTION: PLAYER 0x0072efd0
	// STUB: STUDIO 0x0079cc10
	RunService::RunService()
		: runState(RS_STOPPED)
		, totalGameTime(0.0)
		, totalGameTimeAtLastHeartbeat(0.0)
		, skippedTimeAccumulated(0.0)
		, totalWallTime(0.0)
	{
		setName("Run Service");
	}

	// mac: 0x236DCE: set runState + fire runTransitionSignal.
	void RunService::setRunState(int state)
	{
		if (state != RS_STOPPED)
		{
			if (state == RS_PAUSED)
			{
				if (runState != RS_RUNNING)
					return;
			}
			else if (state == RS_RUNNING)
			{
				if ((runState | RS_PAUSED) != RS_PAUSED)
					return;
				if (!physicsJob)
					start();
			}
		}
		else
		{
			totalGameTime = 0.0;
			totalGameTimeAtLastHeartbeat = 0.0;
			totalWallTime = 0.0;
			skippedTimeAccumulated = 0.0;
		}
		RunState oldState = runState;
		if (oldState != state)
		{
			runState = (RunState)state;
			runTransitionSignal(RunTransition(oldState, (RunState)state));
		}
	}

} // namespace RBX
