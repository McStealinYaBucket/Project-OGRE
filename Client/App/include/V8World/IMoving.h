/* V8World/IMoving.h -- October 2013 (sleep/wake motion tracking). Mac: IMoving C2 0x576584, D2 0x44B8C8, makeMoving
   0x5765AC, setMovingManager 0x576648, checkSleep 0x5766E8, notifyMoved 0x5767DE, forceSleep 0x576830; IMovingManager
   C2 0x576858, remove 0x576688, onMovingHeartbeat 0x5769D4 (TODO). vtable: 6 pure virtuals, no virtual dtor (makeMoving
   0x00822e00 calls onSleepingChanged through slot 0); the dtor is inline here, assert at IMoving.h:33 (inlined into
   PartInstance::~PartInstance 0x00723190). 2013 has no MovementHistory/lastCFrame (2016-only), and
   onNetworkIsSleepingChanged takes no Time. makeMoving/notifyMoved/setMovingManager/forceSleep return void here
   (binary returns insert-status/char, ignored on every known caller path - TODO-verify). */

#pragma once

#include <set>
#include "rbx/Debug.h"

namespace RBX {

	class IMovingManager;











	class IMoving
	{
	public:
		IMoving();
		~IMoving() {
			RBXASSERT(!iMovingManager);
		}

		// Slot 0. Called with false on wake (makeMoving), true on sleep (checkSleep).
		virtual void onSleepingChanged(bool sleeping) = 0;
		// Slot 1.
		virtual bool reportTouches() const = 0;  // Slot 1 (0x4446A6; thunk casts to u8).
		// Slot 2.
		virtual void onClumpChanged() = 0;  // TODO-verify signature (binary slot only).
		// Slot 3. NOTE: no Time arg in 2013 (2016 takes one).
		virtual void onNetworkIsSleepingChanged() = 0;  // TODO-verify signature (binary slot only).
		// Slot 4.
		virtual void onBuoyancyChanged(bool buoyant) = 0;  // TODO-verify signature (binary slot only).
		// Slot 5. False while settling (checkSleep counts down), true in continuous motion.
		virtual bool isInContinousMotion() = 0;  // NOTE: binary spells Continous (one u).

		// mac: 0x5765AC: assert steps>0; onSleepingChanged(false); insert into manager set.
		void makeMoving();
		// mac: 0x576648: detach from old manager, attach to new (re-makeMoving if awake).
		void setMovingManager(IMovingManager* manager);
		// mac: 0x5766E8: heartbeat tick. True when this call put us to sleep (0x00822bc0 returns al).
		bool checkSleep();
		// mac: 0x5767DE: steps=30; wake (makeMoving) if we were asleep.
		void notifyMoved();
		// mac: 0x576830: steps=0 + detach (no sleep notification).
		void forceSleep();

	private:
		IMovingManager* iMovingManager;
		int stepsToSleep;
	};

	class IMovingManager
	{
	public:
		IMovingManager();
		virtual ~IMovingManager();

		// mac: 0x576688: iterator-aware erase (bumps current past the erased member).
		void remove(IMoving* moving);

		// mac: TODO: onMovingHeartbeat 0x5769D4 (needs Heartbeat pump + checkSleep/remove loop).
		// Bring-up query (not in binary): membership test for unit tests.
		bool contains(IMoving* moving) const;

	private:
		friend class IMoving;
		std::set<IMoving*> moving;
		std::set<IMoving*>::iterator current;
	};

} // namespace RBX
