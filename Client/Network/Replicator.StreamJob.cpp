// Network/Replicator.StreamJob.cpp -- October 2013, partial (see Replicator.StreamJob.h).

#include "Replicator.StreamJob.h"
#include "decomp.h"

namespace RBX {
namespace Network {

// STUB: PLAYER 0x005cff10
// STUB: STUDIO 0x0066e9f0
void Replicator::StreamJob::sendPackets(int maxPackets)
{
	STUB(0x005cff10);
}

// STUB: PLAYER 0x005d1310
// STUB: STUDIO 0x0066fb00
void Replicator::StreamJob::updateClientQuota(int diff, short maxRegionRadius)
{
	STUB(0x005d1310);
}

// STUB: PLAYER 0x005d25f0
// STUB: STUDIO 0x00670da0
__declspec(noinline) void Replicator::StreamJob::receiveInstanceGcMessage(const Guid::Data& id)
{
	STUB(0x005d25f0);
	RBX::Name::getNullName(); // stand-in cross-TU call: a same-TU STUB that clobbers nothing changes the caller's registers
}

// STUB: PLAYER 0x005d2860
// STUB: STUDIO 0x00671010
void Replicator::StreamJob::readRegionRemoval(RakNet::BitStream& bitStream)
{
	STUB(0x005d2860);
}

// 94.1%: our Guid::Data local runs Guid::Scope's default ctor (name = &Name::getNullName()); 2013's (and
// iOS's) local is not initialized, so 2013's Scope has no initializing default ctor (App's Util/Guid.h).
// STUB: PLAYER 0x005d29f0
// STUB: STUDIO 0x006711a0
void Replicator::StreamJob::readInstanceRemoval(RakNet::BitStream& bitStream)
{
	Guid::Data id;
	replicator->deserializeId(bitStream, id);
	receiveInstanceGcMessage(id);
}

} // namespace Network
} // namespace RBX
