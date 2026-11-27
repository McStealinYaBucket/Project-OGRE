/* WindowsClient/View.cpp -- October 2013 client shell (PLAYER only; the path is the 2013 __FILE__,
   assert `false` line 739). Written so far: the fullscreen and resolution code, PLAYER
   0x00467410..0x004682b7, from the 2013 bytes. The shell is /GL, so these functions take arguments
   in whatever registers LTCG chose (SetFullscreen `this` in eax and the flag in bl,
   restoreResolution `this` in edi, changeResolution `this` on the stack).

   Against 2016:
     - FFlag::UseWindowPlacementToRememberNonFullscreenedState (initializer 0x00a5c5b0, default on)
       picks between WINDOWPLACEMENT and a plain RECT (GetWindowRect / SetWindowPos);
     - restoreResolution always restores restoreWindowStyle (no WS_OVERLAPPEDWINDOW fallback);
     - initializeSizes caches getFullscreenSizes() in fullScreenSizes and takes the automatic
       fullscreen size from calcDefaultResolution, not from the desktop;
     - SetFullscreen's GameBasicSettings::setFullScreen fires its signal before it stores.

   initializeInput (0x00466840) needs two App accessors that are not declared yet
   (Game::getOverlayDataModel, the shell's copy 0x0041ce60, and ControllerService::setHardwareDevice
   for +0x64); its written body is compiled under OGRE_APP_INPUT_ACCESSORS until they exist.
   HandleWindowsMessage (0x00466510) and ShowWindow (0x004682c0) are the other callers of the
   resolution code; its caller is main.cpp's WndProc (0x004426a0, not written). ShowWindow's assert
   must stay on line 739, so the file keeps a blank gap above it until the rest of View.cpp is laid
   out. Open differences, all LTCG context
   rather than code as far as measured: HandleWindowsMessage and findBestMonitorMatch realign their
   frames (and esp,-8) in 2013 and take other argument registers (their callers are not written);
   changeResolution passes findBestMonitorMatch dmBest in ebx in 2013; LTCG still inlines
   SetFullscreen into ToggleFullscreenVerb::doIt (2013 inlines it only into ShowWindow).
   FLog::RobloxWndInit is App's (FastLogSettings.cpp, initializer 0x00a65100, .data 0x00d30edd,
   default 1); View logs through it. */

// App's headers first: they include winsock2.h ahead of windows.h.
#include "../ClientBase/RenderSettingsItem.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/DebugSettings.h"
#include "V8DataModel/Game.h"
#include "V8DataModel/GameBasicSettings.h"
#include "V8DataModel/OverlayDataModel.h"
#include "V8DataModel/UserController.h"
#include "../Win/LogManager.h"
#include "FastLog.h"
#include "RbxFormat.h"
#include "util/RegistryUtil.h"

#include "View.h"

#include <algorithm>

LOGGROUP(RobloxWndInit)
FASTFLAG(OpenNativeBrowserWindowFromLua)

// RegisterFlag without FASTFLAGVARIABLE's PlaceFilter string: 2013 registers only the flag.
namespace FFlag {
	// Initializer PLAYER 0x00a5c590 (.bss, default off)
	// GLOBAL: PLAYER 0x00e25118
	bool UseTopmostSettingToBringWindowToFront = false;
	int UseTopmostSettingToBringWindowToFrontIniter = FLog::RegisterFlag(
		"UseTopmostSettingToBringWindowToFront", &UseTopmostSettingToBringWindowToFront, NULL, FASTVARTYPE_STATIC);

	// Initializer PLAYER 0x00a5c5b0
	// GLOBAL: PLAYER 0x00dc6c1d
	bool UseWindowPlacementToRememberNonFullscreenedState = true;
	int UseWindowPlacementToRememberNonFullscreenedStateIniter = FLog::RegisterFlag(
		"UseWindowPlacementToRememberNonFullscreenedState", &UseWindowPlacementToRememberNonFullscreenedState, NULL,
		FASTVARTYPE_STATIC);
}

