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
	// FUNCTION: PLAYER 0x008309d0
	Point* Kernel::insertPoint(Point* point)
	{	for (int i = 0; i < data->points.size(); ++i) { if (Point::sameBodyAndOffset(*point, *data->points[i])) { data->points[i]->numOwners++; delete point; return data->points[i]; } }
		RBXASSERT(!inStepCode);
		data->points.fastAppend(point); return point;
	}
	// FUNCTION: PLAYER 0x00832370
	void Kernel::insertConnector(Connector* connector)
	{
		RBXASSERT(!inStepCode);
		data->addConnector(connector);
	}
	// FUNCTION: PLAYER 0x00830970
	void Kernel::removePoint(Point* point)
	{
		RBXASSERT(!inStepCode);
		data->points.fastRemove(point); }
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


	// FUNCTION: PLAYER 0x00832430
	// FUNCTION: STUDIO 0x009a4a90
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

	// Before the solve: refresh dirty sims; a contact body under an external force or impulse moves to the realtime list.
	// FUNCTION: PLAYER 0x008318e0
	// FUNCTION: STUDIO 0x009a3f40
	void Kernel::preStep()
	{
		IndexArray<SimBody, &SimBody::getRealTimeBodyIndex>& realTimeBodies(data->realTimeBodies);
		IndexArray<SimBody, &SimBody::getJointBodyIndex>& jointBodies(data->jointBodies);
		IndexArray<SimBody, &SimBody::getContactBodyIndex>& contactBodies(data->contactBodies);
		IndexArray<Connector, &Connector::getContactIndex>& contactConnectors(data->contactConnectors);
		{
			Profiling::Mark mark(*profilingKernelBodies, false);
			for (int i = 0; i < realTimeBodies.size(); ++i)
				realTimeBodies[i]->updateIfDirty();
			for (int i = 0; i < jointBodies.size(); ++i)
				jointBodies[i]->updateIfDirty();
			for (int i = 0; i < contactBodies.size(); ++i) {
				SimBody* simBody = contactBodies[i];
				simBody->updateIfDirty();
				if (simBody->hasExternalForceOrImpulse()) {
					contactBodies.fastRemove(simBody);
					realTimeBodies.fastAppend(simBody);
					data->addLeafBodies(simBody->getBody());
					simBody->setDt(Constants::kernelDt());
					--i;
				}
				else {
					simBody->updateSymmetricContactState();
					simBody->stepVelocity();
				}
			}
		}
		{
			Profiling::Mark mark(*profilingKernelConnectors, false);
			for (int i = 0; i < contactConnectors.size(); ++i)
				rbx_static_cast<ContactConnector*>(contactConnectors[i])->clearImpulseComputed();
		}
	}

	// FUNCTION: PLAYER 0x0082ebc0
	// FUNCTION: STUDIO 0x009a1100
	void Kernel::preStepThrottled()
	{
		const IndexArray<SimBody, &SimBody::getRealTimeBodyIndex>& realTimeBodies(data->realTimeBodies);
		Profiling::Mark mark(*profilingKernelBodies, false);
		for (int i = 0; i < realTimeBodies.size(); ++i)
			realTimeBodies[i]->updateIfDirty();
	}
















	// Free fall, then the impulse solver over the contact connectors, then the kernel substeps.
	// FUNCTION: PLAYER 0x00830b20
	// FUNCTION: STUDIO 0x009a3180
	void Kernel::stepWorld()
	{
		IndexArray<SimBody, &SimBody::getFreeFallBodyIndex>& freeFallBodies(data->freeFallBodies);
		IndexArray<SimBody, &SimBody::getRealTimeBodyIndex>& realTimeBodies(data->realTimeBodies);
		IndexArray<SimBody, &SimBody::getContactBodyIndex>& contactBodies(data->contactBodies);
		const IndexArray<SimBody, &SimBody::getJointBodyIndex>& jointBodies(data->jointBodies);
		const IndexArray<Body, &Body::getLeafBodyIndex>& leafBodies(data->leafBodies);
		const IndexArray<Point, &Point::getKernelIndex>& points(data->points);
		const IndexArray<Connector, &Connector::getHumanoidIndex>& humanoidConnectors(data->humanoidConnectors);
		const IndexArray<Connector, &Connector::getRealTimeIndex>& realtimeConnectors(data->realTimeConnectors);
		const IndexArray<Connector, &Connector::getSecondPassIndex>& secondPassConnectors(data->secondPassConnectors);
		const IndexArray<Connector, &Connector::getJointIndex>& jointConnectors(data->jointConnectors);
		const IndexArray<Connector, &Connector::getContactIndex>& contactConnectors(data->contactConnectors);
		{
			Profiling::Mark mark(*profilingKernelBodies, false);
			for (int i = 0; i < freeFallBodies.size(); ++i) {
				SimBody* simBody = freeFallBodies[i];
				simBody->updateIfDirty();
				if (simBody->hasExternalForceOrImpulse()) {
					simBody->updateAngMomentum();
					freeFallBodies.fastRemove(simBody);
					realTimeBodies.fastAppend(simBody);
					simBody->setDt(Constants::kernelDt());
					--i;
				}
				else {
					simBody->stepFreeFall();
				}
			}
		}
		if (contactConnectors.size() > 0) {
			Profiling::Mark mark(*profilingKernelConnectors, false);
			float residualVelocity = 0.0f;
			int i;
			float tolerance = Constants::impulseSolverAccuracy()
				* (contactBodies.size() / Constants::impulseSolverAccuracyScalar() + 1);
			for (i = 0; i < Constants::impulseSolverMaxIterations(); ++i) {
				residualVelocity = 0.0f;
				for (int j = 0; j < contactConnectors.size(); ++j) {
					RBXASSERT(validateConnector(contactConnectors[j]));
					contactConnectors[j]->computeImpulse(residualVelocity);
				}
				residualVelocity /= contactConnectors.size();
				if (residualVelocity < tolerance) {
					++i;
					break;
				}
			}
			numLastIterations = i;
			if (numLastIterations > numOfMaxIterations)
				numOfMaxIterations = numLastIterations;
			error = residualVelocity;
			if (error > maxError)
				maxError = error;
		}
		{
			Profiling::Mark mark(*profilingKernelBodies, false);
			for (int i = 0; i < contactBodies.size(); ++i)
				contactBodies[i]->stepPosition();
		}







		for (int i = 0; i < Constants::kernelStepsPerWorldStep(); i++) {
			{
				Profiling::Mark mark(*profilingKernelConnectors, false);
				for (int j = 0; j < points.size(); ++j)
					points[j]->step();
				for (int j = 0; j < realtimeConnectors.size(); ++j) {
					RBXASSERT(validateConnector(realtimeConnectors[j]));
					realtimeConnectors[j]->computeForce(false);
				}

				for (int j = 0; j < jointConnectors.size(); ++j) {
					RBXASSERT(validateConnector(jointConnectors[j]));
					jointConnectors[j]->computeForce(false);
				}

				for (int j = 0; j < points.size(); ++j)
					points[j]->forceToBody();

				for (int j = 0; j < humanoidConnectors.size(); ++j)
					humanoidConnectors[j]->computeForce(false);

				for (int j = 0; j < secondPassConnectors.size(); ++j) {
					RBXASSERT(validateConnector(secondPassConnectors[j]));
					secondPassConnectors[j]->computeForce(false);
				}
			}
			{
				Profiling::Mark mark(*profilingKernelBodies, false);
				for (int j = 0; j < realTimeBodies.size(); ++j)
					realTimeBodies[j]->step();
				for (int j = 0; j < jointBodies.size(); ++j)
					jointBodies[j]->step();
				int leafBodiesSize = leafBodies.size();
				for (int j = 0; j < leafBodiesSize; ++j)
					leafBodies[j]->updatePV();
			}
		}
	}








	// The throttled step runs only the realtime bodies and their connectors.
	// FUNCTION: PLAYER 0x0082ec40
	// FUNCTION: STUDIO 0x009a1180
	void Kernel::stepWorldThrottled()
	{
		const IndexArray<SimBody, &SimBody::getRealTimeBodyIndex>& realtimeBodies(data->realTimeBodies);
		const IndexArray<Body, &Body::getLeafBodyIndex>& leafBodies(data->leafBodies);
		const IndexArray<Point, &Point::getKernelIndex>& points(data->points);
		const IndexArray<Connector, &Connector::getRealTimeIndex>& realtimeConnectors(data->realTimeConnectors);
		const IndexArray<Connector, &Connector::getHumanoidIndex>& humanoidConnectors(data->humanoidConnectors);
		for (int i = 0; i < Constants::kernelStepsPerWorldStep(); i++) {
			{
				Profiling::Mark mark(*profilingKernelConnectors, false);
				for (int j = 0; j < points.size(); ++j)
					points[j]->step();
				for (int j = 0; j < realtimeConnectors.size(); ++j) {
					RBXASSERT(validateConnector(realtimeConnectors[j]));
					realtimeConnectors[j]->computeForce(false);
				}
				for (int j = 0; j < points.size(); ++j)
					points[j]->forceToBody();
				for (int j = 0; j < humanoidConnectors.size(); ++j)
					humanoidConnectors[j]->computeForce(false);
			}
			{
				Profiling::Mark mark(*profilingKernelBodies, false);
				for (int j = 0; j < realtimeBodies.size(); ++j)
					realtimeBodies[j]->step();
				int leafBodiesSize = leafBodies.size();
				for (int j = 0; j < leafBodiesSize; ++j)
					leafBodies[j]->updatePV();
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

DECOMP_SIZE_ASSERT(RBX::Kernel, 0x30);
