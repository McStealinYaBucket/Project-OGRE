/* Base/rbx/TaskScheduler.Job.cpp, October 2013 (no cyclic executive: no tryJobAgain/cyclic mangles or strings in 2013; ctor/dtor FASTLOG strings verified in the 0.126 Player). */
#include "rbx/TaskScheduler.Job.h"
#include "rbx/Tasks/Coordinator.h"
#include "rbx/Log.h"
#include "FastLog.h"
using namespace RBX;
LOGGROUP(TaskSchedulerSteps)
#if HANG_DETECTION
#include <../../App/include/util/standardout.h>
#define STEPTIME_SAMPLE_INTEVAL 60.0
double RBX::TaskScheduler::Job::stepTimeThreshold = 0.0;
#endif
TaskScheduler::Job::SleepAdjustMethod TaskScheduler::Job::sleepAdjustMethod(AverageInterval);
double RBX::TaskScheduler::Job::throttledSleepTime = 0.01;
// FUNCTION: PLAYER 0x009b9a80
// FUNCTION: STUDIO 0x00a70aa0
double TaskScheduler::Job::averageDutyCycle() const
{
	return dutyCycle.dutyCycle();
}
double TaskScheduler::Job::averageSleepRate() const
{
	return sleepRate.value();
}
// FUNCTION: PLAYER 0x009b9a50
// FUNCTION: STUDIO 0x00a70a70
double TaskScheduler::Job::averageStepsPerSecond() const
{
	return dutyCycle.stepInterval().rate();
}
// FUNCTION: PLAYER 0x009b9990
// FUNCTION: STUDIO 0x00a70960
double TaskScheduler::Job::averageStepTime() const
{
	return dutyCycle.stepTime().value();
}
// FUNCTION: STUDIO 0x00a70970
Time::Interval TaskScheduler::Job::getSleepingTime() const
{
	if (state!=Sleeping)
		return Time::Interval::zero();
	else
		return Time::now<Time::Fast>() - timeofLastSleep;
}
// FUNCTION: PLAYER 0x009b99a0
// FUNCTION: STUDIO 0x00a709c0
double TaskScheduler::Job::averageError() const
{
	return runningAverageError.value();
}
// STUB: STUDIO 0x00a71210
void TaskScheduler::Job::removeCoordinator(class boost::shared_ptr<class RBX::Tasks::Coordinator> coordinator)
{
	coordinator->onRemoved(this);
	{
		RBX::mutex::scoped_lock lock(coordinatorMutex);
		coordinators.erase(std::find(coordinators.begin(), coordinators.end(), coordinator));
	}
}
// STUB: PLAYER 0x009baa80
// STUB: STUDIO 0x00a718d0
void TaskScheduler::Job::addCoordinator(class boost::shared_ptr<class RBX::Tasks::Coordinator> coordinator)
{
	{
		RBX::mutex::scoped_lock lock(coordinatorMutex);
		coordinators.push_back(coordinator);
	}
	coordinator->onAdded(this);
}
// FUNCTION: PLAYER 0x009ba160
// FUNCTION: STUDIO 0x00a71180
bool TaskScheduler::Job::isDisabled()
{
	if (coordinators.size()==0)
		return false;
	RBX::mutex::scoped_lock lock(coordinatorMutex);
	// TODO: Write more efficiently without boost bind
	return std::find_if(
		coordinators.begin(), coordinators.end(),
		boost::bind(&Tasks::Coordinator::isInhibited, _1, this)) != coordinators.end();
}
const double lerpJob = 0.05;
extern RBX::Time::Interval maxDutyCycleWindow;
// FUNCTION: PLAYER 0x009babf0
// FUNCTION: STUDIO 0x00a71a40
TaskScheduler::Job::Job(const char* name, shared_ptr<TaskScheduler::Arbiter> arbiter, Time::Interval stepBudget)
	:name(name)
	,sharedArbiter(arbiter)
	,weakArbiter(boost::weak_ptr<Arbiter>())
	,baldArbiter(NULL)
	,state(Unknown)
	,sleepRate(lerpJob)
	,dutyCycle(lerpJob, 8)
	,runningAverageError(lerpJob)
	,timespanOfLastStep(0.0)
	,isRemoveRequested(false)
	,dutyCycleWindow(maxDutyCycleWindow)
	,stepBudget(stepBudget)
	,overStepTimeThresholdCount(0)
	,allotedConcurrency(-1)
	// NOTE (2013): no ,cyclicExecutive(false) ,cyclicPriority(Default) (2016-only)
{
	FASTLOG2(FLog::TaskSchedulerInit, "Job Created - this(%p) arbiter(%p)", this, arbiter.get());
	FASTLOGS(FLog::TaskSchedulerInit, "JobName(%s)", name);
}
// STUB: PLAYER 0x009ba1f0
// STUB: STUDIO 0x00a71310
TaskScheduler::Job::~Job()
{
	RBXASSERT(!TaskScheduler::SleepingHook::is_linked());
	RBXASSERT(!TaskScheduler::WaitingHook::is_linked());
	while (coordinators.size()>0)
	{
		coordinators.back()->onRemoved(this);
		coordinators.pop_back();
	}
	FASTLOG1(FLog::TaskSchedulerInit, "Job Destroyed - this(%p)", this);
	FASTLOGS(FLog::TaskSchedulerInit, "JobName(%s)", name);
}
// FUNCTION: PLAYER 0x009b98b0
// FUNCTION: STUDIO 0x00a70870
TaskScheduler::Job::Error TaskScheduler::Job::computeStandardError(const Stats& stats, double desiredHz)
{
	return stats.timespanSinceLastStep.seconds() * desiredHz;
}
// STUB: PLAYER 0x009b9a90
// FUNCTION: STUDIO 0x00a70ab0
Time::Interval TaskScheduler::Job::computeStandardSleepTime(const Stats& stats, double desiredHz)
{
	shared_ptr<Arbiter> ar(getArbiter());
	Time::Interval minTime = ar && ar->isThrottled() ? Time::Interval(throttledSleepTime) : Time::Interval::zero();
	const double interval = 1.0 / desiredHz;
	switch (sleepAdjustMethod)
	{
	case AverageInterval:
		if (dutyCycle.stepInterval().value().seconds() > 1.05 * interval)
			return minTime;
		break;
	case LastSample:
		if (dutyCycle.lastStepInterval().seconds() > 1.05 * interval)
			return minTime;
		break;
	default:
		break;
	}
	Time::Interval result = Time::Interval(interval) - stats.timespanSinceLastStep;
	if (result < minTime)
		return minTime;
	return result;
}
// FUNCTION: PLAYER 0x009b98d0
// FUNCTION: STUDIO 0x00a70890
TaskScheduler::Job::Stats::Stats(Job& job, Time now)
{
	// (2013 has no timeNow member; this line keeps the later asserts on their lines)
	timespanSinceLastStep = now - job.timeofLastStep;
	timespanOfLastStep = job.timespanOfLastStep;
}
// FUNCTION: PLAYER 0x009b9930
// FUNCTION: STUDIO 0x00a708f0
void TaskScheduler::Job::startWaiting()
{
	state = Waiting;
}
// FUNCTION: PLAYER 0x009b9940
// FUNCTION: STUDIO 0x00a70900
void TaskScheduler::Job::startSleeping()
{
	state = Sleeping;
	timeofLastSleep = Time::now<Time::Fast>();
}
// FUNCTION: PLAYER 0x009b99b0
// FUNCTION: STUDIO 0x00a709d0
void TaskScheduler::Job::updateWakeTime()
{
	Time now = Time::now<Time::Fast>();
	Job::Stats stats(*this, now);
	wakeTime = now + sleepTime(stats);
}
// FUNCTION: PLAYER 0x009b9cd0
// FUNCTION: STUDIO 0x00a70cf0
void TaskScheduler::Job::updateError(const Time& now)
{
	Job::Stats stats(*this, now);
	currentError = error(stats);
	RBXASSERT(currentError.error < std::numeric_limits<double>::max());
	if (currentError.error>0)
		runningAverageError.sample(currentError.error);
}


