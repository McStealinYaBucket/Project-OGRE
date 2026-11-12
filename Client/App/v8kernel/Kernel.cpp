/* App/v8kernel/Kernel.cpp -- October 2013 (pipeline terminal bodies).
   Bodies: C2 0x52A20E / D2 (batch221b) / getKernel 0x52E95A (batch218).
   getStageType header-inline. numKernels static++/--. */

#include "stdafx.h"

#include "V8Kernel/Kernel.h"
#include "V8Kernel/KernelData.h"
#include "V8Kernel/Point.h"
#include "V8Kernel/Body.h"
#include "V8Kernel/SimBody.h"
#include "V8Kernel/Connector.h"
#include "V8Kernel/Constants.h"
#include "Util/Math.h"
#include "Util/Profiling.h"

namespace RBX {

	int Kernel::numKernels = 0;

	// mac: 0x52A20E (296B, batch221b).
	// STUB: PLAYER 0x00831590
	// STUB: STUDIO 0x009a3bf0
	Kernel::Kernel(IStage* upstreamArg)
		: inStepCode(false)
		, numLastIterations(0)
		, numOfMaxIterations(0)
		, error(0.0f)
		, maxError(0.0f)
		, data(new KernelData())
		, profilerBodies(NULL)      // DELTA: binary news 0xA01C CodeProfiler
		, profilerConnectors(NULL)  // ("Kernel Bodies"/"Kernel Connectors").
	{
		upstream = upstreamArg;
		downstream = NULL;  // terminal.
		world = NULL;       // binary +12 0.
		++numKernels;
	}

	// D2 (batch221b: :41 !inStepCode + delete data + profiler deletes +
	// delete +8[NULL]).
	Kernel::~Kernel()
	{
		--numKernels;
		RBXASSERT(!inStepCode);  // :41
		delete data;
		// Profilers always NULL (see header) -- deletes skipped.
		// downstream NULL (terminal) -- delete skipped.
	}

	// mac: 0x52A7F4 (batch244).
	// STUB: PLAYER 0x008309d0
	// STUB: STUDIO 0x009a3030
	void Kernel::insertPoint(Point* point)
	{
		RBXASSERT(!inStepCode);  // :98
		RBXASSERT(point != NULL);  // IndexArray.h:60 "item"
		RBXASSERT(point->indexInKernel() == false);  // :61 (index -1)
		point->kernelIndex = (int)data->points.size();
		data->points.append(point);
	}

	// mac: 0x52A9AE (133B, batch249).
	// STUB: PLAYER 0x00830970
	// STUB: STUDIO 0x009a2fd0
	void Kernel::removePoint(Point* point)
	{
		RBXASSERT(!inStepCode);  // :110
		// IndexArray::fastRemove(data->points, point) equivalent
		// (back-swap + moved-occupant fix + index reset -- World::
		// removePrimitive precedent for the spelling).
		int idx = point->kernelIndex;
		RBXASSERT(idx >= 0 && idx < (int)data->points.size());
		RBXASSERT(data->points[idx] == point);
		data->points[idx] = data->points.back();
		data->points.pop_back();
		if (idx < (int)data->points.size())
			data->points[idx]->kernelIndex = idx;
		point->kernelIndex = -1;
	}

	// mac: 0x52AB7E (488B, batch249).
	Point* Kernel::searchForDuplicatePoint(Point* input)
	{
		for (size_t i = 0; i < data->points.size(); ++i)
		{
			Point* candidate = data->points[i];
			RBXASSERT(candidate->kernelIndex == (int)i);  // :103
			if (Point::sameBodyAndOffset(*input, *candidate))
			{
				RBXASSERT(candidate->kernelIndex == (int)i);  // :103
				++candidate->numOwners;
				if (input != NULL)
					delete input;
				return candidate;
			}
		}
		insertPoint(input);
		return input;
	}

	// (batch244: post-dec equals 1 releases the last owner).
	void Kernel::deletePoint(Point* point)
	{
		RBXASSERT(!inStepCode);  // :172
		if (point != NULL && point->numOwners-- == 1)
		{
			removePoint(point);
			delete point;
		}
	}

	// (batch222: :104 !inStepCode + delegate, returns the kind).
	int Kernel::insertConnector(Connector* connector)
	{
		RBXASSERT(!inStepCode);  // :104
		return data->addConnector(connector);
	}

