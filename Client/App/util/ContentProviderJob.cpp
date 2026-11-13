/* App/util/ContentProviderJob.cpp -- October 2013.
   One .obj in PLAYER 0x008ef650-0x008f0316, just before LegacyContentTable.obj. Functions in
   2013 address order; the deque/safe_queue/boost::function template bodies between them
   (0x008ef680, 0x008ef720-0x008efb10, 0x008efd50) are emitted by this TU. */
#include "stdafx.h"

#include "Util/ContentProviderJob.h"
#include "V8DataModel/DataModel.h"
#include "util/standardout.h"

#include <sstream>

namespace RBX
{
// FUNCTION: PLAYER 0x008ef650
// FUNCTION: STUDIO 0x00a0d3d0
Time::Interval ContentProviderJob::sleepTime(const Stats& stats)
{
	return computeStandardSleepTime(stats, 30);
}

// STUB: PLAYER 0x008ef6f0
// STUB: STUDIO 0x00a0d4f0
TaskScheduler::Job::Error ContentProviderJob::error(const Stats& stats)
{
	Job::Error result;
	result.error = tasks.size() * stats.timespanSinceLastStep.seconds();
	return result;
}

// FUNCTION: PLAYER 0x008efb70
// FUNCTION: STUDIO 0x00a0d960
TaskScheduler::StepResult ContentProviderJob::processTask(const ContentProviderTask& task)
{
	try
	{
		if (processFunc(task.id, task.data) == TaskScheduler::Done)
			return TaskScheduler::Done;
	}
	catch (RBX::base_exception& e)
	{
		errorFunc(task.id);
		RBX::StandardOut::singleton()->printf(RBX::MESSAGE_ERROR, "%s failed to process %s because %s", name.c_str(), task.id.c_str(), e.what());
	}
	return TaskScheduler::Stepped;
}

// FUNCTION: PLAYER 0x008efca0
// FUNCTION: STUDIO 0x00a0da90
TaskScheduler::StepResult ContentProviderJob::stepDataModelJob(const Stats& stats)
{
	ContentProviderTask task;
	if (!aborted && tasks.pop_if_present(task))
	{
		if (processTask(task) == TaskScheduler::Done)
			return TaskScheduler::Done;
	}
	return TaskScheduler::Stepped;
}

// STUB: PLAYER 0x008efdc0
// STUB: STUDIO 0x00a0dbb0
ContentProviderJob::ContentProviderJob(shared_ptr<DataModel> dataModel, const char* name,
									   boost::function<TaskScheduler::StepResult(std::string, shared_ptr<const std::string>)> processFunc,
									   boost::function<void(std::string)> errorFunc)
	:DataModelJob(name, DataModelJob::None, false, dataModel, Time::Interval(0.01))
	,processFunc(processFunc)
	,errorFunc(errorFunc)
	,aborted(false)
	,execMode(JobMode)
{
}

// FUNCTION: PLAYER 0x008f0110
// FUNCTION: STUDIO 0x00a0df00
void ContentProviderJob::addTask(const std::string& id,
								 AsyncHttpQueue::RequestResult result, std::istream* filestream, shared_ptr<const std::string> data)
{
	try
	{
		if (result == AsyncHttpQueue::Succeeded)
		{
			if (!data)
			{
				std::ostringstream strbuffer;
				strbuffer << filestream->rdbuf();
				data.reset(new std::string(strbuffer.str()));
			}
		}
		else
		{
			data.reset();
		}

		ContentProviderTask task;
		task.id = id;
		task.data = data;

		switch (execMode)
		{
		case JobMode:
			tasks.push(task);
			break;
		case ImmediateMode:
			processTask(task);
			break;
		}
	}
	catch (RBX::base_exception& e)
	{
		RBX::StandardOut::singleton()->printf(RBX::MESSAGE_ERROR, "%s failed to load %s because %s", name.c_str(), id.c_str(), e.what());
		errorFunc(id);
	}
}

}
