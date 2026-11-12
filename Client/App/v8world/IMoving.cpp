/* App/v8world/IMoving.cpp -- October 2013.
   Bodies per decomp (see header EAs). Asserts carried with their
   IMoving.cpp line numbers (18/40/50) + IMoving.h:33; RBXASSERT maps to the
   tree's FLog::Asserts-gated crash assert (live in test exes).
   TODO: FastLog GfxClusters/GfxClustersFull calls in notifyMoved (0x5767DE)
   and onMovingHeartbeat (0x5769D4); onMovingHeartbeat body itself.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/IMoving.cpp */

#include "stdafx.h"

#include "V8World/IMoving.h"
#include "rbx/Debug.h"

namespace RBX {

	IMoving::IMoving()
		: iMovingManager(NULL)
		, stepsToSleep(0)
	{
	}

	IMoving::~IMoving()
	{
		// IMoving.h:33: must be detached before destruction.
		RBXASSERT(iMovingManager == NULL);
	}

	// STUB: PLAYER 0x00822e00
	// STUB: STUDIO 0x00916d20
	void IMoving::makeMoving()
	{
		RBXASSERT(stepsToSleep > 0);  // IMoving.cpp:18
		onSleepingChanged(false);
		if (iMovingManager != NULL)
			iMovingManager->moving.insert(this);
	}

	// STUB: PLAYER 0x00822e80
	// STUB: STUDIO 0x00916da0
	void IMoving::setMovingManager(IMovingManager* manager)
	{
		if (iMovingManager != NULL)
			iMovingManager->remove(this);
		iMovingManager = manager;
		if (manager != NULL && stepsToSleep > 0)
			makeMoving();
	}

	// STUB: PLAYER 0x00822bc0
	// STUB: STUDIO 0x00916960
	int IMoving::checkSleep()
	{
		RBXASSERT(stepsToSleep > 0);  // IMoving.cpp:40
		if (!isInContinousMotion())
		{
			if (stepsToSleep >= 2)
			{
				--stepsToSleep;
				return 0;
			}
			RBXASSERT(stepsToSleep == 1);  // IMoving.cpp:50
			stepsToSleep = 0;
			onSleepingChanged(true);
			return 1;
		}
		return 0;
	}

	// STUB: PLAYER 0x00822ef0
	// STUB: STUDIO 0x00916e10
	void IMoving::notifyMoved()
	{
		// TODO: FastLog "notifyMoved on %p" via FLog::GfxClustersFull (channel absent).
		bool wasAsleep = (stepsToSleep == 0);
		stepsToSleep = 30;
		if (wasAsleep)
			makeMoving();
	}

	void IMoving::forceSleep()
	{
		stepsToSleep = 0;
		if (iMovingManager != NULL)
			iMovingManager->remove(this);
	}

	IMovingManager::IMovingManager()
		: moving()
		, current(moving.end())
	{
	}

	IMovingManager::~IMovingManager()
	{
	}

	void IMovingManager::remove(IMoving* m)
	{
		if (current != moving.end() && *current == m)
			++current;
		moving.erase(m);
	}

	bool IMovingManager::contains(IMoving* m) const
	{
		return moving.find(m) != moving.end();
	}

} // namespace RBX
