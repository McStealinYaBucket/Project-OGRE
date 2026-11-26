/* WindowsClient/Teleporter.h -- October 2013 client shell (PLAYER only). No 2013 __FILE__ string names
   Teleporter.cpp; the path is the 2016 tree's. Application holds one at +0x12c (the ctor 0x00403000
   stores only the vftable: app and marshaller stay unset until initialize).
   vftables: TeleportCallback 0x00bd93ac {deleting dtor 0x004022b0, two pure slots}, Teleporter
   0x00bd93bc {0x004022b0, doTeleport 0x0045d450, isTeleportEnabled folded with Verb::isEnabled
   0x004595b0}. */

#pragma once

#include <string>

namespace RBX {

	class Application;
	class FunctionMarshaller;

	// App's v8datamodel/TeleportCallback.h is not in this tree yet: the slots the shell overrides.
	// VTABLE: PLAYER 0x00bd93ac
	class TeleportCallback
	{
	public:
		virtual ~TeleportCallback() {} // vtable+0x00
		virtual void doTeleport(const std::string& url, const std::string& ticket,
			const std::string& script) = 0; // vtable+0x04
		virtual bool isTeleportEnabled() const = 0; // vtable+0x08
	};

	// VTABLE: PLAYER 0x00bd93bc
	// SIZE 0xc
	class Teleporter : public TeleportCallback
	{
		Application* app;               // 0x04
		FunctionMarshaller* marshaller; // 0x08

	public:
		// Submits a callback to the function marshaller to teleport the player.
		virtual void doTeleport(const std::string& url, const std::string& ticket,
			const std::string& script);                          // vtable+0x04
		virtual bool isTeleportEnabled() const { return true; } // vtable+0x08
	};

} // namespace RBX
