/* DataModelJob.cpp — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation
   (F:\2013Decompile\exports\decomp\Svc_DataModelJob.cpp.txt)
     - ctor 0x3120D2: DataModelJob(name, TaskType, isPerPlayer, shared_ptr<DataModelArbiter>,
       Time::Interval) → TaskScheduler::Job(name, arbiter, interval); TaskType at +472,
       isPerPlayer at +476; asserts "arbiter" at App/v8datamodel/DataModelJob.cpp line 36
     - getPriorityFactor 0x3122C6: returns arbiter->getNumPlayers() (>1) when !isPerPlayer,
       else 1.0
   Note: stepDataModelJob/updateStepsRequiredForCyclicExecutive are provided by subclasses /
   are inline in the 2013 binary (no out-of-line symbols captured here yet). */

#include "stdafx.h"
#include "decomp.h"

#include "V8DataModel/DataModelJob.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/ScriptContext.h"
#include "V8DataModel/LuaSettings.h"

// FLog::DataModelJobs is defined harness-side (test_support.cpp); the
// LOGGROUP extern above is the lib-side declaration. Do NOT define it
// here (LNK2005).


LOGGROUP(DataModelJobs)

namespace RBX {

	// FUNCTION: PLAYER 0x0070ca20
	// FUNCTION: STUDIO 0x008450c0
	DataModelJob::DataModelJob(const char* name, TaskType taskType, bool isPerPlayer, shared_ptr<DataModelArbiter> arbiter, Time::Interval stepBudget)
		:TaskScheduler::Job(name, arbiter, stepBudget)
		,taskType(taskType)
		,isPerPlayer(isPerPlayer)
	{
		RBXASSERT(arbiter);		// "arbiter" (DataModelJob.cpp:36)
	}

	/* mac: 0x3122C6 — matches 2016 semantics: max(1, arbiter->getNumPlayers()) unless per-player */
	// STUB: PLAYER 0x0070ce10
	// STUB: STUDIO 0x008454e0
	double DataModelJob::getPriorityFactor()
	{
		const shared_ptr<TaskScheduler::Arbiter>& baseArbiter = getArbiter();
		DataModelArbiter* arbiter = dynamic_cast<DataModelArbiter*>(baseArbiter.get());
		if (!arbiter || isPerPlayer)
			return 1;

		int numPlayers = arbiter->getNumPlayers();
		return (numPlayers > 1) ? (double)numPlayers : 1.0;
	}


	static void copy(bool src[DataModelJob::TaskTypeCount][DataModelJob::TaskTypeCount], bool dst[DataModelJob::TaskTypeCount][DataModelJob::TaskTypeCount])
	{
		for (int j = 0; j < DataModelJob::TaskTypeCount; ++j)
			for (int k = 0; k < DataModelJob::TaskTypeCount; ++k)
				dst[j][k] = src[j][k];
	}

