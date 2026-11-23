/* V8World/ContactManagerSpatialHash.h -- October 2013 reconstruction.
   ContactManager's broadphase: SpatialHash<Primitive, Contact,
   ContactManager, 4>. Only the constructor is its own; the
   (World*, ContactManager*) signature is iOS 10-29's
   RBX::ContactManagerSpatialHash::ContactManagerSpatialHash. */
#pragma once

#include "V8World/SpatialHashMultiRes.h"

namespace RBX {

class World;
class Primitive;
class Contact;
class ContactManager;
class Assembly;

// Number of grid resolutions. Kept as a macro: callers size arrays with it.
#define CONTACTMANAGER_MAXLEVELS 4

class ContactManagerSpatialHash
	: public SpatialHash<Primitive, Contact, ContactManager, CONTACTMANAGER_MAXLEVELS>
{
public:
	ContactManagerSpatialHash(World* world, ContactManager* contactManager);
};

} // namespace RBX
