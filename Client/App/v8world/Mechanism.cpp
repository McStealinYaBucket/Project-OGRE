/* App/v8world/Mechanism.cpp -- October 2013 (lookups: mac 0x57DDB4 0x57E0BA 0x57E0EC 0x57DF74 0x57DFAC). */

#include "stdafx.h"

#include "V8World/Mechanism.h"
#include "V8World/Assembly.h"
#include "V8World/Primitive.h"

namespace RBX {

	Mechanism::Mechanism()
	{
	}

	Mechanism* Mechanism::getPrimitiveMechanism(Primitive* prim)
	{
		if (prim == NULL)
			return NULL;
		Assembly* assembly = Assembly::getPrimitiveAssembly(prim);
		if (assembly == NULL)
			return NULL;
		IndexedMesh* computed = assembly->getMesh()->getComputedUpper();
		if (computed == NULL)
			return NULL;
		return reinterpret_cast<Mechanism*>(reinterpret_cast<char*>(computed) - 8);
	}

	const Mechanism* Mechanism::getConstPrimitiveMechanism(const Primitive* prim)
	{
		if (prim == NULL)
			return NULL;
		const Assembly* assembly = Assembly::getConstPrimitiveAssembly(prim);
		if (assembly == NULL)
			return NULL;
		const IndexedMesh* computed = assembly->getMesh()->getConstComputedUpper();
		if (computed == NULL)
			return NULL;
		return reinterpret_cast<const Mechanism*>(reinterpret_cast<const char*>(computed) - 8);
	}

	bool Mechanism::isMovingAssemblyRoot(const Assembly* assembly)
	{
		if (assembly->computeIsGrounded())
			return false;
		const IndexedMesh* meshParent = assembly->getMesh()->getConstTypedParent<IndexedMesh>();
		if (meshParent == NULL)
			return true;
		const Assembly* parent = reinterpret_cast<const Assembly*>(reinterpret_cast<const char*>(meshParent) - 8);
		return parent == NULL || parent->computeIsGrounded();
	}

	Assembly* Mechanism::getMovingAssemblyRoot(Assembly* assembly)
	{
		Assembly* current = assembly;
		IndexedMesh* up = current->getMesh()->getTypedParent<IndexedMesh>();
		while (up != NULL)
		{
			Assembly* parent = reinterpret_cast<Assembly*>(reinterpret_cast<char*>(up) - 8);
			if (parent == NULL || parent->computeIsGrounded())
				break;
			current = parent;
			up = current->getMesh()->getTypedParent<IndexedMesh>();
		}
		return current;
	}

	const Assembly* Mechanism::getConstMovingAssemblyRoot(const Assembly* assembly)
	{
		const Assembly* current = assembly;
		const IndexedMesh* up = current->getMesh()->getConstTypedParent<IndexedMesh>();
		while (up != NULL)
		{
			const Assembly* parent = reinterpret_cast<const Assembly*>(reinterpret_cast<const char*>(up) - 8);
			if (parent == NULL || parent->computeIsGrounded())
				break;
			current = parent;
			up = current->getMesh()->getConstTypedParent<IndexedMesh>();
		}
		return current;
	}


	// FUNCTION: PLAYER 0x007f9f90
	// FUNCTION: STUDIO 0x00915520
	const Primitive* Mechanism::getConstRootMovingPrimitive(const Primitive* p)
	{
		const Assembly* a = p->getConstAssembly();
		RBXASSERT(a);
		if (!a)
			return NULL;
		if (a->computeIsGrounded())
			return NULL;
		return getConstMovingAssemblyRoot(a)->getConstAssemblyPrimitive();
	}

	// FUNCTION: PLAYER 0x007fa010
	// FUNCTION: STUDIO 0x009155a0
	Primitive* Mechanism::getRootMovingPrimitive(Primitive* p)
	{
		return const_cast<Primitive*>(getConstRootMovingPrimitive(p));
	}



















	// mac: 0x57E11E (batch281: lower-mesh -8 container + :127 answer assert).
	// FUNCTION: PLAYER 0x007fa020
	// FUNCTION: STUDIO 0x009155b0
	Assembly* Mechanism::getRootAssembly()
	{
		Assembly* answer = getMesh()->getTypedLower<Assembly>();
		RBXASSERT(answer);
		return answer;
	}

	// mac: 0x57DEBE (28B) __ZNK3RBX9Mechanism26getConstMechanismPrimitiveEv:
	//   *(void**)  (this+0x28) -> +0x20 -> +0x20, test only the final
	//   pointer, then the -8 owner/vptr strip (same shape as the
	//   computedUpper/Assembly walks above; cf. :1258 in
	//   PartInstance.cpp which compares this against the mechanism-root
	//   primitive).
	// DELTA: the binary trusts the two front hops (raw 0x57DEC4/0x57DEC7
	// have no null branch) but the chain storage is untyped/unpoured in
	// this shell, so guarded here (weaker-than-binary behavior noted;
	// the null mechanism case is already excluded by both landed
	// callers -- getRenderingCoordinateFrame and the :1257 assert arm).
	Primitive const* Mechanism::getConstMechanismPrimitive() const
	{
		Primitive const* result = NULL;
		void* chain = *(void**)((char*)this + 0x28);
		void* hop2 = chain != NULL ? *(void**)((char*)chain + 0x20) : NULL;
		void* hop3 = hop2 != NULL ? *(void**)((char*)hop2 + 0x20) : NULL;
		if (hop3 != NULL)
			result = reinterpret_cast<Primitive const*>(
				reinterpret_cast<char*>(hop3) - 8);
		return result;
	}

} // namespace RBX
