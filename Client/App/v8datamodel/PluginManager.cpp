/* App/v8datamodel/PluginManager.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/PluginManager.h"

namespace RBX {
	const char* const sPluginManager = "PluginManager";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::PluginManager);
}

// mac: 0x69DE60/0x69E716/0x69E688 (batches 646/647): call_once singleton.
#include <boost/thread/once.hpp>
#include "Util/Object.h"
#include "V8Tree/Instance.h"

namespace RBX {
namespace {
boost::shared_ptr<PluginManager>& pluginmanagerStatic()
{
	static boost::shared_ptr<PluginManager> inst;
	return inst;
}
boost::once_flag pluginmanagerFlag;
void initPluginManagerSingleton()
{
	// DELTA: Limits::Counter::Activator omitted (limits-tracking only,
	// no functional effect; cf. DataModelJob::step note).
	if (!pluginmanagerStatic())
		pluginmanagerStatic() = Creatable<Instance>::create<PluginManager>();
}
}

boost::shared_ptr<PluginManager> PluginManager::singleton()
{
	boost::call_once(&initPluginManagerSingleton, pluginmanagerFlag);
	return pluginmanagerStatic();
}
} // namespace RBX
