// Network/NetworkOwnerJob.cpp -- October 2013, partial (see NetworkOwnerJob.h).

#include "NetworkOwnerJob.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/NetworkSettings.h"

namespace RBX {
namespace Network {

// Folded with Replicator::SendDataJob::sleepTime.
// FUNCTION: PLAYER 0x005f65c0
// FUNCTION: STUDIO 0x0063c240
Time::Interval NetworkOwnerJob::sleepTime(const Stats& stats)
{
	return computeStandardSleepTime(stats, networkOwnerRate);
}

// FUNCTION: PLAYER 0x005f65f0
// FUNCTION: STUDIO 0x0066a260
TaskScheduler::Job::Error NetworkOwnerJob::error(const Stats& stats)
{
	return computeStandardError(stats, networkOwnerRate);
}

// FUNCTION: PLAYER 0x005f7370
NetworkOwnerJob::NetworkOwnerJob(shared_ptr<DataModel> dataModel)
	: DataModelJob("Distributed Physics Ownership", DataModelJob::Write, false, dataModel, Time::Interval(0))
	, dataModel(dataModel)
	, networkOwnerRate(NetworkSettings::singleton().networkOwnerRate)
{
}

// STUB: PLAYER 0x005f7b20
TaskScheduler::StepResult NetworkOwnerJob::stepDataModelJob(const Stats& stats)
{
	STUB(0x005f7b20);
	return TaskScheduler::Done;
}

} // namespace Network
} // namespace RBX
