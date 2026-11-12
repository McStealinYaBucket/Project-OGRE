/* App/v8datamodel/GlobalBasicSettings.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/GlobalBasicSettings.h"

namespace RBX {
	const char* const sGlobalBasicSettings = "GlobalBasicSettings";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::GlobalBasicSettings);
}

// mac: 0x3909B6/0x390915/0x390887 (batches 646/647): call_once singleton.
#include <boost/thread/once.hpp>
#include "Util/Object.h"
#include "V8Tree/Instance.h"

namespace RBX {
namespace {
boost::shared_ptr<GlobalBasicSettings>& globalbasicsettingsStatic()
{
	static boost::shared_ptr<GlobalBasicSettings> inst;
	return inst;
}
boost::once_flag globalbasicsettingsFlag;
void initBasicSingleton()
{
	// DELTA: Limits::Counter::Activator omitted (limits-tracking only,
	// no functional effect; cf. DataModelJob::step note).
	if (!globalbasicsettingsStatic())
		globalbasicsettingsStatic() = Creatable<Instance>::create<GlobalBasicSettings>();
}
}

boost::shared_ptr<GlobalBasicSettings> GlobalBasicSettings::singleton()
{
	boost::call_once(&initBasicSingleton, globalbasicsettingsFlag);
	return globalbasicsettingsStatic();
}
} // namespace RBX
