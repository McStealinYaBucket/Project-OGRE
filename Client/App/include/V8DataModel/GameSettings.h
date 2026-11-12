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

	class GameSettings
		: public DescribedCreatable<GameSettings, GlobalAdvancedSettings, sGameSettings>
	{
	public:
		GameSettings()
			: soundEnabled(true),
			  highQualitySound(false),
			  chatLinesMax(10),
			  chatLinesShown(10)
		{
		}

		// preloadSound 0x2449EC gate: +0x6C in the binary; landed by name
		// (LAYOUT DELTA -- the binary routes through GlobalAdvancedSettingsItem).
		bool soundEnabled;
		bool getSoundEnabled() const { return soundEnabled; }

		// Sound::get 0x244C78-area reads +109 (0x6D): when 0 -> mode 32,
		// else mode 64.  BSS-zero default (false -> mode 32).
		bool highQualitySound;

		// ChatOutput::removeExpired 0x5C2FD4 reads +120 (0x78) and clamps
		// to 10 (cmovle arm 0x5C2FDC): the per-character ChatLine deque
		// pop cap.  Name unresolved in dbq24; DELTA: behavior-pinned
		// placeholder, init 10 = the clamp's upper value (the binary
		// delivers the value at run time through the settings item).
		int chatLinesMax;

		// ChatOutput::onPlayerChatMessage 0x5C2B64 reads +104 (0x68): the
		// +236 ACTIVE-lines pop cap (`cmp size, [eax+68h]; ja -> pop` --
		// UNCLAMPED, unlike chatLinesMax +120).  Name unresolved;
		// DELTA: behavior-pinned placeholder, init 10 to keep the same
		// history posture as chatLinesMax (the binary delivers both at
		// run time).
		int chatLinesShown;

		// Same-route singleton for the unported GlobalAdvancedSettingsItem
		// (house pattern per DebugSettings::singleton; DELTA as above).
		static GameSettings& singleton()
		{
			static GameSettings instance;
			return instance;
		}
	};

} // namespace RBX