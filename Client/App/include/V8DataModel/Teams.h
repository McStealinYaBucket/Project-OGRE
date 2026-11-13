/* V8DataModel/Teams.h -- October 2013 STUB (minimal, unblocks DataModel).
   Evidence: referenced from DataModel::clearContents 0x2CF1D0 / doCloseDataModel
     0x2D3952 (clearContents 0x2CF1D0 references it.); "Teams" string check TODO per service.
   Functionality + Service base carried from March 2016 conventions.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Teams.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "Util/BrickColor.h"

namespace RBX {

namespace Network { class Player; }

class Team;

	extern const char* const sTeams;

	// 2013 (PLAYER): RTTI DescribedCreatable<Teams, Instance, sTeams, 11 = PERSISTENT_HIDDEN> and
	// Service @0x60; ctor 0x007a4f10 builds teams (copy_on_write_ptr<Instances> @0x64) from an
	// empty vector and calls setName("Teams"); ~Teams 0x007a44f0; operator new 0x6c (0x0053d150).
	// VTABLE: PLAYER 0x00b62794 RBX::Debugable
	// SIZE 0x6c
	class Teams
		: public DescribedCreatable<Teams, Instance, sTeams, Reflection::ClassDescriptor::PERSISTENT_HIDDEN>
		, public Service
	{
	public:
		Teams();
		~Teams();

		bool isTeamGame();                                       // 0x4BBB9A
		Team* getTeamFromTeamColor(BrickColor teamColor);         // 0x4BBE88
		Team* getTeamFromPlayer(Network::Player* player);        // 0x4BBF28

	private:
		copy_on_write_ptr<Instances> teams;	// 0x64

	protected:
		// 2016 defines it in the header; 2013 PLAYER has the body at 0x007a4610.
		/*override*/ bool askAddChild(const Instance* child) const;
	};

} // namespace RBX
