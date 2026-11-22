#pragma once

// Network/Replicator.PingJob.h -- October 2013, partial. The 2013 job runs at 2 Hz and its error()
// calls computeStandardError directly (sleepTime PLAYER 0x00598da0, error 0x00598dd0); 2016 uses
// computeStandardErrorCyclicExecutiveSleeping. 2016 defines these inline here; ours are out of line
// in Replicator.cpp until the job's ctor (0x00598c90) is written and emits them.

#include "Replicator.h"

namespace RBX {
namespace Network {

class Replicator::PingJob : public ReplicatorJob
{
private:
	virtual Time::Interval sleepTime(const Stats& stats);
	virtual Error error(const Stats& stats);
	virtual TaskScheduler::StepResult stepDataModelJob(const Stats& stats);
};

} // namespace Network
} // namespace RBX
