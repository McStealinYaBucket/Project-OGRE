// Network/Replicator.cpp -- October 2013, partial: PingJob's rate functions only.

#include "Replicator.PingJob.h"
#include "decomp.h"

namespace RBX {
namespace Network {

// FUNCTION: PLAYER 0x00598da0
// FUNCTION: STUDIO 0x0063be70
Time::Interval Replicator::PingJob::sleepTime(const Stats& stats)
{
	return computeStandardSleepTime(stats, 2);
}

// FUNCTION: PLAYER 0x00598dd0
// FUNCTION: STUDIO 0x0063bea0
Replicator::PingJob::Error Replicator::PingJob::error(const Stats& stats)
{
	return computeStandardError(stats, 2);
}

// VMProtect-mutated in PLAYER.
// STUB: PLAYER 0x0059ea20
// STUB: STUDIO 0x00642040
TaskScheduler::StepResult Replicator::PingJob::stepDataModelJob(const Stats& stats)
{
	STUB(0x0059ea20);
	return TaskScheduler::Done;
}

} // namespace Network
} // namespace RBX
