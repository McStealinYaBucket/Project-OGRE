/* WindowsClient/Application.cpp -- October 2013 client shell (PLAYER only; path from the 2016 tree, see
   Application.h). Only the functions main.cpp's WndProc calls are here, all still STUB.
   __declspec(noinline) on UploadSessionLogs and AboutToShutdown is a stand-in until they are written:
   LTCG otherwise inlines the partial bodies into WndProc, which 2013 calls. */

#include "../ClientBase/RenderSettingsItem.h"

#include "Application.h"

#include <algorithm>

namespace RBX {

	// Logs a fake crash dump through the MainLogManager at +0xac and tells the user (MessageBox on the
	// View's window); first a call on the gfx view (PLAYER 0x00487d50, RbxOgre). Loads appPtr itself.
	// STUB: PLAYER 0x00403f40
	__declspec(noinline) void Application::UploadSessionLogs()
	{
		STUB(0x00403f40);
	}

	// Against 2016, 2013 also sets ptMaxTrackSize, from CRenderSettings+0x2e, and std::max takes the
	// work-area extent first (`cmp eax,edx`). LTCG: lpMMI in esi. Left: the two max temporaries'
	// stack slots are swapped (max(width, s.x) through the locals is worse, 63.6).
	// STUB: PLAYER 0x00403fa0
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
				s.x = std::max((int)(maxWorkArea.right - maxWorkArea.left), (int)s.x);
				s.y = std::max((int)(maxWorkArea.bottom - maxWorkArea.top), (int)s.y);
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

	// Disposes the threads (SetEvent, boost::thread::join, the thread at +0x124), then saves
	// GlobalBasicSettings and GlobalAdvancedSettings. LTCG: `this` in eax. Written: the first two steps.
	// STUB: PLAYER 0x00404ea0
	__declspec(noinline) void Application::AboutToShutdown()
	{
		enteredShutdown.compare_and_swap(1, 0);

		if (m_unk0x04)
			SetEvent(m_unk0x04);

		STUB(0x00404ea0);
	}

} // namespace RBX
