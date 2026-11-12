/* App/v8world/Mechanism.cpp -- October 2013 (C2 + null-safe lookups).
   Bodies: C2 0x57DDB4 / getPrimitiveMechanism 0x57E0BA /
     getConstPrimitiveMechanism 0x57E0EC (batch202) / isMovingAssemblyRoot
     0x57DF74 / getMovingAssemblyRoot 0x57DFAC (batch269). */

#include "stdafx.h"

#include "V8World/Mechanism.h"
#include "V8World/Assembly.h"
#include "V8World/Primitive.h"

namespace RBX {

	// mac: 0x57DDB4 (96B: IPipelined C2 zeroes +4/currentStage; mesh C2).
	Mechanism::Mechanism()
	{
	}

	// mac: 0x57E0BA (50B).
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
		return reinterpret_cast<Mechanism*>(
			reinterpret_cast<char*>(computed) - 8);
	}

	// mac: 0x57E0EC (50B, symmetric).
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
		return reinterpret_cast<const Mechanism*>(
			reinterpret_cast<const char*>(computed) - 8);
	}

	// mac: 0x57DF74 (batch269).
	bool Mechanism::isMovingAssemblyRoot(const Assembly* assembly)
	{
		if (assembly->computeIsGrounded())
			return false;
		const IndexedMesh* meshParent = assembly->getMesh()->getConstTypedParent<IndexedMesh>();
		if (meshParent == NULL)
			return true;
		const Assembly* parent = reinterpret_cast<const Assembly*>(
			reinterpret_cast<const char*>(meshParent) - 8);
		return parent == NULL || parent->computeIsGrounded();
	}

	// mac: 0x57DFAC (batch269: climb while the parent link is live and
	// ungrounded; null links and grounded parents stop the walk).
	Assembly* Mechanism::getMovingAssemblyRoot(Assembly* assembly)
	{
		Assembly* current = assembly;
		IndexedMesh* up = current->getMesh()->getTypedParent<IndexedMesh>();
		while (up != NULL)
		{
			Assembly* parent = reinterpret_cast<Assembly*>(
				reinterpret_cast<char*>(up) - 8);
			if (parent == NULL || parent->computeIsGrounded())
				break;
			current = parent;
			up = current->getMesh()->getTypedParent<IndexedMesh>();
		}
		return current;
	}

	// Const mirror (2016 shape).
	const Assembly* Mechanism::getConstMovingAssemblyRoot(const Assembly* assembly)
	{
		const Assembly* current = assembly;
		const IndexedMesh* up = current->getMesh()->getConstTypedParent<IndexedMesh>();
		while (up != NULL)
		{
			const Assembly* parent = reinterpret_cast<const Assembly*>(
				reinterpret_cast<const char*>(up) - 8);
			if (parent == NULL || parent->computeIsGrounded())
				break;
			current = parent;
			up = current->getMesh()->getConstTypedParent<IndexedMesh>();
		}
		return current;
	}

	// mac: 0x57E11E (batch281: lower-mesh -8 container + :127 answer assert).
	// STUB: PLAYER 0x007fa020
	// STUB: STUDIO 0x009155b0
	Assembly* Mechanism::getRootAssembly()
	{
		IndexedMesh* lower = getMesh()->getTypedLower<IndexedMesh>();
		Assembly* answer = NULL;
		if (lower != NULL)
			answer = reinterpret_cast<Assembly*>(
				reinterpret_cast<char*>(lower) - 8);
		RBXASSERT(answer != NULL);  // :127 "answer"
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
