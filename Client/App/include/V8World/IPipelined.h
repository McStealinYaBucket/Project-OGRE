/* V8World/IPipelined.h -- October 2013 (pipeline-link base). Laid out on the 2013 assert lines:
   ~IPipelined :25 (inline; Primitive::~Primitive 0x00755540 inlines it), inStage(type) :44,
   inStage(iStage) :49-50, inOrDownstreamOfStage :60-61, downstreamOfStage :66-67 (bodies emitted
   as COMDATs in several TUs). currentStage +4. Method bodies in App/v8world/IPipelined.cpp.
   downstreamOfStage's `mov eax,1 / xor eax,eax` tail is the && chain below. */
#pragma once

#include "V8Kernel/IStage.h"

namespace RBX {

	class IStage;
	class World;
	class Kernel;

	class IPipelined
	{
	public:
		IPipelined()
			: currentStage(NULL)
		{}
		// mac: 0x576E4A tail-calls putInStage; first IPipelined virtual after the dtor.

		virtual ~IPipelined() {
			RBXASSERT(currentStage == NULL);
			currentStage = static_cast<IStage*>(Debugable::badMemory());
		}
		virtual void putInKernel(Kernel* kernel);
		virtual void removeFromKernel();
		void putInPipeline(IStage* stage);
		void removeFromPipeline(IStage* stage);
		void putInStage(IStage* stage);
		void removeFromStage(IStage* stage);
		bool inPipeline() const { return currentStage != NULL; }
		const IStage* getCurrentStage() const { return currentStage; }
		// mac: 0x57A224: nearest World up-chain (kernel hops one upstream first).
		World* findWorld();
		IStage* getStage(IStage::StageType stageType) const;
		Kernel* getKernel() const;
		// mac: 0x5869C0
		// FUNCTION: PLAYER 0x006d51c0
		// FUNCTION: STUDIO 0x007a0470
		bool inStage(IStage::StageType stageType) const {
			RBXASSERT(currentStage);
			return (currentStage && (currentStage->getStageType() == stageType)); }
		// FUNCTION: PLAYER 0x008e6230
		// FUNCTION: STUDIO 0x00a04530
		bool inStage(IStage* iStage) const {
			RBXASSERT(iStage);
			RBXASSERT(currentStage);
			return (currentStage == iStage);
		}

		bool inKernel() const { return inStage(IStage::KERNEL_STAGE); }

		// mac: 0x5A6150
		// FUNCTION: PLAYER 0x008e6670
		// FUNCTION: STUDIO 0x00a04970
		bool inOrDownstreamOfStage(IStage* iStage) const {
			RBXASSERT(iStage);
			RBXASSERT(currentStage);
			return (currentStage && iStage && (currentStage->getStageType() >= iStage->getStageType())); }
		// FUNCTION: PLAYER 0x008e3880
		// FUNCTION: STUDIO 0x009cef50
		bool downstreamOfStage(IStage* iStage) const {
			RBXASSERT(iStage);
			RBXASSERT(currentStage);
			return (currentStage && iStage && currentStage->getStageType() > iStage->getStageType());
		}

	private:
		IStage* currentStage;  // +4.
	};

} // namespace RBX
