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
#include <boost/shared_ptr.hpp>

#include "rbx/TaskScheduler.h"

#include "UserInput.h"

#include "decomp.h"

namespace RBX {

	// GfxBase's ViewBase (Rendering) is not declared in this tree yet; only the slot the shell calls.
	// Delete this when GfxBase/ViewBase.h lands and include that instead.
	class ViewBase
	{
	public:
		virtual void VTable0x00(); // vtable+0x00
		virtual void VTable0x04(); // vtable+0x04
		virtual void VTable0x08(); // vtable+0x08
		virtual void VTable0x0c(); // vtable+0x0c
		virtual void VTable0x10(); // vtable+0x10
		virtual void VTable0x14(); // vtable+0x14
		virtual void VTable0x18(); // vtable+0x18
		virtual void VTable0x1c(); // vtable+0x1c
		virtual void VTable0x20(); // vtable+0x20
		virtual void VTable0x24(); // vtable+0x24
		virtual void VTable0x28(); // vtable+0x28
		virtual void VTable0x2c(); // vtable+0x2c
		virtual void onResize(int cx, int cy) = 0; // vtable+0x30 (WndProc's WM_SIZE, PLAYER 0x00442718)

		// A non-virtual RbxOgre call UploadSessionLogs (0x00403f40) makes on the gfx view: unnamed.
		void FUN_00487d50(); // PLAYER 0x00487d50
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
		ViewBase* view;                               // 0xa0 (2016: scoped_ptr<ViewBase>)
		undefined m_unk0xa4[0xa8 - 0xa4];             // 0xa4
		// The three jobs resetScheduler (0x00466460) adds and RemoveJobs (0x00466a30) removes.
		boost::shared_ptr<TaskScheduler::Job> m_unk0xa8; // 0xa8
		boost::scoped_ptr<UserInput> userInput;       // 0xb0 HandleWindowsMessage forwards to it
		undefined m_unk0xb4[0xbc - 0xb4];             // 0xb4
		boost::shared_ptr<TaskScheduler::Job> m_unk0xbc; // 0xbc
		boost::shared_ptr<TaskScheduler::Job> m_unk0xc4; // 0xc4

		void modifyWindow(DWORD argMask, const RECT& area);
		bool findBestMonitorMatch(LPCTSTR szDevice, int desiredX, int desiredY, bool resolutionAuto, DEVMODE& dmBest);
		void changeResolution();
		void restoreResolution();
		G3D::Vector2int16 calcDefaultResolution(float aspect_XdivY);
		G3D::Vector2int16 getCurrentDesktopResolution();
		std::vector<G3D::Vector2int16> getFullscreenSizes();
		void initializeSizes();
		void resetScheduler();

	public:
		HWND GetHWnd() const { return hWnd; }
		ViewBase* GetGfxView() const { return view; }
		void CloseWindow() { ::PostMessage(GetHWnd(), WM_CLOSE, 0, 0); }
		bool IsFullscreen() const { return fullscreen; }
		void SetFullscreen(bool value);
		void HandleWindowsMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		void AboutToShutdown(); // PLAYER 0x00465d60
		void ShowWindow();
	};

} // namespace RBX
