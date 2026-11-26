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
   63%. AboutToShutdown takes `this` in eax once WinMain's catch is its second caller. WndProc keeps
   the message in ecx, as 2013 does, only since UserInput::PostUserInputMessage 0x0045f1b0 is a shell
   (LTCG) body: `this` on the stack, uMsg in ecx.
   The ctor (0x00403000) differs from 2013 only in that LTCG inlines rbx::atomic<int>() where 2013
   calls the shell's out-of-line copy 0x004022d0 (which also swaps ebx/edi throughout). */

#pragma once

#include "../Win/DumpErrorUploader.h"
#include "../Win/LogManager.h"

#include <windows.h>
#include <shellapi.h>
#include <atlsync.h>

#include "rbx/atomic.h"

#include <stdexcept>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/program_options/variables_map.hpp>

#include "Document.h"
#include "Teleporter.h"
#include "View.h"

#include "decomp.h"

class CProcessPerfCounter;

namespace RBX {

	class ProfanityFilter;

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
		// Member names are 2016's where the ctor (PLAYER 0x00403000) initialises the same slot the same
		// way. 2013 has no mainWindow, mainView, webView, verbs or analyticsPoints, one thread fewer
		// at 0x124/0x128, and the Document at 0x24 (Shutdown 0x00404f10 deletes it).
		rbx::atomic<int> enteredShutdown;                  // 0x00 AboutToShutdown: compare_and_swap(1, 0)
		HANDLE processLocal_stopPreventMultipleJobsThread; // 0x04 AboutToShutdown: SetEvent when set
		HANDLE processLocal_stopWaitForVideoPrerollThread; // 0x08 Shutdown: SetEvent when set
		HANDLE mapFileForWnd;                              // 0x0c
		LPCTSTR bufForWnd;                                 // 0x10
		LONG stopLogsCleanup;                              // 0x14
		ATL::CEvent clenupFinishedEvent;                   // 0x18 ctor 0x0041b060 (LTCG: `this` pushed)
		boost::scoped_ptr<boost::thread> logsCleanUpThread; // 0x1c on logsCleanUpHelper
		undefined4 m_unk0x20;                              // 0x20 zeroed (2016's launchMode slot)

	public:
		boost::scoped_ptr<Document> m_unk0x24;             // 0x24

	private:
		boost::program_options::variables_map vm;          // 0x28 ctor 0x0047b790
		std::string moduleFilename;                        // 0x54
		std::string globalBasicSettingsPath;               // 0x70
		std::string waitEventName;                         // 0x8c
		bool crashReportEnabled;                           // 0xa8
		bool hideChat;                                     // 0xa9
		MainLogManager logManager;                         // 0xac ctor 0x0043fa50 (`this` in esi)
		boost::scoped_ptr<DumpErrorUploader> dumpErrorUploader;     // 0x110
		boost::shared_ptr<CProcessPerfCounter> processPerfCounter;  // 0x114
		boost::shared_ptr<ProfanityFilter> profanityFilter;         // 0x11c
		boost::scoped_ptr<boost::thread> m_unk0x124;       // 0x124 AboutToShutdown joins and resets it
		boost::scoped_ptr<boost::thread> m_unk0x128;       // 0x128 Shutdown joins and resets it
		Teleporter teleporter;                             // 0x12c
		FunctionMarshaller* marshaller;                    // 0x138 Shutdown releases it
		bool spoofMD5;                                     // 0x13c

		void logsCleanUpHelper(); // PLAYER 0x004034c0

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
