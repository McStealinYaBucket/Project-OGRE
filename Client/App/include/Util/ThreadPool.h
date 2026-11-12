#pragma once

// 2013 differs from 2016 (PLAYER): no maxScheduleSize / shouldSchedule. PoolData's vftable
// 0x00b744d8 has 2 slots (dtor, getNextTask); BaseThreadPool::poolData is at +0x2c and
// PriorityThreadPool::schedule 0x0082cd60 pushes and calls taskAdded with no size check and
// no return value. Mac Http::init builds ThreadPool(8, 3): 3 is the ShutdownPolicy NoAction.

#include "rbx/threadsafe.h"

namespace RBX
{
	
	// TODO: Implement shutdown policies: WaitForPendingTasks, InterruptTasks, etc.
	class BaseThreadPool
	{
	public:
		enum ShutdownPolicy
		{
			WaitForRunningTasks,
			WaitForRunningTasksWithTimeout,
			LockAndKill,
			NoAction
		};

		struct PoolData
		{			
			volatile bool done;
			volatile bool fired;	// a task has been scheduled
			boost::condition_variable cond;
			boost::mutex mut;
			PoolData()
				:done(false) 
				,fired(false)
			{}
			virtual ~PoolData()
			{}
			// 2013 vftable 0x00b744d8 has 2 slots: dtor, getNextTask. shouldSchedule is 2016.
			virtual bool getNextTask(boost::function<void(boost::shared_ptr<rbx::spin_mutex>)>& task) = 0;
		};
	private:
		int count;
		std::vector< boost::shared_ptr<rbx::spin_mutex> > poolLocks;
		std::vector< boost::shared_ptr<boost::thread> > pool;
		ShutdownPolicy shutdownPolicy;

		static void loop(boost::shared_ptr<PoolData> poolData, boost::shared_ptr<rbx::spin_mutex> lock, ShutdownPolicy shutdownPolicy);

	protected:
		boost::shared_ptr<PoolData> poolData;  // +0x2c (PriorityThreadPool::schedule 0x0082cd60)

		void taskAdded();
		
	public:
		BaseThreadPool(int count, ShutdownPolicy shutdownPolicy, PoolData* poolData);
		int getThreadCount() const;
		virtual ~BaseThreadPool();
	};

	//A ThreadPool with fifo ordering
	class ThreadPool:
		public BaseThreadPool
	{
	private:
		struct ThreadPoolData
			: public PoolData
		{
			typedef rbx::safe_queue<boost::function<void(boost::shared_ptr<rbx::spin_mutex>)> > Queue;
			Queue queue;

			/*override*/ bool getNextTask(boost::function<void(boost::shared_ptr<rbx::spin_mutex>)>& task)
			{
				return queue.pop_if_present(task);
			}

		};
	protected:
		
	public:
		ThreadPool(int count, ShutdownPolicy shutdownPolicy = NoAction);

		void schedule(boost::function<void(boost::shared_ptr<rbx::spin_mutex>)> task);
	};

	//A ThreadPool with job priorities
	class PriorityThreadPool:
		public BaseThreadPool
	{
	private:
		struct PriorityTask
		{
			boost::function<void(boost::shared_ptr<rbx::spin_mutex>)> func;
			float priority;
			PriorityTask(boost::function<void(boost::shared_ptr<rbx::spin_mutex>)> func, float priority)
				: func(func)
				, priority(priority)
			{}
			PriorityTask()
			{}

			bool operator<(const PriorityTask& rhs) const
			{
				return this->priority > rhs.priority;
			}
		};
		struct PriorityThreadPoolData
			: public PoolData
		{
			typedef rbx::safe_heap<PriorityTask> Heap;
			Heap heap;

			/*override*/ bool getNextTask(boost::function<void(boost::shared_ptr<rbx::spin_mutex>)>& task);

		};
	protected:
		
	public:
		PriorityThreadPool(int count, ShutdownPolicy shutdownPolicy = NoAction);

		void schedule(boost::function<void(boost::shared_ptr<rbx::spin_mutex>)> task, float priority);
	};
}
