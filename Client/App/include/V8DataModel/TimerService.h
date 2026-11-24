/* V8DataModel/TimerService.h -- October 2013.
   Layout from the PLAYER ctor 0x007a6d10 (STUDIO 0x008bb430) and dtor 0x007a6de0:
   - DescribedCreatable<TimerService, Instance, sTimerService, 11 = PERSISTENT_HIDDEN> (RTTI) to 0x60;
   - HeartbeatInstance at 0x60 (vftable 0x00b32cd4 stored by its inline ctor, connection at 0x64);
     its slots in TimerService's vftable 0x00b62d38 are onHeartbeat 0x007a6b70, then the deleting
     dtor adjustor 0x007a6dd0;
   - Service at 0x68 (the ctor stores 1);
   - the std::list of Items at 0x6c (head 0x70, size 0x74). A node is 0x30 (new 0x30 in
     _Buynode 0x007a6940): Item.time at node+8, Item.func (boost::function0, 8-aligned
     functor) at node+0x10.
   TimerService::onServiceProvider is inline: Instance's is empty, so the vftable slot 0x007a6da0
   is `add ecx,0x60` plus a tail jump to onServiceProviderHeartbeatInstance. */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "Util/HeartbeatInstance.h"
#include "rbx/rbxtime.h"
#include <boost/function.hpp>
#include <list>

namespace RBX {

	extern const char* const sTimerService;

	// VTABLE: PLAYER 0x00b62d38 RBX::HeartbeatInstance
	// VTABLE: STUDIO 0x00c30000 RBX::HeartbeatInstance
	// VTABLE: PLAYER 0x00b62d6c RBX::Debugable
	// VTABLE: STUDIO 0x00c30034 RBX::Debugable
	// SIZE 0x78
	class TimerService
		: public DescribedCreatable<TimerService, Instance, sTimerService, Reflection::ClassDescriptor::PERSISTENT_HIDDEN>
		, public Service
		, public HeartbeatInstance	// 0x60 (polymorphic, so ahead of Service)
	{
		typedef DescribedCreatable<TimerService, Instance, sTimerService, Reflection::ClassDescriptor::PERSISTENT_HIDDEN> Super;

		class Item
		{
		public:
			Time time;						// node+0x08
			boost::function0<void> func;	// node+0x10
		};

		std::list<Item> items;	// 0x6c, kept sorted by time

		// FUNCTION: PLAYER 0x007a6da0
		// FUNCTION: STUDIO 0x008bb4c0
		/*override*/ void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
		{
			onServiceProviderHeartbeatInstance(oldProvider, newProvider);
		}

		/*override*/ void onHeartbeat(const Heartbeat& heartbeat);

	public:
		TimerService();

		// Calls func once, the first heartbeat after `seconds` have passed.
		void delay(boost::function0<void> func, double seconds);
	};

} // namespace RBX
