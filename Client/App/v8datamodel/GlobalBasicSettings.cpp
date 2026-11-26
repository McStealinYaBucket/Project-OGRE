/* App/v8datamodel/GlobalBasicSettings.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/GlobalBasicSettings.h"
#include "decomp.h"

namespace RBX {
	const char* const sGlobalBasicSettings = "UserSettings";	// 2013 string (PLAYER 0x00b31088 -> 0x00b31060)
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::GlobalBasicSettings);
}

#include "Util/FileSystem.h"

namespace RBX {
// "<user directory>\GlobalBasicSettings_13.xml", or "" when there is no user directory.
// FUNCTION: PLAYER 0x00656390
// FUNCTION: STUDIO 0x0073eed0
static std::string basicSettingsFile()
{
	std::string result = FileSystem::getUserDirectory(true, DirAppData, NULL);
	if (result.size() > 0)
		result += "\\GlobalBasicSettings_13.xml";
	return result;
}

// The settings file reaches Settings through the out-of-line DescribedNonCreatable,
// Described and NonFactoryProduct ctors (PLAYER 0x00658500, 0x006583b0, 0x006581f0).
// FUNCTION: PLAYER 0x00658640
// FUNCTION: STUDIO 0x00740dd0
GlobalBasicSettings::GlobalBasicSettings()
	: DescribedNonCreatable<GlobalBasicSettings, Settings, sGlobalBasicSettings>(basicSettingsFile())
{
	setName("GlobalBasicSettings");
}
} // namespace RBX

// mac: 0x3909B6/0x390915/0x390887 (batches 646/647): call_once singleton.
// 2013: the same shape as GlobalAdvancedSettings::singleton (GlobalAdvancedSettings.cpp), without
// the raw-pointer publish: the static getter (return slot in esi), initBasicSingleton under an
// empty Limits::Counter::Activator, and singleton() through call_once.
#include <boost/thread/once.hpp>
#include "Util/Object.h"
#include "Util/Limits.h"
#include "V8Tree/Instance.h"

namespace RBX {
namespace {
boost::once_flag globalbasicsettingsFlag;
}
// FUNCTION: PLAYER 0x006589b0
// FUNCTION: STUDIO 0x00741140
static boost::shared_ptr<GlobalBasicSettings> globalbasicsettingsStatic()
{
	static boost::shared_ptr<GlobalBasicSettings> inst = Creatable<Instance>::create<GlobalBasicSettings>();
	return inst;
}
// FUNCTION: PLAYER 0x00658a40
// FUNCTION: STUDIO 0x007411d0
static void initBasicSingleton()
{
	boost::shared_ptr<Limits::Counter> noCounter;
	Limits::Counter::Activator activator(noCounter);
	globalbasicsettingsStatic();
}

// FUNCTION: PLAYER 0x00658ae0
// FUNCTION: STUDIO 0x00741270
boost::shared_ptr<GlobalBasicSettings> GlobalBasicSettings::singleton()
{
	boost::call_once(&initBasicSingleton, globalbasicsettingsFlag);
	return globalbasicsettingsStatic();
}
} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::GlobalBasicSettings, 0xb4);
