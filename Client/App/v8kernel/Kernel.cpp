/* App/v8kernel/Kernel.cpp -- October 2013.
   The ctor, dtor (:41) and insert/removeConnector (:104/:116) sit on their 2013 lines. */
#include "stdafx.h"
#include "decomp.h"
#include "V8Kernel/Kernel.h"
#include "V8Kernel/KernelData.h"
#include "V8Kernel/Point.h"
#include "V8Kernel/Body.h"
#include "V8Kernel/SimBody.h"
#include "V8Kernel/Connector.h"
#include "V8Kernel/Constants.h"
#include "Util/Math.h"
#include "Util/Profiling.h"
#include <algorithm>
namespace RBX {

	// GLOBAL: PLAYER 0x00e118c0
	// GLOBAL: STUDIO 0x00f9b690
	int Kernel::numKernels = 0;
	// FUNCTION: PLAYER 0x00831590
	// FUNCTION: STUDIO 0x009a3bf0
	Kernel::Kernel(IStage* upstreamArg)
		: IStage(upstreamArg, NULL)
		, maxBodies(0), inStepCode(false)
		, numLastIterations(0)
		, numOfMaxIterations(0)
		, error(0.0f)
		, maxError(0.0f)
		, data(new KernelData())
		, profilingKernelBodies(new Profiling::CodeProfiler("Kernel Bodies"))
		, profilingKernelConnectors(new Profiling::CodeProfiler("Kernel Connectors"))
	{
		++numKernels;
	}

	// FUNCTION: PLAYER 0x0082f670
	// FUNCTION: STUDIO 0x009a1c20
	Kernel::~Kernel()
	{
		--numKernels;
		RBXASSERT(!inStepCode);
		delete data;
	}





























	// FUNCTION: PLAYER 0x008322d0
	// FUNCTION: STUDIO 0x009a4930
	void Kernel::insertBody(Body* b)
	{
		RBXASSERT(b->getRootSimBody()->getDt() == 0.0f);
		data->insertBody(b);
		maxBodies = std::max(maxBodies, numContactBodies() + numJointBodies() + numRealTimeBodies() + numFreeFallBodies());
	}



	// FUNCTION: PLAYER 0x00831690
	// FUNCTION: STUDIO 0x009a3cf0
	void Kernel::removeBody(Body* b)
	{
		RBXASSERT(!inStepCode);
		RBXASSERT((!b->getRootSimBody()->isContactBody() && !b->getRootSimBody()->isFreeFallBody())
			|| Math::fuzzyEq(b->getRootSimBody()->getDt(), Constants::freeFallDt(), 1e-6f));
		RBXASSERT((b->getRootSimBody()->isContactBody() || b->getRootSimBody()->isFreeFallBody())
			|| Math::fuzzyEq(b->getRootSimBody()->getDt(), Constants::kernelDt(), 1e-6f));
		data->removeBody(b);
	}
	// STUB: PLAYER 0x008309d0
	Point* Kernel::insertPoint(Point* point)
	{	for (int i = 0; i < data->points.size(); ++i) { if (Point::sameBodyAndOffset(*point, *data->points[i])) { data->points[i]->numOwners++; delete point; return data->points[i]; } }
		RBXASSERT(!inStepCode);
		point->kernelIndex = (int)data->points.size(); data->points.append(point); return point;	// 2013: an IndexArray fastAppend
	}
	// FUNCTION: PLAYER 0x00832370
	int Kernel::insertConnector(Connector* connector)
	{
		RBXASSERT(!inStepCode);
		return data->addConnector(connector);
	}
	// STUB: PLAYER 0x00830970
	void Kernel::removePoint(Point* point)
	{
		RBXASSERT(!inStepCode); int idx = point->kernelIndex;	// 2013: an IndexArray fastRemove on one line
		data->points[idx] = data->points.back(); data->points.pop_back(); if (idx < (int)data->points.size()) data->points[idx]->kernelIndex = idx; point->kernelIndex = -1; }
	// FUNCTION: PLAYER 0x008323d0
	// FUNCTION: STUDIO 0x009a4a30
	void Kernel::removeConnector(Connector* connector)
	{
		RBXASSERT(!inStepCode);
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








	// FUNCTION: PLAYER 0x00831820
	// FUNCTION: STUDIO 0x009a3e80
	Point* Kernel::newPointLocal(Body* body, const G3D::Vector3& localPos)
	{
		RBXASSERT(!inStepCode);
		Point* point = new Point(body);
		point->setLocalPos(localPos);
		return insertPoint(point);
	}

















	// FUNCTION: PLAYER 0x00830ab0
	// FUNCTION: STUDIO 0x009a3110
	void Kernel::deletePoint(Point* point)
	{
		RBXASSERT(!inStepCode);
		if (point != NULL && --point->numOwners == 0)
		{
			removePoint(point);
			delete point;
		}
	}


	// STUB: PLAYER 0x00832430
	// STUB: STUDIO 0x009a4a90
	void Kernel::step(bool throttling, int numThreads)
	{
		RBXASSERT(!inStepCode);
		inStepCode = true;
		if (throttling) {
			preStepThrottled();
			stepWorldThrottled();
		}
		else {
			preStep();
			stepWorld();
		}
		inStepCode = false;
	}

	// STUB: PLAYER 0x0082ebc0
	// STUB: STUDIO 0x009a1100
	void Kernel::preStepThrottled()
	{
		Profiling::Mark mark(*profilingKernelBodies, false);
		for (int i = 0; i < data->realTimeBodies.size(); ++i) {
			SimBody* b = data->realTimeBodies[i];
			if (b->getDirty())
				b->update();
		}
	}

	// STUB: PLAYER 0x008318e0
	// STUB: STUDIO 0x009a3f40
	void Kernel::preStep()
	{
		STUB(0x008318e0);
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
				Profiling::Mark markConnectors(*profilingKernelConnectors, false);
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
				Profiling::Mark markBodies(*profilingKernelBodies, false);
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
			Profiling::Mark mark(*profilingKernelBodies, false);
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
			Profiling::Mark mark(*profilingKernelConnectors, false);
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
			Profiling::Mark mark(*profilingKernelBodies, false);
			for (size_t i = 0; i < kd->contactBodies.size(); ++i)
				kd->contactBodies[i]->stepPosition();
		}
		int steps = Constants::kernelStepsPerWorldStep();
		if (steps <= 0)
			return;
		for (int k = 0; k < steps; ++k)
		{
			{
				Profiling::Mark mark(*profilingKernelConnectors, false);
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
				Profiling::Mark mark(*profilingKernelBodies, false);
				for (size_t i = 0; i < kd->realTimeBodies.size(); ++i)
					kd->realTimeBodies[i]->step();
				for (size_t i = 0; i < kd->jointBodies.size(); ++i)
					kd->jointBodies[i]->step();
				for (size_t i = 0; i < kd->leafBodies.size(); ++i)
					kd->leafBodies[i]->updatePV();
			}
		}
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

} // namespace RBX
