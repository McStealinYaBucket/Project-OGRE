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
RBX_REGISTER_CLASS(RBX::Network::Player);
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
