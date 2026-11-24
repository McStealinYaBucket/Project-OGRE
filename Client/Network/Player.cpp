/* Network/Player.cpp — October 2013. Layout and evidence in Network/Player.h.
   TODO: the remaining descriptors and methods. */

#include "stdafx.h"

#include "Network/Player.h"
#include "Network/Players.h"
#include "FastLog.h"
#include "decomp.h"

DECOMP_SIZE_ASSERT(RBX::Network::Player, 0x1d8);

// Registration 0x00a5f330 (kind 0, static); the flag byte is PLAYER 0x00dd8bfb.
FASTFLAGVARIABLE(CalculatePlayerSpawnLocationAsEarlyAsPossible, false)

// Defined by 2013's ScriptContext.obj (registration 0x00a66090, byte 0x00d30f1b, default 1).
LOGGROUP(Network)

namespace RBX {
namespace Network {
	const char* const sPlayer = "Player";

	// Descriptors in the order of their 2013 dynamic initializers (only those this file's functions
	// use so far): 0x00a5f430 HasBuildTools (SCRIPTING, WritePlayer), 0x00a5ff10
	// CanLoadCharacterAppearance (SCRIPTING), then the ones below.
	static Reflection::PropDescriptor<Player, bool> prop_HasBuildTools("HasBuildTools", category_Data, &Player::getHasGroupBuildTools, &Player::setHasGroupBuildTools, Reflection::PropertyDescriptor::SCRIPTING, Security::WritePlayer);
	static Reflection::PropDescriptor<Player, bool> prop_CanLoadCharacterAppearance("CanLoadCharacterAppearance", category_Behavior, &Player::getCanLoadCharacterAppearance, &Player::setCanLoadCharacterAppearance, Reflection::PropertyDescriptor::SCRIPTING);

	// Dynamic initializers 0x00a602e0 / 0x00a60320 (BoundProp<float> ctor 0x005409d0): flags 0x13
	// (SCRIPTING) and 0x15 (UI), security 2 (RobloxPlace).
	Reflection::BoundProp<float> Player::prop_SimulationRadius("SimulationRadius", category_Data, &Player::simulationRadius, Reflection::PropertyDescriptor::SCRIPTING, Security::RobloxPlace);
	Reflection::BoundProp<float> Player::prop_MaxSimulationRadius("MaximumSimulationRadius", category_Data, &Player::maxSimulationRadius, Reflection::PropertyDescriptor::UI, Security::RobloxPlace);

	// Dynamic initializers 0x00a60430 / 0x00a60490: flags 0x1f (STANDARD), security 0.
	static Reflection::PropDescriptor<Player, BrickColor> prop_teamColor("TeamColor", category_Team, &Player::getTeamColor, &Player::setTeamColor);
	static Reflection::PropDescriptor<Player, bool> prop_Neutral("Neutral", category_Team, &Player::getNeutral, &Player::setNeutral);

	// 0x00a60530 / 0x00a60570 / 0x00a605c0 / 0x00a60600: a read-only UI property plus a
	// REPLICATE_ONLY twin that carries the setter.
	static Reflection::EnumPropDescriptor<Player, Player::MembershipType> prop_MembershipType("MembershipType", category_Data, &Player::getMembershipType, NULL, Reflection::PropertyDescriptor::UI);
	static Reflection::EnumPropDescriptor<Player, Player::MembershipType> prop_MembershipTypeReplicate("MembershipTypeReplicate", category_Data, &Player::getMembershipType, &Player::setMembershipType, Reflection::PropertyDescriptor::REPLICATE_ONLY);
	static Reflection::PropDescriptor<Player, int> prop_AccountAge("AccountAge", category_Data, &Player::getAccountAge, NULL, Reflection::PropertyDescriptor::UI);
	static Reflection::PropDescriptor<Player, int> prop_AccountAgeReplicate("AccountAgeReplicate", category_Data, &Player::getAccountAge, &Player::setAccountAge, Reflection::PropertyDescriptor::REPLICATE_ONLY);

	// FUNCTION: PLAYER 0x00527390
	int Player::getDataComplexityLimit() const
	{
		return dataComplexityLimit;
	}

	// FUNCTION: PLAYER 0x005273a0
	bool Player::getCanLoadCharacterAppearance() const
	{
		return canLoadCharacterAppearance;
	}

	// FUNCTION: PLAYER 0x005273b0
	int Player::getPersonalServerRank() const
	{
		return personalServerRank;
	}

	// FUNCTION: PLAYER 0x005273e0
	// FUNCTION: STUDIO 0x005d7f60
	std::string Player::getCharacterAppearance() const
	{
		return characterAppearance;
	}

	// FUNCTION: PLAYER 0x00527410
	// FUNCTION: STUDIO 0x005d7f90
	bool Player::getUnder13()
	{
		return under13;
	}

	// FUNCTION: PLAYER 0x00527420
	int Player::getUserID() const
	{
		return userId;
	}

	// FUNCTION: PLAYER 0x00527430
	// FUNCTION: STUDIO 0x005d7fa0
	bool Player::isGuest() const
	{
		return userId < 0;
	}

