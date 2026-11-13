/* App/v8world/TreeStage.cpp -- October 2013 (tree stage node).
   Bodies here: C2 0x5A46E4 (283B, batch214: news 0x10 MovingStage +
     SpanningTree secondary-base C2 + maxTreeDepth 0 + 2 sets) / D2
     (pattern: set asserts + delete +8; D0 0x5A4800 / D1 0x5A4830) /
     onEdgeAdded 0x5A5D1E (370B) / onEdgeRemoving 0x5A5E90 (394B)
     (batch214) / onPrimitiveAdded 0x5A601A (135B: :464 assert +
     putInStage, TERMINAL -- no forward) / onPrimitiveRemoving 0x5A60A2
     (135B: removeFromStage + :472) (batch214).
   assemble + mechanism/clump/destroy fns + SpanningTree hook overrides:
   mechanism flow, deferred (World::assemble walk stops at the node).
   getStageType header-inline. getKernel: inherited. */

#include "stdafx.h"

#include "V8World/TreeStage.h"
#include "V8World/MovingStage.h"
#include "V8World/Primitive.h"
#include "V8World/Joint.h"
#include "V8World/RigidJoint.h"
#include "V8World/Clump.h"
#include "V8World/Assembly.h"
#include "V8World/Mechanism.h"
#include "V8World/World.h"
#include "V8World/IMoving.h"
#include "V8Kernel/Body.h"

#include <boost/bind.hpp>

namespace RBX {

	void assertNotInPipeline(Assembly* a)
	{
		RBXASSERT(!a->inPipeline());
	}

	bool noAssembliesInPipeline(Mechanism* m)
	{
		Assembly* a = m->getMesh()->getTypedLower<Assembly>();
		a->visitAssemblies(boost::bind(&assertNotInPipeline, _1));
		return true;
	}

	// 2016 shape (batch271: NULL-safe; asserts live at the
	// onSpanningEdgeAdded use sites :152-:156).
	bool chainToGround(Primitive* prim)
	{
		if (prim == NULL)
			return true;
		Primitive* root = prim->getRoot<Primitive>();
		Joint* joint = static_cast<Joint*>(root->getEdgeToParent());
		return joint != NULL && Joint::isGroundJoint(joint);
	}

	// mac: 0x5A46E4 (283B, batch214).
	TreeStage::TreeStage(IStage* upstreamArg, World* worldArg)
		: SpanningTree()
		, maxTreeDepth(0)
	{
		downstream = new MovingStage(this, worldArg);
		upstream = upstreamArg;
		world = worldArg;
		// Sets default-construct empty (binary zeroes + sentinels at +48/+72;
		// same net state -- cf. Sleep C2 precedent).
	}

	// STUB: PLAYER 0x008ec2f0
	// STUB: STUDIO 0x00a0a520
	TreeStage::~TreeStage()
	{
		// D2 pattern (D0/D1 per batch214; SpanningTree member + sets clean
		// themselves -- same net state as the explicit erases).
		delete static_cast<MovingStage*>(downstream);
	}

	// Forwarded unless the joint is tree-internal (rigid {1,2,3} or ground
	// {0,11} per the binary (jt & ~2) gate -- WELD/MANUAL_WELD/SNAP and
	// ANCHOR/FREE stay in the tree; the 13 default is NO_JOINT).
	static bool treeForwardsEdge(Edge* edge)
	{
		int jt = (edge->getEdgeType() == Edge::JOINT)
			? static_cast<Joint*>(edge)->getJointType()
			: Joint::NO_JOINT;
		if (((jt & ~2) == 1) || (jt == 2))
			return false;
		if (edge->getEdgeType() != Edge::JOINT)
			return true;
		jt = static_cast<Joint*>(edge)->getJointType();
		return jt != Joint::ANCHOR_JOINT && jt != Joint::FREE_JOINT;
	}

