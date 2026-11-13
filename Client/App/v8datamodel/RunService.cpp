/* App/v8datamodel/RunService.cpp -- October 2013. stopTasks, onServiceProvider, start and the ctor from
   the PLAYER bytes (0x0072bb60..0x0072efd0); reflection and the stepping bodies not written yet. */

#include "stdafx.h"

#include "V8DataModel/RunService.h"
#include "V8DataModel/PhysicsJob.h"
#include "V8DataModel/HeartbeatTask.h"
#include "V8DataModel/DataModel.h"
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

	// mac: 0x236688: create PhysicsJob (parent DataModel) + HeartbeatTask (self),
	// scheduler.add both.
	// STUB: PLAYER 0x0072d300
	// STUB: STUDIO 0x0079af80
	void RunService::start()
	{
		shared_ptr<DataModel> dataModel = shared_from_dynamic_cast<DataModel>(getParent());
		physicsJob = shared_ptr<PhysicsJob>(new PhysicsJob(dataModel));
		heartbeatTask = shared_ptr<HeartbeatTask>(
			new HeartbeatTask(shared_from_dynamic_cast<RunService>(this)));
		TaskScheduler::singleton().add(physicsJob);
		TaskScheduler::singleton().add(heartbeatTask);
	}

	// FUNCTION: PLAYER 0x0072efd0
	// STUB: STUDIO 0x0079cc10
	RunService::RunService()
		: runState(RS_STOPPED)
		, totalGameTime(0.0)
		, totalGameTimeAtLastHeartbeat(0.0)
		, totalWallTime(0.0)
		, skippedTimeAccumulated(0.0)
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
