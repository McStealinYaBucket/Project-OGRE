/* WindowsClient/Application.cpp -- October 2013 client shell (PLAYER only; path from the 2016 tree, see
   Application.h). Only the functions main.cpp's WndProc calls are here, all still STUB.
   __declspec(noinline) on the stubs is a stand-in until they are written: LTCG otherwise inlines the
   partial bodies into their callers, which 2013 calls. */

#include "../ClientBase/RenderSettingsItem.h"
#include "V8DataModel/Game.h"
#include "V8DataModel/GlobalSettings.h"

#include "Application.h"
#include "FunctionMarshaller.h"

#include <algorithm>

#include <boost/bind.hpp>

DECOMP_SIZE_ASSERT(RBX::Application, 0x140);

namespace RBX {

	// Against 2016: no mainWindow store. LTCG: `this` on the stack (ret 4); MainLogManager's ctor
	// takes its constant arguments folded in (`this` in esi only). 2013 calls rbx::atomic<int>()
	// out of line (0x004022d0): that needs OGRE_ATOMIC_CTOR_NOINLINE defined for every shell TU (one
	// TU alone does not do it, LTCG keeps the other TUs' inline definition); then this scores 96.7%,
	// the rest being three callees that pair once the CSVs are regenerated (100% in sweep26).
	// FUNCTION: PLAYER 0x00403000
	Application::Application()
		: processLocal_stopPreventMultipleJobsThread(NULL)
		, processLocal_stopWaitForVideoPrerollThread(NULL)
		, mapFileForWnd(NULL)
		, bufForWnd(NULL)
		, stopLogsCleanup(0)
		, clenupFinishedEvent(FALSE, FALSE)
		, m_unk0x20(0)
		, crashReportEnabled(true)
		, hideChat(false)
		, logManager("Roblox", ".Client.dmp", ".Client.crashevent")
		, marshaller(NULL)
		, spoofMD5(false)
	{
		enteredShutdown = 0;
		logsCleanUpThread.reset(new boost::thread(boost::bind(&Application::logsCleanUpHelper, this)));
	}

	// STUB: PLAYER 0x004032d0
	__declspec(noinline) Application::~Application()
	{
		STUB(0x004032d0);
	}

	// STUB: PLAYER 0x004034c0
	__declspec(noinline) void Application::logsCleanUpHelper()
	{
		STUB(0x004034c0);
	}

	// Against 2016: first a call on the gfx view (RbxOgre, PLAYER 0x00487d50), and both message boxes go
	// to the View's window (2013 has no mainWindow). LTCG: no `this`; it loads appPtr itself (WndProc's
	// ID_UPLOADSESSIONLOGS is the only caller).
	// FUNCTION: PLAYER 0x00403f40
	void Application::UploadSessionLogs()
	{
		if (ViewBase* gfxView = m_unk0x24->m_unk0x00->GetGfxView())
			gfxView->FUN_00487d50();

		if (!logManager.CreateFakeCrashDump())
		{
			MessageBoxA(m_unk0x24->m_unk0x00->GetHWnd(), "Failed to upload logs", "Warning",
				MB_OK | MB_ICONWARNING);
		}
		else
		{
			MessageBoxA(m_unk0x24->m_unk0x00->GetHWnd(),
				"Logfiles will be uploaded next time you start Roblox. Please restart roblox now.", "ROBLOX",
				MB_OK);
		}
	}

