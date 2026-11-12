/* V8DataModel/NetworkMarker.h -- October 2013 STUB (minimal shell).
   Evidence: SIMPLIFIED NonFactoryProduct<Network::Marker, Instance> 0x90AE50 + player RTTI DescribedNonCreatable<VMarker>; flattened from Network ns; Lua Marker (meta alias NetworkMarker);
     sMarker = "Marker".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/NetworkMarker.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sMarker;

	class NetworkMarker
		: public DescribedNonCreatable<NetworkMarker, Instance, sMarker>
	{
	public:
		NetworkMarker() {}
	};

} // namespace RBX
