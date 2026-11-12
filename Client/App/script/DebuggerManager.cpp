/* App/v8datamodel/DebuggerManager.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/DebuggerManager.h"

namespace RBX {
	const char* const sDebuggerManager = "DebuggerManager";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::DebuggerManager);
}

// mac: 0x725F74/0x726035/sub_725FA7 (batches 646/647): call_once singleton.
#include <boost/thread/once.hpp>
#include "Util/Object.h"
#include "V8Tree/Instance.h"

namespace RBX {
namespace {
boost::shared_ptr<DebuggerManager>& debuggermanagerStatic()
{
	static boost::shared_ptr<DebuggerManager> inst;
	return inst;
}
boost::once_flag debuggermanagerFlag;
void initDebuggerManagerSingleton()
{
	// DELTA: Limits::Counter::Activator omitted (limits-tracking only,
	// no functional effect; cf. DataModelJob::step note).
	if (!debuggermanagerStatic())
		debuggermanagerStatic() = Creatable<Instance>::create<DebuggerManager>();
}
}

boost::shared_ptr<DebuggerManager> DebuggerManager::singleton()
{
	boost::call_once(&initDebuggerManagerSingleton, debuggermanagerFlag);
	return debuggermanagerStatic();
}
} // namespace RBX
