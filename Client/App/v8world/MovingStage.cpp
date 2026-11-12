/* App/v8world/MovingStage.cpp -- October 2013 (moving stage node bodies).
   C2 0x58740E (105B, batch216: news 0xA8 SpatialFilter, uniform) / D2
   (pattern: delete +8; D1 0x5874C2 / D0 0x587478 per batch224).
   getStageType header-inline. onEdge pair: inherited IWorldStage base bodies
   (batch225 vtable proof). onMechanism pair (0x587560/0x5876A8, batch220):
   different dispatch family; lands with the assembly flow. getKernel:
   inherited IStage::getKernel. */

#include "stdafx.h"

#include "V8World/MovingStage.h"
#include "V8World/SpatialFilter.h"
#include "V8World/World.h"
#include "V8World/Mechanism.h"
#include "V8World/Assembly.h"

#include <boost/bind.hpp>

namespace RBX {

	// Shared with TreeStage.cpp (its noAssembliesInPipeline twin).
	void assertNotInPipeline(Assembly* a);

	// mac: 0x587560-assert twin (see TreeStage note): the binary inlines the
	// same lowers walk here; shared shape, local name per 2016 (...2).
	static bool noAssembliesInPipeline2(Mechanism* m)
	{
		Assembly* a = m->getMesh()->getTypedLower<Assembly>();
		a->visitAssemblies(boost::bind(&assertNotInPipeline, _1));
		return true;
	}

	SpatialFilter* MovingStage::getSpatialFilter()
	{
		return static_cast<SpatialFilter*>(downstream);
	}

	// mac: 0x58740E (105B, batch216).
	// STUB: PLAYER 0x0099a420
	// STUB: STUDIO 0x00a5a6d0
	MovingStage::MovingStage(IStage* upstreamArg, World* worldArg)
	{
		downstream = new SpatialFilter(this, worldArg);
		upstream = upstreamArg;
		world = worldArg;
	}

	// STUB: PLAYER 0x0099a4e0
	// STUB: STUDIO 0x00a5a7a0
	MovingStage::~MovingStage()
	{
		// D2 pattern (uniform 0x10 node: delete +8; D1/D0 per batch224;
		// members: none).
		delete static_cast<SpatialFilter*>(downstream);
	}

	// mac: 0x587560 (batch220).
	// STUB: PLAYER 0x0099a510
	// STUB: STUDIO 0x00a5a7d0
	void MovingStage::onMechanismAdded(Mechanism* mechanism)
	{
		RBXASSERT(noAssembliesInPipeline2(mechanism));
		mechanism->putInStage(this);
		Assembly* root = mechanism->getRootAssembly();
		bool grounded = root->computeIsGrounded();
		if (grounded)
			getSpatialFilter()->onFixedAssemblyRootAdded(root);
		else {
			getSpatialFilter()->onMovingAssemblyRootAdded(root);
			for (int i = 0; i < root->numChildren(); ++i) {
				Assembly* child = root->getTypedChild<Assembly>(i);
				getSpatialFilter()->onMovingAssemblyRootAdded(child);
			}
		}
	}

	// mac: 0x5876A8 (batch295; 2016 text -- binary-identical flow incl. the
	// inlined noAssemblies walk).
	// STUB: PLAYER 0x0099a5b0
	// STUB: STUDIO 0x00a5a870
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
