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
class Humanoid;

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
		Team* getTeamFromTeamColor(BrickColor c);                 // PLAYER 0x007a4660
		Team* getTeamFromPlayer(Network::Player* p);              // PLAYER 0x007a4770
		int getNumPlayersInTeam(BrickColor color);                // PLAYER 0x007a4bf0
		void assignNewPlayerToTeam(Network::Player* p);           // PLAYER 0x007a4fc0
		G3D::Color3 getTeamColorForHumanoid(Humanoid* h);         // PLAYER 0x007a5130
		bool teamExists(BrickColor color);                        // PLAYER 0x007a4d40

	private:
		copy_on_write_ptr<Instances> teams;	// 0x64

	protected:
		// 2016 defines it in the header; 2013 PLAYER has the body at 0x007a4610.
		/*override*/ bool askAddChild(const Instance* child) const;
	};

} // namespace RBX

// Template, inline and implicit bodies this .obj emits whose 2013 copy has no oracle name: paired
// by identical masked bytes plus string/RTTI/callee operands (tools/unpaired_twins.py).

// TEMPLATE: PLAYER 0x007a44c0
// TEMPLATE: STUDIO 0x008a2dd0
// ??_E?$DescribedCreatable@VTeams@RBX@@VInstance@2@$E?sTeams@2@3QBDB$0L@$0A@@RBX@@WBM@AEPAXI@Z

// TEMPLATE: PLAYER 0x007a44d0
// TEMPLATE: STUDIO 0x008a2de0
// ??1?$Described@VTeams@RBX@@$E?sTeams@2@3QBDBV?$FactoryProduct@VTeams@RBX@@VInstance@2@$E?sTeams@2@3QBDBV32@@2@$0L@$0A@@Reflection@RBX@@UAE@XZ

// TEMPLATE: PLAYER 0x007a45e0
// ??_GTeams@RBX@@UAEPAXI@Z

