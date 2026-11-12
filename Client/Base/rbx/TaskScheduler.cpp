/* Base/rbx/TaskScheduler.cpp — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation
     (F:\2013Decompile\exports\decomp\Base_TaskScheduler.cpp.txt) + PE string/mangle scans.
      - static_init 0x14A9CC / singleton 0x14AA6A: function-static + setThreadCount(Auto).
      - ctor 0x14AA9C: threadAffinityPreference 1.5 (+0 double), taskCount 0 (+8), mutex (+12),
        3 intrusive-list inits, 3× RunningAverage<int> (0.05 lerp), RunningAverageDutyCycle,
        CEvent(+404, manualReset), counter thread ("Roblox sampleRunningJobCount" string
        verified in PE). NO FFlag::TaskSchedulerCyclicExecutive assignment (flag string absent).
      - ExclusiveArbiter::areExclusive 0x14A8E8: hasArbiter asserts + return true.
      - add 0x14B856: lock + allJobs.find==end assert + insert + scheduleJob. NO cyclic branch.
      - remove 0x14B30A / scheduleJob 0x14B74E / enqueueWaitingJob 0x14B944 /
        wakeSleepingJobs 0x14BA68 / findJobToRun 0x14BB16: no cyclic branches.
      - findJobToRun: nextScheduledJob fast-path (+104/+108), then the waitingJobs
        priority loop ("Starting to iterate through waitingJobs" + "Removing
        nextScheduledJob" strings verified). DIVERGENCE from 2016: NO error-calc
        batching (updateError/updatePriority called for every job every pass; the
        DFInt::TaskSchedularBatchErrorCalcFPS fastint is absent) and NO
        cyclicExecutiveEnabled guards anywhere.
      - sampleRunningJobCount 0x14AF78: 3 average samples on CEvent::Wait(71).
      - dtor 0x14B006: "Destroying TaskScheduler" FastLog + Set + join + endAllThreads.
      - remove() FASTLOGs ("Removing job %p...") + allJobs asserts verified in PE.
   2013 has NO cyclic executive at all: zero cyclic/Cyclic strings in 40k PE strings,
   no mangles for tryJobAgain/computeStandardErrorCyclicExecutiveSleeping/
   findJobToRunNonCyclicJobs/numNonCyclicJobsWithWork/isCyclicExecutive/
   cancelCyclicExecutive/releaseCyclicExecutive/checkStillWaitingNextFrame, no
   TaskSchedulerCyclicExecutive / TaskSchedularBatchErrorCalcFPS flags.
   2016 source (Base/rbx/TaskScheduler.cpp) used as style template with all cyclic
   paths excised; binary wins. */

#include "rbx/TaskScheduler.h"
#include "rbx/TaskScheduler.Job.h"
#include "rbx/Debug.h"
#include "FastLog.h"
#include "FastLog.h"
#include "rbx/boost.hpp"
#include "boost/scoped_ptr.hpp"
#include "rbx/ProcessPerfCounter.h"
#include "rbx/Profiler.h"

using namespace RBX;
using boost::shared_ptr;

LOGGROUP(TaskSchedulerInit)
LOGGROUP(TaskSchedulerRun)
LOGGROUP(TaskSchedulerFindJob)

RBX::TaskScheduler::PriorityMethod TaskScheduler::priorityMethod = AccumulatedError;
// TODO(decompile): verify 2013 default (no string/mangle evidence; 2016 value carried).

#ifdef RBX_TEST_BUILD
	int TaskScheduler::findJobFPS = 100;
	bool TaskScheduler::updateJobPriorityOnWake = false;
#endif

double TaskScheduler::getSchedulerDutyCyclePerThread() const
{
	return threads.size() ? schedulerDutyCycle.dutyCycle() / threads.size() : 0;
}

rbx::thread_specific_reference<TaskScheduler::Job> TaskScheduler::currentJob;

RBX::ExclusiveArbiter RBX::ExclusiveArbiter::singleton;

// STUB: PLAYER 0x009b37c0
// STUB: STUDIO 0x00a6b920
bool RBX::ExclusiveArbiter::areExclusive(TaskScheduler::Job* task1, TaskScheduler::Job* task2)
{
	RBXASSERT(task1->hasArbiter(this));
	RBXASSERT(task2->hasArbiter(this));
	return true;
}

