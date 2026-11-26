#pragma once

// Network/Replicator.ItemSender.h -- October 2013. Batches replication items into one ID_DATA
// BitStream per packet. Layout from the ctor PLAYER 0x005da3f0 / STUDIO 0x00672030; names from iOS
// 10-29 (ItemSender(Replicator&, ConcurrentRakPeer*), openPacket, closePacket, send(Item&),
// getNumberOfBytesUsed) and 2016 for the members. 2013 orders the .obj getNumberOfBytesUsed, ctor,
// closePacket, dtor, openPacket, send.

#include "Replicator.h"
#include "BitStream.h"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace RBX {
namespace Network {

class Item;

// SIZE 0x1c
class Replicator::ItemSender : boost::noncopyable
{
	Replicator& replicator;                       // 0x00
	ConcurrentRakPeer* rakPeer;                   // 0x04
	boost::shared_ptr<RakNet::BitStream> bitStream; // 0x08
	PacketPriority packetPriority;                // 0x10 NetworkSettings::getDataSendPriority()
	const unsigned int maxStreamSize;             // 0x14 Replicator::getAdjustedMtuSize()

	void openPacket();
	void closePacket();

public:
	bool sentItems;                               // 0x18

	ItemSender(Replicator& replicator, ConcurrentRakPeer* rakPeer);
	~ItemSender();

	enum SendStatus
	{
		SEND_BITSTREAM_FULL = 0,
		SEND_OK = 1,
	};

	SendStatus send(Item& item);
	int getNumberOfBytesUsed() const;
};

} // namespace Network
} // namespace RBX
