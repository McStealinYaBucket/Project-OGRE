/* Copyright 2003-2006 ROBLOX Corporation, All Rights Reserved */

#pragma once
#include "Util/Limits.h"

#include "rbx/TaskScheduler.Job.h"
#include "rbx/RunningAverage.h"
#include "rbx/atomic.h"

LOGGROUP(DataModelJobs)

namespace RBX {

	class DataModelArbiter;
	class ScriptContext;

	// 2013 layout (ctor PLAYER 0x0070ca20 stores taskType at 0x0070ca96, isPerPlayer at
	// 0x0070ca9c): TaskScheduler::Job (0x210) at 0, taskType @0x210, isPerPlayer @0x214.
	// 2016's stepsAccumulated, profilingToken and updateStepsRequiredForCyclicExecutive
	// did not exist yet.
	// SIZE 0x218
	class DataModelJob : public TaskScheduler::Job
	{
	public:
		typedef enum { 
			Read = 0,		// general read-only access to the DataModel 
			Write,			// general read-writes access to the DataModel 
			Render, 
			Physics, 
			DataOut, 
			PhysicsOut, 
			PhysicsOutSort,
			DataIn, 
			PhysicsIn,
			RaknetPeer,
			None,
			TaskTypeMax
		} 
		TaskType;
		static const int TaskTypeCount = TaskTypeMax;
		TaskType taskType;	// 0x210

		virtual TaskScheduler::StepResult step(const Stats& stats);

	protected:
		virtual TaskScheduler::StepResult stepDataModelJob(const Stats& stats) = 0;
		const bool isPerPlayer;	// 0x214
		DataModelJob(const char* name, TaskType taskType, bool isPerPlayer, shared_ptr<DataModelArbiter> arbiter, Time::Interval stepBudget);
		/*implement*/ double getPriorityFactor();	
	};

	// mac: 0x1A9CE8 C2 / 0x1A9E0C D1 / 0x1A9EE8 sleepTime / 0x1A9F16 error /
	// mac: 0x1A9F56 stepDataModelJob (batches 573-575). Scheduler job that
	// drains ScriptContext::resumeWaitingScripts under a write lock.
	// Vtable (Itanium [0]=offset [1]=typeinfo, then D1/D0/sleepTime/
	// error/DataModelJob::getPriorityFactor/DataModelJob::step/
	// Job::getDesiredConcurrencyCount/stepDataModelJob) matches the
	// binary exactly -- no new virtuals beyond stepDataModelJob.
	class WaitingScriptsJob : public DataModelJob
	{
	public:
		WaitingScriptsJob(shared_ptr<ScriptContext> scriptContext);
	private:
		virtual Time::Interval sleepTime(const Stats& stats);
		virtual Error error(const Stats& stats);
	protected:
		virtual TaskScheduler::StepResult stepDataModelJob(const Stats& stats);
	private:
		// Dtor pinned in the lib TU (mixed-CRT rule).
		const weak_ptr<ScriptContext> scriptContext;	// 0x218
	};

	// mac: 0x1AA0EA C2 / 0x1AA226 D1 / 0x1AA302 sleepTime / 0x1AA32E error /
	// mac: 0x1AA362 stepDataModelJob (batches 573-575). Scheduler job that
	// steps Lua GC under a write lock, then refreshes its Hz from the
	// context's frequency base. Vtable shape identical to
	// WaitingScriptsJob above (binary 0xF04A20).
	// 2013 layout (ctor PLAYER 0x00679c00, stores at 0x00679ce1..0x00679d1c): weak_ptr
	// @0x218, frequency @0x220 (from fild [ScriptContext+0xf8]). No operator new for it was
	// found; 0x224 of data, rounded to 0x228 by TaskScheduler::Job's 8-byte alignment.
	// SIZE 0x228
	class GcJob : public DataModelJob
	{
	public:
		GcJob(shared_ptr<ScriptContext> scriptContext);
	private:
		virtual Time::Interval sleepTime(const Stats& stats);
		virtual Error error(const Stats& stats);
	protected:
		virtual TaskScheduler::StepResult stepDataModelJob(const Stats& stats);
	private:
		// Dtor pinned in lib.
		const weak_ptr<ScriptContext> scriptContext;	// 0x218
		int frequency;									// 0x220
	};

	// 2013 layout (ctor PLAYER 0x0070cf60; DataModel's next base starts at 0xa8 + 0xa20):
	// SimpleThrottlingArbiter (0x830: TaskScheduler::Arbiter's vfptr + ActivityMeter<2>
	// @0x8, throttled @0x828, updatingThrottle @0x82c), shared_ptr @0x830, lookup tables
	// @0x838..0xa1c, padded to 0xa20. 2016's own stepActivity/stepCounter are not here:
	// preStep 0x0070cbd0 / postStep 0x0070cc10 update the base Arbiter's ActivityMeter at
	// this+8 (counter at this+0x80c), then call the OnScreenProfiler hook.
	// SIZE 0xa20
	class DataModelArbiter : public SimpleThrottlingArbiter
	{
	public:
		typedef enum
		{
			Serial = 0,
			Safe,
			Logical,
			Empirical
		}
		ConcurrencyModel;
		static const int ConcurrencyModelCount = 4;
		static ConcurrencyModel concurrencyModel;
		DataModelArbiter();
		virtual ~DataModelArbiter();
		virtual bool areExclusive(TaskScheduler::Job* job1, TaskScheduler::Job* job2);
		virtual int getNumPlayers() const = 0;
		bool areExclusive(DataModelJob::TaskType task1, DataModelJob::TaskType task2);

		virtual void preStep(TaskScheduler::Job* job);
		virtual void postStep(TaskScheduler::Job* job);

	private:
		// 2013 0x830 (ctor 0x0070cf60: `new 4`, zeroed, wrapped by shared_ptr ctor 0x0070cc50).
		// A 4-byte zeroed object: most likely the Limits::Counter that DataModelJob::step's
		// Counter::Activator uses (mac DataModelJob::step reads arbiter+2088/2092), not proven.
		shared_ptr<Limits::Counter> m_unk0x830;

		bool lookup[ConcurrencyModelCount][DataModelJob::TaskTypeCount][DataModelJob::TaskTypeCount];	// 0x838
	};


} // namespace
