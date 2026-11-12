/* V8DataModel/Team.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Team, Instance> 0x4BAB94;
     sTeam = "Team".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Team.h */

#pragma once

#include "V8Tree/Instance.h"
#include "Util/BrickColor.h"

namespace RBX {

	extern const char* const sTeam;

	class Team
		: public DescribedCreatable<Team, Instance, sTeam>
	{
	public:
		Team() {}

		// Team::getTeamColor 0x4BA284 (11B): return *(BrickColor*)(this+96)
		// -- the +96 BrickColor dword copied straight out (Teams::
		// getTeamFromTeamColor compares this eax verbatim against the
		// requested color at 0x4BBF06).  Landed by name DELTA (property
		// name unresolved).
		BrickColor teamColor;          // +96

		BrickColor getTeamColor() const { return teamColor; }
	};

} // namespace RBX
