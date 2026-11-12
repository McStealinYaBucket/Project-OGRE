/* V8World/HumanoidStage.h -- October 2013 (humanoid stage node, 0x28).
   Evidence: C1 0x57631E / C2 0x575ED8 (batch220: news 0x180 SleepStage +
     +20 set-header init) / D1 0x575F9E / D0 0x575F6E (EAs via vtable;
     bodies pattern-derived: set-empty assert + delete +8) /
     getStageType 0x576328 -> 12 / onAssemblyAdded 0x57628E /
     onAssemblyRemoving 0x5762DA / toHumanoid/fromHumanoid (batch222).
   Members: std::set<Assembly*> movingHumanoidAssemblies (+20 -- name
   carried from 2016 HumanoidStage.h, first-party; binary +20 init matches
   the libstdc++ set-header shape, cf. Sleep C2 analysis).
   onEdge pair: inherited IWorldStage base bodies (no override in vtable --
   verified absent across sweeps 208/213/221b/222; explicit forwarders for
   MSVC concreteness, behavior == base).
   onAssemblyAdded/Removing + toHumanoid/fromHumanoid (need the Humanoid
   module): declared with the assembly flow, not here.
   Layout: IStage base (+4 upstream / +8 downstream=SleepStage / +12 world).
   Original file: Client/App/v8world/HumanoidStage.h (path inferred) */

#pragma once

#include "V8World/IWorldStage.h"

#include <set>

namespace RBX {

	class World;
	class Assembly;

	class HumanoidStage : public IWorldStage
	{
	public:
		HumanoidStage(IStage* upstream, World* world);
		virtual ~HumanoidStage();

		// mac: 0x576328 (10B): HUMANOID_STAGE.
		// FUNCTION: PLAYER 0x006c8740 FOLDED
		// FUNCTION: STUDIO 0x007a6470 FOLDED
		virtual StageType getStageType() const { return HUMANOID_STAGE; }
		// onEdge pair: inherited base bodies (see MovingStage.h note).
		virtual void onEdgeAdded(Edge* edge) { IWorldStage::onEdgeAdded(edge); }
		virtual void onEdgeRemoving(Edge* edge) { IWorldStage::onEdgeRemoving(edge); }

	private:
		std::set<Assembly*> movingHumanoidAssemblies;  // +20.
	};

} // namespace RBX
