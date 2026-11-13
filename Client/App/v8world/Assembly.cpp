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
	Assembly::Assembly()		: unknown04(0)
		, unknown48(0)
		, unknown52(0)
		, flag56(false)
		, flag57(false)
		, history(NULL)
		, assemblyState(Sim::ANCHORED)
		, simJob(NULL)
		, unknown104(-1)
		, unknown108(-1)
		, filterPhase(6)
		, maxRadius(0.0f)
		, flag120(true)
		, self124(this)
		, radiusCallback(NULL)  // DELTA: binary stores &computeAssemblyMaxRadius (see header).
		, unknown132(0)
	{
	}

	// D2 (batch241).
	Assembly::~Assembly()
	{
		RBXASSERT(history == NULL);  // :36 "history == NULL"
		RBXASSERT(assemblyState == Sim::ANCHORED);  // :37
		RBXASSERT(simJob == NULL);  // :38 "simJob == NULL"
		RBXASSERT(unknown108 == -1);  // :39 "recursiveDepth == -1"
		// Arrays/mesh/base tear down implicitly (same net state as the
		// explicit Array/~IndexedMesh/~IPipelined calls).
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
	// STUB: PLAYER 0x008f9070
	// STUB: STUDIO 0x00a0e130
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

	// mac: 0x555572 (151B).
	// STUB: PLAYER 0x007b14c0
	// STUB: STUDIO 0x008cd550
	Assembly* Assembly::otherAssembly(Edge* e)
	{
		Assembly* a0 = e->getPrim0()->getAssembly();
		Assembly* a1 = e->getPrim1()->getAssembly();
		RBXASSERT(a0 != a1);  // Assembly.cpp:262 "a0 != a1"
		if (a0 != this)
			return a0;
		return a1;
	}

	// mac: 0x554A3C (batch241/242: lower.lower - 8 container adjust +
	// :102 answer assert. Walks DOWN (assembly -> clump -> prim mesh);
	// the old computedUpper-up draft was backwards -- it read the
	// mechanism mesh as a prim. Binary disasm pins lower.lower:
	// [this+0x28] then [+0x20], -8).
	// STUB: PLAYER 0x007b18e0
	// STUB: STUDIO 0x008cd970
	Primitive* Assembly::getAssemblyPrimitive()
	{
		IndexedMesh* lower = getMesh()->getTypedLower<IndexedMesh>();
		IndexedMesh* primMesh = (lower != NULL)
			? lower->getTypedLower<IndexedMesh>() : NULL;
		Primitive* answer = (primMesh != NULL)
			? reinterpret_cast<Primitive*>((char*)primMesh - 8) : NULL;
		RBXASSERT(answer != NULL);  // Assembly.cpp:102 "answer"
		return answer;
	}

	// 2016 shape (upper-chain test + walk cross-check assert).
	bool Assembly::isAssemblyRootPrimitive(const Primitive* prim)
	{
		bool answer = false;
		if (prim != NULL)
		{
			const Clump* clump = prim->getMesh()->getConstTypedUpper<Clump>();
			if (clump != NULL && clump->getConstTypedUpper<Assembly>() != NULL)
				answer = true;
		}
		RBXASSERT(!answer || (getConstPrimitiveAssembly(prim)->getConstTypedLower<Clump>()->getConstTypedLower<Primitive>() == prim));
		return answer;
	}

	// Const mirror (same lower.lower shape).
	const Primitive* Assembly::getConstAssemblyPrimitive() const
	{
		const IndexedMesh* lower = getMesh()->getConstTypedLower<IndexedMesh>();
		const IndexedMesh* primMesh = (lower != NULL)
			? lower->getConstTypedLower<IndexedMesh>() : NULL;
		const Primitive* answer = (primMesh != NULL)
			? reinterpret_cast<const Primitive*>((const char*)primMesh - 8) : NULL;
		RBXASSERT(answer != NULL);  // Assembly.cpp:102 "answer"
		return answer;
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

	// mac: 0x554ABE (128B, batch259: :94 assert + return the +64 word).
	// The assert short-circuits on ANCHORED, so fresh (stageless)
	// assemblies never touch the stage.
	// STUB: PLAYER 0x007b1390
	// STUB: STUDIO 0x008cd420
	Sim::AssemblyState Assembly::getAssemblyState() const
	{
		RBXASSERT(assemblyState == Sim::ANCHORED  // :94
			|| getCurrentStage()->getStageType() >= IStage::HUMANOID_STAGE);
		return assemblyState;
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
