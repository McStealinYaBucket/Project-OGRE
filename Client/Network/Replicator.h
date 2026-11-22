#pragma once

// Network/Replicator.h -- October 2013, partial: only the nested classes the item files declare.
// Replicator's members are not reconstructed yet (ctor PLAYER 0x005a5e70, dtor 0x005a55a0,
// sizeof unproven), so this declares no members and no SIZE.

#include "V8DataModel/DataModelJob.h"

namespace RBX {
namespace Network {

// The base of Replicator's jobs (RTTI: ReplicatorJob : DataModelJob). Its members (2016: a
// weak_ptr<Replicator>) are not proven yet; the ctor (PingJob() PLAYER 0x00598c90) is not written.
class ReplicatorJob : public DataModelJob
{
};

class Replicator
{
public:
	class PingJob;
	class PingBackItem;
	class PingItem;
	class MarkerItem;
};

} // namespace Network
} // namespace RBX
