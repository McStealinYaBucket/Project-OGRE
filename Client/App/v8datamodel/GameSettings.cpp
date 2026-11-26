/* App/v8datamodel/GameSettings.cpp -- October 2013. Descriptor order, kinds, attributes and
   security from the static initializers PLAYER 0x00a6f690..0x00a6fa50 (no 2013 property here is
   deprecated yet; VideoRecordingChangeRequest is RobloxScript). GameBasicSettings is its own object,
   see GameBasicSettings.cpp. */

#include "stdafx.h"

#include "V8DataModel/GameSettings.h"
#include "decomp.h"

namespace RBX {
	const char* const sGameSettings = "GameSettings";
}

using namespace RBX;

REFLECTION_BEGIN();
static Reflection::BoundProp<int> prop_ChatHistory("ChatHistory", "Online", &GameSettings::chatHistory);
static Reflection::BoundProp<int> prop_ReportAbuseChatHistory("ReportAbuseChatHistory", "Online", &GameSettings::reportAbuseChatHistory);
static Reflection::BoundProp<int> prop_ChatScrollLength("ChatScrollLength", "Online", &GameSettings::chatLinesShown);
static Reflection::BoundProp<bool> prop_SoundEnabled("SoundEnabled", "Sound", &GameSettings::soundEnabled);
static Reflection::BoundProp<bool> prop_SoftwareSound("SoftwareSound", "Sound", &GameSettings::highQualitySound);
static Reflection::BoundProp<bool> prop_CollisionSoundEnabled("CollisionSoundEnabled", "Sound", &GameSettings::collisionSoundEnabled);
static Reflection::BoundProp<float> prop_CollisionSoundVolume("CollisionSoundVolume", "Sound", &GameSettings::collisionSoundVolume);
static Reflection::BoundProp<int> prop_MaxCollisionSounds("MaxCollisionSounds", "Sound", &GameSettings::maxCollisionSounds);
static Reflection::BoundProp<int> prop_bubbleChatMaxBubbles("BubbleChatMaxBubbles", "Online", &GameSettings::chatLinesMax);
static Reflection::BoundProp<float> prop_bubbleChatLifetime("BubbleChatLifetime", "Online", &GameSettings::bubbleChatLifetime);
static Reflection::BoundProp<bool> prop_hardwareMouse("HardwareMouse", "Input", &GameSettings::hardwareMouse);
static const Reflection::EnumPropDescriptor<GameSettings, GameSettings::VideoQuality> prop_videoSettings("VideoQuality", category_Video, &GameSettings::getVideoQualitySetting, &GameSettings::setVideoQualitySetting);
static const Reflection::EnumPropDescriptor<GameSettings, GameSettings::UploadSetting> prop_uploadScreenshots("ImageUploadPromptBehavior", "Screenshots", &GameSettings::getPostImageSetting, &GameSettings::setPostImageSetting);
static Reflection::EventDesc<GameSettings, void(bool)> event_videoRecordingRequest(&GameSettings::videoRecordingSignal, "VideoRecordingChangeRequest", "recording", Security::RobloxScript);
static Reflection::BoundProp<bool> prop_videoCaptureEnabled("VideoCaptureEnabled", category_Video, &GameSettings::videoCaptureEnabled);
REFLECTION_END();

namespace RBX {

	// FUNCTION: PLAYER 0x006f4810
	// FUNCTION: STUDIO 0x00791df0
	void GameSettings::setPostImageSetting(UploadSetting value)
	{
		if (postImageSetting != value)
		{
			postImageSetting = value;
			raisePropertyChanged(prop_uploadScreenshots);
		}
	}

	// FUNCTION: PLAYER 0x006f4840
	// FUNCTION: STUDIO 0x00791e20
	void GameSettings::setVideoQualitySetting(VideoQuality value)
	{
		if (videoQuality != value)
		{
			videoQuality = value;
			raisePropertyChanged(prop_videoSettings);
		}
	}

} // namespace RBX

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

DECOMP_SIZE_ASSERT(RBX::GameSettings, 0x94);
