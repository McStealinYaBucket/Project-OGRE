/* App/v8world/Clump.cpp -- October 2013. PLAYER .obj 0x008e4f40-0x008e51c0 (STUDIO 0x009d00f0-):
   isClumpRootPrimitive, ~Clump (inlined into its D0), getConstPrimitiveClump, loadConstMotors (asserts
   :42/:44), loadMotors (:59/:61). 2013 has no nonAnimatedOnly argument (both loaders `ret 4`).
   getPrimitiveClump has no body of its own (inlined at every caller). */
#include "stdafx.h"

#include "V8World/Clump.h"
#include "V8World/Primitive.h"
#include "V8World/Joint.h"

namespace RBX {

	Clump::~Clump()
	{}

	// FUNCTION: PLAYER 0x008e4f40
	// FUNCTION: STUDIO 0x009d00f0
	bool Clump::isClumpRootPrimitive(const Primitive* p)
	{
		return IndexedMesh::isUpperRoot(p);
	}

	Clump* Clump::getPrimitiveClump(Primitive* p)
	{
		return rbx_static_cast<Clump*>(p->getMesh()->getComputedUpper());
	}

	// STUB: PLAYER 0x008e4f90
	// STUB: STUDIO 0x009d0140
	const Clump* Clump::getConstPrimitiveClump(const Primitive* p)
	{
		return rbx_static_cast<const Clump*>(p->getMesh()->getConstComputedUpper());
	}




	// FUNCTION: PLAYER 0x008e4fa0
	// FUNCTION: STUDIO 0x009d0150
	void Clump::loadConstMotors(G3D::Array<const Joint*>& load) const
	{
		RBXASSERT(getConstTypedLower<Primitive>());
		const SpanningEdge* edge = getConstTypedLower<Primitive>()->getConstEdgeToParent();
		RBXASSERT(edge);

		const Joint* j = rbx_static_cast<const Joint*>(edge);

		if (Joint::isMotorJoint(j)) {
			load.append(j);
		}
		for (int i = 0; i < numChildren(); ++i) {
			getConstTypedChild<Clump>(i)->loadConstMotors(load);
		}
	}
	// FUNCTION: PLAYER 0x008e50b0
	// FUNCTION: STUDIO 0x009d0260
	void Clump::loadMotors(G3D::Array<Joint*>& load)
	{
		RBXASSERT(getTypedLower<Primitive>());
		SpanningEdge* edge = getTypedLower<Primitive>()->getEdgeToParent();
		RBXASSERT(edge);

		Joint* j = rbx_static_cast<Joint*>(edge);

		if (Joint::isMotorJoint(j)) {
			load.append(j);
		}
		for (int i = 0; i < numChildren(); ++i) {
			getTypedChild<Clump>(i)->loadMotors(load);
		}
	}

} // namespace RBX
