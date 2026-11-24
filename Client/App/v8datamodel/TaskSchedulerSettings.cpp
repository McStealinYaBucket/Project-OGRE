/* App/v8datamodel/TaskSchedulerSettings.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/TaskSchedulerSettings.h"

namespace RBX {
	const char* const sTaskSchedulerSettings = "TaskScheduler";	// 2013 string (pushed by GlobalAdvancedSettingsItem ctor 0x0065ecd0)
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::TaskSchedulerSettings);
}

namespace RBX {

// FUNCTION: PLAYER 0x0065fb60
// FUNCTION: STUDIO 0x00778ae0
TaskSchedulerSettings::TaskSchedulerSettings()
	: m_unk0x64(0)
{
	Instance::setName("Task Scheduler");
}

} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::TaskSchedulerSettings, 0x68);
