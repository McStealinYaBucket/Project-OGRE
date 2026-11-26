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
   63%. With one caller, LTCG also moves appPtr into AboutToShutdown (2013 passes it in eax: more
   callers there). */

#pragma once

#include <windows.h>
#include <shellapi.h>

#include "rbx/atomic.h"

#include "Document.h"
#include "View.h"

#include "decomp.h"

namespace RBX {

	class Application
	{
		rbx::atomic<int> enteredShutdown;  // 0x00 AboutToShutdown: compare_and_swap(1, 0)
		HANDLE m_unk0x04;                  // 0x04 AboutToShutdown: SetEvent when set
		undefined m_unk0x08[0x24 - 0x08]; // 0x08

	public:
		Document* m_unk0x24;              // 0x24

	public:
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
