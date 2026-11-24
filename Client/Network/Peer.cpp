/* Copyright 2003-2006 ROBLOX Corporation, All Rights Reserved */

// Network/Peer.cpp -- October 2013, partial. One assert (line 238, Peer::onServiceProvider
// 0x005ba1d0, not written yet). VC9 emits ~Peer (0x005b8bf0) well before Peer() (0x005b9ac0).

#include "Peer.h"
#include "decomp.h"

namespace RBX {
namespace Network {

const char* const sPeer = "NetworkPeer";

unsigned char Peer::aesKey[16];

// GLOBAL: PLAYER 0x00d25280
// GLOBAL: STUDIO 0x00e9fa98
static double lerp = 0.05;

DECOMP_SIZE_ASSERT(Peer, 0xac0);

// STUB: PLAYER 0x005b8bf0
// STUB: STUDIO 0x0064ab60
Peer::~Peer()
{
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

} }

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
// Peer is registered in Network/API.cpp (2013 API.obj carries its classDescriptor copy).
}
