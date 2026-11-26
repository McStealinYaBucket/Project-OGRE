// Network/Replicator.ItemSender.cpp -- October 2013 (see Replicator.ItemSender.h). No 2013 __FILE__
// names this file; the functions sit together after RakNet's StatisticsToString (PLAYER 0x005d9de0).

#include "Replicator.ItemSender.h"
#include "ConcurrentRakPeer.h"
#include "Item.h"
#include "V8DataModel/NetworkSettings.h"

DECOMP_SIZE_ASSERT(RBX::Network::Replicator::ItemSender, 0x1c);

namespace RBX {
namespace Network {

namespace {
	// 2016 PacketIds.h ID_DATA; 0x83 in 2013 (openPacket 0x005da5fa).
	const unsigned char ID_DATA = 0x83;
}

// FUNCTION: PLAYER 0x005da3d0
// FUNCTION: STUDIO 0x00672010
int Replicator::ItemSender::getNumberOfBytesUsed() const
{
	if (bitStream)
		return bitStream->GetNumberOfBytesUsed();
	return 0;
}

// FUNCTION: PLAYER 0x005da3f0
// FUNCTION: STUDIO 0x00672030
Replicator::ItemSender::ItemSender(Replicator& replicator, ConcurrentRakPeer* rakPeer)
	: replicator(replicator)
	, rakPeer(rakPeer)
	, packetPriority((PacketPriority)replicator.settings().getDataSendPriority())
	, maxStreamSize(replicator.getAdjustedMtuSize())
	, sentItems(false)
{
}

// RELIABLE_ORDERED on channel 0, to the remote peer, then two ReplicatorStats samples.
// FUNCTION: PLAYER 0x005da460
// FUNCTION: STUDIO 0x006720a0
void Replicator::ItemSender::closePacket()
{
	if (bitStream)
	{
		const int bytes = bitStream->GetNumberOfBytesUsed();
		if (bytes > 0)
		{
			Item::writeItemType(*bitStream, Item::ItemTypeEnd);
			rakPeer->Send(bitStream, packetPriority, RELIABLE_ORDERED, 0, replicator.remotePlayerId, false);
			replicator.replicatorStats.dataPacketsSent.sample();
			replicator.replicatorStats.dataPacketsSentSize.sample(bytes);
		}
		bitStream.reset();
	}
}

// FUNCTION: PLAYER 0x005da520
// FUNCTION: STUDIO 0x00672160
Replicator::ItemSender::~ItemSender()
{
	closePacket();
}

// FUNCTION: PLAYER 0x005da5a0
// FUNCTION: STUDIO 0x006721e0
void Replicator::ItemSender::openPacket()
{
	if (!bitStream)
	{
		bitStream.reset(new RakNet::BitStream());
		*bitStream << ID_DATA;
	}
}

// FUNCTION: PLAYER 0x005da620
// FUNCTION: STUDIO 0x00672260
Replicator::ItemSender::SendStatus Replicator::ItemSender::send(Item& item)
{
	if (bitStream && bitStream->GetNumberOfBytesUsed() >= maxStreamSize)
		return SEND_BITSTREAM_FULL;

	openPacket();

	if (!item.write(*bitStream))
		return SEND_BITSTREAM_FULL;

	sentItems = true;
	return SEND_OK;
}

} // namespace Network
} // namespace RBX