	// (batch222: :116 !inStepCode + delegate).
	void Kernel::removeConnector(Connector* connector)
	{
		RBXASSERT(!inStepCode);  // :116
		data->removeConnector(connector);
	}

	// Array-size counters (batch222).
	int Kernel::numFreeFallBodies() const { return data->freeFallBodies.size(); }
	int Kernel::numRealTimeBodies() const { return data->realTimeBodies.size(); }
	int Kernel::numJointBodies() const { return data->jointBodies.size(); }
	int Kernel::numContactBodies() const { return data->contactBodies.size(); }
	int Kernel::numLeafBodies() const { return data->leafBodies.size(); }
	int Kernel::numConnectors() const
	{
		return data->secondPassConnectors.size()
			+ data->realTimeConnectors.size()
			+ data->jointConnectors.size()
			+ data->contactConnectors.size();
	}
	int Kernel::numJointConnectors() const { return data->jointConnectors.size(); }

	// 0x52Axxx (batch222: :77 root-sim dt assert + KernelData insert;
	// +12 count maintenance skipped -- see header DELTA).
	void Kernel::insertBody(Body* body)
	{
		RBXASSERT(body->getRootSimBody()->getDt() == 0.0f);  // :77
		data->insertBody(body);
	}

	// 2016 shapes (first-party; stepWorldThrottled validates each
	// realtime connector before solving; EAs queued).
	bool Kernel::validateConnector(Connector* connector) const
	{
		bool oneInKernel = validateConnectorBody(connector->getBody(Connector::body0));
		oneInKernel = validateConnectorBody(connector->getBody(Connector::body1)) || oneInKernel;
		return oneInKernel;
	}

	bool Kernel::validateConnectorBody(Body* body) const
	{
		if (body != NULL)
		{
			if (body->getRootSimBody()->isInKernel())
				return true;
			else if (body->isLeafBody())
				return true;
		}
		return false;
	}

	// mac: 0x52B058 (batch298: kernel-substep loop over points, realtime +
	// humanoid connectors, realTime sims and leaf bodies. IndexArray
	// :113 asserts dropped -- KernelData lists are plain G3D arrays
	// with no index protocol (loud DELTA); loop membership identical.
	// STUB: PLAYER 0x0082ec40
	// STUB: STUDIO 0x009a1180
	void Kernel::stepWorldThrottled()
	{
		KernelData* kd = data;
		int steps = Constants::kernelStepsPerWorldStep();
		if (steps <= 0)
			return;
		for (int k = 0; k < steps; ++k)
		{
			{
				Profiling::Mark markBodies(NULL, false, false);
				for (size_t i = 0; i < kd->points.size(); ++i)
					kd->points[i]->step();
				for (size_t j = 0; j < kd->realTimeConnectors.size(); ++j)
				{
					Connector* c = kd->realTimeConnectors[j];
					RBXASSERT(validateConnector(c));  // :393
					c->computeForce(false);
				}
				for (size_t i = 0; i < kd->points.size(); ++i)
					kd->points[i]->forceToBody();
				for (size_t j = 0; j < kd->humanoidConnectors.size(); ++j)
					kd->humanoidConnectors[j]->computeForce(false);
			}
			{
				Profiling::Mark markConnectors(NULL, false, false);
				for (size_t i = 0; i < kd->realTimeBodies.size(); ++i)
					kd->realTimeBodies[i]->step();
				for (size_t i = 0; i < kd->leafBodies.size(); ++i)
					kd->leafBodies[i]->updatePV();
			}
		}
	}

