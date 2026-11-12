/* Copyright 2003-2005 ROBLOX Corporation, All Rights Reserved */

/* Util/SoundWorld.h — October 2013 enum fixed against
   EnumDesc<SoundType>::C2 0x24D14A (batch782): the 15-entry addPair
   dictionary runs pairs 0..14; the value-6 name string is "Slingshot".
   Names/values otherwise match the vendored 2016-carry.  Fidelity ~99%:
   value set fixed; helper return values unchanged (binary TODO). */

#pragma once

namespace RBX
{
	// Values 0..14 per EnumDesc<SoundType>::C2 0x24D14A.
	typedef enum SoundType {	NO_SOUND = 0,
								BOING_SOUND,
								BOMB_SOUND,
								BREAK_SOUND,
								CLICK_SOUND,
								CLOCK_SOUND,
								SLINGSHOT_SOUND,	// value 6: "Slingshot" (the
													// 2016-carry name
													// RUBBERBAND_SOUND was wrong)
								PAGE_SOUND,
								PING_SOUND,
								SNAP_SOUND,
								SPLAT_SOUND,
								STEP_SOUND,
								STEP_ON_SOUND,
								SWOOSH_SOUND,
								VICTORY_SOUND
								} SoundType;


	class SoundWorld {
	public:

		static SoundType ActionSound() {return PING_SOUND;}
		static SoundType TrashSound() {return PAGE_SOUND;}
		static SoundType ClickSound() {return CLICK_SOUND;}
		static SoundType SplatSound() {return SPLAT_SOUND;}
		static SoundType StepSound() {return STEP_SOUND;}
		static SoundType StepOnSound() {return STEP_ON_SOUND;}
		static SoundType SwooshSound() {return SWOOSH_SOUND;}
		static SoundType LimitSound() {return BOING_SOUND;}
		static SoundType WinSound() {return VICTORY_SOUND;}
		static SoundType LoseSound() {return BOING_SOUND;}
	};
}
