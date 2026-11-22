/* App/v8world/Assembly.cpp -- October 2013 (C2 + null-safe lookups).
   Bodies: C2 0x554424 / getPrimitiveAssembly 0x554C50 /
     getConstPrimitiveAssembly 0x554C82 (batch202). */
#include "stdafx.h"
#include "V8World/Assembly.h"
#include "V8World/AssemblyHistory.h"
#include "V8World/Clump.h"
#include "V8World/Primitive.h"
#include "V8World/Joint.h"
#include "V8World/Contact.h"
#include "V8World/IMoving.h"
#include "V8Kernel/Body.h"
#include <boost/bind.hpp>
namespace RBX {

	// mac: 0x554424 (243B).  Member order mirrors the C2 init order.
	Assembly::Assembly()
		: flag56(false), flag57(false), history(NULL)
		, state(Sim::ANCHORED), simJob(NULL), unknown104(-1), recursiveDepth(-1), filterPhase(6)
		, maxRadius(0.0f), flag120(true), self124(this)
		, radiusCallback(NULL)  // DELTA: binary stores &computeAssemblyMaxRadius (see header).
		, unknown132(0)
	{
	}






	// D2 (batch241).
	// FUNCTION: PLAYER 0x007b1cd0
	// FUNCTION: STUDIO 0x008cdd60
	Assembly::~Assembly()
	{
		RBXASSERT(history == NULL);
		RBXASSERT(state == Sim::ANCHORED);
		RBXASSERT(simJob == NULL);
		RBXASSERT(recursiveDepth == -1);
	}
















































	// mac: 0x554ABE (128B, batch259).
	// FUNCTION: PLAYER 0x007b1390
	// FUNCTION: STUDIO 0x008cd420
	Sim::AssemblyState Assembly::getAssemblyState() const
	{
		RBXASSERT((state == Sim::ANCHORED) || (getCurrentStage()->getStageType() >= IStage::HUMANOID_STAGE));
		return state;
	}
	// FUNCTION: PLAYER 0x007b18e0
	// FUNCTION: STUDIO 0x008cd970
	Primitive* Assembly::getAssemblyPrimitive()
	{
		Primitive* answer = getTypedLower<Clump>()->getTypedLower<Primitive>();
		RBXASSERT(answer);
		return answer;
	}
	// FUNCTION: PLAYER 0x007b1940
	// FUNCTION: STUDIO 0x008cd9d0
	const Primitive* Assembly::getConstAssemblyPrimitive() const {
		const Primitive* answer = getConstTypedLower<Clump>()->getConstTypedLower<Primitive>();
		RBXASSERT(answer);
		return answer;
	}

	// mac: 0x554C50 (50B).
	Assembly* Assembly::getPrimitiveAssembly(Primitive* prim)
	{
		if (prim == NULL)
			return NULL;
		IndexedMesh* first = prim->getMesh()->getComputedUpper();
		if (first == NULL)
			return NULL;
		IndexedMesh* second = first->getComputedUpper();
		if (second == NULL)
			return NULL;
		return reinterpret_cast<Assembly*>(
			reinterpret_cast<char*>(second) - 8);
	}

	// mac: 0x554C82 (50B, symmetric).
	// (0x008f9070 is IndexedMesh::getComputedUpper, 8b 41 24 c3: not this function.)
	const Assembly* Assembly::getConstPrimitiveAssembly(const Primitive* prim)
	{
		if (prim == NULL)
			return NULL;
		const IndexedMesh* first = prim->getMesh()->getConstComputedUpper();
		if (first == NULL)
			return NULL;
		const IndexedMesh* second = first->getConstComputedUpper();
		if (second == NULL)
			return NULL;
		return reinterpret_cast<const Assembly*>(
			reinterpret_cast<const char*>(second) - 8);
	}

	// (batch242: grounded iff an anchor joint touches the root prim).
	bool Assembly::computeIsGrounded() const
	{
		return Joint::findConstJoint(getConstAssemblyPrimitive(), Joint::ANCHOR_JOINT) != NULL;
	}

