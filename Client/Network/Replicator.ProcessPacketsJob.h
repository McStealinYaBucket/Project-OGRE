#pragma once

// Network/Replicator.ProcessPacketsJob.h -- October 2013, partial: the ctor, isAwake and sleepTime.
// error (0x00598fc0) and stepDataModelJob (0x0059eab0) are not written.

#include "Replicator.h"
#include "V8DataModel/NetworkSettings.h"
#include "decomp.h"

namespace RBX {
namespace Network {

class Replicator::ProcessPacketsJob : public ReplicatorJob
{
	rbx::atomic<int> isAwake;                          // 0x220 Replicator::disableProcessPackets 0x00581d30
public:
	// 2013: no dm member and no cyclic-executive setup (2016). Assert on line 23.
	// FUNCTION: PLAYER 0x00598e00
	// FUNCTION: STUDIO 0x0063bef0
	ProcessPacketsJob(Replicator& replicator)
		: ReplicatorJob("Net ProcessPackets", replicator, DataIn)
	{
		RBXASSERT(getArbiter());
		isAwake = 1;
	}

	void sleep() { isAwake = false; }

private:
	// FUNCTION: PLAYER 0x00598f20
	// FUNCTION: STUDIO 0x0063c020
	virtual Time::Interval sleepTime(const Stats& stats)
	{
		if (!isAwake)
			return Time::Interval::max();

		if (replicator)
		{
			return replicator->incomingPacketsCount() > 0
				? computeStandardSleepTime(stats, replicator->settings().getReceiveRate())
				: Time::Interval(0.1);
		}
		return Time::Interval(0.1);
	}

	// STUB: PLAYER 0x00598fc0
	// STUB: STUDIO 0x0063c0c0
	virtual Error error(const Stats& stats)
	{
		STUB(0x00598fc0);
		return Error();
	}

	// STUB: PLAYER 0x0059eab0
	virtual TaskScheduler::StepResult stepDataModelJob(const Stats& stats)
	{
		STUB(0x0059eab0);
		return TaskScheduler::Stepped;
	}
};

} // namespace Network
} // namespace RBX
