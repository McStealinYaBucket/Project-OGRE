/* App/v8kernel/Constants.cpp -- October 2013 (worldStepsPerUiStep only). */

#include "stdafx.h"

#include "V8Kernel/Constants.h"
#include "Util/FastFlags.h"

namespace RBX {

	// mac: 0x52701A (33B).
	int Constants::worldStepsPerUiStep()
	{
		return 4 * (FFlag::Physics60HZ == false) + 4;
	}

	// mac: 0x527202 (123B, batch250).
	float Constants::getElasticMultiplier(float elasticity)
	{
		if (elasticity < 0.05f)
			return 0.28f;
		else if (elasticity < 0.26f)
			return 0.42f;
		else if (elasticity < 0.51f)
			return 0.57f;
		else if (elasticity < 0.76f)
			return 0.8f;
		else
			return 1.0f;
	}

	// Batch253 (disasm-verbatim).
	int Constants::worldStepsPerSec()
	{
		int hz = FFlag::Physics60HZ ? 60 : 30;
		return hz * worldStepsPerUiStep();
	}

	float Constants::worldDt()
	{
		int hz = FFlag::Physics60HZ ? 60 : 30;
		return (float)(1.0 / (float)(hz * worldStepsPerUiStep()));
	}

	float Constants::kernelDt()
	{
		int hz = FFlag::Physics60HZ ? 60 : 30;
		int sub = FFlag::Physics60HZ ? 76 : 152;
		return (float)(1.0 / (float)(hz * sub));
	}

	float Constants::freeFallDt()
	{
		int hz = FFlag::Physics60HZ ? 60 : 30;
		return (float)(1.0 / (float)(hz * worldStepsPerUiStep()));
	}

	int Constants::kernelStepsPerWorldStep()
	{
		return 19;
	}

	float Constants::longUiStepDt()
	{
		return 0.033333335f;
	}

	// 2016 shape (first-party; the (1e-20) vector step() adds to beat
	// denormals; EA queues with the stepper).
	const G3D::Vector3& Constants::denormalSmall()
	{
		static const G3D::Vector3 v(1e-20f, 1e-20f, 1e-20f);
		return v;
	}

	// 2016 shapes (impulse-solver tuning; EAs queued with stepWorld).
	int Constants::impulseSolverMaxIterations() { return 60; }
	float Constants::impulseSolverAccuracy() { return 0.1f; }
	int Constants::impulseSolverAccuracyScalar() { return 400; }

	// Batch593 (disasm-verbatim; Physics60HZ false matches shipped).
	int Constants::longUiStepsPerSec() { return 30; }
	int Constants::worldStepsPerLongUiStep() { return 8; }
	int Constants::uiStepsPerSec()
	{
		return FFlag::Physics60HZ ? 60 : 30;
	}
	float Constants::uiDt()
	{
		float hz = FFlag::Physics60HZ ? 60.0f : 30.0f;
		return 1.0f / hz;
	}
	float Constants::impulseSolverSymStateTorqueBound() { return 0.05f; }
	float Constants::impulseSolverSymStateForceBound() { return 0.001f; }

} // namespace RBX
