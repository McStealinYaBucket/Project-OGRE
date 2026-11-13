/* App/v8datamodel/Teams.cpp -- October 2013. ctor/dtor, askAddChild and the team functions from
   the PLAYER bytes (0x007a44f0..0x007a5130), in address order; blank runs put the asserts on
   their 2013 lines (97, 160). Reflection not written. */

#include "stdafx.h"
#include <algorithm>

#include "V8DataModel/Teams.h"
#include "V8DataModel/Team.h"
#include "Util/BrickColor.h"
#include "Network/Player.h"
#include "Network/Players.h"
#include "V8DataModel/Humanoid.h"

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

	// 2016 name and body; PLAYER bytes: the child loop with copy_on_write_ptr's asserts, Team
	// classDescriptor isA, then the out-of-line Team::getTeamColor 0x006aa0d0.
	// FUNCTION: PLAYER 0x007a4660
	// FUNCTION: STUDIO 0x008a2f50
	Team* Teams::getTeamFromTeamColor(BrickColor c)
	{
		for (unsigned int i = 0; i < numChildren(); i++)
		{
			Team* t = Instance::fastDynamicCast<Team>(getChild(i));
			if (t && t->getTeamColor() == c)
				return t;
		}
		return NULL;
	}

	// 2013 reads Player +0x6c (neutral) and +0x68 (team color).
	// FUNCTION: PLAYER 0x007a4770
	// FUNCTION: STUDIO 0x008a3060
	Team* Teams::getTeamFromPlayer(Network::Player* p)
	{
		if (p->neutral)
			return NULL;
		return getTeamFromTeamColor(p->teamColor);
	}

	// onChildAdded 0x007a4790 / onChildRemoving 0x007a48c0 (97.9% / 97.4% when written as 2016's
	// teams.write() push_back / erase of shared_from(team); only call pairing differs) are kept
	// out: instantiating shared_from<Team> adds a name to our linker's one folded shared_from<>
	// body, and Instance::onAncestorChanged/onDescendantAdded and DataModel::onChildChanged then
	// lose their call pairing (measured 2026-11-13, P1). Bodies: scratchpad dm5b/Teams.cpp.withhooks.

	// FUNCTION: PLAYER 0x007a4ae0
	// FUNCTION: STUDIO 0x008a33d0
	bool Teams::isTeamGame()
	{
		/* returns true if any player is non-neutral */
		if (Network::Players* players = ServiceProvider::find<Network::Players>(this)) {
			for (unsigned int n = 0; n < players->numChildren(); n++)
			{
				Network::Player* p = Instance::fastDynamicCast<Network::Player>(players->getChild(n));
				if (p == NULL) continue; // only care about player objects
				if (p->neutral == false)
					return true;
			}
		}
		return false;
	}













	// FUNCTION: PLAYER 0x007a4bf0
	// FUNCTION: STUDIO 0x008a34e0
	int Teams::getNumPlayersInTeam(BrickColor color)
	{
		int result = 0;
		Network::Players* players = ServiceProvider::find<Network::Players>(this);
		RBXASSERT(players);	// 2013 line 97
		for (unsigned int n = 0; n < players->numChildren(); n++)
		{
			Network::Player* p = Instance::fastDynamicCast<Network::Player>(players->getChild(n));
			if (p == NULL) continue; // only care about player objects
			if (p->neutral == false && p->teamColor == color) result++;
		}
		return result;
	}

	// FUNCTION: PLAYER 0x007a4d40
	// FUNCTION: STUDIO 0x008a3630
	bool Teams::teamExists(BrickColor color)
	{
		return (getTeamFromTeamColor(color) != NULL);
	}

	// FUNCTION: PLAYER 0x007a4f10
	// FUNCTION: STUDIO 0x008a3730
	Teams::Teams()
		: teams(Instances())	// non-NULL from the start
	{
		setName(sTeams);
	}

	// 2016 starts from BrickColor::brickGreen() (28, stored inline at 0x007a4fc9).
	// FUNCTION: PLAYER 0x007a4fc0
	// FUNCTION: STUDIO 0x008a37e0
	void Teams::assignNewPlayerToTeam(Network::Player* p)
	{
		BrickColor best = BrickColor::brick_28;
		int best_count = 10000;
		bool found = false;

		for (unsigned int i = 0; i < this->numChildren(); i++)
		{
			Team* child = Instance::fastDynamicCast<Team>(this->getChild(i));
			if (child != NULL) {
				if (child->getAutoAssignable() == true) {
					int count = getNumPlayersInTeam(child->getTeamColor());
					if (count < best_count) {
						best = child->getTeamColor();
						best_count = count;
						found = true;
					}
				}
			}
		}

		if (found)
		{
			p->setTeamColor(best);
			p->setNeutral(false);
		}
	}



	// FUNCTION: PLAYER 0x007a5130
	// FUNCTION: STUDIO 0x008a3950
	G3D::Color3 Teams::getTeamColorForHumanoid(Humanoid* h)
	{
		Network::Players* players = ServiceProvider::find<Network::Players>(this);
		RBXASSERT(players);	// 2013 line 160

		if (players)
		{
			for (unsigned int n = 0; n < players->numChildren(); n++)
			{
				if (Network::Player* p = Instance::fastDynamicCast<Network::Player>(players->getChild(n)))
					if (p->neutral == false)
						if (Instance* m = p->character.get())
							if (m->findFirstChildOfType<Humanoid>() == h)
								return p->getTeamColor().color3();
			}
		}
		return G3D::Color3::white();
	}
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Teams);
}
