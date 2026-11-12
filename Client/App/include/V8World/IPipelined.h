/* V8World/IPipelined.h -- October 2013 (pipeline-link base).
   Evidence: putInStage 0x576E54 (:63-:66) / removeFromStage 0x5770B0
     (:72-:75) / putInPipeline 0x577220 (:47-:48) / removeFromPipeline
     0x5772E8 (:54-:56) (batch211); downstreamOfStage 0x5A069C /
     inOrDownstreamOfStage (batch211 -- need IStage::getStageType vtbl+8,
     deferred to its firewall section). 2016 shape carried for the member
     (currentStage +4, C2 NULL) and the query inlines; method bodies in
     App/v8world/IPipelined.cpp (new TU; anchors the vtable in the lib).
     Primitive/Edge derive it (binary +4 stage link; 2016 derives too);
     Assembly/Mechanism already did. inPipeline() == (currentStage != NULL)
     retires Primitive's bool flag (same observable; World :858/:876/:892
     verified in section 116).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/IPipelined.h (path inferred) */

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
		// D2 asserts empty (2016 shape); body in the lib TU (CRT rule).
		virtual ~IPipelined();
		// mac: 0x576E4A (9B, batch247): stage into the terminal Kernel
		// (Kernel : IStage; first IPipelined virtual -- Primitive vtable
		// slot [4]). Tail-calls putInStage.
		virtual void putInKernel(Kernel* kernel);
		// (batch247: asserts currentStage (:23) + KERNEL_STAGE (:24),
		// then removeFromStage(currentStage)).
		virtual void removeFromKernel();

		// Fresh pipeline entry (asserts :47 stage / :48 !currentStage).
		void putInPipeline(IStage* stage);
		// Pipeline exit (asserts :54 stage / :55 currentStage /
		// :56 currentStage == stage).
		void removeFromPipeline(IStage* stage);
		// Chained stage move (asserts :63 stage / :64 currentStage /
		// :65 stage->upstream == currentStage / :66 downstream == stage).
		void putInStage(IStage* stage);
		// Stage pop to upstream (asserts :72 currentStage / :73 stage /
		// :74 stage == currentStage / :75 stage->upstream).
		void removeFromStage(IStage* stage);

		// 2016 shapes.
		bool inPipeline() const { return currentStage != NULL; }
		const IStage* getCurrentStage() const { return currentStage; }
		// mac: 0x57A224 (53B, batch219a): nearest World up-chain (kernel hops
		// one upstream first). NULL when stageless.
		World* findWorld();
		// mac: 0x5869C0 (140B, batch218): assert currentStage (IPipelined.h:44).
		bool inStage(IStage::StageType type) const;
		// mac: 0x5A07F0 (205B, batch218): asserts :49/:50.
		bool inStage(const IStage* stage) const;
		// 2016 shape (stage walk by type ordering; getKernel asserts
		// inKernel first -- used by BlockBlockContactData::find).
		IStage* getStage(IStage::StageType stageType) const;
		Kernel* getKernel() const;
		// mac: 0x5A069C (235B, batch211): asserts IPipelined.h:66/67.
		bool downstreamOfStage(IStage* stage) const;
		// mac: 0x5A6150 (235B, batch211): asserts IPipelined.h:60/61.
		bool inOrDownstreamOfStage(IStage* stage) const;

	private:
		IStage* currentStage;  // +4.
	};

} // namespace RBX
