/* V8DataModel/TeleportCallback.h -- October 2013 (RTTI .?AVTeleportCallback@RBX@@; iOS 10-29
   TeleportService::SetCallback(TeleportCallback*)). The interface the client shell implements
   (WindowsClient's Teleporter).

   2013 vftable PLAYER 0x00bd93ac: three slots, {deleting dtor 0x004022b0, _purecall, _purecall}.
   2016's fourth slot (xBox_getGamerTag) did not exist yet. The VTABLE marker stays with the shell's
   Teleporter.h, whose TU emits the vftable. */

#pragma once

#include <string>

namespace RBX {

	class TeleportCallback
	{
	public:
		virtual ~TeleportCallback() {}	// vtable+0x00

		// Teleports the local player to url, using ticket and running script on arrival.
		virtual void doTeleport(const std::string& url, const std::string& ticket,
			const std::string& script) = 0;	// vtable+0x04
		virtual bool isTeleportEnabled() const = 0;	// vtable+0x08
	};

} // namespace RBX
