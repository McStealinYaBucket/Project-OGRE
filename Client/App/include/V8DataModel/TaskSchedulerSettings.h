/* V8DataModel/TaskSchedulerSettings.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED FactoryProduct<TaskSchedulerSettings, GlobalAdvancedSettings::Item> 0x369BC2 (Item template TODO; C++ TaskScheduler singleton separate);
     sTaskSchedulerSettings = "TaskSchedulerSettings".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/TaskSchedulerSettings.h */

#pragma once

#include "V8DataModel/GlobalAdvancedSettings.h"

namespace RBX {

	extern const char* const sTaskSchedulerSettings;

	class TaskSchedulerSettings
		: public DescribedCreatable<TaskSchedulerSettings, GlobalAdvancedSettings, sTaskSchedulerSettings>
	{
	public:
		TaskSchedulerSettings() {}
	};

} // namespace RBX
