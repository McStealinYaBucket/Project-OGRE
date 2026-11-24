/* Network/Player.cpp — October 2013 STUB (sName + empty reflection).
   TODO: full decompile. */

#include "stdafx.h"

#include "Network/Player.h"
#include "decomp.h"

namespace RBX {
namespace Network {
	const char* const sPlayer = "Player";

	// STUB: PLAYER 0x00535090
	// STUB: STUDIO 0x005e4c10
	void Player::setTeamColor(BrickColor color)
	{
		STUB(0x00535090);
		teamColor = color;
	}

	// STUB: PLAYER 0x005350e0
	// STUB: STUDIO 0x005e4c60
	void Player::setNeutral(bool value)
	{
		STUB(0x005350e0);
		neutral = value;
	}
}
}

using namespace RBX;
using namespace RBX::Network;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
// Player is registered in Network/API.cpp (2013 API.obj carries its classDescriptor copy).
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x0053eb60
// FUNCTION: STUDIO 0x005ed900
template<>
EnumDesc<Network::Player::MembershipType>::EnumDesc()
	: EnumDescriptor("MembershipType", typeid(Network::Player::MembershipType))
{
	addPair(Network::Player::MEMBERSHIP_NONE, "None");
	addPair(Network::Player::MEMBERSHIP_BUILDERS_CLUB, "BuildersClub");
	addPair(Network::Player::MEMBERSHIP_TURBO_BUILDERS_CLUB, "TurboBuildersClub");
	addPair(Network::Player::MEMBERSHIP_OUTRAGEOUS_BUILDERS_CLUB, "OutrageousBuildersClub");
}

template<>
const Type& Type::getSingleton<Network::Player::MembershipType>()
{
	return EnumDesc<Network::Player::MembershipType>::singleton();
}

template<> EnumRegistrar<Network::Player::MembershipType> EnumRegistrar<Network::Player::MembershipType>::registrar(0);
template<> TypeRegistrar<Network::Player::MembershipType> TypeRegistrar<Network::Player::MembershipType>::registrar(0);

// FUNCTION: PLAYER 0x0053ec90
// FUNCTION: STUDIO 0x005eda30
template<>
EnumDesc<Network::Player::ChatMode>::EnumDesc()
	: EnumDescriptor("ChatMode", typeid(Network::Player::ChatMode))
{
	addPair(Network::Player::CHAT_MODE_MENU, "Menu");
	addPair(Network::Player::CHAT_MODE_TEXT_AND_MENU, "TextAndMenu");
}

template<>
const Type& Type::getSingleton<Network::Player::ChatMode>()
{
	return EnumDesc<Network::Player::ChatMode>::singleton();
}

template<> EnumRegistrar<Network::Player::ChatMode> EnumRegistrar<Network::Player::ChatMode>::registrar(0);
template<> TypeRegistrar<Network::Player::ChatMode> TypeRegistrar<Network::Player::ChatMode>::registrar(0);

} // namespace Reflection
} // namespace RBX

// Stand-in, not 2013 code (no marker). 2013's Player.obj and the other Network TUs call
// ServiceProvider::find<T>()/create<T>() for these services from functions this tree does not have
// yet (find<SpawnerService> PLAYER 0x0053be80, find<StarterGuiService> 0x00539450,
// find<PersonalServerService> 0x005391f0, find<SocialService> 0x00539ea0, find<JointsService>
// 0x00590c10, create<MarketplaceService> 0x00598400), so their isNullClassName/Creator copies spell
// include\Util/Object.h. Without a Network instantiation, PLAYER's DataModel.obj supplies them, and it
// spells include\util/object.h (see DataModel.cpp). Drop each line once its real caller is written.
// create<Lighting>: 2013's Creator<Lighting> copies come from the client shell (getClassName 0x00449980,
// Creator ctor 0x0044b820), which this tree does not have yet.
#include "V8DataModel/SpawnerService.h"
#include "V8DataModel/StarterGuiService.h"
#include "V8DataModel/StarterPackService.h"
#include "V8DataModel/PersonalServerService.h"
#include "V8DataModel/SocialService.h"
#include "V8DataModel/JointsService.h"
#include "V8DataModel/PhysicsService.h"
#include "V8DataModel/ScriptService.h"
#include "V8DataModel/MarketplaceService.h"
#include "V8DataModel/Lighting.h"
template RBX::SpawnerService* RBX::ServiceProvider::find<RBX::SpawnerService>() const;
template RBX::StarterGuiService* RBX::ServiceProvider::find<RBX::StarterGuiService>() const;
template RBX::StarterPackService* RBX::ServiceProvider::find<RBX::StarterPackService>() const;
template RBX::PersonalServerService* RBX::ServiceProvider::find<RBX::PersonalServerService>() const;
template RBX::SocialService* RBX::ServiceProvider::find<RBX::SocialService>() const;
template RBX::JointsService* RBX::ServiceProvider::find<RBX::JointsService>() const;
template RBX::PhysicsService* RBX::ServiceProvider::find<RBX::PhysicsService>() const;
template RBX::ScriptService* RBX::ServiceProvider::find<RBX::ScriptService>() const;
template RBX::MarketplaceService* RBX::ServiceProvider::create<RBX::MarketplaceService>() const;
template RBX::Lighting* RBX::ServiceProvider::create<RBX::Lighting>() const;
