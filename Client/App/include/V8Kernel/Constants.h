/* V8Kernel/Constants.h -- October 2013 (physics stepping constants).
   Evidence: full method list from the batch170 name sweep
     (worldStepsPerUiStep 0x52701A / uiStepsPerSec / worldStepsPerSec /
     worldDt / kernelDt / freeFallDt / ...).  Only worldStepsPerUiStep is
     ported so far (its body is 33B, fully decoded); the rest queue behind
     their own dumps -- no stubs (see the header TODO).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8kernel/Constants.cpp (path inferred) */

#pragma once

#include "G3D/Vector3.h"

namespace RBX {

	class Constants
	{
	public:
		// mac: 0x52701A (33B): 4 * (Physics60HZ == 0) + 4 -- 8 normally, 4 in
		// 60Hz mode.  The FFlag default (false) matches the shipped 2013
		// client; see Util/FastFlags.h for the server-value DELTA.
		static int worldStepsPerUiStep();
		// mac: 0x527202 (123B, batch250): elasticity ladder 0.28/0.42/0.57/
		// 0.8/1.0 at 0.05/0.26/0.51/0.76 -- thresholds disasm-pinned;
		// values 2016-carried (identical function; top pair lives in
		// rodata -- byte-verification queued, batch250 has the EA).
		static float getElasticMultiplier(float elasticity);
		// Batch253 getter family (disasm-verbatim arithmetic; Physics60HZ
		// false matches the shipped client -- see FastFlags DELTA above).
		static int worldStepsPerSec();       // 0x527048
		static float worldDt();              // 0x527110
		static float kernelDt();             // 0x527160
		static float freeFallDt();           // 0x5271B0
		static int kernelStepsPerWorldStep();  // 0x52703C
		static float longUiStepDt();         // 0x5270FE
		// 2016 shape (first-party inline; EA queues with the stepper --
		// read by SimBody::update 0x534E72 via rodata g).
		static float getKmsGravity() { return -9.81f; }
		// 2016 shape (denormal guard for the stepper; EA queued).
		static const G3D::Vector3& denormalSmall();
		// 2016 shapes (impulse-solver tuning; EAs queued with stepWorld).
		static int impulseSolverMaxIterations();
		static float impulseSolverAccuracy();
		static int impulseSolverAccuracyScalar();
		// mac: 0x526FE4 (10B, batch593): long-UI rate is always 30Hz.
		static int longUiStepsPerSec();
		// mac: 0x526FEE (10B, batch593): 8 world steps per long UI step.
		static int worldStepsPerLongUiStep();
		// mac: 0x526FF8 (33B, batch593): 60 in 60Hz mode, else 30.
		static int uiStepsPerSec();
		// mac: 0x5270C6 (55B, batch593): 1.0f / (60.0f | 30.0f) from the
		// C81FC8 pair (single float rounding, disasm-verbatim).
		static float uiDt();
		// mac: 0x5270A2 (17B, batch593): rodata CA6FBC = 0.05f.
		static float impulseSolverSymStateTorqueBound();
		// mac: 0x5270B4 (17B, batch593): rodata C35400 = 0.001f.
		static float impulseSolverSymStateForceBound();
		// TODO (batch170 sweep): uiStepsPerSec, ... Each needs its own
		// dump; do not stub.
	};

} // namespace RBX
