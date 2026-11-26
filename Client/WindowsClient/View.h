/* WindowsClient/View.h -- October 2013 client shell (Client\WindowsClient\View.cpp, assert `false`
   line 739). The game window's view. Declared so far: what the verbs read and the fullscreen /
   resolution code (PLAYER 0x00467410..0x004682b7), HandleWindowsMessage and ShowWindow (View.cpp).
     hWnd       +0xc   SetFullscreen passes it to GetWindowLongA, LeaveGameVerb::doIt (0x004382d0)
                       to PostMessageA;
     +0x45..+0x47      fullscreen, desireFullscreen, changedResolution (SetFullscreen, the
                       resolution functions; ToggleFullscreenVerb::isChecked 0x004397e0 reads +0x45);
     +0x48             fullScreenSizes (initializeSizes assigns getFullscreenSizes() to it);
     +0x58             a RECT, 2013 only: changeResolution fills it with GetWindowRect and
                       restoreResolution puts the window back with SetWindowPos when
                       FFlag::UseWindowPlacementToRememberNonFullscreenedState is off;
     +0x68             nonFullscreenPlacement (WINDOWPLACEMENT, length 0x2c), used when it is on;
     +0x94 restoreWindowStyle, +0x98 changingResolution, +0x9c hMonitor;
     gfx view   +0xa0  the ViewBase* Document hands to RecordToggleVerb (caller 0x0041de18).
     userInput  +0xb0  HandleWindowsMessage (0x00466510) forwards other messages to it
                       (UserInput 0x0045f1b0, `this` pushed, the message in ecx).
   Names from the 2016 View.h where the 2013 code uses the member the same way.
   2016's View starts with shared_ptr<Game> and an OSContext; 2013's first 0xc bytes are not proven.
   CloseWindow is inlined into LeaveGameVerb::doIt; it is written inline here. */

#pragma once

#include <windows.h>

#include <vector>

#include "G3D/Vector2int16.h"

#include <boost/scoped_ptr.hpp>

#include "decomp.h"

namespace RBX {

	// WindowsClient/UserInput.cpp is not written; only the call View makes is declared.
	class UserInput
	{
	public:
		void PostUserInputMessage(UINT uMsg, WPARAM wParam, LPARAM lParam); // PLAYER 0x0045f1b0
	};

	class View
	{
		undefined m_unk0x00[0x0c - 0x00];             // 0x00
		HWND hWnd;                                    // 0x0c
		undefined m_unk0x10[0x45 - 0x10];             // 0x10
		bool fullscreen;                              // 0x45
		bool desireFullscreen;                        // 0x46
		bool changedResolution;                       // 0x47
		std::vector<G3D::Vector2int16> fullScreenSizes; // 0x48
		RECT nonFullscreenRect;                       // 0x58
		WINDOWPLACEMENT nonFullscreenPlacement;       // 0x68
		DWORD restoreWindowStyle;                     // 0x94
		bool changingResolution;                      // 0x98
		HMONITOR hMonitor;                            // 0x9c
		undefined m_unk0xa0[0xb0 - 0xa0];             // 0xa0
		boost::scoped_ptr<UserInput> userInput;       // 0xb0 HandleWindowsMessage forwards to it

		void modifyWindow(DWORD argMask, const RECT& area);
		bool findBestMonitorMatch(LPCTSTR szDevice, int desiredX, int desiredY, bool resolutionAuto, DEVMODE& dmBest);
		void changeResolution();
		void restoreResolution();
		G3D::Vector2int16 calcDefaultResolution(float aspect_XdivY);
		G3D::Vector2int16 getCurrentDesktopResolution();
		std::vector<G3D::Vector2int16> getFullscreenSizes();
		void initializeSizes();

	public:
		HWND GetHWnd() const { return hWnd; }
		void CloseWindow() { ::PostMessage(GetHWnd(), WM_CLOSE, 0, 0); }
		bool IsFullscreen() const { return fullscreen; }
		void SetFullscreen(bool value);
		void HandleWindowsMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		void ShowWindow();
	};

} // namespace RBX
