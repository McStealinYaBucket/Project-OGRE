/* App/v8world/ContactManager.cpp -- October 2013 (ctor/dtor shell).
   Bodies are stubs: the sizes, the (World*) ctor signature and the World+184
   raw-pointer wiring are binary-proven; the SpatialHash / pair-callback work
   is not decompiled yet (see include/V8World/ContactManager.h). */

#include "stdafx.h"

#include "V8World/ContactManager.h"
#include "V8World/ContactManagerSpatialHash.h"
#include "V8World/Primitive.h"
#include "V8World/Contact.h"
#include "V8World/World.h"
#include "Util/Extents.h"
#include "Util/FastFlags.h"
#include "G3D/Array.h"
#include "rbx/Debug.h"

namespace RBX {

	// mac: 0x563FAC (62B) -- defined below; used by getPrimitivesTouchingExtents.
	static void appendIfTouching(Primitive* prim, const Extents& extents,
		G3D::Array<Primitive*, 10, 32>& out);

	// mac: 0x563626 (C1 0x56361C is the 9-byte C1 thunk).
	// STUB: PLAYER 0x007b63a0
	ContactManager::ContactManager(World* world)
		: spatialHash(new ContactManagerSpatialHash(world, this))
		, myMegaClusterPrim(NULL)
		, world(world)
		, byte52(0)
		, profiler(NULL)
	{
		// scratch (11 buckets) + updatedTerrainRegions (11 buckets) default-
		// construct empty; the binary sizes them at 11 up front (a pure
		// capacity choice -- iteration order is address-hashed either way,
		// so no test can observe the difference).
		// The +80 profiler (new 0xA01C CodeProfiler, unported type) stays
		// NULL -- nothing in any ported body reads it.
	}

	// mac: 0x563774: vptr reset (implicit), ~SpatialHash + delete spatialHash,
	// profiler via vtab, delete both sets, ~ConcurrencyValidator.  Ours:
	// members destroy themselves; the hash is explicitly deleted.
	ContactManager::~ContactManager()
	{
		delete spatialHash;
	}

	// mac: 0x565536 (190B).  MEGACLUSTER prims (type 10) land in myMegaClusterPrim
	// (+20); the terrain FastLog line + Voxel::Grid::connectListener hookup
	// are unported (voxel/log modules -- only reachable for megacluster
	// prims, which no green path creates).  Every prim then enters the hash
	// with createContacts = (FFlag::DeferredContacts == 0).
	// STUB: PLAYER 0x007bc270
	// STUB: STUDIO 0x008d8c10
	void ContactManager::onPrimitiveAdded(Primitive* prim)
	{
		if (prim->getGeometryType() == Primitive::GEOMETRY_MEGACLUSTER)
		{
			myMegaClusterPrim = prim;
			// DELTA: FastLog terrain line + Voxel::Grid::connectListener.
		}
		spatialHash->onPrimitiveAdded(prim, FFlag::DeferredContacts == 0);
	}

	// mac: 0x565616 (186B).
	// STUB: PLAYER 0x007bc2f0
	// STUB: STUDIO 0x008d8ce0
	void ContactManager::onPrimitiveRemoved(Primitive* prim)
	{
		if (prim->getGeometryType() == Primitive::GEOMETRY_MEGACLUSTER)
		{
			// DELTA: FastLog terrain line + Voxel::Grid::disconnectListener
			// (same gate/modules as onPrimitiveAdded).
			myMegaClusterPrim = NULL;
		}
		spatialHash->primitiveRemoved(prim);
	}

	void ContactManager::updatePrimitiveForTest(Primitive* prim)
	{
		spatialHash->primitiveExtentsChanged(prim, prim->getFastFuzzyExtents());
	}

	// mac: 0x5656D0 (32B).
	void ContactManager::onPrimitiveExtentsChanged(Primitive* prim)
	{
		spatialHash->onPrimitiveExtentsChanged(prim);
	}

	// mac: 0x563F30 (124B).  Scratch-clears, fills via the hash walk (maxCount 0
	// = unlimited), then re-filters every scratch member by fuzzy-extents
	// overlap into out.  The second loop walks scratch's own bucket chain in
	// the binary; public iterators follow the same chain in the same order
	// (see the LAYOUT_AUDIT note with the boost table proof).
	void ContactManager::getPrimitivesTouchingExtents(const Extents& extents,
		const Primitive* ignorePrim, int maxCount,
		G3D::Array<Primitive*, 10, 32>& out)
	{
		scratch.clear();
		spatialHash->getPrimitivesTouchingGrids(extents, ignorePrim,
			(unsigned long)maxCount, scratch);
		for (boost::unordered_set<Primitive*>::iterator it = scratch.begin();
			it != scratch.end();
			++it)
		{
			appendIfTouching(*it, extents, out);
		}
	}