namespace {

	// Restores a value when it goes out of scope (2013 inlines it: {V*, V} on the stack, and the
	// destructor stores without a null test).
	template<class V>
	class ScopedAssign
	{
		V* value;
		V oldValue;

	public:
		ScopedAssign(V& value, const V& newValue)
			: value(&value)
			, oldValue(value)
		{
			*this->value = newValue;
		}
		~ScopedAssign() { *value = oldValue; }
	};

} // namespace

namespace RBX {

// The window placement AboutToShutdown and ShowWindow use (the literal is pushed at each use).
static const char* const windowPlacementKey =
	"HKEY_CURRENT_USER\\Software\\ROBLOX Corporation\\Roblox\\Settings\\RobloxPlayerV4WindowSizeAndPosition";

// 2016's rememberWindowSettings, inlined: no taskbar clamp, and without
// FFlag::UseWindowPlacementToRememberNonFullscreenedState only GetWindowPlacement's placement is saved.
// Application::AboutToShutdown tail-calls it (`this` in ecx).
// FUNCTION: PLAYER 0x00465d60
void View::AboutToShutdown()
{
	// try to save window size and position
	if (HWND hWnd = GetHWnd())
	{
		WINDOWPLACEMENT placement;
		placement.length = sizeof(WINDOWPLACEMENT);

		if (FFlag::UseWindowPlacementToRememberNonFullscreenedState)
		{
			bool foundPlacement = false;
			if (!fullscreen)
				foundPlacement = GetWindowPlacement(hWnd, &placement) != 0;
			else
			{
				foundPlacement = true;
				placement = nonFullscreenPlacement;
			}

			if (foundPlacement)
				RegistryUtil::writeBinaryData(windowPlacementKey, (const BYTE*)&placement, sizeof(WINDOWPLACEMENT));
		}
		else if (GetWindowPlacement(hWnd, &placement))
			RegistryUtil::writeBinaryData(windowPlacementKey, (const BYTE*)&placement, sizeof(WINDOWPLACEMENT));
	}
}

// 2016 adds only the render job. LTCG: `this` in eax (callers 0x0041d16d, 0x00466e37).
// FUNCTION: PLAYER 0x00466460
void View::resetScheduler()
{
	TaskScheduler& taskScheduler = TaskScheduler::singleton();
	taskScheduler.add(m_unk0xc4);
	taskScheduler.add(m_unk0xbc);
	taskScheduler.add(m_unk0xa8);
}

// 2013 against 2016: activation restores the fullscreen mode for any non-zero wParam and then
// shows the window SW_RESTORE; deactivation restores the desktop resolution and minimizes
// (SW_SHOWMINNOACTIVE). FFlag::OpenNativeBrowserWindowFromLua gates the GetParent test as a ternary (setne/test).
// FUNCTION: PLAYER 0x00466510
void View::HandleWindowsMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_ACTIVATE)
	{
		if ((fullscreen || (!fullscreen && desireFullscreen)) && !changingResolution && wParam != WA_INACTIVE)
		{
			changeResolution();
			HWND hWnd = GetHWnd();
			::ShowWindow(hWnd, SW_RESTORE);
			::SetFocus(hWnd);
			::SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		else if (fullscreen && !changingResolution && wParam == WA_INACTIVE
			&& (FFlag::OpenNativeBrowserWindowFromLua ? ::GetParent((HWND)lParam) != GetHWnd() : true))
		{
			restoreResolution();
			HWND hWnd = GetHWnd();
			::ShowWindow(hWnd, SW_SHOWMINNOACTIVE);
			::SetFocus(hWnd);
			::SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
	}
	else if (userInput)
	{
		userInput->PostUserInputMessage(uMsg, wParam, lParam);
	}
}

// Against 2016: no `if (userInput)` test after the reset, and the overlay DataModel's
// ControllerService gets the same hardware device. LTCG passes `this` in edi and builds the
// UserInput with `this` in esi.
// STUB: PLAYER 0x00466840
void View::initializeInput()
{
	userInput.reset(new UserInput(hWnd, this, game));
#ifdef OGRE_APP_INPUT_ACCESSORS

	{
		DataModel::LegacyLock lock(game->getDataModel(), DataModelJob::Write);
		ControllerService* service = ServiceProvider::create<ControllerService>(game->getDataModel().get());
		service->setHardwareDevice(userInput.get());
	}

	if (game->getOverlayDataModel())
	{
		ControllerService* service = ServiceProvider::create<ControllerService>(game->getOverlayDataModel().get());
		service->setHardwareDevice(userInput.get());
	}
#else
	STUB(0x00466840);
#endif
}

// STUB: PLAYER 0x00467410
bool View::findBestMonitorMatch(LPCTSTR szDevice, int desiredX, int desiredY, bool resolutionAuto, DEVMODE& dmBest)
{
	DEVMODE dm;
	ZeroMemory(&dm, sizeof(dm));
	dm.dmSize = sizeof(dm);

	if (EnumDisplaySettingsEx(szDevice, ENUM_CURRENT_SETTINGS, &dm, 0) == 0)
	{
		LogManager::ReportEvent(EVENTLOG_ERROR_TYPE,
			RBX::format("In view::findBestMonitorMatch EnumDisplaySettings failed. GetLastError() == %d", GetLastError()).c_str());
		return false;
	}

	FASTLOG2(FLog::RobloxWndInit, "Current screen resolution - %d x %d", dm.dmPelsWidth, dm.dmPelsHeight);

	// The current mode is no larger than asked for: keep it.
	if (dm.dmPelsHeight <= (DWORD)desiredY && resolutionAuto)
		return true;

	int desiredPixelCount = desiredX * desiredY;
	const double currentAspectRatio = double(dm.dmPelsWidth) / double(dm.dmPelsHeight);

	dmBest = dm;
	bool match = true;

	DWORD iModeNum = 0;
	while (EnumDisplaySettingsEx(szDevice, iModeNum++, &dm, 0) != 0)
	{
		const double aspectRatio = double(dm.dmPelsWidth) / double(dm.dmPelsHeight);
		if (std::abs(currentAspectRatio - aspectRatio) / currentAspectRatio <= 0.1)
		{
			if (dmBest.dmBitsPerPel == dm.dmBitsPerPel && dmBest.dmDisplayFrequency == dm.dmDisplayFrequency)
			{
				int a = desiredPixelCount - dm.dmPelsWidth * dm.dmPelsHeight;
				int b = desiredPixelCount - dmBest.dmPelsWidth * dmBest.dmPelsHeight;
				if (std::abs(a) < std::abs(b))
				{
					dmBest = dm;
					match = false;
				}
			}
		}
	}

	dmBest.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
	return match;
}

// FUNCTION: PLAYER 0x004676a0
void View::restoreResolution()
{
	fullscreen = false;
	FASTLOG(FLog::RobloxWndInit, "Start View::restoreResolution");

	ScopedAssign<bool> assign(changingResolution, true);

	if (hMonitor == NULL)
	{
		LogManager::ReportEvent(EVENTLOG_ERROR_TYPE, "Unable to restore resolution, no handle to monitor");
		return;
	}

	if (changedResolution)
	{
		MONITORINFOEX mi;
		mi.cbSize = sizeof(mi);
		if (GetMonitorInfo(hMonitor, &mi))
		{
			// Hide the window first, or ChangeDisplaySettingsEx does not restore the mode.
			::ShowWindow(GetHWnd(), SW_HIDE);

			LONG result = ChangeDisplaySettingsEx(mi.szDevice, NULL, NULL, 0, NULL);
			if (result != DISP_CHANGE_SUCCESSFUL)
			{
				LogManager::ReportEvent(EVENTLOG_ERROR_TYPE,
					RBX::format("Unable to restore resolution, ChangeDisplaySettingsEx returned %d", result).c_str());
			}

			::ShowWindow(GetHWnd(), SW_SHOWNORMAL);
		}
		changedResolution = false;
	}

	if (FFlag::UseWindowPlacementToRememberNonFullscreenedState)
	{
		SetWindowLongPtr(GetHWnd(), GWL_STYLE, restoreWindowStyle);
		SetWindowPlacement(GetHWnd(), &nonFullscreenPlacement);
	}
	else
	{
		modifyWindow(restoreWindowStyle, nonFullscreenRect);
	}

	FASTLOG(FLog::RobloxWndInit, "Done Vew::restoreResolution");
}

// FUNCTION: PLAYER 0x00467880
void View::SetFullscreen(bool value)
{
	if (fullscreen != value)
	{
		if (value)
		{
			restoreWindowStyle = GetWindowLong(GetHWnd(), GWL_STYLE);
			changeResolution();
		}
		else
		{
			restoreResolution();
		}
	}
	desireFullscreen = value;
	GameBasicSettings::singleton().setFullScreen(value);
}

// STUB: PLAYER 0x004678e0
void View::initializeSizes()
{
	if (GetHWnd() == NULL)
	{
		LogManager::ReportEvent(EVENTLOG_WARNING_TYPE, "Attempt to initialize monitor sizes without valid HWND");
		return;
	}

	G3D::Vector2int16 currentDisplaySize = getCurrentDesktopResolution();
	fullScreenSizes = getFullscreenSizes();

	G3D::Vector2int16 defaultSize = calcDefaultResolution((float)currentDisplaySize.x / currentDisplaySize.y);

	G3D::Vector2int16 fullscreenSize;
	CRenderSettings::ResolutionPreset preference = CRenderSettingsItem::singleton().getResolutionPreference();
	if (preference == CRenderSettings::ResolutionAuto)
	{
		fullscreenSize = defaultSize;
	}
	else
	{
		const CRenderSettings::RESOLUTIONENTRY& res = CRenderSettingsItem::singleton().getResolutionPreset(preference);
		fullscreenSize = G3D::Vector2int16(res.width, res.height);
	}

	G3D::Vector2int16 largest;
	largest.x = std::max(defaultSize.x, fullscreenSize.x);
	largest.y = std::max(defaultSize.y, fullscreenSize.y);

	G3D::Vector2int16 windowSize = fullscreenSize;
	windowSize.x = std::min((int)windowSize.x, (int)currentDisplaySize.x);
	windowSize.y = std::min((int)windowSize.y, (int)currentDisplaySize.y);

	CRenderSettingsItem::singleton().m_unk0x2e = largest;
	CRenderSettingsItem::singleton().windowSize = windowSize;
	CRenderSettingsItem::singleton().fullscreenSize = fullscreenSize;
}

// FUNCTION: PLAYER 0x00467ab0
void View::modifyWindow(DWORD argMask, const RECT& area)
{
	SetWindowLongPtr(GetHWnd(), GWL_STYLE, argMask);
	SetWindowPos(GetHWnd(), NULL, area.left, area.top, area.right - area.left, area.bottom - area.top, 0);
}

// STUB: PLAYER 0x00467ae0
void View::changeResolution()
{
	// Remember where the window was, unless it is fullscreen already.
	if (FFlag::UseWindowPlacementToRememberNonFullscreenedState && !fullscreen)
	{
		ZeroMemory(&nonFullscreenPlacement, sizeof(nonFullscreenPlacement));
		nonFullscreenPlacement.length = sizeof(nonFullscreenPlacement);
		GetWindowPlacement(GetHWnd(), &nonFullscreenPlacement);
	}

	fullscreen = true;

	if (!FFlag::UseWindowPlacementToRememberNonFullscreenedState && nonFullscreenRect.bottom == nonFullscreenRect.top)
		GetWindowRect(GetHWnd(), &nonFullscreenRect);

	hMonitor = MonitorFromWindow(GetHWnd(), MONITOR_DEFAULTTONEAREST);
	if (hMonitor == NULL)
	{
		LogManager::ReportEvent(EVENTLOG_ERROR_TYPE, "Cannot changeResolution, MonitorFromWindow returned NULL");
		return;
	}

	// The user may have changed the desktop since the last toggle.
	initializeSizes();

	ScopedAssign<bool> assign(changingResolution, true);

	MONITORINFOEX mi;
	mi.cbSize = sizeof(mi);
	if (!GetMonitorInfo(hMonitor, &mi))
	{
		LogManager::ReportEvent(EVENTLOG_ERROR_TYPE, "Cannot changeResolution, GetMonitorInfo failed");
		return;
	}

	G3D::Vector2int16 size = CRenderSettingsItem::singleton().getFullscreenSize();
	DEVMODE dm;
	bool matches = findBestMonitorMatch(mi.szDevice, size.x, size.y,
		CRenderSettingsItem::singleton().getResolutionPreference() == CRenderSettings::ResolutionAuto, dm);

	LONG result = matches ? DISP_CHANGE_SUCCESSFUL : ChangeDisplaySettingsEx(mi.szDevice, &dm, NULL, CDS_FULLSCREEN, NULL);

	if (result == DISP_CHANGE_SUCCESSFUL)
	{
		if (!changedResolution)
			changedResolution = !matches;

		if (!matches)
			FASTLOG2(FLog::RobloxWndInit, "Changed screen resolution to %d x %d", dm.dmPelsWidth, dm.dmPelsHeight);

		// Size the window to the monitor's (possibly new) resolution.
		MONITORINFOEX monitorInfo;
		monitorInfo.cbSize = sizeof(monitorInfo);
		if (!GetMonitorInfo(hMonitor, &monitorInfo))
		{
			LogManager::ReportEvent(EVENTLOG_ERROR_TYPE, "Cannot changeResolution, GetMonitorInfo failed");
			return;
		}

		int cxBorder = ::GetSystemMetrics(SM_CXBORDER);
		int cyBorder = ::GetSystemMetrics(SM_CYBORDER);
		cxBorder = cyBorder = 0;

		G3D::Vector2int16 newResolution = getCurrentDesktopResolution();

		RECT area = monitorInfo.rcMonitor;
		area.left -= cxBorder;
		area.right += cxBorder * 2;
		area.top -= cyBorder;
		area.bottom += cyBorder * 2;

		modifyWindow(WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, area);
	}
	else
	{
		LogManager::ReportEvent(EVENTLOG_ERROR_TYPE, RBX::format("ChangeDisplaySettings returned %d", result).c_str());
	}
	FASTLOG(FLog::RobloxWndInit, "Done View::changeResolution");
}

// The line count comes from DebugSettings: an Intel card gives 1200 (600 when its name also
// contains "828"); otherwise up to 32, 64, 128 and 256 MB of video memory give 600, 1024, 1200 and
// 1280, more gives 1600. `this` is unused, so LTCG passes only the result pointer (edi).
// FUNCTION: PLAYER 0x00467dd0
G3D::Vector2int16 View::calcDefaultResolution(float aspect_XdivY)
{
	int videoMemory = DebugSettings::singleton().videoMemory();
	std::string gfxcard = DebugSettings::singleton().gfxcard();

	int numlines;
	if (gfxcard.find("Intel") != std::string::npos)
		numlines = gfxcard.find("828") != std::string::npos ? 600 : 1200;
	else if (videoMemory <= 32)
		numlines = 600;
	else if (videoMemory <= 64)
		numlines = 1024;
	else if (videoMemory <= 128)
		numlines = 1200;
	else
		numlines = videoMemory <= 256 ? 1280 : 1600;

	int numrows = (int)(numlines * aspect_XdivY);
	return G3D::Vector2int16(numrows, numlines);
}

// FUNCTION: PLAYER 0x00467f00
G3D::Vector2int16 View::getCurrentDesktopResolution()
{
	G3D::Vector2int16 defaultr(800, 600);

	if (hMonitor == NULL)
	{
		hMonitor = MonitorFromWindow(GetHWnd(), MONITOR_DEFAULTTONEAREST);
		if (!hMonitor)
			return defaultr;
	}

	MONITORINFOEX mi;
	mi.cbSize = sizeof(mi);
	if (!GetMonitorInfo(hMonitor, &mi))
	{
		LogManager::ReportEvent(EVENTLOG_ERROR_TYPE,
			RBX::format("view::geCurrentDesctopResolution GetMonitorInfo failed, GetLastError returned %d", GetLastError()).c_str());
		return defaultr;
	}

	DEVMODE dm;
	ZeroMemory(&dm, sizeof(dm));
	dm.dmSize = sizeof(dm);
	if (EnumDisplaySettingsEx(mi.szDevice, ENUM_CURRENT_SETTINGS, &dm, 0))
		return G3D::Vector2int16(dm.dmPelsWidth, dm.dmPelsHeight);

	return defaultr;
}

// FUNCTION: PLAYER 0x00468070
std::vector<G3D::Vector2int16> View::getFullscreenSizes()
{
	std::vector<G3D::Vector2int16> sizes;

	if (hMonitor == NULL)
	{
		hMonitor = MonitorFromWindow(GetHWnd(), MONITOR_DEFAULTTONEAREST);
		if (!hMonitor)
			return sizes;
	}

	MONITORINFOEX mi;
	mi.cbSize = sizeof(mi);
	if (!GetMonitorInfo(hMonitor, &mi))
	{
		LogManager::ReportEvent(EVENTLOG_ERROR_TYPE,
			RBX::format("view::getFullscreenSizes GetMonitorInfo failed, GetLastError returned %d", GetLastError()).c_str());
		return sizes;
	}

	DEVMODE dm;
	ZeroMemory(&dm, sizeof(dm));
	dm.dmSize = sizeof(dm);

	DWORD iModeNum = 0;
	while (EnumDisplaySettingsEx(mi.szDevice, iModeNum++, &dm, 0))
	{
		G3D::Vector2int16 size(dm.dmPelsWidth, dm.dmPelsHeight);
		if (std::find(sizes.begin(), sizes.end(), size) == sizes.end())
			sizes.push_back(size);
	}

	return sizes;
}

// View.cpp lines up to 739 hold code that is not written yet (the 2013 assert below is on line
// 739); keep this function at the end until the rest of the file is laid out.


































































































































































































// 2013 against 2016: the placement comes from the registry (a WINDOWPLACEMENT blob), not from
// GameBasicSettings; SetFocus is called directly; the TOPMOST toggle is behind
// FFlag::UseTopmostSettingToBringWindowToFront (HWND_TOP otherwise). SetFullscreen(true) is inlined.
// FUNCTION: PLAYER 0x004682c0
void View::ShowWindow()
{
	HWND hWnd = GetHWnd();
	::ShowWindow(hWnd, SW_SHOWNORMAL);

	UINT32 size = 0;
	bool hasPlacement = RegistryUtil::readBinaryData(windowPlacementKey, NULL, size) && size == sizeof(WINDOWPLACEMENT);
	if (hasPlacement)
	{
		WINDOWPLACEMENT placement;
		bool read = RegistryUtil::readBinaryData(windowPlacementKey, (BYTE*)&placement, size);
		if (read)
			SetWindowPlacement(hWnd, &placement);
		else
			RBXASSERT(false);
	}

	::SetFocus(hWnd);

	if (FFlag::UseTopmostSettingToBringWindowToFront)
	{
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	else
	{
		SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	if (GameBasicSettings::singleton().getFullScreen())
		SetFullscreen(true);
}

} // namespace RBX
