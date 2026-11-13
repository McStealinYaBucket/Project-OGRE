/* Network/Player.h — October 2013 STUB (per-player instance shell).
   Evidence: Player C2 0x8DB1B2 (RBX::Network::Player); Network::sPlayer
     declare symbols; 0.125 metadata "Player" class.
     Parent TODO (Instance stands in; verify vs player-construction path).
   TODO: full decompile (player data, character, replication).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/Network/Player.cpp (assert path TBD). */

#pragma once

#include "V8Tree/Instance.h"
#include "Util/BrickColor.h"

namespace RBX {
namespace Network {

	extern const char* const sPlayer;

	class Player
		// 2013 RTTI says functionality 19 (RUNTIME: DescribedCreatable<...$0BD@...>). Switching to it
		// renames the ??_E/??_G template bodies the CSVs pair by name, and FactoryProduct<Player>'s
		// vftable and ??_G 0x00529050 leave 100% (measured 2026-11-13); kept at the default until
		// the CSVs are regenerated with it.
		: public DescribedCreatable<Player, Instance, sPlayer>
	{
	public:
		enum ChatMode
		{
			CHAT_MODE_MENU = 0,
			CHAT_MODE_TEXT_AND_MENU = 1,
		};

		enum MembershipType
		{
			MEMBERSHIP_NONE = 0,
			MEMBERSHIP_BUILDERS_CLUB = 1,
			MEMBERSHIP_TURBO_BUILDERS_CLUB = 2,
			MEMBERSHIP_OUTRAGEOUS_BUILDERS_CLUB = 3,
		};

		Player() {}

		// Members pinned by PlayerChatLine ctor 0x5BEFFA + ChatMessage::
		// isVisibleToPlayer 0x89BDD6 reads (GCC offsets; MSVC layout DELTA --
		// member ORDER is the invariant; the gcc-class members between the
		// base and these slots are undecoded TOs).
		shared_ptr<Instance> character;   // binary +92 (px; rc@+96):
			// PlayerChatLine stores *(Player+92) into ChatLine::origin (+28);
			// createBillboardGuiHelper call passes *(msg.player.px + 92).
		BrickColor teamColor;             // 0x68 (mac +100): Teams::getTeamFromPlayer PLAYER
			// 0x007a4770 passes it unchanged as the BrickColor argument (2016: getTeamColor()).
		bool neutral;                     // 0x6c (mac +104; getTeamFromPlayer 0x007a4774): visTo gates +104==0 on
			// both sides for team chat; PlayerChatLine colors white when set.

		// Out of line in 2013 (PLAYER 0x00535090 / 0x005350e0): store, character refresh, then
		// raisePropertyChanged. Their 2013 .obj is not identified (they sit among Network templates).
		BrickColor getTeamColor() const { return teamColor; }
		void setTeamColor(BrickColor color);
		void setNeutral(bool neutral);
	};

} // namespace Network
} // namespace RBX
