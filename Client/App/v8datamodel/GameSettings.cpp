/* App/v8datamodel/GameSettings.cpp -- October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "V8DataModel/GameSettings.h"
#include "V8DataModel/GameBasicSettings.h"
#include "decomp.h"

namespace RBX {
	const char* const sGameSettings = "GameSettings";
	const char* const sGameBasicSettings = "UserGameSettings";
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

namespace RBX {

// FUNCTION: PLAYER 0x006f6030
// FUNCTION: STUDIO 0x00792df0
GameSettings::GameSettings()
	: chatHistory(100)
	, reportAbuseChatHistory(50)
	, chatLinesShown(5)
	, soundEnabled(true)
	, highQualitySound(false)
	, collisionSoundEnabled(true)
	, collisionSoundVolume(10.0f)
	, maxCollisionSounds(-1)
	, chatLinesMax(3)
	, bubbleChatLifetime(30.0f)
	, hardwareMouse(false)
	, videoCaptureEnabled(true)
	, postImageSetting(ASK)
	, videoQuality(MEDIUM_RES)
{
	Instance::setName("Game Options");
}

} // namespace RBX

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x006f7c20
// FUNCTION: STUDIO 0x00793de0
template<>
EnumDesc<GameBasicSettings::ControlMode>::EnumDesc()
	: EnumDescriptor("ControlMode", typeid(GameBasicSettings::ControlMode))
{
	addPair(GameBasicSettings::CONTROL_MOUSELOCK, "MouseLockSwitch");
	addLegacyName("Mouse Lock Switch", GameBasicSettings::CONTROL_MOUSELOCK);
	addPair(GameBasicSettings::CONTROL_CLASSIC, "Classic");
}

template<>
const Type& Type::getSingleton<GameBasicSettings::ControlMode>()
{
	return EnumDesc<GameBasicSettings::ControlMode>::singleton();
}

// FUNCTION: PLAYER 0x006f7d60
// FUNCTION: STUDIO 0x00793f20
template<>
EnumDesc<GameBasicSettings::RenderQualitySetting>::EnumDesc()
	: EnumDescriptor("SavedQualitySetting", typeid(GameBasicSettings::RenderQualitySetting))
{
	addPair(GameBasicSettings::QUALITY_AUTO, "Automatic");
	addPair(GameBasicSettings::QUALITY_1, "QualityLevel1");
	addPair(GameBasicSettings::QUALITY_2, "QualityLevel2");
	addPair(GameBasicSettings::QUALITY_3, "QualityLevel3");
	addPair(GameBasicSettings::QUALITY_4, "QualityLevel4");
	addPair(GameBasicSettings::QUALITY_5, "QualityLevel5");
	addPair(GameBasicSettings::QUALITY_6, "QualityLevel6");
	addPair(GameBasicSettings::QUALITY_7, "QualityLevel7");
	addPair(GameBasicSettings::QUALITY_8, "QualityLevel8");
	addPair(GameBasicSettings::QUALITY_9, "QualityLevel9");
	addPair(GameBasicSettings::QUALITY_10, "QualityLevel10");
}

template<>
const Type& Type::getSingleton<GameBasicSettings::RenderQualitySetting>()
{
	return EnumDesc<GameBasicSettings::RenderQualitySetting>::singleton();
}

} // namespace Reflection
} // namespace RBX

namespace RBX {

// The base GlobalBasicSettingsItem ctor stays out of line (PLAYER 0x006f8550); the instance is
// renamed "GameSettings" after the members are built.
// FUNCTION: PLAYER 0x006f8e70
// FUNCTION: STUDIO 0x00794e10
GameBasicSettings::GameBasicSettings()
	: controlMode(CONTROL_CLASSIC)
	, renderQualitySetting(QUALITY_AUTO)
	, mouseLocked(false)
	, canMousePan(true)
	, freeLook(false)
	, uploadVideos(GameSettings::ASK)
	, fullscreen(false)
	, studio(false)
	, allTutorialsDisabled(false)
{
	Instance::setName("GameSettings");
}

} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::GameSettings, 0x94);
DECOMP_SIZE_ASSERT(RBX::GameBasicSettings, 0x90);
