/* Network/NetworkOwner.h -- October 2013 (ownership sentinels).
   Carried verbatim from March 2016 first-party NetworkOwner.h: protocol
   sentinel values Server{1,0} / ServerUnassigned{1,1} / Unassigned{} /
   AssignedOther{0,1} + isClient/isServer predicates. These constants are
   wire-protocol stable (asserted mutually-distinct in Unassigned());
   carried as exact values, flagged TODO-verify against a 2013
   Server()/Unassigned() disasm if one surfaces (batch284 swept the
   callers, not the statics themselves).
   Original file: Client/Network/include/network/NetworkOwner.h (2016
   path; our Network tree location may differ -- see below). */

#pragma once

#include "Util/SystemAddress.h"
#include "G3D/Color3.h"
#include "rbx/Debug.h"

namespace RBX {
	namespace Network {

	class NetworkOwner
	{
	public:
		static const RBX::SystemAddress Server() {
			static RBX::SystemAddress s(1, 0);
			return s;
		}

		// created on server, have not be properly assigned via network owner job
		static const RBX::SystemAddress ServerUnassigned() {
			static RBX::SystemAddress s(1, 1);
			return s;
		}

		// default
		static const RBX::SystemAddress Unassigned() {
			static RBX::SystemAddress s;
			RBXASSERT(s == SystemAddress());
			RBXASSERT(s != NetworkOwner::Server());
			RBXASSERT(s != NetworkOwner::ServerUnassigned());
			RBXASSERT(s != NetworkOwner::AssignedOther());
			return s;
		}

		// generic value used on client indicating assigned to other clients or server (i.e. not self)
		static const RBX::SystemAddress AssignedOther() {
			static RBX::SystemAddress s(0, 1);
			return s;
		}

		static bool isClient(const RBX::SystemAddress& address) {
			return (	(address != Server())
					&&	(address != Unassigned())
					&&  (address != ServerUnassigned()));
		}

		static bool isServer(const RBX::SystemAddress& address) {
			return address == Server() || address == ServerUnassigned();
		}
	};

}}	// namespace
