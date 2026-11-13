/* App/v8datamodel/Team.cpp -- October 2013. Layout and evidence in V8DataModel/Team.h.
   Functions in PLAYER address order (setScore 0x006aa0b0 .. ctor 0x006aa970). getScore has
   no body of its own in PLAYER (its `mov eax,[ecx+0x60]` is folded with another getter). */

#include "stdafx.h"

#include "V8DataModel/Team.h"
#include "V8DataModel/Teams.h"

namespace RBX {

const char* const sTeam = "Team";

REFLECTION_BEGIN();
static const Reflection::PropDescriptor<Team, int> prop_Score("Score", category_Data, &Team::getScore, &Team::setScore, Reflection::PropertyDescriptor::Attributes::deprecated());
static const Reflection::PropDescriptor<Team, BrickColor> prop_Color("TeamColor", category_Data, &Team::getTeamColor, &Team::setTeamColor);
static const Reflection::PropDescriptor<Team, bool> prop_AutoAssignable("AutoAssignable", category_Data, &Team::getAutoAssignable, &Team::setAutoAssignable);
Reflection::BoundProp<bool> Team::prop_AutoColorCharacters("AutoColorCharacters", category_Data, &Team::autoColorCharacters, Reflection::PropertyDescriptor::Attributes::deprecated());
REFLECTION_END();

int Team::getScore() const
{
	return score;
}

// FUNCTION: PLAYER 0x006aa0b0
// FUNCTION: STUDIO 0x00826c30
void Team::setScore(int score)
{
	if (this->score != score)
	{
		this->score = score;
		raisePropertyChanged(prop_Score);
	}
}

// FUNCTION: PLAYER 0x006aa0d0
// FUNCTION: STUDIO 0x00826c50
BrickColor Team::getTeamColor() const
{
	return color;
}

// FUNCTION: PLAYER 0x006aa0e0
// FUNCTION: STUDIO 0x00826c60
void Team::setTeamColor(BrickColor color)
{
	if (this->color != color)
	{
		this->color = color;
		raisePropertyChanged(prop_Color);
	}
}

// FUNCTION: PLAYER 0x006aa100
// FUNCTION: STUDIO 0x00826c80
bool Team::getAutoAssignable() const
{
	return autoAssignable;
}

// FUNCTION: PLAYER 0x006aa110
// FUNCTION: STUDIO 0x00826c90
void Team::setAutoAssignable(bool autoAssign)
{
	if (this->autoAssignable != autoAssign)
	{
		this->autoAssignable = autoAssign;
		raisePropertyChanged(prop_AutoAssignable);
	}
}

// FUNCTION: PLAYER 0x006aa130
// FUNCTION: STUDIO 0x00826cb0
Team::~Team()
{
}

// FUNCTION: PLAYER 0x006aa8b0
// FUNCTION: STUDIO 0x00827330
bool Team::askSetParent(const Instance* parent) const
{
	return Instance::fastDynamicCast<Teams>(parent) != NULL;
}

// FUNCTION: PLAYER 0x006aa970
// FUNCTION: STUDIO 0x008273f0
Team::Team()
	: score(0)
	, autoAssignable(true)
	, autoColorCharacters(true)
{
	setName(sTeam);
	color = BrickColor::brick_1;	// 2016: BrickColor::brickWhite()
}

} // namespace RBX

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::Team);
}
