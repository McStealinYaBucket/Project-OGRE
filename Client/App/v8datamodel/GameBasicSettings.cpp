/* App/v8datamodel/GameBasicSettings.cpp -- October 2013.
   "UserGameSettings". No 2013 __FILE__ names this file (it has no assert), but the static
   initializers say it is its own object: GameSettings.obj's run ends at 0x00a6fa50 and three boost
   category initializers (0x00a6fa90..0x00a6fab0, the start of every TU that includes boost.system)
   open this one. The code runs PLAYER 0x006f61f0..0x006f9050, right after GameSettings'.

   Descriptor order, kinds, attributes and security from the initializers 0x00a6fac0..0x00a6fe40.
   2013 differs from 2016: CompletedTutorials is STREAMING with no security, VideoUploadPromptBehavior
   and AllTutorialsDisabled have no security, and InFullScreen calls inFullScreen() (ios 1029) where
   2016 binds getFullScreen. */

#include "stdafx.h"

#include "V8DataModel/GameBasicSettings.h"
#include "reflection/enumconverter.h"
#include "decomp.h"

#include <sstream>

namespace RBX {
	const char* const sGameBasicSettings = "UserGameSettings";
}

using namespace RBX;

REFLECTION_BEGIN();
static const Reflection::EnumPropDescriptor<GameBasicSettings, GameBasicSettings::ControlMode> prop_controlMode("ControlMode", category_Control, &GameBasicSettings::getControlMode, &GameBasicSettings::setControlMode);
static const Reflection::EnumPropDescriptor<GameBasicSettings, GameSettings::UploadSetting> prop_uploadVideo("VideoUploadPromptBehavior", category_Video, &GameBasicSettings::getUploadVideoSetting, &GameBasicSettings::setUploadVideoSetting);
static const Reflection::EnumPropDescriptor<GameBasicSettings, GameBasicSettings::RenderQualitySetting> prop_renderQuality("SavedQualityLevel", category_Appearance, &GameBasicSettings::getRenderQuality, &GameBasicSettings::setRenderQuality);
static Reflection::BoundFuncDesc<GameBasicSettings, bool(std::string)> func_getTutorialState(&GameBasicSettings::getTutorialState, "GetTutorialState", "tutorialId", Security::RobloxScript);
static Reflection::BoundFuncDesc<GameBasicSettings, void(std::string, bool)> func_setTutorialState(&GameBasicSettings::setTutorialState, "SetTutorialState", "tutorialId", "value", Security::RobloxScript);
static const Reflection::PropDescriptor<GameBasicSettings, std::string> prop_CompletedTutorials("CompletedTutorials", category_Data, &GameBasicSettings::getCompletedTutorials, &GameBasicSettings::setCompletedTutorials, Reflection::PropertyDescriptor::STREAMING);
static const Reflection::PropDescriptor<GameBasicSettings, bool> prop_AllTutorialsDisabled("AllTutorialsDisabled", category_Data, &GameBasicSettings::getAllTutorialsDisabled, &GameBasicSettings::setAllTutorialsDisabled);
static const Reflection::PropDescriptor<GameBasicSettings, bool> prop_isFullscreen("Fullscreen", category_Data, &GameBasicSettings::getFullScreen, &GameBasicSettings::setFullScreen);
static Reflection::BoundFuncDesc<GameBasicSettings, bool()> func_inFullscreenMode(&GameBasicSettings::inFullScreen, "InFullScreen", Security::None);
static Reflection::BoundFuncDesc<GameBasicSettings, bool()> func_inStudioMode(&GameBasicSettings::inStudioMode, "InStudioMode", Security::None);
static Reflection::EventDesc<GameBasicSettings, void(bool)> event_StudioModeChanged(&GameBasicSettings::studioModeChangedSignal, "StudioModeChanged", "isStudioMode", Security::None);
static Reflection::EventDesc<GameBasicSettings, void(bool)> event_FullscreenChanged(&GameBasicSettings::fullscreenChangedSignal, "FullscreenChanged", "isFullscreen", Security::None);
REFLECTION_END();

namespace RBX {

	// FUNCTION: PLAYER 0x006f61f0
	// FUNCTION: STUDIO 0x00792fd0
	bool GameBasicSettings::inFullScreen()
	{
		return fullscreen;
	}

	// FUNCTION: PLAYER 0x006f6200
	// FUNCTION: STUDIO 0x00792fe0
	void GameBasicSettings::setControlMode(ControlMode value)
	{
		if (controlMode != value)
		{
			controlMode = value;
			raisePropertyChanged(prop_controlMode);
		}
	}

