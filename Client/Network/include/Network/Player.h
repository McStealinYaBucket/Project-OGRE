/* Network/Player.h — October 2013 STUB (per-player instance shell).
   Evidence: Player C2 0x8DB1B2 (RBX::Network::Player); Network::sPlayer
     declare symbols; 0.125 metadata "Player" class.
     Parent TODO (Instance stands in; verify vs player-construction path).
   TODO: full decompile (player data, character, replication).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/Network/Player.cpp (assert path TBD). */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {
namespace Network {

	extern const char* const sPlayer;

	class Player
		: public DescribedCreatable<Player, Instance, sPlayer>
	{
	public:
		Player() {}

		// Members pinned by PlayerChatLine ctor 0x5BEFFA + ChatMessage::
		// isVisibleToPlayer 0x89BDD6 reads (GCC offsets; MSVC layout DELTA --
		// member ORDER is the invariant; the gcc-class members between the
		// base and these slots are undecoded TOs).
		shared_ptr<Instance> character;   // binary +92 (px; rc@+96):
			// PlayerChatLine stores *(Player+92) into ChatLine::origin (+28);
			// createBillboardGuiHelper call passes *(msg.player.px + 92).
		int teamNum;                      // binary +100: team id (visTo +100==+100).
		bool neutral;                     // binary +104: visTo gates +104==0 on
			// both sides for team chat; PlayerChatLine colors white when set.
	};

} // namespace Network
} // namespace RBX
