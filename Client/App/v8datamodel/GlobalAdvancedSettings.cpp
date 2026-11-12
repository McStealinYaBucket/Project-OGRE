/* App/v8datamodel/GlobalAdvancedSettings.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/GlobalAdvancedSettings.h"

namespace RBX {
	const char* const sGlobalAdvancedSettings = "GlobalAdvancedSettings";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::GlobalAdvancedSettings);
}

// mac: 0x3903F8/0x390351/0x3902C3 (batches 646/647): call_once singleton.
#include <boost/thread/once.hpp>
#include "Util/Object.h"
#include "V8Tree/Instance.h"

namespace RBX {
namespace {
boost::shared_ptr<GlobalAdvancedSettings>& globaladvancedsettingsStatic()
{
	static boost::shared_ptr<GlobalAdvancedSettings> inst;
	return inst;
}
boost::once_flag globaladvancedsettingsFlag;
void initAdvancedSingleton()
{
	// DELTA: Limits::Counter::Activator omitted (limits-tracking only,
	// no functional effect; cf. DataModelJob::step note).
	if (!globaladvancedsettingsStatic())
		globaladvancedsettingsStatic() = Creatable<Instance>::create<GlobalAdvancedSettings>();
}
}

boost::shared_ptr<GlobalAdvancedSettings> GlobalAdvancedSettings::singleton()
{
	boost::call_once(&initAdvancedSingleton, globaladvancedsettingsFlag);
	return globaladvancedsettingsStatic();
}
} // namespace RBX
