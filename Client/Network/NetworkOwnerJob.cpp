// Network/NetworkOwnerJob.cpp -- October 2013, partial (see NetworkOwnerJob.h): STUBs only.

#include "NetworkOwnerJob.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/NetworkSettings.h"

namespace RBX {
namespace Network {

// Folded with Replicator::SendDataJob::sleepTime.
// STUB: PLAYER 0x005f65c0
Time::Interval NetworkOwnerJob::sleepTime(const Stats& stats)
{
	STUB(0x005f65c0);
	return Time::Interval(0);
}

// STUB: PLAYER 0x005f65f0
TaskScheduler::Job::Error NetworkOwnerJob::error(const Stats& stats)
{
	STUB(0x005f65f0);
	return Job::Error();
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
