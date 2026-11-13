/* App/v8world/IPipelined.cpp -- October 2013, laid out on its assert lines: getKernel :11, removeFromKernel
   :23-24, getStage :32, putInPipeline :47-48, removeFromPipeline :54-56, putInStage :63-66, removeFromStage :72-75. */
#include "stdafx.h"

#include "V8World/IPipelined.h"
#include "V8World/IWorldStage.h"
#include "V8Kernel/Kernel.h"
#include "rbx/Debug.h"
namespace RBX {
	Kernel* IPipelined::getKernel() const {
		RBXASSERT(this->inKernel());
		IStage* answer = getStage(IStage::KERNEL_STAGE);
		return rbx_static_cast<Kernel*>(answer);
	}

	// FUNCTION: PLAYER 0x00909d50
	// FUNCTION: STUDIO 0x009bfd30
	void IPipelined::putInKernel(Kernel* kernel)
	{
		putInStage(kernel); }
	void IPipelined::removeFromKernel()
	{
		RBXASSERT(currentStage);
		RBXASSERT(currentStage->getStageType() == IStage::KERNEL_STAGE);
		removeFromStage(currentStage);
	}



	IStage* IPipelined::getStage(IStage::StageType stageType) const
	{
		RBXASSERT(currentStage);
		IStage* tryStage = currentStage;
		while (true)
		{
			if (tryStage->getStageType() == stageType)
				return tryStage;
			tryStage = (tryStage->getStageType() > stageType)
				? tryStage->getUpstream()
				: tryStage->getDownstream();
		}
	}
	// FUNCTION: PLAYER 0x00909900
	// FUNCTION: STUDIO 0x009bf8e0
	void IPipelined::putInPipeline(IStage* stage)
	{
		RBXASSERT(stage);
		RBXASSERT(!currentStage);
		currentStage = stage; }
	// FUNCTION: PLAYER 0x009099b0
	// FUNCTION: STUDIO 0x009bf990
	void IPipelined::removeFromPipeline(IStage* stage)
	{
		RBXASSERT(stage);
		RBXASSERT(currentStage);
		RBXASSERT(currentStage == stage);
		currentStage = NULL;
	}
	// FUNCTION: PLAYER 0x00909a90
	// FUNCTION: STUDIO 0x009bfa70
	void IPipelined::putInStage(IStage* stage)
	{
		RBXASSERT(stage);
		RBXASSERT(currentStage);
		RBXASSERT(stage->getUpstream() == currentStage);
		RBXASSERT(currentStage->getDownstream() == stage);
		currentStage = stage; }
	// FUNCTION: PLAYER 0x00909bc0
	// FUNCTION: STUDIO 0x009bfba0
	void IPipelined::removeFromStage(IStage* stage)
	{
		RBXASSERT(currentStage);
		RBXASSERT(stage);
		RBXASSERT(stage == currentStage);
		RBXASSERT(stage->getUpstream());
		currentStage = currentStage->getUpstream();
	}

	// mac: 0x57A224 (53B).
	World* IPipelined::findWorld()
	{
		if (currentStage == NULL)
			return NULL;
		IStage* s = currentStage;
		if (inKernel())
			s = s->getUpstream();
		return s->getWorld();
	}

} // namespace RBX