static TaskScheduler* sing;
// FUNCTION: PLAYER 0x009b58a0
// FUNCTION: STUDIO 0x00a6d690
void TaskScheduler::static_init()
{
	static TaskScheduler s;
	s.setThreadCount(TaskScheduler::Auto);	// Use auto by default
	sing = &s;
}
TaskScheduler& TaskScheduler::singleton()
{
	static boost::once_flag flag = BOOST_ONCE_INIT;
	boost::call_once(static_init, flag);
	return *sing;
}

const static double lerpTaskScheduler = 0.05;

// STUB: PLAYER 0x009b55e0
// STUB: STUDIO 0x00a6d3d0
TaskScheduler::TaskScheduler()
// NOTE (2013): member order/content mirrors the 0x14AA9C store sequence.
// No DataModel30fpsThrottle / lastCyclcTimestamp / cyclic members (see header).
// No FFlag::TaskSchedulerCyclicExecutive tail (2016-only; flag absent from 2013 PE).
:sampleRunningJobCountEvent(true)
,threadCount(0)
,threadAffinityPreference(1.5)	// TODO: Come up with a good number here
,runningJobCount(0)
,waitingJobCount(lerpTaskScheduler)
,sleepingJobCount(lerpTaskScheduler)
,averageRunningJobCount(lerpTaskScheduler)
,schedulerDutyCycle(lerpTaskScheduler)
,averageThreadAffinity(lerpTaskScheduler, 1)
,nextWakeTime(Time::max())
,lastSortTime(Time())
,desiredThreadCount(0)
{
	runningJobCounterThread.reset(new boost::thread(RBX::thread_wrapper(boost::bind(&TaskScheduler::sampleRunningJobCount, this), "Roblox sampleRunningJobCount")));
}

// STUB: PLAYER 0x009b4e00
// STUB: STUDIO 0x00a6cbf0
TaskScheduler::~TaskScheduler()
{
	FASTLOG(FLog::TaskSchedulerInit, "Destroying TaskScheduler");
	sampleRunningJobCountEvent.Set();
	runningJobCounterThread->join();

	endAllThreads();
}

// STUB: PLAYER 0x009b52b0
// STUB: STUDIO 0x00a6d0a0
void TaskScheduler::remove(boost::shared_ptr<TaskScheduler::Job> task, bool joinTask, boost::function<void()> callbackPing)
{
	if (!task)
		return;

	// You can't join to yourself, but logically
	// there should be no concurrency risk to not join
	joinTask &= task.get() != currentJob.get();

	shared_ptr<CEvent> joinEvent(joinTask ? new CEvent(true) : NULL);

	remove(task, joinEvent);

	if (joinTask)
	{
		if (callbackPing)
		{
			for (int i = 0; i < 10 * 60 * 5; ++i)
			{
				callbackPing();
				if (joinEvent->Wait(100))
					return;
			}
#ifdef _DEBUG
			RBXASSERT(false);	// Why did it take so long to join?
#endif
			RBXCRASH();	// We want to learn about this. Blocking a long time means a thread in the pool is locked up!
		}
		else
		{
			if (!joinEvent->Wait(1000 * 60 * 5))
			{
#ifdef _DEBUG
				RBXASSERT(false);	// Why did it take so long to join?
#endif
				RBXCRASH();	// We want to learn about this. Blocking a long time means a thread in the pool is locked up!
			}
		}
	}
}



void TaskScheduler::reschedule(boost::shared_ptr<Job> job)
{
	{
		RBX::mutex::scoped_lock lock(mutex);
		if (job->SleepingHook::is_linked())
		{
			// NOTE (2013): no cyclic assert/branch.
			sleepingJobs.erase(sleepingJobs.iterator_to(*job));
			scheduleJob(*job);
		}
	}
}

// STUB: PLAYER 0x009b4560
// STUB: STUDIO 0x00a6c350
void TaskScheduler::add(boost::shared_ptr<Job> job)
{
	RBX::mutex::scoped_lock lock(mutex);
	RBXASSERT(allJobs.find(job)==allJobs.end());
	allJobs.insert(job);

	// NOTE (2013): no cyclicExecutiveEnabled branch — straight to scheduleJob.
	scheduleJob(*job);
}

template<class List, class Item, class IsLess>
void insert_from_back(List& list, Item& f, IsLess isLess)
{
	for (typename List::reverse_iterator iter = list.rbegin(); iter!=list.rend(); ++iter)
		if (!isLess(f, *iter))
		{
			list.insert(iter.base(), f);
			return;
		}
		list.push_front(f);
}

