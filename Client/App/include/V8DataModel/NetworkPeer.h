/* V8DataModel/NetworkPeer.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED Described<Network::Peer, sPeer, Instance> 0x83999C (abstract, NO Product wrapper in binary; NonFactory approx) + player RTTI; flattened; Lua Peer (meta alias NetworkPeer);
     sPeer = "Peer".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/NetworkPeer.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sPeer;

	class NetworkPeer
		: public DescribedNonCreatable<NetworkPeer, Instance, sPeer>
	{
	public:
		NetworkPeer() {}
	};

} // namespace RBX
