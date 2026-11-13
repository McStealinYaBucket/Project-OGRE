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
	// pinned by RunService::raiseHeartbeat 0x236C76; see the raiseHeartbeat
	// note there.  Upstream C++ paired the struct with its raising service;
	// the landed tree keeps the signal definitions beside the stdafx-era
	// run-state types.

	class RuntimeScriptService;	// the scripts-run tracker (IScriptFilter.h); reads runState.

	class RunService
		: public DescribedNonCreatable<RunService, Instance, sRunService, Reflection::ClassDescriptor::RUNTIME_LOCAL>
		, public Service
	{
	public:
		RunService()
			: runState(RS_STOPPED)
			, gameTimeValue(0.0)
		{}
		void start();  // 0x236688

		// mac: 0x236F9A / 0x236FE2 / 0x237000: run-state transitions.
		void run()   { setRunState(RS_RUNNING); }
		void pause() { setRunState(RS_PAUSED); }
		void stop()  { setRunState(RS_STOPPED); }

		// mac: 0x236DCE: set runState + fire runTransitionSignal.
		void setRunState(int state);

		// +116: simulation game time (DataModel::getGameTime 0x2D83C4 and
		// Workspace::updateDistributedGameTime 0x515A7E read this slot).
		double gameTime() const { return gameTimeValue; }

		// mac: 0x2D3336: initializeContents connects DataModel::onRunTransition here.
		rbx::signal<void(RunTransition)> runTransitionSignal;

		// mac: 0x204C28: HeartbeatInstance::onServiceProviderHeartbeatInstance
		// connects HeartbeatInstance::onHeartbeat here (signal storage at
		// RunService+0x9C in the mac build).
		rbx::signal<void(const Heartbeat&)> heartbeatSignal;

	private:
		shared_ptr<TaskScheduler::Job> physicsJob;   // +96 (TODO: verify).
		shared_ptr<TaskScheduler::Job> heartbeatJob; // +104 (TODO: verify).
		RunState runState;		// +112 (mac). setRunState writes it; the 2013 DataModel::
			// onRunTransition 0x0062ef20 does not (no mirror store there).
		double gameTimeValue;  // +116 (TODO: writer — HeartbeatTask steps it)

		friend class RuntimeScriptService;	// onServiceProvider 0x196AC4 reads runState (+112).
	};

} // namespace RBX
