/* WindowsClient/View.h -- October 2013 client shell (Client\WindowsClient\View.cpp, assert `false`
   line 739). The game window's view. Only what the verbs read is declared so far:
     fullscreen +0x45  ToggleFullscreenVerb::isChecked (PLAYER 0x004397e0) reads it inline;
     SetFullscreen     PLAYER 0x00467880 (the shell passes the flag in bl), not written yet.
   The rest of the 2013 layout is not proven. */

#pragma once

#include "decomp.h"

namespace RBX {

	class View
	{
		undefined m_unk0x00[0x45]; // 0x00
		bool fullscreen;           // 0x45
	public:
		bool IsFullscreen() const { return fullscreen; }
		void SetFullscreen(bool value);
	};

} // namespace RBX