	// Batch260 (0x554A04).
	void Assembly::wakeUp()
	{
		getAssemblyPrimitive()->setNetworkIsSleeping(false);
		if (history != NULL)
			history->wakeUp();
	}

	// Batch257.
	bool Assembly::getCanThrottle() const
	{
		return getConstAssemblyPrimitive()->getCanThrottle();
	}

	// Batch257 (member per the mangling; no this-use).
	bool Assembly::computeCanThrottle(Edge* edge)
	{
		const Primitive* p0 = getConstPrimitiveAssembly(edge->getPrim0())->getConstAssemblyPrimitive();
		if (p0->getCanThrottle())
		{
			const Primitive* p1 = getConstPrimitiveAssembly(edge->getPrim1())->getConstAssemblyPrimitive();
			return p1->getCanThrottle();
		}
		return false;
	}



































































	// STUB: PLAYER 0x007b19a0
	// STUB: STUDIO 0x008cda30
	bool Assembly::isAssemblyRootPrimitive(const Primitive* p)
	{
		bool answer = false;
		if (p != NULL)
		{
			const Clump* clump = p->getMesh()->getConstTypedUpper<Clump>();
			if (clump != NULL && clump->getConstTypedUpper<Assembly>() != NULL)
				answer = true;
		}

		RBXASSERT(!answer || (getConstPrimitiveAssembly(p)->getConstTypedLower<Clump>()->getConstTypedLower<Primitive>() == p));
		return answer;
	}
	// FUNCTION: PLAYER 0x007b14c0
	// FUNCTION: STUDIO 0x008cd550
	Assembly* Assembly::otherAssembly(Edge* e) {
		Assembly* a0 = e->getPrim0()->getAssembly();
		Assembly* a1 = e->getPrim1()->getAssembly();
		RBXASSERT(a0 != a1);
		return (a0 == this) ? a1 : a0;
	}

	// mac: 0x554CB4 (12B, batch273: flag120 = 1, invalidating the maxRadius
	// cache highVelocityNewTouch recomputes through +128).
	void Assembly::onLowersChanged()
	{
		flag120 = true;
	}

	// 2016 shapes (maxRadius engine; binary stores the callback at +128
	// and highVelocityNewTouch funnels through it -- direct call here,
	// same value, indirection as DELTA).
	static void computeAssemblyPrimitiveMaxRadius(Primitive* prim,
		const G3D::Vector3& cofm, float& answer)
	{
		G3D::Vector3 offsetCenter = Math::vector3Abs(prim->getBody()->getPos() - cofm);
		G3D::Vector3 offsetCorner = prim->getSize() * 0.5f;
		float offset = (offsetCenter + offsetCorner).length();
		answer = std::max(answer, offset);
	}

	float Assembly::computeAssemblyMaxRadius()
	{
		Primitive* assemblyPrim = getAssemblyPrimitive();
		G3D::Vector3 cofm(assemblyPrim->getBody()->getBranchCofmPos());
		float answer = 0.0f;
		visitPrimitives(boost::bind(&computeAssemblyPrimitiveMaxRadius, _1,
			boost::cref(cofm), boost::ref(answer)));
		return answer;
	}

	// 2016 shape (free function; owner notify + contact reset fan).
	void notifyAssemblyPrimitiveMoved(Primitive* prim, bool resetContacts)
	{
		prim->getOwner()->notifyMoved();
		if (resetContacts)
		{
			for (int i = 0; i < prim->getNumContacts(); ++i)
				static_cast<Contact*>(prim->getContact(i))->primitiveMovedExternally();
		}
	}

	// mac: 0x555420 (batch299: assembly-prim assert + moved fan).
	void Assembly::notifyMovedFromExternal()
	{
		Primitive* assemblyPrim = getAssemblyPrimitive();
		RBXASSERT(assemblyPrim != NULL);  // Assembly.h:203 "p"
		visitPrimitivesImpl(boost::bind(&notifyAssemblyPrimitiveMoved, _1, true), assemblyPrim);
	}

} // namespace RBX
