/* App/v8world/HumanoidStage.cpp -- October 2013 (humanoid node).
   C2 0x575ED8 (batch220: news 0x180 SleepStage; set default-constructs).
   D2 pattern (D1 0x575F9E / D0 0x575F6E EAs via vtable; bodies not dumped
   -- uniform 0x28 set-node: set-empty assert + delete +8, cf. AssemblyStage
   precedent). getStageType header-inline. getKernel: inherited.
   onAssembly pair + to/fromHumanoid: assembly flow, deferred. */

#include "stdafx.h"

#include "V8World/HumanoidStage.h"
#include "V8World/SleepStage.h"
#include "V8World/World.h"

namespace RBX {

	// mac: 0x575ED8 (batch220).
	// STUB: PLAYER 0x0099b050
	// STUB: STUDIO 0x00a5abf0
	HumanoidStage::HumanoidStage(IStage* upstreamArg, World* worldArg)
	{
		downstream = new SleepStage(this, worldArg);
		upstream = upstreamArg;
		world = worldArg;
	}

	// STUB: PLAYER 0x0099b100
	// STUB: STUDIO 0x00a5aca0
	HumanoidStage::~HumanoidStage()
	{
		RBXASSERT(movingHumanoidAssemblies.empty());
		delete static_cast<SleepStage*>(downstream);
	}

} // namespace RBX
