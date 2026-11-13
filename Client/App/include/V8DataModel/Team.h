/* V8DataModel/Team.h -- October 2013.
   Evidence (PLAYER): ctor 0x006aa970 (score 0 at +0x60, BrickColor default 194 then 1 at +0x64,
     autoAssignable/autoColorCharacters true at +0x68/+0x69, setName("Team")); operator new 0x6c in
     Creatable<Instance>::create<Team> 0x00421790; setters 0x006aa0b0/0x006aa0e0/0x006aa110 raise
     the three property descriptors; askSetParent 0x006aa8b0 (fastDynamicCast<Teams>); the
     vftable 0x00b39304 slot +0x3c askAddChild is the shared `return true` body 0x006ceaa0.
   Bases (RTTI): DescribedCreatable<Team, Instance, sTeam> only. The 2016 header agrees.
   Original file: Client/App/v8datamodel/Team.h (2016 path; no 2013 assert names it). */

#pragma once

#include "V8Tree/Instance.h"
#include "Util/BrickColor.h"

namespace RBX {

	extern const char* const sTeam;

	// VTABLE: PLAYER 0x00b39304 RBX::Debugable
	// SIZE 0x6c
	class Team
		: public DescribedCreatable<Team, Instance, sTeam>
	{
	protected:
		int score;					// 0x60
		BrickColor color;			// 0x64
		bool autoAssignable;		// 0x68
		bool autoColorCharacters;	// 0x69

	public:
		Team();
		~Team();

		// SYNTHETIC: PLAYER 0x006aa160
		// SYNTHETIC: STUDIO 0x008274a0
		// RBX::Team::`scalar deleting destructor'

		int getScore() const;
		void setScore(int score);
		BrickColor getTeamColor() const;
		void setTeamColor(BrickColor color);
		bool getAutoAssignable() const;
		void setAutoAssignable(bool autoAssign);

		static Reflection::BoundProp<bool> prop_AutoColorCharacters;

	protected:
		/*override*/ bool askSetParent(const Instance* parent) const;
		/*override*/ bool askAddChild(const Instance* instance) const { return true; }
	};

} // namespace RBX

// Template, inline and implicit bodies this .obj emits whose 2013 copy has no oracle name: paired
// by identical masked bytes plus string/RTTI/callee operands (tools/unpaired_twins.py).

// TEMPLATE: PLAYER 0x006aa1a0
// TEMPLATE: STUDIO 0x00827490
// ??_ETeam@RBX@@WBM@AEPAXI@Z

// TEMPLATE: PLAYER 0x006aa240
// TEMPLATE: STUDIO 0x00826d40
// ??$getset@P8Team@RBX@@BEHXZP812@AEXH@Z@?$PropDescriptor@VTeam@RBX@@H@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@H@Reflection@RBX@@@std@@P8Team@2@BEHXZP852@AEXH@Z@Z

// TEMPLATE: PLAYER 0x006aa2a0
// TEMPLATE: STUDIO 0x00826da0
// ??$getset@P8Team@RBX@@BE?AVBrickColor@2@XZP812@AEXV32@@Z@?$PropDescriptor@VTeam@RBX@@VBrickColor@2@@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@VBrickColor@RBX@@@Reflection@RBX@@@std@@P8Team@2@BE?AVBrickColor@2@XZP852@AEXV62@@Z@Z

// TEMPLATE: PLAYER 0x006aa300
// TEMPLATE: STUDIO 0x00826e00
// ??$getset@P8Team@RBX@@BE_NXZP812@AEX_N@Z@?$PropDescriptor@VTeam@RBX@@_N@Reflection@RBX@@SA?AV?$auto_ptr@VGetSet@?$TypedPropertyDescriptor@_N@Reflection@RBX@@@std@@P8Team@2@BE_NXZP852@AEX_N@Z@Z

