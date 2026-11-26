/* WindowsClient/Application.h -- October 2013 client shell (PLAYER only). No 2013 __FILE__ string names
   Application.cpp (none of its code asserts), so the path is the 2016 tree's. Declared so far: what
   main.cpp's WndProc (PLAYER 0x004426a0) calls.
     appPtr          PLAYER 0x00e250a0, read by WndProc and by UploadSessionLogs itself;
     +0x24 Document  WndProc reaches the View through it (`[[app+0x24]]`, Document+0 is the View*):
                     no null tests in 2013, against 2016's mainView checks.
   The shell is /GL: AboutToShutdown 0x00404ea0 takes `this` in eax, OnGetMinMaxInfo 0x00403fa0 the
   MINMAXINFO in esi, UploadSessionLogs 0x00403f40 nothing (it loads appPtr). HandleWindowsMessage is
   inlined into WndProc. 2016's OnHelp and OnResize are written out in WndProc: as Application members
   (header inline or in Application.cpp) LTCG keeps them out of line, and WndProc drops from 75% to
   63%. AboutToShutdown takes `this` in eax once WinMain's catch is its second caller. WndProc
   still keeps the message in eax where 2013 has ecx: 2013's UserInput::PostUserInputMessage
   0x0045f1b0 (an LTCG function, `this` on the stack, uMsg in ecx) shapes HandleWindowsMessage's
   convention, and ours is a native link stub (thiscall). */

#pragma once

#include <windows.h>
#include <shellapi.h>

#include "rbx/atomic.h"

#include <stdexcept>

#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>

#include "Document.h"
#include "View.h"

#include "decomp.h"

namespace RBX {

	// Thrown by Application::Initialize; WinMain catches it as `const initialization_error&` (FuncInfo
	// 0x00ce3e8c, RTTI .?AVinitialization_error@RBX@@). 2016 has it in InitializationError.h.
	class initialization_error : public std::runtime_error
	{
	public:
		initialization_error(const char* const errorMessage)
			: std::runtime_error(errorMessage)
		{
		}
	};

	// SIZE 0x140 (WinMain 0x00442900: the object at [ebp], its title buffer at [ebp+0x140])
	class Application
	{
		rbx::atomic<int> enteredShutdown;  // 0x00 AboutToShutdown: compare_and_swap(1, 0)
		HANDLE m_unk0x04;                  // 0x04 AboutToShutdown: SetEvent when set
		undefined m_unk0x08[0x24 - 0x08]; // 0x08

	public:
		Document* m_unk0x24;              // 0x24

	private:
		undefined m_unk0x28[0x124 - 0x28]; // 0x28 ctor 0x00403000: strings at +0x54/+0x70/+0x8c, MainLogManager at +0xac
		boost::scoped_ptr<boost::thread> m_unk0x124; // 0x124 AboutToShutdown joins and resets it
		boost::scoped_ptr<boost::thread> m_unk0x128; // 0x128 Shutdown joins and resets it
		undefined m_unk0x12c[0x140 - 0x12c]; // 0x12c

	public:
		Application();  // PLAYER 0x00403000 (LTCG: `this` on the stack, ret 4)
		~Application(); // PLAYER 0x004032d0

		bool LoadAppSettings(HINSTANCE hInstance); // PLAYER 0x004049b0
		bool ParseArguments(LPTSTR lpCmdLine);     // PLAYER 0x00405070
		bool Initialize(HWND hWnd);                // PLAYER 0x00404150 (2013 passes only the window)
		void Shutdown();                           // PLAYER 0x00404f10

		// Notification that Shutdown will be happening soon.
		void AboutToShutdown(); // PLAYER 0x00404ea0

		// Creates a crash dump which results in an upload the next time the process runs.
		void UploadSessionLogs(); // PLAYER 0x00403f40

		static void OnGetMinMaxInfo(MINMAXINFO* lpMMI); // PLAYER 0x00403fa0

		void HandleWindowsMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			m_unk0x24->m_unk0x00->HandleWindowsMessage(uMsg, wParam, lParam);
		}
	};

} // namespace RBX

extern RBX::Application* appPtr;
