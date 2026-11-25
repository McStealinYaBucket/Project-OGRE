/* Copyright 2003-2006 ROBLOX Corporation, All Rights Reserved */

// Network/Peer.cpp -- October 2013, partial. One assert (line 238, Peer::onServiceProvider
// 0x005ba1d0, not written yet). VC9 emits ~Peer (0x005b8bf0) well before Peer() (0x005b9ac0).

#include "Peer.h"
#include "ConcurrentRakPeer.h"
#include "Replicator.h"
#include "RakPeerInterface.h"
#include "decomp.h"

namespace RBX {
namespace Network {

const char* const sPeer = "NetworkPeer";

unsigned char Peer::aesKey[16];

// GLOBAL: PLAYER 0x00d25280
// GLOBAL: STUDIO 0x00e9fa98
static double lerp = 0.05;

DECOMP_SIZE_ASSERT(Peer, 0xac0);

// The peer's own GUID seeds the random generator; occasional pings on.
// FUNCTION: PLAYER 0x005b8910
// FUNCTION: STUDIO 0x0064a880
void Peer::onCreateRakPeer()
{
	RakNet::RakNetGUID guid = rakPeer->rawPeer()->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS);
	rnr.SeedMT((unsigned int)((guid.g >> 32) ^ guid.g));
	rakPeer->rawPeer()->SetOccasionalPing(true);
}

// STUB: PLAYER 0x005b8bf0
// STUB: STUDIO 0x0064ab60
Peer::~Peer()
{
}

// FUNCTION: PLAYER 0x005b9960
// FUNCTION: STUDIO 0x0064b5b0
bool Peer::askAddChild(const Instance* instance) const
{
	return Instance::fastDynamicCast<Replicator>(instance) != NULL;
}

// FUNCTION: PLAYER 0x005b9ac0
// FUNCTION: STUDIO 0x0064b710
Peer::Peer()
	:rakDutyCycle(lerp)
{
	for (int i = 0; i < 16; ++i)
		aesKey[i] = 0xFE ^ 7*i;

	protocolVersion = 18;
}

// Removes the old StatsService "Network" item and the PacketReceiveJob, drops the RakPeer; on a new
// provider builds ProfiledRakPeer (0x5f0) in a ConcurrentRakPeer, calls onCreateRakPeer, adds a
// PacketReceiveJob (0x230) and a PeerStatsItem named "Network" (assert line 238).
// STUB: PLAYER 0x005ba1d0
// STUB: STUDIO 0x0064be30
void Peer::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
{
	STUB(0x005ba1d0);
}

} }

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
// Peer is registered in Network/API.cpp (2013 API.obj carries its classDescriptor copy).
}