// FUNCTION: PLAYER 0x009ba060
// FUNCTION: STUDIO 0x00a71080
void TaskScheduler::Job::notifyCoordinatorsPreStep()
{
	if (coordinators.size()>0)
	{
		RBX::mutex::scoped_lock lock(coordinatorMutex);
		std::for_each(
			coordinators.begin(),
			coordinators.end(),
			boost::bind(&Tasks::Coordinator::onPreStep, _1, this)
			);
	}
}

// STUB: PLAYER 0x009b9de0
// STUB: STUDIO 0x00a70e00
void TaskScheduler::Job::preStep()
{
	state = Running;

	FASTLOG4(FLog::TaskSchedulerRun, "JobStart. this: %p arbiter: %p time: %u error: %d", this, getArbiter().get(), (unsigned)stepStartTime.timestampSeconds(),
		currentError.urgent ? -1 : (int)currentError.error);
	FASTLOGS(FLog::TaskSchedulerRun, "JobStart %s", name.c_str());


	shared_ptr<Arbiter> ar(getArbiter());
	if (ar)
		ar->preStep(this);
}

// FUNCTION: PLAYER 0x009bae40
// FUNCTION: STUDIO 0x00a71c90
void TaskScheduler::Job::postStep(StepResult result)
{
	state = Unknown;
	timeofLastStep = stepStartTime;
	const Time now = Time::now<Time::Fast>();
	timespanOfLastStep = now - stepStartTime;

	// TODO: extract stepTime from dutyCycle?
	dutyCycle.sample(timespanOfLastStep);

	if (!stepBudget.isZero() && timespanOfLastStep > stepBudget) {
		//We were over budget
	}

#if HANG_DETECTION
	if ((stepTimeThreshold > 0) && (timespanOfLastStep.seconds() > stepTimeThreshold))
	{
		overStepTimeThresholdCount++;
		StandardOut::singleton()->printf(MESSAGE_WARNING, "TaskScheduler::Job: %s step time over threshold", name.c_str());

		// send to log
		if (RBX::Log::current() && ((Time::now<Time::Fast>() - stepTimeSampleTime).seconds() > STEPTIME_SAMPLE_INTEVAL))
		{
			std::string msg = "TaskScheduler::Job: " + name + " step time over threshold.";
			RBX::Log::current()->writeEntry(RBX::Log::Error, msg.c_str());

			stepTimeSampleTime = Time::now<Time::Fast>();
		}
	}
#endif

	if (dutyCycleWindow.getMaxWindow().seconds() > 0)
	{
		dutyCycleWindow.sample(timespanOfLastStep);
	}

	FASTLOG3(FLog::TaskSchedulerRun, "JobStop. This: %p arbiter: %p time: %u", this, getArbiter().get(), (unsigned)now.timestampSeconds());
	FASTLOGS(FLog::TaskSchedulerRun, "JobStop %s", name.c_str());

	shared_ptr<Arbiter> ar(getArbiter());
	if (ar)
		ar->postStep(this);
}


// FUNCTION: PLAYER 0x009ba0e0
// FUNCTION: STUDIO 0x00a71100
void TaskScheduler::Job::notifyCoordinatorsPostStep()
{
	if (coordinators.size()>0)
	{
		RBX::mutex::scoped_lock lock(coordinatorMutex);
		std::for_each(
			coordinators.begin(),
			coordinators.end(),
			boost::bind(&Tasks::Coordinator::onPostStep, _1, this)
			);
	}
}

// FUNCTION: PLAYER 0x009b9c10
// FUNCTION: STUDIO 0x00a70c30
void TaskScheduler::Job::updatePriority()
{
	switch (TaskScheduler::priorityMethod)
	{
	case FIFO:
		priority = 1.0;
		break;

	case LastError:
		priority = currentError.error;
		break;

	case AccumulatedError:
		double avg = averageError();
		if (avg>0)
			priority = avg;
		else
			priority = currentError.error;

		priority *= getPriorityFactor();

		double st = std::max(0.01, averageDutyCycle());
		priority /= st;
		break;
	}
}
