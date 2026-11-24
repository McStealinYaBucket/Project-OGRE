/* V8DataModel/RunService.h -- October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<RunService> from DataModel::initializeContents
     0x2D3336 (stored at +2944 (= DataModel::runService slot); start() called when startHeartbeat; gameTime at +116 (getGameTime 0x2D83C4).); "RunService" string in both 2013 PEs.
   Functionality + Service base carried from March 2016 conventions.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/RunService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "Util/RunTransition.h"
#include "rbx/signal.h"
#include "rbx/TaskScheduler.Job.h"

namespace RBX {

	extern const char* const sRunService;

	// RBX::Heartbeat (5 doubles) is defined in Util/RunTransition.h -- layout
	// pinned by RunService::raiseHeartbeat 0x236C76.

	// 2016 Util/RunStateOwner.h; the 2013 PLAYER fires it through the two signals at +0x98/+0x9c.
	class Stepped
	{
	public:
		const double gameTime;
		const double gameStep;
		const bool longStep;
		Stepped(double gameTime, double gameStep, bool longStep):gameTime(gameTime),gameStep(gameStep),longStep(longStep) {}
	};

	class RuntimeScriptService;	// the scripts-run tracker (IScriptFilter.h); reads runState.
	class PhysicsJob;
	class HeartbeatTask;

	// 2013 layout (PLAYER ctor 0x0072efd0, stopTasks 0x0072bb60): Instance 0x60, Service @0x60,
	// physicsJob @0x64, heartbeatTask @0x6c, runState @0x74, four doubles @0x78..0x90 (2016's
	// totalWallTime and skippedTimeAccumulated swap places: raiseHeartbeat 0x0072da30 accumulates at 0x90), six
	// signals @0x98..0xac (no 2016 dummyTasksPadding / renderSteppedEarlyCallbackMap).
	// Signal names: runTransitionSignal @0xa4 (initializeContents 0x0064f559 connects
	// onRunTransition there), heartbeatSignal @0xa0 (HeartbeatInstance 0x00871186); the others
	// follow the 2016 order (the two @0x98/@0x9c share one type; @0xa8 is RunService-only, the
	// Mac raiseHeartbeat fires a void(double) signal 0xc past heartbeat, i.e. @0xac here).
	// VTABLE: PLAYER 0x00b4d964 RBX::Debugable
	// SIZE 0xb0
	class RunService
		: public DescribedNonCreatable<RunService, Instance, sRunService, Reflection::ClassDescriptor::RUNTIME_LOCAL>
		, public Service
	{
	private:
		typedef DescribedNonCreatable<RunService, Instance, sRunService, Reflection::ClassDescriptor::RUNTIME_LOCAL> Super;

		// Typed, as 2013 (iOS 10-29 has shared_ptr<PhysicsJob>::operator= and
		// shared_ptr<HeartbeatTask>::operator=): with Job-typed members RunService::start 0x0072d300
		// keeps two temporaries in two stack slots where the original reuses one; typed, start and
		// stopTasks are both identical. PhysicsJob and HeartbeatTask are defined in RunService.cpp.
		friend class PhysicsJob;
		shared_ptr<PhysicsJob> physicsJob;		// 0x64
		friend class HeartbeatTask;
		shared_ptr<HeartbeatTask> heartbeatTask;	// 0x6c

		RunState runState;							// 0x74
		double totalGameTime;						// 0x78
		double totalGameTimeAtLastHeartbeat;		// 0x80
		double skippedTimeAccumulated;				// 0x88
		double totalWallTime;						// 0x90  raiseHeartbeat 0x0072da30 adds wallStep here

		friend class DataModel;
		friend class RuntimeScriptService;	// onServiceProvider reads runState.

	public:
		RunService();

		rbx::signal<void(const Stepped&)> steppedSignal;			// 0x98
		rbx::signal<void(const Stepped&)> renderSteppedSignal;		// 0x9c
		rbx::signal<void(const Heartbeat&)> heartbeatSignal;		// 0xa0
		rbx::signal<void(RunTransition)> runTransitionSignal;		// 0xa4
		rbx::signal<void(double, double)> scriptSteppedSignal;		// 0xa8
		rbx::signal<void(double)> scriptHeartbeatSignal;			// 0xac

		void start();

		// mac: 0x236F9A / 0x236FE2 / 0x237000: run-state transitions.
		void run()   { setRunState(RS_RUNNING); }
		void pause() { setRunState(RS_PAUSED); }
		void stop()  { setRunState(RS_STOPPED); }

		// mac: 0x236DCE: set runState + fire runTransitionSignal.
		void setRunState(int state);

		void stopTasks();

		void raiseHeartbeat(double wallStep, const Time::Interval& stepBudget);

		// PhysicsJob::getDesiredConcurrencyCount 0x0072b760 returns 1024 when set (the 2016
		// name; a settings class exposes it as "ParallelPhysics", initializer 0x00a81880).
		static bool parallelPhysicsUserEnabled;

		RunState getRunState() const		{return runState;}
		double wallTime() const {return totalWallTime;}
		double gameTime() const {return totalGameTime;}

	protected:
		void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider);
	};

} // namespace RBX

// Template, inline and implicit bodies this .obj emits whose 2013 copy has no oracle name: paired
// by identical masked bytes plus string/RTTI/callee operands (tools/unpaired_twins.py).

// TEMPLATE: PLAYER 0x0072b4c0
// TEMPLATE: STUDIO 0x00798ec0
// ??$?0VPhysicsJob@RBX@@@shared_count@detail@boost@@QAE@PAVPhysicsJob@RBX@@@Z

// TEMPLATE: PLAYER 0x0072b550
// TEMPLATE: STUDIO 0x00798f50
// ??$?0VHeartbeatTask@RBX@@@shared_count@detail@boost@@QAE@PAVHeartbeatTask@RBX@@@Z

// TEMPLATE: PLAYER 0x0072e090
// TEMPLATE: STUDIO 0x0079bc30
// ??0?$Described@VRunService@RBX@@$E?sRunService@2@3QBDBV?$NonFactoryProduct@VInstance@RBX@@$E?sRunService@2@3QBDB@2@$0BB@$0A@@Reflection@RBX@@QAE@XZ


// The six signal constructors RunService::RunService (PLAYER 0x0072efd0, STUDIO 0x0079cc10) calls,
// in member order (+0x98/+0x9c share one).

// TEMPLATE: PLAYER 0x0072b9e0
// TEMPLATE: STUDIO 0x007994c0
// ??0?$signal@$$A6AXABVStepped@RBX@@@Z@signals@rbx@@QAE@XZ

// TEMPLATE: PLAYER 0x0072ba40
// TEMPLATE: STUDIO 0x00799520
// ??0?$signal@$$A6AXABVHeartbeat@RBX@@@Z@signals@rbx@@QAE@XZ

// TEMPLATE: PLAYER 0x0072baa0
// TEMPLATE: STUDIO 0x00799580
// ??0?$signal@$$A6AXVRunTransition@RBX@@@Z@signals@rbx@@QAE@XZ

// TEMPLATE: PLAYER 0x0072bb00
// TEMPLATE: STUDIO 0x007995e0
// ??0?$signal@$$A6AXNN@Z@signals@rbx@@QAE@XZ

// TEMPLATE: PLAYER 0x0052e6c0
// TEMPLATE: STUDIO 0x005de760
// ??0?$signal@$$A6AXN@Z@signals@rbx@@QAE@XZ

// TEMPLATE: PLAYER 0x0072bc60
// TEMPLATE: STUDIO 0x00799770
// ??$sp_pointer_construct@VPhysicsJob@RBX@@V12@@detail@boost@@YAXPAV?$shared_ptr@VPhysicsJob@RBX@@@1@PAVPhysicsJob@RBX@@AAVshared_count@01@@Z

// TEMPLATE: PLAYER 0x0072bcd0
// TEMPLATE: STUDIO 0x007997e0
// ??$sp_pointer_construct@VHeartbeatTask@RBX@@V12@@detail@boost@@YAXPAV?$shared_ptr@VHeartbeatTask@RBX@@@1@PAVHeartbeatTask@RBX@@AAVshared_count@01@@Z
