#pragma once

// Network/Peer.h -- October 2013, partial: the base of Client and Server.
// Layout proven by Peer::Peer (PLAYER 0x005b9ac0) and Peer::~Peer (0x005b8bf0): Instance (0x60),
// RakNet::PluginInterface2 at 0x60 (its vftable 0x00b1f284, 17 slots), then the 2016 members in
// 2016 order. RTTI: Described<Peer, sPeer, Instance, 27 (PERSISTENT), 0>. The Instance-side
// vftable has 29 slots (Instance's 28 + onCreateRakPeer).
// No 2013 assert names Peer.h; the path is 2016's.

#include "V8Tree/Instance.h"
#include "PluginInterface2.h"
#include "Rand.h"
#include "rbx/RunningAverage.h"

namespace RBX {
namespace Network {

	class ConcurrentRakPeer;
	class PacketReceiveJob;

	// Client and Server descend from this class
	extern const char* const sPeer;

	// VTABLE: PLAYER 0x00b1f2f4
	// VTABLE: STUDIO 0x00bd5074
	// SIZE 0xac0 (8-aligned: Client::Client 0x005b0880 stores its first member at 0xac0)
	class Peer
		: public Reflection::Described<Peer, sPeer, Instance>
		, public RakNet::PluginInterface2
	{
	private:
		typedef Reflection::Described<Peer, sPeer, Instance> Super;
		shared_ptr<PacketReceiveJob> receiveJob;    // 0x6c
		static unsigned char aesKey[16];            // PLAYER 0x00ddf950
		RakNet::RakNetRandom rnr;                   // 0x74

	public:
		RunningAverageDutyCycle<> rakDutyCycle;     // 0xa40
		boost::shared_ptr<ConcurrentRakPeer> rakPeer; // 0xab0

	protected:
		int protocolVersion;                        // 0xab8 (18 in this build)

		Peer();
		~Peer();

		// Overrides and the one new virtual, vftable 0x00b1f2f4 (STUDIO 0x00bd5074).
		/*override*/ void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider); // +0x20
		/*override*/ bool askAddChild(const Instance* instance) const;                                    // +0x3c
		virtual void onCreateRakPeer();                                                                   // +0x70
	};

} }