	// mac: 0x563ABE (823B).  The MEGACLUSTER assert is ContactManager.cpp:534 in
	// the binary; the "0" asserts are :558 ( Ball-family fallthrough) and
	// :573 (Block-family fallthrough).  Ordering by collide type first (v6
	// holds the lower, v3 the higher) makes (2,1)-style inversions
	// unreachable -- the switch only ever sees t0 <= t1.
	// STUB: PLAYER 0x007b9120
	// STUB: STUDIO 0x008d5ac0
	Contact* ContactManager::createContact(Primitive* p0, Primitive* p1)
	{
		// ContactManager.cpp:534.
		RBXASSERT(p0->getGeometryType() != Primitive::GEOMETRY_MEGACLUSTER
			&& p1->getGeometryType() != Primitive::GEOMETRY_MEGACLUSTER);

		Primitive* a = p0;
		Primitive* b = p1;
		if (p0->getCollideType() > p1->getCollideType())
		{
			a = p1;
			b = p0;
		}
		int t0 = a->getCollideType();
		int t1 = b->getCollideType();
		if (t0 == 3)
			return new PolyPolyContact(a, b);
		if (t0 == 2)
		{
			if (t1 == 3)
				return new PolyPolyContact(a, b);
			if (t1 == 2)
				return new BlockBlockContact(a, b);
			RBXASSERT(false);  // :573 "0"
			return NULL;
		}
		if (t0 == 1)
		{
			if (t1 == 3)
				return new BallPolyContact(a, b);
			if (t1 == 2)
				return new BallBlockContact(a, b);
			if (t1 == 1)
				return new BallBallContact(a, b);
			RBXASSERT(false);  // :558 "0"
			return NULL;
		}
		RBXCRASH();
		return NULL;
	}

	// mac: 0x56539A (185B).  Verbatim for the shipped (flag-off) path.
	// DELTA: the flag-on path calls Assembly::getConstPrimitiveAssembly,
	// Assembly::computeIsGrounded, Mechanism::getConstMovingAssemblyRoot and
	// World::getSpatialFilter -- none of those types exist in the tree
	// (Assembly/Mechanism are whole unported modules), so with the flag on
	// this correctly does nothing yet.  The flag defaults off (see
	// FastFlags.cpp), matching the shipped 2013 client.
	void ContactManager::onNewPair(Primitive* p0, Primitive* p1)
	{
		if (!FFlag::DeferredContacts)
		{
			Contact* contact = createContact(p0, p1);
			world->insertContact(contact);
			return;
		}
		// Flag-on Assembly/Mechanism grounded dance: unported (see above).
	}

	// mac: 0x565454 (226B).
	// STUB: PLAYER 0x007b35f0
	// STUB: STUDIO 0x008cf670
	void ContactManager::releasePair(Primitive* p0, Primitive* p1)
	{
		if (p0 == NULL || p1 == NULL)
			return;
		if (p0->getGeometryType() == Primitive::GEOMETRY_MEGACLUSTER
			|| p1->getGeometryType() == Primitive::GEOMETRY_MEGACLUSTER)
		{
			Contact* contact;
			while ((contact = Primitive::getContact(p0, p1)) != NULL)
				world->destroyContact(contact);
			return;
		}
		Contact* contact = Primitive::getContact(p0, p1);
		if (contact != NULL)
		{
			world->destroyContact(contact);
			return;
		}
		RBXASSERT(false);  // ContactManager.cpp:637 "0"
	}

	// mac: 0x563FAC (62B).  File-static in the binary (the symbol is literally
	// `sub_563FAC`), so the NAME is invented and the address is the identity.
	// The broadphase overlap filter used by getPrimitivesTouchingExtents'
	// MegaCluster path (0x563F93): append the candidate when the query
	// extents overlap-or-touch its cached fuzzy extents, else skip it.
	// G3D::Array<Primitive*,10,32>::append takes `const T&`, so the local
	// copy (var_4 in the disasm) is exactly what the binary builds.
	static void appendIfTouching(
		Primitive* prim,
		const Extents& extents,
		G3D::Array<Primitive*, 10, 32>& out)
	{
		if (extents.overlapsOrTouches(prim->getFastFuzzyExtents()))
			out.append(prim);
	}

} // namespace RBX
