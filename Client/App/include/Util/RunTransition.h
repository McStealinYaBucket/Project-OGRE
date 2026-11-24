/* Util/RunTransition.h — October 2013 (run-state types, minimal).
   Evidence: initializeContents 0x2D3336 binds DataModel::onRunTransition via
     signal<void(RunTransition)> (2016 source: runService->runTransitionSignal
     + identical bind). RunState/RunTransition shape carried from March 2016
     Util/RunStateOwner.h (RS_STOPPED/RUNNING/PAUSED + old/newState).
   TODO: full run-state module (RunService::setRunState/run/pause/stop,
     workspace reset/start/stop, Stepped/Heartbeat events).
   Original file: (2013 TU unknown; 2016 equivalent: Client/App/include/util/RunStateOwner.h) */

#pragma once

#include "rbx/rbxTime.h"

namespace RBX {

	enum RunState {
		RS_STOPPED,
		RS_RUNNING,
		RS_PAUSED
	};

	class RunTransition {
	public:
		RunState oldState;
		RunState newState;
		RunTransition(RunState oldState, RunState newState)
			: oldState(oldState), newState(newState)
		{}
	};

	// Heartbeat event -- layout PINNED by RunService::raiseHeartbeat
	// mac: 0x236C76 (186B): it fires a 5-double stack array through
	// signal_with_args<1, void(RBX::Heartbeat const&)>(this+156), then a
	// void(double) signal (this+168) with the gameTime delta.  Names carried
	// from March 2016 Util/RunStateOwner.h -- they map 1:1 onto the binary
	// double order. +32 is a Time: raiseHeartbeat PLAYER 0x0072da30 copies the
	// Time::nowFast()+stepBudget result into it as two dwords, which a double
	// member (fld/fstp) does not give.
	// Members are non-const so the raiseHeartbeat body can fill them.
	//
	//
	class Heartbeat 		// This event occurs at regular intervals
	{
	public:
		double wallTime;         // +0 : (RunService+140 accumulator) + dt
		double wallStep;         // +8 : raiseHeartbeat's dt argument
		                         //      (ChatOutput::onHeartbeat 0x5C298C reads +8).
		double gameTime;         // +16: RunService+116 (gameTimeValue).
		double gameStep;         // +24: gameTime - RunService+124 (prev).
		Time expirationTime;     // +32: Time::nowFast() + interval value.

		Heartbeat() {}

		Heartbeat(double wallTime, double wallStep, double gameTime, double gameStep, Time expirationTime)
			: wallTime(wallTime), wallStep(wallStep), gameTime(gameTime), gameStep(gameStep)
			, expirationTime(expirationTime)
		{}
	};

} // namespace RBX
