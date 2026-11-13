/* ContentProviderJob.h -- October 2013.
   Layout from the ctor (PLAYER 0x008efdc0): DataModelJob (0x218), processFunc @0x218,
   errorFunc @0x238 (boost::function, 0x20 each), aborted @0x258, tasks @0x25c (safe_queue:
   deque 0x20 + spin_mutex), execMode @0x280. No 2016 cyclic-executive stepping in 2013
   (stepDataModelJob 0x008efca0 pops one task per step; no DFInt/FLog strings in the binary).
   No __FILE__ string names this file; the path follows the 2016 tree. */
#pragma once

#include "Util/AsyncHttpQueue.h"
#include "V8DataModel/DataModelJob.h"
#include "rbx/threadsafe.h"

namespace RBX
{
	class DataModel;

	// VTABLE: PLAYER 0x00b98414
	// VTABLE: STUDIO 0x00c6b31c
	// SIZE 0x288
	class ContentProviderJob : public DataModelJob
	{
	public:
		enum ExecutionMode
		{
			JobMode,
			ImmediateMode
		};

	private:
		boost::function<TaskScheduler::StepResult(std::string, shared_ptr<const std::string>)> processFunc;	// 0x218
		boost::function<void(std::string)> errorFunc;	// 0x238

		struct ContentProviderTask
		{
			std::string id;							// 0x00
			shared_ptr<const std::string> data;		// 0x1c
		};
		bool aborted;								// 0x258
		rbx::safe_queue<ContentProviderTask> tasks;	// 0x25c
		ExecutionMode execMode;						// 0x280

		TaskScheduler::StepResult processTask(const ContentProviderTask& task);

	public:
		ContentProviderJob(shared_ptr<DataModel> dataModel, const char* name,
						   boost::function<TaskScheduler::StepResult(std::string, shared_ptr<const std::string>)> processFunc,
						   boost::function<void(std::string)> errorFunc);

		/*override*/ Time::Interval sleepTime(const Stats& stats);
		/*override*/ Job::Error error(const Stats& stats);
		/*override*/ TaskScheduler::StepResult stepDataModelJob(const Stats& stats);

		void abort()
		{
			aborted = true;
		}
		void addTask(const std::string& id, AsyncHttpQueue::RequestResult result, std::istream* filestream, shared_ptr<const std::string> data);
		void setExecutionMode(ExecutionMode execMode)
		{
			this->execMode = execMode;
		}
	};
}
