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
