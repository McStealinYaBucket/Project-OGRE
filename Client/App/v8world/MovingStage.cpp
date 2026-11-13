/* App/v8world/MovingStage.cpp -- October 2013 (moving stage node; mac C2 0x58740E, onMechanism pair
   0x587560/0x5876A8). Its own assertNotInPipeline2 (MovingStage.cpp:35) feeds the walk. */
#include "stdafx.h"
#include "V8World/MovingStage.h"
#include "V8World/SpatialFilter.h"
#include "V8World/World.h"
#include "V8World/Mechanism.h"
#include "V8World/Assembly.h"
#include <boost/bind.hpp>

namespace RBX {

	SpatialFilter* MovingStage::getSpatialFilter()
	{
		return static_cast<SpatialFilter*>(downstream);
	}

	// STUB: PLAYER 0x0099a420
	// STUB: STUDIO 0x00a5a6d0
	MovingStage::MovingStage(IStage* upstreamArg, World* worldArg)
	{
		downstream = new SpatialFilter(this, worldArg);
		upstream = upstreamArg;
		world = worldArg;
	}





	// FUNCTION: PLAYER 0x0099a490
	// FUNCTION: STUDIO 0x00a5a750
	void assertNotInPipeline2(Assembly* a)
	{
		RBXASSERT(!a->inPipeline());
	}

	static bool noAssembliesInPipeline2(Mechanism* m)
	{
		Assembly* a = m->getMesh()->getTypedLower<Assembly>();
		a->visitAssemblies(boost::bind(&assertNotInPipeline2, _1));
		return true;
	}

	// STUB: PLAYER 0x0099a4e0
	// STUB: STUDIO 0x00a5a7a0
	MovingStage::~MovingStage()
	{
		delete static_cast<SpatialFilter*>(downstream);
	}

	// mac: 0x587560 (batch220).
	// FUNCTION: PLAYER 0x0099a510
	// FUNCTION: STUDIO 0x00a5a7d0
	void MovingStage::onMechanismAdded(Mechanism* mechanism)
	{
		RBXASSERT(noAssembliesInPipeline2(mechanism));
		mechanism->putInStage(this);
		Assembly* root = mechanism->getRootAssembly();
		if (root->computeIsGrounded()) {
			getSpatialFilter()->onFixedAssemblyRootAdded(root);
			for (int i = 0; i < root->numChildren(); ++i) {
				Assembly* child = root->getTypedChild<Assembly>(i);
				getSpatialFilter()->onMovingAssemblyRootAdded(child);
			}
		}
		else
			getSpatialFilter()->onMovingAssemblyRootAdded(root);
	}

	// mac: 0x5876A8 (batch295; 2016 text -- binary-identical flow incl. the
	// inlined noAssemblies walk).
	// FUNCTION: PLAYER 0x0099a5b0
	// FUNCTION: STUDIO 0x00a5a870
	void MovingStage::onMechanismRemoving(Mechanism* mechanism)
	{
		Assembly* root = mechanism->getRootAssembly();

		getSpatialFilter()->onAssemblyRootRemoving(root);

		for (int i = 0; i < root->numChildren(); ++i) {
			Assembly* child = root->getTypedChild<Assembly>(i);
			getSpatialFilter()->onAssemblyRootRemoving(child);
		}

		mechanism->removeFromStage(this);

		RBXASSERT(noAssembliesInPipeline2(mechanism));
	}

} // namespace RBX
