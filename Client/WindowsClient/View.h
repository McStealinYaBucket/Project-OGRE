/* WindowsClient/View.h -- October 2013 client shell (Client\WindowsClient\View.cpp, assert `false`
   line 739). The game window's view. Only what the verbs read is declared so far:
     hWnd       +0xc   SetFullscreen (PLAYER 0x00467880) passes it to GetWindowLongA,
                       LeaveGameVerb::doIt (0x004382d0) to PostMessageA;
     gfx view   +0xa0  the ViewBase* Document hands to RecordToggleVerb (caller 0x0041de18);
     fullscreen +0x45  ToggleFullscreenVerb::isChecked (0x004397e0) reads it inline;
     desireFullscreen +0x46, restoreWindowStyle +0x94  SetFullscreen 0x00467880 (the shell passes
                       `this` in eax, the flag in bl). Not written: its tail is
                       GameBasicSettings::singleton().setFullScreen(value) (singleton 0x00439bf0,
                       fullscreen +0x7c, signal +0x64), which App does not have yet, and without it
                       LTCG inlines the body into ToggleFullscreenVerb::doIt (measured: 64.9 -> 58.2).
   2016's View starts with shared_ptr<Game> and an OSContext; 2013's first 0xc bytes are not proven.
   CloseWindow is inlined into LeaveGameVerb::doIt; it is written inline here. */

#pragma once

#include <windows.h>

#include "decomp.h"

namespace RBX {

	class View
	{
		undefined m_unk0x00[0x0c - 0x00]; // 0x00
		HWND hWnd;                        // 0x0c
		undefined m_unk0x10[0x45 - 0x10]; // 0x10
		bool fullscreen;                  // 0x45
		bool desireFullscreen;            // 0x46
		undefined m_unk0x47[0x94 - 0x47]; // 0x47
		DWORD restoreWindowStyle;         // 0x94

		void changeResolution();          // PLAYER 0x00467ae0, not written (`this` on the stack)
		void restoreResolution();         // PLAYER 0x004676a0, not written (`this` in edi)
	public:
		HWND GetHWnd() const { return hWnd; }
		void CloseWindow() { ::PostMessage(GetHWnd(), WM_CLOSE, 0, 0); }
		bool IsFullscreen() const { return fullscreen; }
		void SetFullscreen(bool value);
	};

} // namespace RBX
