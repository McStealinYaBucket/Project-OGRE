/* Network/Client.h — October 2013 STUB (presence-gate shell).
   Evidence: Client::findClient 0x844AEC (findServiceProvider + assert
     "!testInDatamodel || serviceProvider!=NULL" at Network/Client.cpp:82 +
     find<Client>), clientIsPresent 0x844B7C (findClient != NULL),
     Players::clientIsPresent 0x89B10E + backendProcessing 0x89B206 call into
     these; 2016 Network/Client.h shows the static shape + Service base
     (DescribedCreatable<Client, Peer, sClient> — Peer/RakNet absent here, so
     Instance stands in directly).
   sClient = "NetworkClient" (PE-verified: 0.125 Studio string table).
   TODO: full decompile (replication peer machinery).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/Network/Client.cpp (assert path). */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {
namespace Network {

	extern const char* const sClient;

	class Client
		: public DescribedCreatable<Client, Instance, sClient, Reflection::ClassDescriptor::INTERNAL_LOCAL>
		, public Service
		// TRUE parent: Peer (see above).
	{
	public:
		Client() {}

		// mac: 0x844AEC / 0x844B7C (static 2016 shape carried).
		static Client* findClient(const Instance* context, bool testInDatamodel = true);
		static bool clientIsPresent(const Instance* context, bool testInDatamodel = true);
	};

} // namespace Network
} // namespace RBX