	static void setConcurrent(bool table[DataModelJob::TaskTypeCount][DataModelJob::TaskTypeCount], DataModelJob::TaskType f1, DataModelJob::TaskType f2)
	{
		table[f1][f2] = false;
		table[f2][f1] = false;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// DataModelArbiter
	// Lookup table COMPLETELY recovered from the binary ctor 0x3123B0:
	//   table layout: bool lookup[4][11][11] at offset +2096; Serial = memset all-true;
	//   Safe (2217), Logical (2338), Empirical (2459) each copied from the previous then
	//   specific pairs zeroed (setConcurrent). The zeroed offsets decode to the pairs below.
	//////////////////////////////////////////////////////////////////////////////////
	DataModelArbiter::ConcurrencyModel DataModelArbiter::concurrencyModel = DataModelArbiter::Logical;

	// STUB: PLAYER 0x0070cf60
	DataModelArbiter::DataModelArbiter()
		: m_unk0x830(new Limits::Counter())
	{
		static const int TableStart = 2096 / 1;		// (documentation only — member offset in the binary)
		(void)TableStart;

		// Serial: everything exclusive
		for (int j = 0; j < DataModelJob::TaskTypeCount; ++j)
			for (int k = 0; k < DataModelJob::TaskTypeCount; ++k)
				lookup[Serial][j][k] = true;

		// Safe = Serial minus:
		copy(lookup[Serial], lookup[Safe]);
		setConcurrent(lookup[Safe], DataModelJob::DataOut, DataModelJob::DataOut);
		setConcurrent(lookup[Safe], DataModelJob::PhysicsOut, DataModelJob::PhysicsOut);
		setConcurrent(lookup[Safe], DataModelJob::RaknetPeer, DataModelJob::Read);
		setConcurrent(lookup[Safe], DataModelJob::RaknetPeer, DataModelJob::Render);
		setConcurrent(lookup[Safe], DataModelJob::RaknetPeer, DataModelJob::DataOut);
		setConcurrent(lookup[Safe], DataModelJob::RaknetPeer, DataModelJob::PhysicsOut);
		setConcurrent(lookup[Safe], DataModelJob::RaknetPeer, DataModelJob::DataIn);
		setConcurrent(lookup[Safe], DataModelJob::RaknetPeer, DataModelJob::PhysicsIn);
		setConcurrent(lookup[Safe], DataModelJob::PhysicsOutSort, DataModelJob::Read);
		setConcurrent(lookup[Safe], DataModelJob::PhysicsOutSort, DataModelJob::Render);
		setConcurrent(lookup[Safe], DataModelJob::PhysicsOutSort, DataModelJob::DataOut);
		setConcurrent(lookup[Safe], DataModelJob::PhysicsOutSort, DataModelJob::DataIn);
		setConcurrent(lookup[Safe], DataModelJob::PhysicsOutSort, DataModelJob::PhysicsIn);
		/* 2013 divergence: None pairs stay EXCLUSIVE in Safe (2016 opens None/None etc.) */

		// Logical = Safe minus:
		copy(lookup[Safe], lookup[Logical]);
		setConcurrent(lookup[Logical], DataModelJob::Read, DataModelJob::Read);
		setConcurrent(lookup[Logical], DataModelJob::Read, DataModelJob::DataOut);
		setConcurrent(lookup[Logical], DataModelJob::DataOut, DataModelJob::PhysicsOut);

		// Empirical = Logical (binary copies the table verbatim at 0x3123B0)
		copy(lookup[Logical], lookup[Empirical]);
	}

	DataModelArbiter::~DataModelArbiter()
	{
	}

	bool DataModelArbiter::areExclusive(TaskScheduler::Job* job1, TaskScheduler::Job* job2)
	{
		DataModelJob* j1 = boost::polymorphic_downcast<DataModelJob*>(job1);
		DataModelJob* j2 = boost::polymorphic_downcast<DataModelJob*>(job2);
		return areExclusive(j1->taskType, j2->taskType);
	}

	bool DataModelArbiter::areExclusive(DataModelJob::TaskType function1, DataModelJob::TaskType function2)
	{
		return lookup[concurrencyModel][function1][function2];
	}

	/* mac: 0x312766/0x3127B6 — activity buckets + step counter; the
	   OnScreenProfiler UpdateJobStart/End hook is null-guarded in the binary
	   and the module isn't in the tree (TODO). */
	// STUB: PLAYER 0x0070cbd0
	// STUB: STUDIO 0x00845270
	void DataModelArbiter::preStep(TaskScheduler::Job* job)
	{
		// 2013: the base Arbiter's meter, then OnScreenProfiler::UpdateJobStart (not in tree).
		TaskScheduler::Arbiter::preStep(job);
	}

	// STUB: PLAYER 0x0070cc10
	// STUB: STUDIO 0x008452b0
	void DataModelArbiter::postStep(TaskScheduler::Job* job)
	{
		// 2013: the base Arbiter's meter, then OnScreenProfiler::UpdateJobEnd (not in tree).
		TaskScheduler::Arbiter::postStep(job);
	}

	/* mac: 0x3121E0 — lock the weak arbiter; run stepDataModelJob under
	   Limits::Counter::Activator; Done when unlocked. The Activator counter
	   (arbiter +2088/2092) is TODO-identified, so the guard is omitted
	   pending that member (limits-tracking only, no functional effect). */
	// STUB: PLAYER 0x0070ccb0
	// STUB: STUDIO 0x00845380
	TaskScheduler::StepResult DataModelJob::step(const Stats& stats)
	{
		TaskScheduler::StepResult result = TaskScheduler::Done;
		if (shared_ptr<TaskScheduler::Arbiter> arbiter = getArbiter())
		{
			(void)arbiter;
			result = stepDataModelJob(stats);
		}
		return result;
	}

	// Binary size note: the 2013 DataModelJob base is 0x1E0 (derived weak
	// at +0x1E0 -- batches 572/575). Ours differs (same members, MSVC
	// layout drift in the Job running-average tail); the derived member
	// is spelled BY NAME so no raw offset is baked in. Exact numbers
	// pinned by parsetest sizeof probe (section 279).

	// mac: 0x1A9CE8 (batch575: shared_from_dynamic_cast<DataModel> of the
	// script context's parent, +0xB8 arbiter alias, "LuaResumeWaitingScripts"
	// / Write / false / waitingThreadsBudget-seconds budget; weak copy at
	// +0x1E0). Spelled by NAME (2016 shape): getParent() inlines the +0x34
	// member read, and the derived-to-base shared_ptr conversion computes
	// the +0xB8 DataModelArbiter adjustment the binary spells manually.
	// STUB: PLAYER 0x00679f90
	// STUB: STUDIO 0x007e1eb0
	WaitingScriptsJob::WaitingScriptsJob(shared_ptr<ScriptContext> scriptContext)
		: DataModelJob(
			"LuaResumeWaitingScripts",
			DataModelJob::Write,
			false,
			shared_from_dynamic_cast<DataModel>(scriptContext->getParent()),
			Time::Interval(LuaSettings::singleton().getWaitingThreadsBudget() / 60.0))
		, scriptContext(scriptContext)
	{
	}

	// mac: 0x1A9EE8: 60Hz standard sleep.
	// FUNCTION: PLAYER 0x0067a0f0
	// FUNCTION: STUDIO 0x007e2010
	Time::Interval WaitingScriptsJob::sleepTime(const Stats& stats)
	{
		return computeStandardSleepTime(stats, 60.0);
	}

	// mac: 0x1A9F16: 60Hz standard error (no cyclic-executive branch -- the
	// 2013 binary calls computeStandardError directly).
	// FUNCTION: PLAYER 0x0067a120
	// FUNCTION: STUDIO 0x007e2040
	WaitingScriptsJob::Error WaitingScriptsJob::error(const Stats& stats)
	{
		return computeStandardError(stats, 60.0);
	}

	// mac: 0x1A9F56 (batch575: weak lock, direct-parent dynamic_cast to
	// DataModel, FLog start/finish, scoped_write_request, resume with
	// nowFast()+stepBudget, Stepped).
	// DELTAs (loud): DataModel::get walks to the ROOT ancestor while the
	// binary casts the DIRECT parent -- identical whenever services hang
	// directly under the DataModel (the production shape); diverges only
	// for deeper service nesting, which 2013 never builds.
	// STUB: PLAYER 0x0068a4e0
	// STUB: STUDIO 0x007efc40
	TaskScheduler::StepResult WaitingScriptsJob::stepDataModelJob(const Stats& stats)
	{
		(void)stats;
		if (shared_ptr<ScriptContext> sc = scriptContext.lock())
		{
			DataModel* d = DataModel::get(sc.get());
			if (FLog::DataModelJobs != 0)
				FLog::FastLog(FLog::DataModelJobs, "Waiting scripts start, data model: %p", d);
			DataModel::scoped_write_request request(d);
			sc->resumeWaitingScripts(RBX::Time::nowFast() + stepBudget);
			if (FLog::DataModelJobs != 0)
				FLog::FastLog(FLog::DataModelJobs, "Waiting scripts finish, data model: %p", d);
		}
		return TaskScheduler::Stepped;
	}

	// mac: 0x1AA0EA (batch575: "LuaGc" / Write / false / arbiter from the
	// parent DataModel / 0.003 budget; weak copy at +0x1E0; frequency =
	// (int)(30.0 / ((double)(float)base + 1.0)) at +0x1E8, by name).
	// STUB: PLAYER 0x00679c00
	// STUB: STUDIO 0x007e1b20
	GcJob::GcJob(shared_ptr<ScriptContext> scriptContext)
		: DataModelJob(
			"LuaGc",
			DataModelJob::Write,
			false,
			shared_from_dynamic_cast<DataModel>(scriptContext->getParent()),
			Time::Interval(0.003))
		, scriptContext(scriptContext)
		, frequency((int)(30.0 / ((double)(float)scriptContext->gcFrequencyBase + 1.0)))
	{
	}

	// mac: 0x1AA302: standard sleep at the live frequency.
	// FUNCTION: PLAYER 0x00679d70
	// FUNCTION: STUDIO 0x007e1c90
	Time::Interval GcJob::sleepTime(const Stats& stats)
	{
		return computeStandardSleepTime(stats, (double)frequency);
	}

	// mac: 0x1AA32E: standard error at the live frequency.
	// FUNCTION: PLAYER 0x00679da0
	// FUNCTION: STUDIO 0x007e1cc0
	GcJob::Error GcJob::error(const Stats& stats)
	{
		return computeStandardError(stats, (double)frequency);
	}

	// mac: 0x1AA362 (batch575: weak lock, DataModel::get, FLog start/finish,
	// scoped_write_request, stepGc, frequency refresh, Stepped). Same
	// direct-parent DELTA as WaitingScriptsJob above.
	// STUB: PLAYER 0x00679e60
	// STUB: STUDIO 0x007e1d80
	TaskScheduler::StepResult GcJob::stepDataModelJob(const Stats& stats)
	{
		(void)stats;
		if (shared_ptr<ScriptContext> sc = scriptContext.lock())
		{
			DataModel* d = DataModel::get(sc.get());
			if (FLog::DataModelJobs != 0)
				FLog::FastLog(FLog::DataModelJobs, "GC Job start, data model: %p", d);
			DataModel::scoped_write_request request(d);
			sc->stepGc();
			frequency = (int)(30.0 / ((double)(float)sc->gcFrequencyBase + 1.0));
			if (FLog::DataModelJobs != 0)
				FLog::FastLog(FLog::DataModelJobs, "GC Job finish, data model: %p", d);
		}
		return TaskScheduler::Stepped;
	}

} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::DataModelJob, 0x218);
DECOMP_SIZE_ASSERT(RBX::GcJob, 0x228);
DECOMP_SIZE_ASSERT(RBX::DataModelArbiter, 0xa20);