	// FUNCTION: PLAYER 0x00527440
	Player::MembershipType Player::getMembershipType() const
	{
		return membershipType;
	}

	// FUNCTION: PLAYER 0x00527460
	// FUNCTION: STUDIO 0x005d7fc0
	bool Player::getAppearanceDidLoad() const
	{
		return appearanceDidLoad;
	}

	// FUNCTION: PLAYER 0x00527690
	// FUNCTION: STUDIO 0x005d8200
	void Player::setUnder13(bool value)
	{
		if (value != under13)
			under13 = value;
	}

	// FUNCTION: PLAYER 0x00527760
	// FUNCTION: STUDIO 0x005d82d0
	void Player::setHasGroupBuildTools(bool value)
	{
		if (value != hasGroupBuildTools)
		{
			hasGroupBuildTools = value;
			raisePropertyChanged(prop_HasBuildTools);
		}
	}

	// FUNCTION: PLAYER 0x00527790
	// FUNCTION: STUDIO 0x005d8300
	void Player::setCanLoadCharacterAppearance(bool value)
	{
		if (value != canLoadCharacterAppearance)
		{
			canLoadCharacterAppearance = value;
			raisePropertyChanged(prop_CanLoadCharacterAppearance);
		}
	}

	// FUNCTION: PLAYER 0x00527800
	// FUNCTION: STUDIO 0x005d8370
	void Player::setMembershipType(MembershipType value)
	{
		if (membershipType != value)
		{
			membershipType = value;
			raisePropertyChanged(prop_MembershipTypeReplicate);
			raisePropertyChanged(prop_MembershipType);
		}
	}

	// FUNCTION: PLAYER 0x00527830
	// FUNCTION: STUDIO 0x005d83a0
	void Player::setAccountAge(int value)
	{
		if (accountAge != value)
		{
			accountAge = value;
			raisePropertyChanged(prop_AccountAgeReplicate);
			raisePropertyChanged(prop_AccountAge);
		}
	}

	// FUNCTION: PLAYER 0x00527890
	// FUNCTION: STUDIO 0x005d8410
	void Player::setForceEarlySpawnLocationCalculation()
	{
		forceEarlySpawnLocationCalculation = true;
	}

	// FUNCTION: PLAYER 0x005278a0
	// FUNCTION: STUDIO 0x005d8420
	bool Player::calculatesSpawnLocationEarly() const
	{
		return FFlag::CalculatePlayerSpawnLocationAsEarlyAsPossible || forceEarlySpawnLocationCalculation;
	}

	// FUNCTION: PLAYER 0x0052e510
	// FUNCTION: STUDIO 0x005de5a0
	void Player::setSimulationRadius(float value)
	{
		value = G3D::clamp(value, 10.0f, maxSimulationRadius);
		if (value != simulationRadius)
			prop_SimulationRadius.setValue(this, value);
	}

	// FUNCTION: PLAYER 0x0052e570
	// FUNCTION: STUDIO 0x005de600
	void Player::setMaxSimulationRadius(float value)
	{
		value = G3D::clamp(value, 10.0f, 1000.0f);
		if (value != maxSimulationRadius)
		{
			prop_MaxSimulationRadius.setValue(this, value);
			if (simulationRadius > value)
				setSimulationRadius(value);
		}
	}

	// STUB: PLAYER 0x00535090
	// STUB: STUDIO 0x005e4c10
	void Player::setTeamColor(BrickColor value)
	{
		if (value != teamColor)
		{
			teamColor = value;
			if (calculatesSpawnLocationEarly() && teamStatusChangedCallback)
				teamStatusChangedCallback();
			raisePropertyChanged(prop_teamColor);
		}
	}

	// STUB: PLAYER 0x005350e0
	// STUB: STUDIO 0x005e4c60
	void Player::setNeutral(bool value)
	{
		if (value != neutral)
		{
			neutral = value;
			if (calculatesSpawnLocationEarly() && teamStatusChangedCallback)
				teamStatusChangedCallback();
			raisePropertyChanged(prop_Neutral);
		}
	}

	// FUNCTION: PLAYER 0x0053ac90
	// FUNCTION: STUDIO 0x005e9d00
	void Player::setName(const std::string& value)
	{
		Security::Context::current().requirePermission(Security::LocalUser);
		Instance::setName(value);
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

// VC9 emits these after the EnumDesc ctors.
namespace RBX {
namespace Network {

	// STUB: PLAYER 0x00553710
	// STUB: STUDIO 0x005ff990
	void Player::setCharacter(ModelInstance* value)
	{
		STUB(0x00553710);
	}

	// STUB: PLAYER 0x00553b40
	// STUB: STUDIO 0x005ffdc0
	void Player::removeCharacter()
	{
		if (!Players::backendProcessing(this))
			throw std::runtime_error("RemoveCharacter can only be called by the backend server");

		FASTLOG(FLog::Network, "Player:removeCharacter");
		setCharacter(NULL);
	}

} // namespace Network
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
