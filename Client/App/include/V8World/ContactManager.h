/* V8World/ContactManager.h -- October 2013 (broadphase shell).
   Evidence: ContactManager D2 0x563774, C1 0x56361C, C2 0x563626 (size 0x54 =
     84B).  World C2 0x5A84A1 does
         v8 = operator new(0x54);
         RBX::ContactManager::ContactManager(v8, this);
         world->contactManager = v8;             // World+184 (raw pointer)
   so the ctor takes the owning World*.  Layout from C2/D2:
     +0  vptr (`vtable for'RBX::ContactManager @ 0xEEE240)
     +4  ConcurrencyValidator (12B)
     +16 ContactManagerSpatialHash* spatialHash = new (0xB0=176)
     +20 Primitive* myMegaClusterPrim = NULL
     +24 World* world
     +28 boost::unordered_set<Primitive*> scratch (cleared per query)
     +56 boost::unordered_set<SpatialRegion::Id> updatedTerrainRegions
     +80 CodeProfiler* profilingBroadphase = new (0xA01C) "Broadphase"
     D2 0x563774: vptr reset, ~SpatialHash + delete spatialHash, profiler via
     vtab[1], delete both sets, ~ConcurrencyValidator.
   Non-reflected (no Described hits).
   TODO: full decompile (getPrimitivesTouchingExtents 0x563F30, createContact
     0x563ABE, onNewPair 0x56539A / releasePair 0x565454, the SpatialHash
     itself -- see handoffsandmemories/QUEUE.md).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/ContactManager.h */

#pragma once

#include <boost/unordered_set.hpp>

#include "Util/Extents.h"
#include "Util/SpatialRegion.h"
#include "G3D/Array.h"

namespace RBX {

	class World;
	class Primitive;
	class Contact;
	class ContactManagerSpatialHash;

	class ContactManager
	{
	public:
		explicit ContactManager(World* world);
		~ContactManager();

		World* getWorld() const { return world; }

		// mac: 0x563ABE (823B).  Factory selecting the concrete Contact by
		// collide-type pair (lower first): (3,x)->PolyPoly, (2,3)->PolyPoly,
		// (2,2)->BlockBlock, (1,3)->BallPoly, (1,2)->BallBlock,
		// (1,1)->BallBall; anything else -> RBXASSERT("0") + NULL, unknown
		// kind -> RBXCRASH.  Asserts non-MEGACLUSTER (ContactManager.cpp:534).
		// See v8world/ContactManager.cpp.
		Contact* createContact(Primitive* p0, Primitive* p1);

		// mac: 0x56539A (185B).  With FFlag::DeferredContacts off (the shipped
		// default): createContact + World::insertContact, done.  The flag-on
		// Assembly/Mechanism grounded dance is unported (types unknown) --
		// documented DELTA in the body.
		void onNewPair(Primitive* p0, Primitive* p1);

		// mac: 0x565454 (226B).  Null-guard both prims; MEGACLUSTER pairs loop
		// destroyContact until getContact goes dry, normal pairs destroy
		// once (else RBXASSERT "0" :637).  destroyContact notifies + counts
		// (World 0x5AB2B8); nothing here unlinks or deletes -- same as the
		// binary.  OPEN (queued, not assumed): the mega loop terminates
		// only if destroyContact (or the stage's onEdgeRemoving) unlinks
		// from prim EdgeLists; ours doesn't yet, so mega pairs would spin.
		// No green path creates megacluster prims -- documented, revisit
		// with CleanStage::onEdgeRemoving's body.
		void releasePair(Primitive* p0, Primitive* p1);

		// mac: 0x565536 (190B).  MEGACLUSTER prims (type 10) are stored to
		// myMegaClusterPrim (+20) with terrain logging + a voxel-grid
		// listener hookup (both unported -- voxel/log modules); every prim
		// then goes to spatialHash->onPrimitiveAdded(prim,
		// FFlag::DeferredContacts == 0), i.e. insertions create contacts.
		void onPrimitiveAdded(Primitive* prim);

		// mac: 0x565616 (186B).  MEGACLUSTER prims clear myMegaClusterPrim (+20)
		// with the voxel disconnect (unported, same gate as above); every
		// prim then leaves the hash.  See ContactManager.cpp.
		void onPrimitiveRemoved(Primitive* prim);

		// mac: 0x5656D0 (32B): forwards to the hash production entry (re-level
		// wrapper).  The voxel hook inside is a DELTA (see the .inl).
		void onPrimitiveExtentsChanged(Primitive* prim);

		// Bring-up helper (not in binary): drive a hash cell update for a
		// moved/resized prim directly.  Retained for white-box moves tests;
		// the production route above covers the wrapper since session 150.
		void updatePrimitiveForTest(Primitive* prim);

		// mac: 0x563F30 (124B).  Clear scratch, fill it via the hash walk
		// (maxCount 0 = unlimited), then re-filter every scratch member by
		// fuzzy-extents overlap into out (same order as the bucket chain --
		// portable via public iterators, see the header note).
		void getPrimitivesTouchingExtents(const Extents& extents,
			const Primitive* ignorePrim, int maxCount,
			G3D::Array<Primitive*, 10, 32>& out);

	private:
		ContactManager(const ContactManager&);
		ContactManager& operator=(const ContactManager&);

		// Binary order (ContactManager C2 0x563626):
		// +16 spatialHash (new 0xB0 ContactManagerSpatialHash -- needs the
		//   complete type, included by the .cpp, not here),
		// +20 myMegaClusterPrim (NULL init),
		// +24 world,
		// +28 scratch (unordered_set<Primitive*>, 11 buckets),
		// +52 byte (0), +56 updatedTerrainRegions, +80 profiler (0xA01C).
		// The profiler has no in-tree type -- void*, never constructed
		// (nothing reads it in any ported body).
		ContactManagerSpatialHash* spatialHash;
		Primitive* myMegaClusterPrim;
		World* world;
		boost::unordered_set<Primitive*> scratch;
		unsigned char byte52;
		boost::unordered_set<SpatialRegion::Id> updatedTerrainRegions;
		void* profiler;
	};

} // namespace RBX
