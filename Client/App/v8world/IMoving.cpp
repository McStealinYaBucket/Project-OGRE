/* App/v8world/IMoving.cpp -- October 2013. Asserts: makeMoving :18, checkSleep :40/:50, IMovingManager :87-88/:109; the
   dtor's is inline at IMoving.h:33. FLog::GfxClustersFull is PLAYER 0x00de3f58 (registered in FastLogSettings.cpp). */
#include "stdafx.h"
#include "V8World/IMoving.h"
#include "rbx/Debug.h"
#include "FastLog.h"
LOGGROUP(GfxClustersFull)
namespace RBX {
	IMoving::IMoving()
		: iMovingManager(NULL)
		, stepsToSleep(0)
	{
	}
	// FUNCTION: PLAYER 0x00822e00
	// FUNCTION: STUDIO 0x00916d20
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


	// FUNCTION: PLAYER 0x00822bc0
	// FUNCTION: STUDIO 0x00916960
	bool IMoving::checkSleep()
	{
		RBXASSERT(stepsToSleep > 0);  // IMoving.cpp:40
		if (!isInContinousMotion())
		{
			if (stepsToSleep > 1)
			{
				--stepsToSleep;
				return false;
			}


			RBXASSERT(stepsToSleep == 1);  // IMoving.cpp:50
			stepsToSleep = 0;
			onSleepingChanged(true);
			return true;
		}
		return false;
	}

	// FUNCTION: PLAYER 0x00822ef0
	// FUNCTION: STUDIO 0x00916e10
	void IMoving::notifyMoved()
	{
		FASTLOG1(FLog::GfxClustersFull, "notifyMoved on part %p", this);
		if (stepsToSleep == 0) {
			stepsToSleep = 30;
			makeMoving();
		}
		else {
			stepsToSleep = 30;
		}
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
