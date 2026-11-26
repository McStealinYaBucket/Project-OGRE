/* App/v8datamodel/GlobalAdvancedSettings.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/GlobalAdvancedSettings.h"
#include "decomp.h"

namespace RBX {
	const char* const sGlobalAdvancedSettings = "GlobalSettings";	// 2013 string (PLAYER 0x00b31084 -> 0x00b31070)
	const char* const sSettingsItem = NULL;
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
// GlobalAdvancedSettings registers in FactoryRegistration.cpp (2013 initializer 0x00a75a20).
}

// mac: 0x3903F8/0x390351/0x3902C3 (batches 646/647): call_once singleton.
// 2013 (PLAYER): singleton 0x00658980 calls the static getter 0x00658850 out of line (return slot
// in esi), which also publishes the raw pointer (0x00de6930, written only); initAdvancedSingleton
// 0x006588e0 runs the getter under a Limits::Counter::Activator for an empty counter.
#include <boost/thread/once.hpp>
#include "Util/Object.h"
#include "Util/Limits.h"
#include "V8Tree/Instance.h"

#include "Util/FileSystem.h"
#include "V8DataModel/Selection.h"

namespace RBX {
// "<user directory>\GlobalSettings_13.xml", or "" when there is no user directory.
// FUNCTION: PLAYER 0x00656320
// FUNCTION: STUDIO 0x0073ee60
static std::string globalAdvancedSettingsFile()
{
	std::string result = FileSystem::getUserDirectory(true, DirAppData, NULL);
	if (result.size() > 0)
		result += "\\GlobalSettings_13.xml";
	return result;
}

// Ends by creating the Selection service (ServiceProvider::create<Selection> 0x00656f10).
// FUNCTION: PLAYER 0x00658580
// FUNCTION: STUDIO 0x00740d10
GlobalAdvancedSettings::GlobalAdvancedSettings()
	: DescribedNonCreatable<GlobalAdvancedSettings, Settings, sGlobalAdvancedSettings>(globalAdvancedSettingsFile())
{
	setName("Global Settings");
	create<Selection>();
}

GlobalAdvancedSettings* g_sing = NULL;
namespace {
boost::once_flag globaladvancedsettingsFlag;
}
// static: VC9 passes the return slot in esi (custom convention for internal functions).
// FUNCTION: PLAYER 0x00658850
// FUNCTION: STUDIO 0x00740fe0
static boost::shared_ptr<GlobalAdvancedSettings> globaladvancedsettingsStatic()
{
	static boost::shared_ptr<GlobalAdvancedSettings> inst = Creatable<Instance>::create<GlobalAdvancedSettings>();
	g_sing = inst.get();
	return inst;
}
// FUNCTION: PLAYER 0x006588e0
// FUNCTION: STUDIO 0x00741070
static void initAdvancedSingleton()
{
	boost::shared_ptr<Limits::Counter> noCounter;
	Limits::Counter::Activator activator(noCounter);
	globaladvancedsettingsStatic();
}

// FUNCTION: PLAYER 0x00658980
// FUNCTION: STUDIO 0x00741110
boost::shared_ptr<GlobalAdvancedSettings> GlobalAdvancedSettings::singleton()
{
	boost::call_once(&initAdvancedSingleton, globaladvancedsettingsFlag);
	return globaladvancedsettingsStatic();
}
} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::GlobalAdvancedSettings, 0xb4);
