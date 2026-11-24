/* V8DataModel/TaskSchedulerSettings.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED FactoryProduct<TaskSchedulerSettings, GlobalAdvancedSettings::Item> 0x369BC2 (Item template TODO; C++ TaskScheduler singleton separate);
     sTaskSchedulerSettings = "TaskSchedulerSettings".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/TaskSchedulerSettings.h */

#pragma once

#include "V8DataModel/GlobalAdvancedSettings.h"
#include "decomp.h"

namespace RBX {

	extern const char* const sTaskSchedulerSettings;

	// 2013 base: GlobalAdvancedSettingsItem<TaskSchedulerSettings, sTaskSchedulerSettings> (ctor
	// PLAYER 0x0065ecd0). The 2013 ctor 0x0065fb60 stores 0 at +0x64 and names the instance
	// "Task Scheduler"; size 0x68 (Creator::create 0x0040d720).
	// SIZE 0x68
	class TaskSchedulerSettings
		: public GlobalAdvancedSettingsItem<TaskSchedulerSettings, sTaskSchedulerSettings>
	{
	public:
		TaskSchedulerSettings();

	private:
		undefined4 m_unk0x64;	// 0x64
	};

} // namespace RBX

// The FactoryProduct base's out-of-line members; their names spell the 2013 base
// GlobalAdvancedSettings::Item (RTTI), so they pair through these markers.
// clang-format off
// TEMPLATE: PLAYER 0x0065a070
// TEMPLATE: STUDIO 0x00774a00
// ?static_getCreator@?$FactoryProduct@VTaskSchedulerSettings@RBX@@VItem@GlobalAdvancedSettings@2@$E?sTaskSchedulerSettings@2@3QBDBVInstance@2@@RBX@@KAABVCreator@12@XZ

// TEMPLATE: PLAYER 0x0065a880
// TEMPLATE: STUDIO 0x007751d0
// ?getClassName@?$FactoryProduct@VTaskSchedulerSettings@RBX@@VItem@GlobalAdvancedSettings@2@$E?sTaskSchedulerSettings@2@3QBDBVInstance@2@@RBX@@UBEABVName@2@XZ

// SYNTHETIC: PLAYER 0x0065eda0
// SYNTHETIC: STUDIO 0x00778b70
// ??_ETaskSchedulerSettings@RBX@@WBM@AEPAXI@Z
// clang-format on
