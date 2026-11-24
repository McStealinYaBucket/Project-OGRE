/* App/v8datamodel/TimerService.cpp -- October 2013. TimerService.obj is PLAYER 0x007a67a0-0x007a6e90
   (STUDIO 0x008bb150...): delay, onHeartbeat and the ctor, in address order, from the PLAYER bytes.
   No __FILE__ is recorded for it (no asserts); the path is the 2016 tree's. Its classDescriptor
   copy sits in the factory registration block (0x00789270), so it registers there. */

#include "stdafx.h"

#include "V8DataModel/TimerService.h"
#include "Util/standardout.h"
#include "decomp.h"

DECOMP_SIZE_ASSERT(RBX::TimerService, 0x78);

namespace RBX {
	const char* const sTimerService = "TimerService";

	// The list stays sorted: the new item goes in front of the first later one, else at the end.
	// STUB: PLAYER 0x007a6a30
	// FUNCTION: STUDIO 0x008bb150
	void TimerService::delay(boost::function0<void> func, double seconds)
	{
		Item item;
		item.time = Time::now<Time::Fast>() + Time::Interval(seconds);
		item.func = func;

		bool inserted = false;
		for (std::list<Item>::iterator iter = items.begin(); iter != items.end(); ++iter)
		{
			if (iter->time > item.time)
			{
				items.insert(iter, item);
				inserted = true;
				break;
			}
		}

		if (!inserted)
			items.push_back(item);
	}

	// Runs every item that is due, front first; a script error in one is printed as a warning.
	// FUNCTION: PLAYER 0x007a6b70
	// FUNCTION: STUDIO 0x008bb290
	void TimerService::onHeartbeat(const Heartbeat& heartbeat)
	{
		Time now = Time::now<Time::Fast>();
		while (items.size() > 0)
		{
			const Item& item = items.front();
			if (!(now >= item.time))
				break;

			try
			{
				item.func();
			}
			catch (RBX::base_exception& e)
			{
				StandardOut::singleton()->print(MESSAGE_WARNING, e);
			}
			items.pop_front();
		}
	}

	// FUNCTION: PLAYER 0x007a6d10
	// FUNCTION: STUDIO 0x008bb430
	TimerService::TimerService()
	{
	}

} // namespace RBX
