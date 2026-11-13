/* App/v8datamodel/DebugSettings.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/DebugSettings.h"

namespace RBX {
	const char* const sDebugSettings = "DebugSettings";
	// 2016-carry ("?" default, written by the build stamp at startup).
	std::string DebugSettings::robloxVersion = "?";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::DebugSettings);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x0065c640
// FUNCTION: STUDIO 0x00775d40
template<>
EnumDesc<TaskScheduler::ThreadPoolConfig>::EnumDesc()
	: EnumDescriptor("ThreadPoolConfig", typeid(TaskScheduler::ThreadPoolConfig))
{
	addPair(TaskScheduler::Auto, "Auto");
	addPair(TaskScheduler::PerCore1, "PerCore1");
	addPair(TaskScheduler::PerCore2, "PerCore2");
	addPair(TaskScheduler::PerCore3, "PerCore3");
	addPair(TaskScheduler::PerCore4, "PerCore4");
	addPair(TaskScheduler::Threads1, "Threads1");
	addPair(TaskScheduler::Threads2, "Threads2");
	addPair(TaskScheduler::Threads3, "Threads3");
	addPair(TaskScheduler::Threads4, "Threads4");
	addPair(TaskScheduler::Threads8, "Threads8");
	addPair(TaskScheduler::Threads16, "Threads16");
	addLegacy(201, "PartialThread", TaskScheduler::Auto);
}

template<>
const Type& Type::getSingleton<TaskScheduler::ThreadPoolConfig>()
{
	return EnumDesc<TaskScheduler::ThreadPoolConfig>::singleton();
}

template<> EnumRegistrar<TaskScheduler::ThreadPoolConfig> EnumRegistrar<TaskScheduler::ThreadPoolConfig>::registrar(0);
template<> TypeRegistrar<TaskScheduler::ThreadPoolConfig> TypeRegistrar<TaskScheduler::ThreadPoolConfig>::registrar(0);

// FUNCTION: PLAYER 0x0065c7e0
// FUNCTION: STUDIO 0x00775ee0
template<>
EnumDesc<TaskScheduler::PriorityMethod>::EnumDesc()
	: EnumDescriptor("PriorityMethod", typeid(TaskScheduler::PriorityMethod))
{
	addPair(TaskScheduler::LastError, "LastError");
	addPair(TaskScheduler::AccumulatedError, "AccumulatedError");
	addPair(TaskScheduler::FIFO, "FIFO");
}

template<>
const Type& Type::getSingleton<TaskScheduler::PriorityMethod>()
{
	return EnumDesc<TaskScheduler::PriorityMethod>::singleton();
}

template<> EnumRegistrar<TaskScheduler::PriorityMethod> EnumRegistrar<TaskScheduler::PriorityMethod>::registrar(0);
template<> TypeRegistrar<TaskScheduler::PriorityMethod> TypeRegistrar<TaskScheduler::PriorityMethod>::registrar(0);

// FUNCTION: PLAYER 0x0065c900
// FUNCTION: STUDIO 0x00776000
template<>
EnumDesc<TaskScheduler::Job::SleepAdjustMethod>::EnumDesc()
	: EnumDescriptor("SleepAdjustMethod", typeid(TaskScheduler::Job::SleepAdjustMethod))
{
	addPair(TaskScheduler::Job::None, "None");
	addPair(TaskScheduler::Job::LastSample, "LastSample");
	addPair(TaskScheduler::Job::AverageInterval, "AverageInterval");
}

template<>
const Type& Type::getSingleton<TaskScheduler::Job::SleepAdjustMethod>()
{
	return EnumDesc<TaskScheduler::Job::SleepAdjustMethod>::singleton();
}

template<> EnumRegistrar<TaskScheduler::Job::SleepAdjustMethod> EnumRegistrar<TaskScheduler::Job::SleepAdjustMethod>::registrar(0);
template<> TypeRegistrar<TaskScheduler::Job::SleepAdjustMethod> TypeRegistrar<TaskScheduler::Job::SleepAdjustMethod>::registrar(0);

// FUNCTION: PLAYER 0x0065ca20
// FUNCTION: STUDIO 0x00776120
template<>
EnumDesc<DebugSettings::ErrorReporting>::EnumDesc()
	: EnumDescriptor("ErrorReporting", typeid(DebugSettings::ErrorReporting))
{
	addPair(DebugSettings::DontReport, "DontReport");
	addPair(DebugSettings::Prompt, "Prompt");
	addPair(DebugSettings::Report, "Report");
}

template<>
const Type& Type::getSingleton<DebugSettings::ErrorReporting>()
{
	return EnumDesc<DebugSettings::ErrorReporting>::singleton();
}

template<> EnumRegistrar<DebugSettings::ErrorReporting> EnumRegistrar<DebugSettings::ErrorReporting>::registrar(0);
template<> TypeRegistrar<DebugSettings::ErrorReporting> TypeRegistrar<DebugSettings::ErrorReporting>::registrar(0);

// FUNCTION: PLAYER 0x0065cfd0
// FUNCTION: STUDIO 0x007763a0
template<>
EnumDesc<Time::SampleMethod>::EnumDesc()
	: EnumDescriptor("TickCountSampleMethod", typeid(Time::SampleMethod))
{
	addPair(Time::Fast, "Fast");
	addPair(Time::Benchmark, "Benchmark");
	addPair(Time::Precise, "Precise");
}

template<>
const Type& Type::getSingleton<Time::SampleMethod>()
{
	return EnumDesc<Time::SampleMethod>::singleton();
}

template<> EnumRegistrar<Time::SampleMethod> EnumRegistrar<Time::SampleMethod>::registrar(0);
template<> TypeRegistrar<Time::SampleMethod> TypeRegistrar<Time::SampleMethod>::registrar(0);

} // namespace Reflection
} // namespace RBX
