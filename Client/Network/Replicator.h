#pragma once

// Network/Replicator.h -- October 2013, partial: only the nested classes the item files declare.
// Replicator's members are not reconstructed yet (ctor PLAYER 0x005a5e70, dtor 0x005a55a0,
// sizeof unproven), so this declares its base chain only, no members and no SIZE.

#include "V8DataModel/DataModelJob.h"
#include "V8Tree/Instance.h"
#include "Streaming.h"

namespace RBX {
namespace Network {

// IdSerializer (Replicator's base) is declared in Streaming.h.

extern const char* const sReplicator;

// The base of Replicator's jobs (RTTI: ReplicatorJob : DataModelJob). Its members (2016: a
// weak_ptr<Replicator>) are not proven yet; the ctor (PingJob() PLAYER 0x00598c90) is not written.
class ReplicatorJob : public DataModelJob
{
};

// RTTI: DescribedNonCreatable<Replicator, IdSerializer, sReplicator, 1 (INTERNAL_LOCAL)> (classDescriptor PLAYER
// 0x00525e20), then RakNet::PluginInterface2 at 0x1094, IMetric 0x10a0, CellChangeListener 0x10a4
// (ctor 0x005a5e70, dtor 0x005a55a0). Those bases and the members are not declared yet.
class Replicator : public DescribedNonCreatable<Replicator, IdSerializer, sReplicator, Reflection::ClassDescriptor::INTERNAL_LOCAL>
{
public:
	class PingJob;
	class PingBackItem;
	class PingItem;
	class MarkerItem;
};

} // namespace Network
} // namespace RBX
