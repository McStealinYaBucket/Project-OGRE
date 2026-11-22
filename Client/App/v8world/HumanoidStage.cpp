/* App/v8world/HumanoidStage.cpp -- October 2013 (humanoid node). Asserts at the 2013 lines 25, 41, 46, 52, 55. */
#include "stdafx.h"
#include "V8World/HumanoidStage.h"
#include "V8World/SleepStage.h"
#include "V8World/World.h"
#include "V8World/Assembly.h"
#include "V8World/Primitive.h"
#include "V8World/SendPhysics.h"

namespace RBX {

	// STUB: PLAYER 0x0099b050
	// STUB: STUDIO 0x00a5abf0
	HumanoidStage::HumanoidStage(IStage* upstream, World* world)
		: IWorldStage(upstream, new SleepStage(this, world), world)
	{
	}



	// FUNCTION: PLAYER 0x0099ae80
	// FUNCTION: STUDIO 0x00a5aa20
	HumanoidStage::~HumanoidStage()
	{
		RBXASSERT(movingHumanoidAssemblies.size() == 0);
	}

	void HumanoidStage::toDynamics(Assembly* a)
	{
		rbx_static_cast<SleepStage*>(getDownstreamWS())->onAssemblyAdded(a);
	}

	void HumanoidStage::fromDynamics(Assembly* a)
	{
		rbx_static_cast<SleepStage*>(getDownstreamWS())->onAssemblyRemoving(a);
	}
	// STUB: PLAYER 0x0099ad70
	// FUNCTION: STUDIO 0x00a5a910
	void HumanoidStage::toHumanoid(Assembly* a)
	{
		RBXASSERT(a->getAssemblyState() == Sim::ANCHORED);
		a->setAssemblyState(Sim::AWAKE);

		getWorld()->getSendPhysics()->onMovingAssemblyRootAdded(a);
		bool ok = movingHumanoidAssemblies.insert(a).second;
		RBXASSERT(ok);
	}
	// STUB: PLAYER 0x0099af50
	// FUNCTION: STUDIO 0x00a5aaf0
	void HumanoidStage::fromHumanoid(Assembly* a) {
		int num = movingHumanoidAssemblies.erase(a);
		RBXASSERT(num);

		getWorld()->getSendPhysics()->onMovingAssemblyRootRemoving(a);
		RBXASSERT(a->getAssemblyState() == Sim::AWAKE);
		a->setAssemblyState(Sim::ANCHORED);
	}

	// FUNCTION: PLAYER 0x0099ae40
	// FUNCTION: STUDIO 0x00a5a9e0
	void HumanoidStage::onAssemblyAdded(Assembly* a)
	{
		a->putInStage(this);

		if (a->getAssemblyPrimitive()->getEngineType() == Primitive::DYNAMICS_ENGINE)
		{
			toDynamics(a);
		}
		else
		{
			toHumanoid(a);
		}
	}

	// FUNCTION: PLAYER 0x0099b010
	// FUNCTION: STUDIO 0x00a5abb0
	void HumanoidStage::onAssemblyRemoving(Assembly* a)
	{
		if (a->inStage(this))
		{
			fromHumanoid(a);
		}
		else
		{
			fromDynamics(a);
		}
		a->removeFromStage(this);
	}

} // namespace RBX
