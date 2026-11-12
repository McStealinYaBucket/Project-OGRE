/* App/v8world/ContactManagerSpatialHash.cpp -- October 2013
 * RBX::ContactManagerSpatialHash (thin subclass over the SpatialHash
 * template instantiation).
 *
 * 0x56CA96 (44B): forwards (World*, ContactManager*, 100) to
 * SpatialHash<Primitive, Contact, ContactManager, 4>::SpatialHash.  The 100
 * is maxNodes (computeLevel scales its volume threshold by it).
 *
 * Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/ContactManagerSpatialHash.cpp
 * (path inferred). */

#include "stdafx.h"

#include "V8World/ContactManagerSpatialHash.h"

namespace RBX {

	ContactManagerSpatialHash::ContactManagerSpatialHash(World* world,
		ContactManager* contactManager)
		: SpatialHash<Primitive, Contact, ContactManager,
			CONTACTMANAGER_MAXLEVELS>(world, contactManager, 100)
	{
	}

} // namespace RBX
