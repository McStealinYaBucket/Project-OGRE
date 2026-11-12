/* Network/Server.h — October 2013 STUB (presence-gate shell).
   Evidence: Server::serverIsPresent 0x87D908 (findServiceProvider + assert
     "!testInDatamodel || serviceProvider!=NULL" at Network/Server.cpp:96 +
     find<Server> non-null); Players::serverIsPresent 0x89B130 delegates;
     Workspace::serverIsPresent 0x51237C delegates.
     2016 Network/Server.h shows the static shape + Service base
     (DescribedCreatable<Server, Peer, sServer> — Peer/RakNet absent here, so
     Instance stands in directly).
   sServer = "NetworkServer" (2016 Server.cpp; 6 hits in the 0.125 PE string
     table, same class-name slot as NetworkClient's 3).
   TODO: full decompile (replication server machinery).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/Network/Server.cpp (assert path). */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {
namespace Network {

	extern const char* const sServer;

	class Server
		: public DescribedCreatable<Server, Instance, sServer, Reflection::ClassDescriptor::INTERNAL_LOCAL>
		, public Service
		// TRUE parent: Peer (see above).
	{
	public:
		Server() {}

		// mac: 0x87D908 (static 2016 shape carried: provider assert + find).
		static bool serverIsPresent(const Instance* context, bool testInDatamodel = true);
	};

} // namespace Network
} // namespace RBX
