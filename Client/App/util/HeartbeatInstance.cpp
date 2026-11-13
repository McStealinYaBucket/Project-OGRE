/* App/util/HeartbeatInstance.cpp — October 2013 (mac 0x204C28: ReleaseAssert pair + connection dance). */
#include "stdafx.h"
#include "Util/HeartbeatInstance.h"
#include "Util/RunTransition.h"
#include "V8Tree/Service.h"
#include "V8DataModel/RunService.h"
#include <boost/bind.hpp>
namespace RBX {
void HeartbeatInstance::onServiceProviderHeartbeatInstance(ServiceProvider* oldProvider, ServiceProvider* newProvider) {
	RBXASSERT((oldProvider == NULL) != (newProvider == NULL));

	heartbeatConnection.disconnect();

	if (newProvider) {
		RunService* runService = ServiceProvider::create<RunService>(newProvider);
		RBXASSERT(runService);
		heartbeatConnection = runService->heartbeatSignal.connect(boost::bind(&HeartbeatInstance::onHeartbeat, this, _1));
	}
}

} // namespace