	// Against 2016, 2013 also sets ptMaxTrackSize, from CRenderSettings+0x2e, and clamps the size to
	// the work area with std::min, not std::max (`cmp work,s.x; jl` picks the work extent's slot).
	// LTCG: lpMMI in esi.
	// FUNCTION: PLAYER 0x00403fa0
	void Application::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
	{
		// don't allow sizing smaller than the non-client area
		G3D::Vector2int16 s = CRenderSettingsItem::singleton().getWindowSize();

		// re-clamp size to the current desktop work area
		int width = 0, height = 0;
		{
			RECT maxWorkArea;
			if (SystemParametersInfo(SPI_GETWORKAREA, 0, &maxWorkArea, 0))
			{
				width = maxWorkArea.right - maxWorkArea.left;
				height = maxWorkArea.bottom - maxWorkArea.top;
				s.x = std::min((int)s.x, (int)(maxWorkArea.right - maxWorkArea.left));
				s.y = std::min((int)s.y, (int)(maxWorkArea.bottom - maxWorkArea.top));
			}
		}

		lpMMI->ptMaxSize.x = s.x;
		lpMMI->ptMaxSize.y = s.y;

		G3D::Vector2int16 maxTrackSize = CRenderSettingsItem::singleton().getUnk0x2e();
		lpMMI->ptMaxTrackSize.x = maxTrackSize.x;
		lpMMI->ptMaxTrackSize.y = maxTrackSize.y;

		// Don't allow window to go below a certain size while playing
		lpMMI->ptMinTrackSize.x = CRenderSettings::minGameWindowSize.x;
		lpMMI->ptMinTrackSize.y = CRenderSettings::minGameWindowSize.y;

		// Center window on a screen
		lpMMI->ptMaxPosition.x = (width - lpMMI->ptMaxSize.x) / 2;
		lpMMI->ptMaxPosition.y = (height - lpMMI->ptMaxSize.y) / 2;
	}

	// STUB: PLAYER 0x00404150
	__declspec(noinline) bool Application::Initialize(HWND hWnd)
	{
		STUB(0x00404150);
		// Stand-in for the 2013 body's initialization_error (View::initializeView): without a throw,
		// LTCG drops WinMain's catch and its second AboutToShutdown call.
		if (!hWnd)
			throw initialization_error("");
		return true;
	}

	// STUB: PLAYER 0x004049b0
	__declspec(noinline) bool Application::LoadAppSettings(HINSTANCE hInstance)
	{
		STUB(0x004049b0);
		// Stand-in use of the argument: a stub that ignores it makes LTCG drop it at WinMain's call.
		return hInstance != NULL;
	}

	// Against 2016: also sets the event at +0x04, joins and drops the thread at +0x124, and has no
	// Document::PrepareShutdown (null tests on both). `this` in eax: WinMain's catch is a second caller.
	// FUNCTION: PLAYER 0x00404ea0
	void Application::AboutToShutdown()
	{
		enteredShutdown.compare_and_swap(1, 0);

		if (processLocal_stopPreventMultipleJobsThread)
			SetEvent(processLocal_stopPreventMultipleJobsThread);

		if (m_unk0x124)
			m_unk0x124->join();
		m_unk0x124.reset();

		if (Document* document = m_unk0x24.get())
		{
			if (View* view = document->m_unk0x00)
				view->AboutToShutdown();
		}
	}

	// Against 2016: no mainWindow or web view to tear down; the Document is shut down and deleted here,
	// and the marshaller's window released last.
	// STUB: PLAYER 0x00404f10
	void Application::Shutdown()
	{
		if (processLocal_stopWaitForVideoPrerollThread)
			SetEvent(processLocal_stopWaitForVideoPrerollThread);
		if (m_unk0x128)
			m_unk0x128->join();
		m_unk0x128.reset();

		m_unk0x24->Shutdown();
		m_unk0x24.reset();

		GlobalBasicSettings::singleton()->saveState();
		GlobalAdvancedSettings::singleton()->saveState();

		Game::globalExit();

		if (marshaller)
			FunctionMarshaller::ReleaseWindow(marshaller);
	}

	// STUB: PLAYER 0x00405070
	__declspec(noinline) bool Application::ParseArguments(LPTSTR lpCmdLine)
	{
		STUB(0x00405070);
		return lpCmdLine != NULL; // stand-in use of the argument, as in LoadAppSettings
	}

} // namespace RBX
