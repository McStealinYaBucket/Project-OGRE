/* V8DataModel/GameSettings.h -- October 2013 STUB (sound gate landed).
   Evidence: GlobalAdvancedSettingsItem<GameSettings, sGameSettings>::
   singleton() + 0x6C (preloadSound 0x2449EC reads that byte; the 2016-carry
   property routes through the same settings item).  The sound gate defaults
   to true (preloadSound runs when the gate is set).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GameSettings.h */

#pragma once

#include "V8DataModel/GlobalAdvancedSettings.h"

namespace RBX {

	extern const char* const sGameSettings;

	// 2013 base: GlobalAdvancedSettingsItem<GameSettings, sGameSettings> (ctor PLAYER 0x006f5e10).
	// Layout from the 2013 ctor 0x006f6030 (instance name "Game Options"), size 0x94 (Creator::create
	// 0x0043afb0). Member order and defaults agree with the 2016 header (no overscan yet), so the
	// unused members carry its names; the four the tree reads keep their earlier names.
	// SIZE 0x94
	class GameSettings
		: public GlobalAdvancedSettingsItem<GameSettings, sGameSettings>
	{
	public:
		enum UploadSetting
		{
			NEVER = 0,
			ASK = 1,
			ALWAYS = 2,
		};

		enum VideoQuality
		{
			LOW_RES = 0,
			MEDIUM_RES = 1,
			HIGH_RES = 2,
		};

		GameSettings();

		int chatHistory;				// 0x64  100
		int reportAbuseChatHistory;		// 0x68  50
		// ChatOutput::onPlayerChatMessage's active-lines cap (Mac +0x68). 2016: chatScrollLength.
		int chatLinesShown;				// 0x6c  5
		// preloadSound's gate (Mac +0x6c).
		bool soundEnabled;				// 0x70  true
		bool getSoundEnabled() const { return soundEnabled; }
		// Sound::get: 0 -> mode 32, else 64 (Mac +0x6d). 2016: softwareSound.
		bool highQualitySound;			// 0x71  false
		bool collisionSoundEnabled;		// 0x72  true
		float collisionSoundVolume;		// 0x74  10.0f
		int maxCollisionSounds;			// 0x78  -1
		// ChatOutput::removeExpired's cap, clamped to 10 (Mac +0x78). 2016: bubbleChatMaxBubbles.
		int chatLinesMax;				// 0x7c  3
		float bubbleChatLifetime;		// 0x80  30.0f
		rbx::signal<void(bool)> videoRecordingSignal;	// 0x84  ctor 0x0055ca50
		bool hardwareMouse;				// 0x88  false
		bool videoCaptureEnabled;		// 0x89  true
		VideoQuality videoQuality;		// 0x8c  MEDIUM_RES
		UploadSetting postImageSetting;	// 0x90  ASK
	};

} // namespace RBX

// The FactoryProduct base's out-of-line members; their names spell the 2013 base
// GlobalAdvancedSettings::Item (RTTI), so they pair through these markers.
// clang-format off
// TEMPLATE: PLAYER 0x00439f70
// TEMPLATE: STUDIO 0x004769d0
// ?getClassName@Creator@?$FactoryProduct@VGameSettings@RBX@@VItem@GlobalAdvancedSettings@2@$E?sGameSettings@2@3QBDBVInstance@2@@RBX@@QBEABVName@3@XZ

// TEMPLATE: PLAYER 0x006f4960
// TEMPLATE: STUDIO 0x00791eb0
// ?static_getCreator@?$FactoryProduct@VGameSettings@RBX@@VItem@GlobalAdvancedSettings@2@$E?sGameSettings@2@3QBDBVInstance@2@@RBX@@KAABVCreator@12@XZ

// TEMPLATE: PLAYER 0x006f4ab0
// TEMPLATE: STUDIO 0x00792020
// ?getClassName@?$FactoryProduct@VGameSettings@RBX@@VItem@GlobalAdvancedSettings@2@$E?sGameSettings@2@3QBDBVInstance@2@@RBX@@UBEABVName@2@XZ
// clang-format on
