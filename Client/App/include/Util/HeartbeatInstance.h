/* Util/HeartbeatInstance.h — October 2013.
   2016 carry (Client/App/include/util/HeartbeatInstance.h) verified against
   the mac binary: HeartbeatInstance::onServiceProviderHeartbeatInstance
   @0x204C28 — RBXASSERT((old==NULL)!=(new==NULL)), heartbeatConnection
   (this+4) disconnect, newProvider -> ServiceProvider::create<RunService>
   -> heartbeatSignal.connect(bind(&HeartbeatInstance::onHeartbeat,this,_1));
   layout: vptr@+0, scoped_connection@+4.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/include/util/HeartbeatInstance.h */

#pragma once

#include "rbx/signal.h"

// hook up by overriding onServiceProvider call in this pattern:
//
//	/*override*/ void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider) {
//		Super::onServiceProvider(oldProvider, newProvider);
//		onServiceProviderHeartbeatInstance(oldProvider, newProvider);		// hooks up heartbeat
//	}
//

namespace RBX {
	class Heartbeat;
	class ServiceProvider;

	class HeartbeatInstance
	{
	private:
		rbx::signals::scoped_connection heartbeatConnection;	// +4 (0x204C9B)

	protected:
		// call this inside onServiceProvider
		void onServiceProviderHeartbeatInstance(ServiceProvider* oldProvider, ServiceProvider* newProvider);	// 0x204C28

		/*implement*/ virtual void onHeartbeat(const Heartbeat& event) = 0;

	public:
		HeartbeatInstance()				{}
		virtual ~HeartbeatInstance()	{}
	};
}	// namespace RBX