	// mac: 0x5A4CAE (batch271).
	// STUB: PLAYER 0x008ec490
	// STUB: STUDIO 0x00a0a6c0
	void TreeStage::onSpanningEdgeAdded(SpanningEdge* edge)
	{
		Joint* joint = static_cast<Joint*>(edge);
		bool isGroundJoint = Joint::isGroundJoint(joint);
		Primitive* parent = static_cast<Primitive*>(edge->getParentSpanningNode());
		Primitive* child = static_cast<Primitive*>(edge->getChildSpanningNode());




















































		RBXASSERT(chainToGround(child));  // :152
		RBXASSERT(child->getBody()->getParent() == NULL);  // :153
		RBXASSERT(child->getTypedUpper<Clump>() == NULL);  // :154
		RBXASSERT(isGroundJoint == (parent == NULL));  // :155
		// :156 DELTA: the binary folds the MI-adjusted null test into `cmp parent+8-reg, 8` (rbx_static_cast<Primitive*> from the +8 SpanningNode socket leaves NULL-derived pointers at +8, which the compare folds). Same truth value on every reachable input (NULL iff ground) -- spelled in source form here.
		RBXASSERT(isGroundJoint == (child->getTypedParent<Primitive>() == NULL));  // :156

		if (parent)
		{
			dirtyMechanism(parent->getMechanism());
		}

		if (Joint::isRigidJoint(joint)) {						// RIGID JOINT - same clump
			RigidJoint* r = static_cast<RigidJoint*>(joint);
			child->getBody()->setParent(parent ? parent->getBody() : NULL);
			child->getBody()->setMeInParent(r->getChildInParent(parent, child));
		}
		else {
			Clump* childClump = new Clump();
			child->setUpper(childClump);
			if (Joint::isMotorJoint(joint)) {					// MOTOR Joint - same assembly (new clump)
				RBXASSERT(parent != NULL);  // :178 (2016 iOS-only; 2013 always)
				child->getBody()->setParent(parent->getBody());
				child->getBody()->setMeInParent(joint->resetLink());
			}
			else {
				Assembly* childAssembly = new Assembly();
				childClump->setUpper(childAssembly);
				if (!isGroundJoint) {								// Spring Joint - same mechanism (new assembly)
					RBXASSERT(Joint::isSpringJoint(joint));  // :195
				}
				else {
					Mechanism* childMechanism = new Mechanism();	// Ground Joint - new mechanism
					// Member-mesh DELTA: our Mechanism carries its mesh
					// as a +8 member (not an IndexedMesh base), so both
					// sides spell the mesh explicitly -- same objects the
					// binary links with its +-8 adjusts.
					childAssembly->getMesh()->setUpper(childMechanism->getMesh());
					RBXASSERT(childMechanism->getMesh()->getIndexedMeshParent() == NULL);  // :200 (2016 _DEBUG-only; 2013 release)
					dirtyMechanism(childMechanism);
				}
			}
		}

		sendClumpChangedMessage(child);

		RBXASSERT(child->getClump() != NULL);  // :215
		RBXASSERT(child->getAssembly() != NULL);  // :216
		RBXASSERT(child->getMechanism() != NULL);  // :217
	}

	// mac: 0x5A54DE (batch296: NULL-safe child -8 adjust + getMechanism
	// (itself NULL-safe) + dirtyMechanism (:336 asserts non-NULL).
	// 2016's _DEBUG block compiles out here.
	// STUB: PLAYER 0x008eca40
	// STUB: STUDIO 0x00a0ac70
	void TreeStage::onSpanningEdgeRemoving(SpanningEdge* edge)
	{
		SpanningNode* childNode = edge->getChildSpanningNode();
		Primitive* child = (childNode != NULL) ? static_cast<Primitive*>(childNode) : NULL;
		dirtyMechanism(child != NULL ? child->getMechanism() : NULL);
	}