	// mac: 0x52BBDA (batch298: freeFall update/promote/stepFreeFall +
	// impulse loop + contact stepPosition + kernel substeps. IndexArray
	// :103/:113 asserts dropped -- plain G3D arrays, no index protocol
	// (loud DELTA); membership and order identical).
	// STUB: PLAYER 0x00830b20
	// STUB: STUDIO 0x009a3180
	void Kernel::stepWorld()
	{
		KernelData* kd = data;
		{
			Profiling::Mark mark(NULL, false, false);
			for (size_t i = 0; i < kd->freeFallBodies.size(); /* manual */)
			{
				SimBody* sim = kd->freeFallBodies[i];
				if (sim->getDirty())
					sim->update();
				if (sim->hasExternalForceOrImpulse())
				{
					sim->updateAngMomentum();
					// Promote freeFall -> realTime (back-swap + reindex):
					SimBody* moved = kd->freeFallBodies.last();
					kd->freeFallBodies[i] = moved;
					moved->freeFallBodyIndex = (int)i;
					kd->freeFallBodies.pop(false);
					sim->freeFallBodyIndex = -1;
					sim->realTimeBodyIndex = (int)kd->realTimeBodies.size();
					kd->realTimeBodies.append(sim);
					sim->setDt(Constants::kernelDt());
				}
				else
				{
					sim->stepFreeFall();
					++i;
				}
			}
		}
		if (kd->contactConnectors.size() > 0)
		{
			Profiling::Mark mark(NULL, false, false);
			float target = Constants::kernelDt()
				* ((int)kd->contactBodies.size() / Constants::impulseSolverAccuracyScalar() + 1);
			int iters = 0;
			float avgResidual = 0.0f;
			do
			{
				if (iters >= Constants::impulseSolverMaxIterations())
					break;
				float residual = 0.0f;
				for (size_t j = 0; j < kd->contactConnectors.size(); ++j)
				{
					Connector* c = kd->contactConnectors[j];
					RBXASSERT(validateConnector(c));  // :303
					c->computeImpulse(residual);
				}
				avgResidual = residual / (float)kd->contactConnectors.size();
				++iters;
			}
			while (target <= avgResidual);
			numLastIterations = iters;
			if (iters > numOfMaxIterations)
				numOfMaxIterations = iters;
			error = avgResidual;
			if (avgResidual > maxError)
				maxError = avgResidual;
		}
		{
			Profiling::Mark mark(NULL, false, false);
			for (size_t i = 0; i < kd->contactBodies.size(); ++i)
				kd->contactBodies[i]->stepPosition();
		}
		int steps = Constants::kernelStepsPerWorldStep();
		if (steps <= 0)
			return;
		for (int k = 0; k < steps; ++k)
		{
			{
				Profiling::Mark mark(NULL, false, false);
				for (size_t i = 0; i < kd->points.size(); ++i)
					kd->points[i]->step();
				for (size_t j = 0; j < kd->realTimeConnectors.size(); ++j)
				{
					Connector* c = kd->realTimeConnectors[j];
					RBXASSERT(validateConnector(c));  // :337
					c->computeForce(false);
				}
				for (size_t i = 0; i < kd->points.size(); ++i)
					kd->points[i]->forceToBody();
				for (size_t j = 0; j < kd->humanoidConnectors.size(); ++j)
					kd->humanoidConnectors[j]->computeForce(false);
				for (size_t j = 0; j < kd->secondPassConnectors.size(); ++j)
				{
					Connector* c = kd->secondPassConnectors[j];
					RBXASSERT(validateConnector(c));  // :353
					c->computeForce(false);
				}
				for (size_t j = 0; j < kd->jointConnectors.size(); ++j)
				{
					Connector* c = kd->jointConnectors[j];
					RBXASSERT(validateConnector(c));  // :342
					c->computeForce(false);
				}
			}
			{
				Profiling::Mark mark(NULL, false, false);
				for (size_t i = 0; i < kd->realTimeBodies.size(); ++i)
					kd->realTimeBodies[i]->step();
				for (size_t i = 0; i < kd->jointBodies.size(); ++i)
					kd->jointBodies[i]->step();
				for (size_t i = 0; i < kd->leafBodies.size(); ++i)
					kd->leafBodies[i]->updatePV();
			}
		}
	}

	// (batch222: :88 !inStepCode + :90 dt-consistency arms + remove).
	void Kernel::removeBody(Body* body)
	{
		RBXASSERT(!inStepCode);  // :88
		{
			SimBody* rootSim = body->getRootSimBody();
			bool throttled = !rootSim->isContactBody() && !rootSim->isFreeFallBody();
			RBXASSERT(throttled || Math::fuzzyEq(rootSim->getDt(), Constants::freeFallDt(), 1e-6f));  // :90
		}
		{
			SimBody* rootSim = body->getRootSimBody();
			bool streaming = rootSim->isContactBody() || rootSim->isFreeFallBody();
			RBXASSERT(!streaming || Math::fuzzyEq(rootSim->getDt(), Constants::kernelDt(), 1e-6f));
		}
		data->removeBody(body);
	}

} // namespace RBX
