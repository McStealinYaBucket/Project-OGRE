/* App/v8datamodel/DebrisService.cpp -- October 2013. cleanup, setMaxItems and the ctor from the PLAYER
   bytes (0x008446f0..0x008455c0); addItem/onServiceProvider need TimerService. */

#include "stdafx.h"

#include "V8DataModel/DebrisService.h"
#include "security/SecurityContext.h"
#include "decomp.h"
#include "Util/standardout.h"

DECOMP_SIZE_ASSERT(RBX::DebrisService, 0x94);

namespace RBX {
	const char* const sDebrisService = "Debris";  // 0.125 metadata + 2016 value (was "DebrisService")
}

using namespace RBX;

REFLECTION_BEGIN();
// prop_MaxItems: PLAYER 0x00e1207c, STUDIO 0x00f9aa34. A file static: a GLOBAL marker on it
// does not parse (decomplint global_not_variable) and does not pair.
static Reflection::PropDescriptor<DebrisService, int> prop_MaxItems("MaxItems", category_Data, &DebrisService::getMaxItems, &DebrisService::setMaxItems);
REFLECTION_END();

// 2013 calls Instance::remove() (2016: destroy()).
// STUB: PLAYER 0x008446f0
// STUB: STUDIO 0x0098a220
static void cleanup(weak_ptr<Instance> item)
{
	if (shared_ptr<Instance> i = item.lock())
		try
		{
			i->remove();
		}
		catch (RBX::base_exception& e)
		{
			StandardOut::singleton()->print(MESSAGE_WARNING, e);
		}
}

namespace RBX {

	// FUNCTION: PLAYER 0x00844c20
	// FUNCTION: STUDIO 0x0098a5f0
	void DebrisService::cleanup()
	{
		while ((int)queue.size() > maxItems)
		{
			::cleanup(queue.front());
			queue.pop();
		}
	}

	// 2013 pushes permission 2 where 2016 names LocalUser (3 in our Security enum).
	// STUB: PLAYER 0x00844e90
	// STUB: STUDIO 0x0098a860
	void DebrisService::setMaxItems(int value)
	{
		if (!legacyMaxItems)
			RBX::Security::Context::current().requirePermission(RBX::Security::LocalUser, "DebrisService MaxItems is restricted");
		if (value != maxItems)
		{
			if (value < 0)
				throw std::runtime_error("DebrisService MaxItems must be greater than 0");
			maxItems = value;
			raisePropertyChanged(prop_MaxItems);
			cleanup();
		}
	}

	// FUNCTION: PLAYER 0x008455c0
	// FUNCTION: STUDIO 0x0098af90
	DebrisService::DebrisService()
		: Super(sDebrisService)
		, maxItems(1000)
		, legacyMaxItems(false)
	{
	}

} // namespace RBX

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::DebrisService);
}
