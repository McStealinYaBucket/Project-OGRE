/* V8DataModel/RemoteEvent.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<RemoteEvent, Instance> 0x7443A8;
     sRemoteEvent = "RemoteEvent".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/RemoteEvent.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sRemoteEvent;

	class RemoteEvent
		: public DescribedCreatable<RemoteEvent, Instance, sRemoteEvent>
	{
	public:
		RemoteEvent() {}
	};

} // namespace RBX
