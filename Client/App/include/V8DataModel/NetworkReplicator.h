/* V8DataModel/NetworkReplicator.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED NonFactoryProduct<Network::Replicator, IdSerializer> 0x83C0B0 + player RTTI DescribedNonCreatable<VReplicator>; base flattened to Instance (IdSerializer absent); Lua Replicator (meta alias NetworkReplicator);
     sReplicator = "Replicator".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/NetworkReplicator.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sReplicator;

	class NetworkReplicator
		: public DescribedNonCreatable<NetworkReplicator, Instance, sReplicator>
	{
	public:
		NetworkReplicator() {}
	};

} // namespace RBX
