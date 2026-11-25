#pragma once

// Network/ServerReplicator.h -- October 2013, partial: the base chain only (RTTI
// DescribedNonCreatable<ServerReplicator, Replicator, sServerReplicator>, vftable 0x00b1848c,
// classDescriptor PLAYER 0x00525ea0). Members not reconstructed yet.

#include "Replicator.h"

namespace RBX {
namespace Network {

extern const char* const sServerReplicator;

class ServerReplicator : public DescribedNonCreatable<ServerReplicator, Replicator, sServerReplicator, Reflection::ClassDescriptor::INTERNAL_LOCAL>
{
};

} // namespace Network
} // namespace RBX