	// mac: 0x5A5538 (batch296: inline range dispatch rigid{1,2,3} /
	// motor{4,5} / spring{6..10} / ground{0,11} + :284 else-assert;
	// ground carries :275 (assembly == clump-upper) + :276 (prim
	// parent NULL -- removeFromSpanningTree cleared it); tail
	// setParent(NULL) via prim+240 body + sendmsg + :291-:294.
	// 2016-statics dispatch identically; _DEBUG/SLOW parts compile out.
	// STUB: PLAYER 0x008eca80
	// STUB: STUDIO 0x00a0acb0
	void TreeStage::onSpanningEdgeRemoved(SpanningEdge* edge, SpanningNode* childNode)
	{
		Joint* joint = static_cast<Joint*>(edge);
		Primitive* childPrim = static_cast<Primitive*>(childNode);

		if (Joint::isRigidJoint(joint))
		{
		}
		else if (Joint::isMotorJoint(joint))
		{
			destroyClump(childPrim);
		}
		else if (Joint::isSpringJoint(joint))
		{
			destroyAssembly(childPrim);
		}
		else if (Joint::isGroundJoint(joint)) 	// anchor or free joint
		{
			// :275 (binary assumes the clump exists on this path --
			// ground hooks always build one).
			RBXASSERT(childPrim->getAssembly() ==
				childPrim->getClump()->getTypedUpper<Assembly>());
			RBXASSERT(childPrim->getTypedParent<Primitive>() == NULL);  // :276
			destroyMechanism(childPrim);
		}
		else
		{
			RBXASSERT(false);  // :284 "0"
		}

		childPrim->getBody()->setParent(NULL);

		sendClumpChangedMessage(childPrim);

		RBXASSERT(childPrim->getTypedUpper<Clump>() == NULL);
		RBXASSERT(childPrim->getClump() == NULL);
		RBXASSERT(childPrim->getAssembly() == NULL);
		RBXASSERT(childPrim->getMechanism() == NULL);
	}

	// mac: 0x5A58D2 / 0x5A590E / 0x5A5972 (batch296: unconditional
	// setUpper(NULL)s + NULL-guarded deletes -- C++ delete covers the
	// guards with identical outcomes).
	void TreeStage::destroyClump(Primitive* prim)
	{
		Clump* c = prim->getClump();
		prim->setUpper(NULL);
		delete c;
	}

	void TreeStage::destroyAssembly(Primitive* prim)
	{
		Assembly* a = prim->getAssembly();
		Clump* c = prim->getClump();

		c->setUpper(NULL);
		prim->setUpper(NULL);

		delete c;
		delete a;
	}

	// FUNCTION: PLAYER 0x008ec160
	// FUNCTION: STUDIO 0x00a0a390
	void TreeStage::destroyMechanism(Primitive* prim)
	{
		Mechanism* m = prim->getMechanism();
		Assembly* a = prim->getAssembly();
		Clump* c = prim->getClump();

		removeFromPipeline(m);
		dirtyMechanisms.erase(m);

		a->setUpper(NULL);
		c->setUpper(NULL);
		prim->setUpper(NULL);

		delete a;
		delete c;
		delete m;
	}














	// mac: 0x5A5A52 (batch295).
	// FUNCTION: PLAYER 0x008ec030
	// FUNCTION: STUDIO 0x00a0a260
	void TreeStage::removeFromPipeline(Mechanism* m)
	{
		if (m->inPipeline()) {
			if (m->downstreamOfStage(this)) {
				static_cast<MovingStage*>(getDownstreamWS())->onMechanismRemoving(m);
				int num = (int)downstreamMechanisms.erase(m);
				RBXASSERT(num == 1);
			}
			m->removeFromPipeline(this);
		}
		RBXASSERT(noAssembliesInPipeline(m));
	}
	// FUNCTION: PLAYER 0x008ec0f0
	// FUNCTION: STUDIO 0x00a0a320
	void TreeStage::dirtyMechanism(Mechanism* m) {
		RBXASSERT(m);
		removeFromPipeline(m);
		dirtyMechanisms.insert(m);
	}

