/* WindowsClient/Application.cpp -- October 2013 client shell (PLAYER only; path from the 2016 tree, see
   Application.h). Only the functions main.cpp's WndProc calls are here, all still STUB.
   __declspec(noinline) on the stubs is a stand-in until they are written: LTCG otherwise inlines the
   partial bodies into their callers, which 2013 calls. */

#include "../ClientBase/RenderSettingsItem.h"

#include "Application.h"

#include <algorithm>

DECOMP_SIZE_ASSERT(RBX::Application, 0x140);

namespace RBX {

	// Starts the members (rbx::atomic ctor 0x004022d0 out of line, three strings, the MainLogManager at
	// +0xac) and a boost::thread on a member function (0x004034c0) into the scoped_ptr at +0x1c.
	// STUB: PLAYER 0x00403000
	__declspec(noinline) Application::Application()
	{
		STUB(0x00403000);
	}

	// STUB: PLAYER 0x004032d0
	__declspec(noinline) Application::~Application()
	{
		STUB(0x004032d0);
	}

	// Logs a fake crash dump through the MainLogManager at +0xac and tells the user (MessageBox on the
	// View's window); first a call on the gfx view (PLAYER 0x00487d50, RbxOgre). Loads appPtr itself.
	// STUB: PLAYER 0x00403f40
	__declspec(noinline) void Application::UploadSessionLogs()
	{
		STUB(0x00403f40);
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
		return true;
	}

	// Against 2016: also sets the event at +0x04, joins and drops the thread at +0x124, and has no
	// Document::PrepareShutdown (null tests on both). `this` in eax: WinMain's catch is a second caller.
	// FUNCTION: PLAYER 0x00404ea0
	void Application::AboutToShutdown()
	{
		enteredShutdown.compare_and_swap(1, 0);

		if (m_unk0x04)
			SetEvent(m_unk0x04);

		if (m_unk0x124)
			m_unk0x124->join();
		m_unk0x124.reset();

		if (Document* document = m_unk0x24)
		{
			if (View* view = document->m_unk0x00)
				view->AboutToShutdown();
		}
	}

	// STUB: PLAYER 0x00404f10
	__declspec(noinline) void Application::Shutdown()
	{
		STUB(0x00404f10);
	}

	// STUB: PLAYER 0x00405070
	__declspec(noinline) bool Application::ParseArguments(LPTSTR lpCmdLine)
	{
		STUB(0x00405070);
		return true;
	}

} // namespace RBX
