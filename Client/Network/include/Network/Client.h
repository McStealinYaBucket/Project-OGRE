/* Network/Client.h -- October 2013, partial.
   Layout from Client::Client (PLAYER 0x005b0880) and Client::~Client (0x005ad760): Peer (0xac0),
   Service at 0xac0 (isPublic = true), then closingConnection 0xac4, serverId 0xac8, ticket 0xadc,
   userId 0xaf8 (-1), networkSettings 0xafc, and the three signals at 0xb00/0xb04/0xb08. 2016 moved
   userId before ticket and added isCloudEditClient; neither is in 2013. RTTI functionality 1
   (INTERNAL_LOCAL). The file path is the prior tree's; 2013 records no assert in Client.h. */

#pragma once

#include "V8Tree/Service.h"
#include "../../Peer.h"
#include "RakNetTypes.h"

namespace RBX {
	class NetworkSettings;

namespace Network {

	extern const char* const sClient;

	// VTABLE: PLAYER 0x00b1da8c
	// SIZE 0xb10
	class Client
		: public DescribedCreatable<Client, Peer, sClient, Reflection::ClassDescriptor::INTERNAL_LOCAL>
		, public Service
	{
	private:
		typedef DescribedCreatable<Client, Peer, sClient, Reflection::ClassDescriptor::INTERNAL_LOCAL> Super;
		rbx::signals::scoped_connection closingConnection;  // 0xac4
		RakNet::SystemAddress serverId;                     // 0xac8
		std::string ticket;                                 // 0xadc
		int userId;                                         // 0xaf8
		NetworkSettings* networkSettings;                   // 0xafc

	public:
		Client();
		~Client();

		rbx::signal<void(std::string, shared_ptr<Instance>)> connectionAcceptedSignal;  // 0xb00
		rbx::signal<void(std::string, int, std::string)> connectionFailedSignal;        // 0xb04
		rbx::signal<void(std::string)> connectionRejectedSignal;                       // 0xb08

		// mac: 0x844AEC / 0x844B7C (static 2016 shape carried).
		static Client* findClient(const Instance* context, bool testInDatamodel = true);
		static bool clientIsPresent(const Instance* context, bool testInDatamodel = true);
	};

} // namespace Network
} // namespace RBX
