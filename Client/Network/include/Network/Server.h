/* Network/Server.h -- October 2013, partial.
   Evidence: Server::serverIsPresent PLAYER 0x005b34f0 (assert "!testInDatamodel || serviceProvider!=NULL"
     at Server.cpp:96). Layout from Server::Server (PLAYER 0x005b4fd0) and Server::~Server (0x005b32b0),
     neither matched yet: Peer (0xac0; the ctor builds it through its base ctor 0x005b3750, the dtor
     calls Peer::~Peer 0x005b8bf0), Service's bool at 0xac0, then the members below. RTTI:
     DescribedCreatable<Server, Peer, sServer, 1 (INTERNAL_LOCAL), 0>, FactoryProduct<Server, Peer, ...>
     (vftables 0x00b1dfbc..0x00b1e02c). sServer = "NetworkServer" (setName in the ctor).
     Size 0xb78 (operator new in 0x0055b600's creator). 2016 names only where noted. */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "../../Peer.h"
#include "decomp.h"
#include <set>
#include <vector>

namespace RBX {
	class NetworkSettings;

namespace Network {

	extern const char* const sServer;

	// SIZE 0xb78
	class Server
		: public DescribedCreatable<Server, Peer, sServer, Reflection::ClassDescriptor::INTERNAL_LOCAL>
		, public Service
	{
	public:
		Server();
		~Server();

		// mac: 0x87D908 (static 2016 shape carried: provider assert + find).
		static bool serverIsPresent(const Instance* context, bool testInDatamodel = true);

	protected:
		// Overrides, vftable 0x00b1e34c (STUDIO 0x00bd00fc).
		/*override*/ void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider); // +0x20
		/*override*/ bool askAddChild(const Instance* instance) const;                                    // +0x3c
		/*override*/ void onCreateRakPeer();                                                              // +0x70

	public:

		undefined4 m_unk0xac4;                          // 0xac4 ctor 0
		shared_ptr<Instance> m_unk0xac8;                // 0xac8 dtor releases +0xacc last; pointee unproven
		shared_ptr<Instance> m_unk0xad0;                // 0xad0 dtor releases +0xad4; pointee unproven
		rbx::signals::scoped_connection m_unk0xad8;     // 0xad8 dtor 0x00483de0
		bool m_unk0xadc;                                // 0xadc ctor false
		undefined m_unk0xadd[0xae8 - 0xadd];            // 0xadd not stored by the ctor
		int m_unk0xae8;                                 // 0xae8 ctor 1; the ctor reads and increments it
		std::vector<int> m_unk0xaec;                    // 0xaec element type unproven (dtor frees +0xaf0)
		NetworkSettings* networkSettings;               // 0xafc ctor GlobalAdvancedSettingsItem<NetworkSettings>::singleton()
		std::set<int> m_unk0xb00;                       // 0xb00 _Buynode 0x00473930 (folded); key unproven
		std::set<int> m_unk0xb0c;                       // 0xb0c same
		// 0xb18: a boost::multi_index_container (ctor 0x005b36e0 allocates a 0x38-byte header node,
		// index headers at +0x18/+0x34/+0x48 of it). Placeholder bytes until its type is known.
		undefined m_unk0xb18[0xb6c - 0xb18];            // 0xb18
		rbx::signals::signal<void()> m_unk0xb6c;        // 0xb6c ctor 0x005b14d0, dtor 0x005b27e0; signature unproven
		rbx::signals::signal<void()> m_unk0xb70;        // 0xb70 same
		rbx::signals::signal<void(std::string, shared_ptr<Instance>)> m_unk0xb74; // 0xb74 ctor 0x0052e7a0, dtor 0x0053b310
	};

} // namespace Network
} // namespace RBX
