#pragma once

// Network/Replicator.ProcessPacketsJob.h -- October 2013, partial: the base chain only, so
// Replicator::getMetricValue (PLAYER 0x00581d50) can read its duty cycle. Members, ctor and
// virtuals are not written.

#include "Replicator.h"

namespace RBX {
namespace Network {

class Replicator::ProcessPacketsJob : public ReplicatorJob
{
};

} // namespace Network
} // namespace RBX