void TaskScheduler::scheduleJob(Job& job)
{
	// NOTE (2013): no cyclicExecutiveEnabled early-out.
	job.updateWakeTime();
	const Time now = Time::now<Time::Fast>();

	if (job.wakeTime <= now)
	{
		job.updateError(now);
		if (job.currentError.error > 0)
		{
			job.sleepRate.sample(0);
			job.startWaiting();

			job.updatePriority();
			enqueueWaitingJob(job);
			return;
		}
	}

	job.sleepRate.sample(1);
	job.startSleeping();
	insert_from_back(sleepingJobs, job, Job::isLowerWakeTime);
}


void TaskScheduler::remove(const boost::shared_ptr<TaskScheduler::Job>& job, boost::shared_ptr<CEvent> joinEvent)
{
	RBX::mutex::scoped_lock lock(mutex);
	RBXASSERT(allJobs.find(job)!=allJobs.end());
	FASTLOG1(FLog::TaskSchedulerRun,
		"Removing job %p from allJobs (::remove)", job.get());
	allJobs.erase(job);

	if (job->SleepingHook::is_linked())
	{
		// NOTE (2013): no cyclic assert.
		FASTLOG1(FLog::TaskSchedulerRun,
			"Removing job %p from sleepingJobs (::remove)", job.get());
		sleepingJobs.erase(sleepingJobs.iterator_to(*job));
		if (joinEvent)
			joinEvent->Set();
	}
	else if (job->WaitingHook::is_linked())
	{
		// NOTE (2013): no cyclic assert.
		FASTLOG1(FLog::TaskSchedulerRun,
			"Removing job %p from waitingJobs (::remove)", job.get());
		waitingJobs.erase(waitingJobs.iterator_to(*job));

		if (joinEvent)
			joinEvent->Set();
	}
	else
	{
		// NOTE (2013): no cyclic-executive running-job branch.
		if (joinEvent)
		{
			RBXASSERT(!job->joinEvent);	// Did somebody else already try to remove this task????
			// TODO: Support multiple joins by using a collection?

			// We must wait for the task to be completed before signaling the joinEvent
			job->joinEvent = joinEvent;
		}

		// We can't remove it yet. Wait for it to finish stepping
		job->isRemoveRequested = true;
	}
}

void TaskScheduler::incrementThreadCount()
{
	++threadCount;
}
void TaskScheduler::decrementThreadCount()
{
	--threadCount;
}

bool TaskScheduler::areExclusive(TaskScheduler::Job* task1, TaskScheduler::Job* task2, const shared_ptr<Arbiter>& arbiterHint)
{
	if (Job::haveDifferentArbiters(task1, task2))
		return false;	// different Arbiter domains can run concurrently

	if (!arbiterHint)
		return false;	// No Arbiter means the task can run concurrently

	return arbiterHint->areExclusive(task1, task2);
}

void TaskScheduler::sampleRunningJobCount()
{
	while (!sampleRunningJobCountEvent.Wait(71))
	{
		waitingJobCount.sample(int(waitingJobs.size()));
		sleepingJobCount.sample(int(sleepingJobs.size()));
		averageRunningJobCount.sample(int(runningJobCount));
	}
}


void TaskScheduler::enqueueWaitingJob(Job& job)
{
	FASTLOG1(FLog::TaskSchedulerFindJob,
		"Adding job %p to waitingJobs (::enqueueWaitingJob)", &job);

	// NOTE (2013): no cyclic assert.
	waitingJobs.push_back(job);
}


Time::Interval TaskScheduler::getShortestSleepTime() const
{
	return nextWakeTime - Time::now<Time::Fast>();
}


// STUB: PLAYER 0x009b41c0
// STUB: STUDIO 0x00a6bfb0
void TaskScheduler::wakeSleepingJobs()
{
	Time now = Time::now<Time::Fast>();

	SleepingJobs::iterator iter = sleepingJobs.begin();
	while (iter!=sleepingJobs.end())
	{
		Job& job(*iter);
		// NOTE (2013): no cyclic assert.

		if (job.wakeTime > now)
		{
			nextWakeTime = job.wakeTime;
			return;
		}
		else
		{
			iter = sleepingJobs.erase(iter);
			job.updateError(now);

			job.updatePriority();
			enqueueWaitingJob(job);
		}
	}
	nextWakeTime = Time::max();
}

