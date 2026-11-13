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

// Template and implicit member bodies the .cpp emits (identical to the 2013 bytes in a private
// compile; markers pair them so their callers match).

// TEMPLATE: PLAYER 0x008ef680
// TEMPLATE: STUDIO 0x00a0d480
// ??$_Construct@UContentProviderTask@ContentProviderJob@RBX@@U123@@std@@YAXPAUContentProviderTask@ContentProviderJob@RBX@@ABU123@@Z

// TEMPLATE: PLAYER 0x008ef730
// TEMPLATE: STUDIO 0x00a0d520
// ?_Tidy@?$deque@UContentProviderTask@ContentProviderJob@RBX@@V?$allocator@UContentProviderTask@ContentProviderJob@RBX@@@std@@@std@@IAEXXZ

// TEMPLATE: PLAYER 0x008ef7b0
// TEMPLATE: STUDIO 0x00a0d5a0
// ?_Growmap@?$deque@UContentProviderTask@ContentProviderJob@RBX@@V?$allocator@UContentProviderTask@ContentProviderJob@RBX@@@std@@@std@@IAEXI@Z

// TEMPLATE: PLAYER 0x008f0020
// TEMPLATE: STUDIO 0x00a0de10
// ??1ContentProviderJob@RBX@@UAE@XZ
