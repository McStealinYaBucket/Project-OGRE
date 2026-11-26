/* V8DataModel/GameBasicSettings.h -- October 2013.
   "UserGameSettings", a GlobalBasicSettingsItem (RTTI .?AVGameBasicSettings@RBX@@ on
   FactoryProduct<GameBasicSettings, GlobalBasicSettings::Item, sGameBasicSettings>). Its code sits in
   GameSettings.obj, after GameSettings (PLAYER 0x006f61f0..0x006f9050); no 2013 __FILE__ names a
   GameBasicSettings file, so the class is written in v8datamodel/GameSettings.cpp.

   Layout from the ctor (PLAYER 0x006f8e70, STUDIO 0x00794e10; size 0x90 from create 0x0043b040).
   Member names from the property setters and their descriptors:
     +0x6c ControlMode      setter 0x006f6200 ("ControlMode")
     +0x70 SavedQualityLevel setter 0x006f6220
     +0x74 mouseLocked      setMouseLock 0x006f6240 (no signal)
     +0x78 VideoUploadPromptBehavior setter 0x006f6250
     +0x7c fullscreen       "Fullscreen" property: inline getFullScreen/setFullScreen (the /GL shell's
                            copies 0x004659c0/0x004659d0 are what the descriptor stores);
                            "InFullScreen" function: inFullScreen() 0x006f61f0, out of line
     +0x8c AllTutorialsDisabled setter 0x006f6270
   Which accessors are inline follows the ios 1029 symbols and the descriptor initializers
   (0x00a6fac0..0x00a6fe40): a getter the descriptors take at a folded address (ControlMode
   0x0062d2a0, SavedQualityLevel 0x0062d290, AllTutorialsDisabled 0x00669200, InStudioMode
   0x0069fe60) or at a shell copy (VideoUploadPromptBehavior 0x004381a0) is inline.
   +0x80 is a std::map whose node has _Isnil at +0x2d, so its value is 0x20 bytes: map<string, bool>.
   2013 has only two ControlModes (EnumDesc ctor 0x006f7c20). */

#pragma once

#include "V8DataModel/GlobalSettings.h"
#include "V8DataModel/GameSettings.h"
#include "rbx/signal.h"

#include <map>
#include <string>

namespace RBX {

	extern const char* const sGameBasicSettings;

	// SIZE 0x90
	class GameBasicSettings
		: public GlobalBasicSettingsItem<GameBasicSettings, sGameBasicSettings>
	{
	public:
		enum ControlMode
		{
			CONTROL_CLASSIC = 0,
			CONTROL_MOUSELOCK = 1,
		};

		enum RenderQualitySetting
		{
			QUALITY_AUTO = 0,
			QUALITY_1 = 1,
			QUALITY_2 = 2,
			QUALITY_3 = 3,
			QUALITY_4 = 4,
			QUALITY_5 = 5,
			QUALITY_6 = 6,
			QUALITY_7 = 7,
			QUALITY_8 = 8,
			QUALITY_9 = 9,
			QUALITY_10 = 10,
		};

		GameBasicSettings();

		void reset();

		ControlMode getControlMode() const { return controlMode; }
		void setControlMode(ControlMode value);
		RenderQualitySetting getRenderQuality() const { return renderQualitySetting; }
		void setRenderQuality(RenderQualitySetting value);
		void setMouseLock(bool value);
		GameSettings::UploadSetting getUploadVideoSetting() const { return uploadVideos; }
		void setUploadVideoSetting(GameSettings::UploadSetting value);
		bool getAllTutorialsDisabled() const { return allTutorialsDisabled; }
		void setAllTutorialsDisabled(bool value);

		bool getTutorialState(std::string tutorialId);
		void setTutorialState(std::string tutorialId, bool value);
		std::string getCompletedTutorials() const;
		void setCompletedTutorials(std::string value);

		bool inFullScreen();
		bool inStudioMode() { return studio; }

		bool getFullScreen() const { return fullscreen; }
		// Inline: the shell's View::SetFullscreen (PLAYER 0x00467880) ends with it. The signal
		// fires before the store (PLAYER 0x004678c1-0x004678ca; shell copy 0x004659d0).
		void setFullScreen(bool value)
		{
			if (value != fullscreen)
			{
				fullscreenChangedSignal(value);
				fullscreen = value;
			}
		}

		rbx::signal<void(bool)> fullscreenChangedSignal;	// 0x64  ctor 0x0055ca50
		rbx::signal<void(bool)> studioModeChangedSignal;	// 0x68

	private:
		ControlMode controlMode;						// 0x6c  CONTROL_CLASSIC
		RenderQualitySetting renderQualitySetting;		// 0x70  QUALITY_AUTO
		bool mouseLocked;								// 0x74  false
		bool canMousePan;								// 0x75  true
		bool freeLook;									// 0x76  false
		GameSettings::UploadSetting uploadVideos;		// 0x78  ASK
		bool fullscreen;								// 0x7c  false
		bool studio;									// 0x7d  false
		std::map<std::string, bool> tutorialState;		// 0x80
		bool allTutorialsDisabled;						// 0x8c  false
	};

} // namespace RBX