// STUB: PLAYER 0x009b4670
// STUB: STUDIO 0x00a6c460
boost::shared_ptr<TaskScheduler::Job> TaskScheduler::findJobToRun(shared_ptr<Thread> requestingThread)
{
	// 2013 findJobToRun 0x14BB16 IS the 2016 findJobToRunNonCyclicJobs selection loop
	// (nextScheduledJob fast-path, then the waitingJobs priority scan), with two
	// divergences: (1) no error-calc batching — updateError/updatePriority run for
	// every job every pass (the DFInt::TaskSchedularBatchErrorCalcFPS fastint is absent);
	// (2) the nextScheduledJob cache block is unconditional (no cyclic guard).
	shared_ptr<TaskScheduler::Job> result;

	// nextScheduledJob fast-path (matches 2016's !cyclicExecutiveEnabled branch)
	if (nextScheduledJob)
	{
		result = nextScheduledJob;
		nextScheduledJob.reset();

		if (result->WaitingHook::is_linked())
			if (!result->isDisabled())
				if (!conflictsWithScheduledJob(result.get()))
				{
					FASTLOG1(FLog::TaskSchedulerFindJob,
						"Removing nextScheduledJob %p from waitingJobs (::findJobToRun)",
						result.get());
					waitingJobs.erase(waitingJobs.iterator_to(*result));
					averageThreadAffinity.sample(result->lastThreadUsed.lock() == requestingThread);
					return result;
				}
		result.reset();
	}

	wakeSleepingJobs();
	Time now = Time::now<Time::Fast>();

	WaitingJobs::iterator bestJob = waitingJobs.end();

	// Note: These definitions don't strictly need to be initialized, but it eliminates pesky compiler warnings
	bool bestHasAffinity = false;
	bool bestIsThrottled = false;

	FASTLOG1(FLog::TaskSchedulerFindJob, "Starting to iterate through waitingJobs. Size = %d", waitingJobs.size());

	for (WaitingJobs::iterator iter = waitingJobs.begin(); iter != waitingJobs.end(); ++iter)
	{
		Job& job(*iter);

		if (job.isDisabled())
			continue;

		if (priorityMethod != FIFO)
		{
			job.updateError(now);

			if (job.currentError.error<=0)
			{
				continue;
			}
		}

		if (conflictsWithScheduledJob(&job))
		{
			continue;
		}

		job.updatePriority();

		const bool hasAffinity = job.lastThreadUsed.lock() == requestingThread;
		bool match = false;

		shared_ptr<Arbiter> const arbiter(job.getArbiter());

		if (bestJob == waitingJobs.end())
		{
			match = true;
		}
		else if (job.currentError.urgent != bestJob->currentError.urgent)
		{
			if (job.currentError.urgent)
				match = true;
		}
		else if (bestIsThrottled && (!arbiter || !arbiter->isThrottled()))
		{
			match = true;
		}
		else if (hasAffinity == bestHasAffinity)
		{
			if (job.priority > bestJob->priority)
				match = true;
		}
		else if (hasAffinity)
		{
			if (job.priority * threadAffinityPreference > bestJob->priority)
				match = true;
		}
		else
		{
			if (job.priority > bestJob->priority * threadAffinityPreference)
				match = true;
		}

		if (match)
		{
			// Cache off old best job
			if (bestJob != waitingJobs.end())
			{
				if (!areExclusive(&job, &*bestJob, arbiter))
				{
					nextScheduledJob = bestJob->shared_from_this();
				}
				else if (nextScheduledJob && areExclusive(&job, &*nextScheduledJob, arbiter))
				{
					nextScheduledJob.reset();
				}
			}

			bestJob = iter;
			bestHasAffinity = hasAffinity;

			if (priorityMethod == FIFO)
				break;

			bestIsThrottled = arbiter && arbiter->isThrottled();
		}
	}

	FASTLOG(FLog::TaskSchedulerFindJob, "Finished iterating through waitingJobs");

	if (bestJob != waitingJobs.end())
	{
		result = bestJob->shared_from_this();
		RBXASSERT(bestJob->WaitingHook::is_linked());
		waitingJobs.erase(waitingJobs.iterator_to(*bestJob));
		averageThreadAffinity.sample(bestHasAffinity);
		FASTLOG3(FLog::TaskSchedulerFindJob, "RunJob, job: %p, arbiter %p error: %u", result.get(), result->getArbiter().get(), (unsigned)result->currentError.error);
	}

	return result;
}

rbx::atomic<int> RBX::SimpleThrottlingArbiter::arbiterCount;
bool RBX::SimpleThrottlingArbiter::isThrottlingEnabled = false;
