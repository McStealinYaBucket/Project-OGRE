/* App/v8datamodel/Teams.cpp -- October 2013. ctor/dtor, askAddChild and the child hooks from
   the PLAYER bytes (0x007a44f0..0x007a4f10); isTeamGame/getTeamFromTeamColor/getTeamFromPlayer
   are older Mac readings, not yet located in PLAYER. Reflection not written. */

#include "stdafx.h"

#include <algorithm>

#include "V8DataModel/Teams.h"
#include "V8DataModel/Team.h"
#include "Util/BrickColor.h"
#include "Network/Player.h"
#include "Network/Players.h"

namespace RBX {
	const char* const sTeams = "Teams";

	// FUNCTION: PLAYER 0x007a44f0
	// FUNCTION: STUDIO 0x008a2e00
	Teams::~Teams()
	{
	}

	// FUNCTION: PLAYER 0x007a4610
	// FUNCTION: STUDIO 0x008a2f00
	bool Teams::askAddChild(const Instance* child) const
	{
		return Instance::fastDynamicCast<Team>(child) != NULL;
	}

	// onChildAdded 0x007a4790 / onChildRemoving 0x007a48c0 (2016 bodies: teams.write() push_back /
	// erase of shared_from(team)) are not written here: instantiating shared_from<Team> changes
	// which folded shared_from<> copy our linker names, and three Instance/DataModel functions
	// then lose their call pairing.

	// FUNCTION: PLAYER 0x007a4f10
	// FUNCTION: STUDIO 0x008a3730
	Teams::Teams()
		: teams(Instances())	// non-NULL from the start
	{
		setName(sTeams);
	}
}

using namespace RBX;

// Teams::isTeamGame 0x4BBB9A (159B): players = ServiceProvider::find
// <Network::Players>(this); when non-null and numChildren() != 0, walk
// the child vector (cow deref @+56, element sp[i] -- landed getChild):
// skip null slots; child isA(Player::classDescriptor()) via the cached
// Instance+0x30 descriptor read (0x4BBBFC + ClassDescriptor::isA
// mac: 0x157C0E); a first child whose neutral byte (+104) is CLEAR returns
// true (0x4BBC0F/0x4BBC16, al=1); neutral and non-Player children just
// advance the loop (0x4BBC1C).  No Player child qualifies -> false.
bool Teams::isTeamGame()
{
	Network::Players* players = ServiceProvider::find<Network::Players>(this);
	if (players == NULL || players->numChildren() == 0)
		return false;
	for (size_t i = 0; i < players->numChildren(); ++i)
	{
		Network::Player* child = players->queryTypedChild<Network::Player>(i);
		if (child != NULL && child->neutral == false)
			return true;
	}
	return false;
}

// Teams::getTeamFromTeamColor 0x4BBE88 (159B): walk this's children
// (same cow morphology); skip null slots; child isA(Team::classDescriptor
// ()) via the Instance+0x30 descriptor (0x4BBEE1-F0); the first Team
// child whose getTeamColor() (0x4BA284, the +96 BrickColor) equals the
// requested color (dword compare at 0x4BBF06) is returned; exhausted ->
// NULL.
Team* Teams::getTeamFromTeamColor(BrickColor teamColor)
{
	for (size_t i = 0; i < this->numChildren(); ++i)
	{
		Team* team = this->queryTypedChild<Team>(i);
		if (team != NULL && team->getTeamColor() == teamColor)
			return team;
	}
	return NULL;
}

// Teams::getTeamFromPlayer 0x4BBF28 (48B): a neutral player (+104 != 0)
// returns NULL (0x4BBF31 jz arm keeps going only when the byte is
// clear); otherwise getTeamFromTeamColor(*(BrickColor*)(player+100)) --
// the +100 dword is the team number / team color number (the same seat
// visTo compares at 0x89BDD6 as teamNum).
Team* Teams::getTeamFromPlayer(Network::Player* player)
{
	if (player->neutral != false)
		return NULL;
	return getTeamFromTeamColor(BrickColor(player->teamNum));  // explicit int ctor
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Teams);
}
