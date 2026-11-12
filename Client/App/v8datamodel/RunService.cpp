/* App/v8datamodel/RunService.cpp -- October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/RunService.h"
#include "V8DataModel/PhysicsJob.h"
#include "V8DataModel/HeartbeatTask.h"
#include "V8DataModel/DataModel.h"
#include "rbx/TaskScheduler.h"

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

	// mac: 0x236688: create PhysicsJob (parent DataModel) + HeartbeatTask (self),
	// store at +96/+104, scheduler.add both.
	// STUB: PLAYER 0x0072d300
	// STUB: STUDIO 0x0079af80
	void RunService::start()
	{
		shared_ptr<DataModel> dataModel = shared_from_dynamic_cast<DataModel>(getParent());
		physicsJob = shared_ptr<PhysicsJob>(new PhysicsJob(dataModel));
		heartbeatJob = shared_ptr<HeartbeatTask>(
			new HeartbeatTask(shared_from_dynamic_cast<RunService>(this)));
		TaskScheduler::singleton().add(physicsJob);
		TaskScheduler::singleton().add(heartbeatJob);
	}

	// mac: 0x236DCE: set runState + fire runTransitionSignal.
	void RunService::setRunState(int a2)
	{
		int* p = (int*)this;
		if (a2 != 0)
		{
			if (a2 == 2)
			{
				if (p[28] != 1)
					return;
			}
			else if (a2 == 1)
			{
				if ((p[28] | 2) != 2)
					return;
				if (p[24] == 0)
					start();
			}
		}
		else
		{
			p[30] = 0;
			p[29] = 0;
			p[32] = 0;
			p[31] = 0;
			p[34] = 0;
			p[33] = 0;
			p[36] = 0;
			p[35] = 0;
		}
		int v2 = p[28];
		if (v2 != a2)
		{
			p[28] = a2;
			runTransitionSignal(RunTransition((RunState)v2, (RunState)a2));
		}
	}

} // namespace RBX