	// Binary shape (batch271: prim+244 owner slot-2 call + joints-list
	// rigid recursion; 2016 text).
	void TreeStage::sendClumpChangedMessage(Primitive* childPrim)
	{
		if (childPrim->getOwner() != NULL) {
			childPrim->getOwner()->onClumpChanged();
		}
		else {
			// Ground Primitive
		}

		// only do the clump
		for (int i = 0; i < childPrim->numChildren(); ++i) {
			Primitive* childChild = childPrim->getTypedChild<Primitive>(i);
			Joint* joint = static_cast<Joint*>(childChild->getEdgeToParent());
			if (RigidJoint::isRigidJoint(joint)) {
				sendClumpChangedMessage(childChild);
			}
		}
	}

















	// mac: 0x5A5B50 (batch297: :385 mesh-parent assert + putInPipeline +
	// :391 inStage + onMechanismAdded + downstream insert (:394) --
	// matches the 2016 text exactly).
	// FUNCTION: PLAYER 0x008ebc40
	// FUNCTION: STUDIO 0x00a09ed0
	void TreeStage::cleanMechanism(Mechanism* m)
	{
		RBXASSERT(!m->getIndexedMeshParent());
		if (!m->inPipeline())
		{
			m->putInPipeline(this);
		}

		RBXASSERT(m->inStage(this));
		static_cast<MovingStage*>(getDownstreamWS())->onMechanismAdded(m);
		bool ok = downstreamMechanisms.insert(m).second;
		RBXASSERT(ok);
	}

	// mac: 0x5A5CB8 (101B, batch214).
	void TreeStage::assemble()
	{
		if (!isAssembled())
		{
			// 1. Assemblies
			std::set<Mechanism*>::iterator it;
			for (it = dirtyMechanisms.begin(); it != dirtyMechanisms.end(); ++it) {
				cleanMechanism(*it);				// clean upstream dirtyAssemblies
			}
			dirtyMechanisms.clear();
		}
	}







	// mac: 0x5A5D1E (370B, batch214).
	// STUB: PLAYER 0x008eb7a0
	// STUB: STUDIO 0x00a099a0
	void TreeStage::onEdgeAdded(Edge* e)
	{
		RBXASSERT(e->getPrimitive(0)->inOrDownstreamOfStage(this));
		RBXASSERT(!e->getPrimitive(1) || e->getPrimitive(1)->inOrDownstreamOfStage(this));
		e->putInStage(this);
		if (!Joint::isSpanningTreeJoint(e))
		{
			getDownstreamWS()->onEdgeAdded(e);
			return;
		}
		Joint* joint = static_cast<Joint*>(e);
		insertSpanningTreeEdge(static_cast<SpanningEdge*>(joint));
		if (treeForwardsEdge(e))
			getDownstreamWS()->onEdgeAdded(e);
	}


	// mac: 0x5A5E90 (394B, batch214).
	// STUB: PLAYER 0x008eb8d0
	// STUB: STUDIO 0x00a09ad0
	void TreeStage::onEdgeRemoving(Edge* e)
	{
		RBXASSERT(e->getPrimitive(0)->inOrDownstreamOfStage(this));
		RBXASSERT(!e->getPrimitive(1) || e->getPrimitive(1)->inOrDownstreamOfStage(this));
		if (Joint::isSpanningTreeJoint(e))
		{
			if (treeForwardsEdge(e))
				getDownstreamWS()->onEdgeRemoving(e);
			SpanningEdge* se = static_cast<SpanningEdge*>(static_cast<Joint*>(e));
			if (se->inSpanningTree())
				removeSpanningTreeEdge(se);
		}
		else
		{
			getDownstreamWS()->onEdgeRemoving(e);
		}
		e->removeFromStage(this);
	}

	// mac: 0x5A601A (135B, batch214).
	// STUB: PLAYER 0x008eba20
	// STUB: STUDIO 0x00a09c20
	void TreeStage::onPrimitiveAdded(Primitive* p)
	{
		RBXASSERT(p->getNumEdges() == 0);
		p->putInStage(this);
	}

	// STUB: PLAYER 0x008eba90
	// STUB: STUDIO 0x00a09c90
	void TreeStage::onPrimitiveRemoving(Primitive* p) {
		p->removeFromStage(this);
		RBXASSERT(p->getNumEdges() == 0);
	}


} // namespace RBX
