/* App/v8datamodel/GameSettings.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/GameSettings.h"

namespace RBX {
	const char* const sGameSettings = "GameSettings";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::GameSettings);
}

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/enumconverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x006f5120
// FUNCTION: STUDIO 0x00792030
template<>
EnumDesc<GameSettings::VideoQuality>::EnumDesc()
	: EnumDescriptor("VideoQualitySettings", typeid(GameSettings::VideoQuality))
{
	addPair(GameSettings::LOW_RES, "LowResolution");
	addPair(GameSettings::MEDIUM_RES, "MediumResolution");
	addPair(GameSettings::HIGH_RES, "HighResolution");
	addLegacyName("Low Resolution", GameSettings::LOW_RES);
	addLegacyName("Medium Resolution", GameSettings::MEDIUM_RES);
	addLegacyName("High Resolution", GameSettings::HIGH_RES);
}

template<>
const Type& Type::getSingleton<GameSettings::VideoQuality>()
{
	return EnumDesc<GameSettings::VideoQuality>::singleton();
}

template<> EnumRegistrar<GameSettings::VideoQuality> EnumRegistrar<GameSettings::VideoQuality>::registrar(0);
template<> TypeRegistrar<GameSettings::VideoQuality> TypeRegistrar<GameSettings::VideoQuality>::registrar(0);

// FUNCTION: PLAYER 0x006f52c0
// FUNCTION: STUDIO 0x007921d0
template<>
EnumDesc<GameSettings::UploadSetting>::EnumDesc()
	: EnumDescriptor("UploadSetting", typeid(GameSettings::UploadSetting))
{
	addPair(GameSettings::NEVER, "Never");
	addPair(GameSettings::ASK, "Ask");
	addLegacyName("Ask me first", GameSettings::ASK);
	addPair(GameSettings::ALWAYS, "Always");
}

template<>
const Type& Type::getSingleton<GameSettings::UploadSetting>()
{
	return EnumDesc<GameSettings::UploadSetting>::singleton();
}

template<> EnumRegistrar<GameSettings::UploadSetting> EnumRegistrar<GameSettings::UploadSetting>::registrar(0);
template<> TypeRegistrar<GameSettings::UploadSetting> TypeRegistrar<GameSettings::UploadSetting>::registrar(0);

} // namespace Reflection
} // namespace RBX