	// FUNCTION: PLAYER 0x006f6220
	// FUNCTION: STUDIO 0x00793000
	void GameBasicSettings::setRenderQuality(RenderQualitySetting value)
	{
		if (renderQualitySetting != value)
		{
			renderQualitySetting = value;
			raisePropertyChanged(prop_renderQuality);
		}
	}

	// FUNCTION: PLAYER 0x006f6240
	// FUNCTION: STUDIO 0x00793020
	void GameBasicSettings::setMouseLock(bool value)
	{
		mouseLocked = value;
	}

	// FUNCTION: PLAYER 0x006f6250
	// FUNCTION: STUDIO 0x00793030
	void GameBasicSettings::setUploadVideoSetting(GameSettings::UploadSetting value)
	{
		if (uploadVideos != value)
		{
			uploadVideos = value;
			raisePropertyChanged(prop_uploadVideo);
		}
	}

	// FUNCTION: PLAYER 0x006f6270
	// FUNCTION: STUDIO 0x00793050
	void GameBasicSettings::setAllTutorialsDisabled(bool value)
	{
		if (allTutorialsDisabled != value)
		{
			allTutorialsDisabled = value;
			raisePropertyChanged(prop_AllTutorialsDisabled);
		}
	}

	// FUNCTION: PLAYER 0x006f62a0
	// FUNCTION: STUDIO 0x00793080
	void GameBasicSettings::reset()
	{
		setControlMode(CONTROL_CLASSIC);
		setRenderQuality(QUALITY_AUTO);
		mouseLocked = true;
		canMousePan = true;
		freeLook = false;
	}

	// FUNCTION: PLAYER 0x006f67f0
	// FUNCTION: STUDIO 0x00793440
	std::string GameBasicSettings::getCompletedTutorials() const
	{
		std::ostringstream stream;
		std::map<std::string, bool>::const_iterator end = tutorialState.end();
		for (std::map<std::string, bool>::const_iterator it = tutorialState.begin(); it != end; ++it)
			if (it->second)
				stream << it->first << ",";
		return stream.str();
	}

	// FUNCTION: PLAYER 0x006f6b80
	// FUNCTION: STUDIO 0x007937b0
	bool GameBasicSettings::getTutorialState(std::string tutorialId)
	{
		if (tutorialId.find(',') != std::string::npos)
			throw std::runtime_error("TutorialId's cannot contain commas");
		if (tutorialId.size() < 1)
			throw std::runtime_error("TutorialId's cannot be empty strings");
		if (allTutorialsDisabled)
			return true;
		std::map<std::string, bool>::const_iterator it = tutorialState.find(tutorialId);
		if (it == tutorialState.end())
			return false;
		return it->second;
	}

	// FUNCTION: PLAYER 0x006f7410
	// FUNCTION: STUDIO 0x00793c60
	void GameBasicSettings::setTutorialState(std::string tutorialId, bool value)
	{
		if (allTutorialsDisabled)
			return;
		if (getTutorialState(tutorialId) != value)
		{
			tutorialState[tutorialId] = value;
			raisePropertyChanged(prop_CompletedTutorials);
		}
	}

	// FUNCTION: PLAYER 0x006f74b0
	// FUNCTION: STUDIO 0x00793d00
	void GameBasicSettings::setCompletedTutorials(std::string value)
	{
		size_t start = 0;
		size_t pos = value.find(',');
		while (pos != std::string::npos)
		{
			int length = pos - start;
			if (length > 0)
			{
				std::string tutorialId = value.substr(start, length);
				tutorialState[tutorialId] = true;
			}
			start = pos + 1;
			pos = value.find(',', start);
		}
	}

} // namespace RBX

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names).
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

template<> EnumRegistrar<GameBasicSettings::ControlMode> EnumRegistrar<GameBasicSettings::ControlMode>::registrar(0);
template<> TypeRegistrar<GameBasicSettings::ControlMode> TypeRegistrar<GameBasicSettings::ControlMode>::registrar(0);

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

template<> EnumRegistrar<GameBasicSettings::RenderQualitySetting> EnumRegistrar<GameBasicSettings::RenderQualitySetting>::registrar(0);
template<> TypeRegistrar<GameBasicSettings::RenderQualitySetting> TypeRegistrar<GameBasicSettings::RenderQualitySetting>::registrar(0);

} // namespace Reflection
} // namespace RBX

namespace RBX {

RBX_REGISTER_CLASS(RBX::GameBasicSettings);

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

DECOMP_SIZE_ASSERT(RBX::GameBasicSettings, 0x90);
