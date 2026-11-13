/* App/v8world/IPipelined.cpp -- October 2013 (pipeline-link bodies).
   Bodies: putInPipeline 0x577220 / removeFromPipeline 0x5772E8 /
     putInStage 0x576E54 / removeFromStage 0x5770B0 (batch211) + D2.
   Asserts via RBXASSERT + binary file:line citations (precedent: World
   IndexArray asserts). */

#include "stdafx.h"

#include "V8World/IPipelined.h"
#include "V8World/IWorldStage.h"
#include "V8Kernel/Kernel.h"
#include "rbx/Debug.h"

namespace RBX {

	IPipelined::~IPipelined()
	{
		// 2016 D2 shape (assert-empty + poison; binary D2 EA unrecovered).
		RBXASSERT(currentStage == NULL);
		currentStage = static_cast<IStage*>(Debugable::badMemory());
	}

	// mac: 0x576E4A (9B, batch247): tail-calls putInStage (Kernel : IStage).
	// FUNCTION: PLAYER 0x00909d50
	// FUNCTION: STUDIO 0x009bfd30
	void IPipelined::putInKernel(Kernel* kernel)
	{
		putInStage(kernel);
	}

	// (218B, batch247).
	void IPipelined::removeFromKernel()
	{
		RBXASSERT(currentStage != NULL);  // IPipelined.cpp:23 "currentStage"
		RBXASSERT(currentStage->getStageType() == IStage::KERNEL_STAGE);  // :24
		removeFromStage(currentStage);
	}

	// mac: 0x577220 (199B).
	// STUB: PLAYER 0x00909900
	// STUB: STUDIO 0x009bf8e0
	void IPipelined::putInPipeline(IStage* stage)
	{
		RBXASSERT(stage != NULL);  // IPipelined.cpp:47 "stage"
		RBXASSERT(currentStage == NULL);  // IPipelined.cpp:48 "!currentStage"
		currentStage = stage;
	}

	// mac: 0x5772E8 (285B).
	// STUB: PLAYER 0x009099b0
	// STUB: STUDIO 0x009bf990
	void IPipelined::removeFromPipeline(IStage* stage)
	{
		RBXASSERT(stage != NULL);  // IPipelined.cpp:54 "stage"
		RBXASSERT(currentStage != NULL);  // IPipelined.cpp:55 "currentStage"
		RBXASSERT(currentStage == stage);  // IPipelined.cpp:56 "currentStage == stage"
		currentStage = NULL;
	}

	// mac: 0x576E54 (386B).
	// STUB: PLAYER 0x00909a90
	// STUB: STUDIO 0x009bfa70
	void IPipelined::putInStage(IStage* stage)
	{
		RBXASSERT(stage != NULL);  // IPipelined.cpp:63 "stage"
		RBXASSERT(currentStage != NULL);  // IPipelined.cpp:64 "currentStage"
		RBXASSERT(stage->getUpstream() == currentStage);  // IPipelined.cpp:65
		RBXASSERT(currentStage->getDownstream() == stage);  // IPipelined.cpp:66
		currentStage = stage;
	}

	// mac: 0x5770B0 (367B).
	// STUB: PLAYER 0x00909bc0
	// STUB: STUDIO 0x009bfba0
	void IPipelined::removeFromStage(IStage* stage)
	{
		RBXASSERT(currentStage != NULL);  // IPipelined.cpp:72 "currentStage"
		RBXASSERT(stage != NULL);  // IPipelined.cpp:73 "stage"
		RBXASSERT(stage == currentStage);  // IPipelined.cpp:74 "stage == currentStage"
		RBXASSERT(stage->getUpstream() != NULL);  // IPipelined.cpp:75 "stage->getUpstream()"
		currentStage = currentStage->getUpstream();
	}

	// mac: 0x57A224 (53B, batch219a).
	World* IPipelined::findWorld()
	{
		if (currentStage == NULL)
			return NULL;
		IStage* s = currentStage;
		if (inStage(IStage::KERNEL_STAGE))
			s = s->getUpstream();
		return s->getWorld();
	}

	// 2016 shapes (first-party).
	IStage* IPipelined::getStage(IStage::StageType stageType) const
	{
		RBXASSERT(currentStage != NULL);
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

	Kernel* IPipelined::getKernel() const
	{
		RBXASSERT(inStage(IStage::KERNEL_STAGE));
		IStage* answer = getStage(IStage::KERNEL_STAGE);
		return rbx_static_cast<Kernel*>(answer);
	}

} // namespace RBX
