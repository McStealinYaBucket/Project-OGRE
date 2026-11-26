/* WindowsClient/main.cpp -- October 2013 client shell (PLAYER only). No 2013 __FILE__ string names this
   file; the path is the 2016 tree's. Written so far: the window procedure, RegisterWindowClass and
   WinMain, from the 2013 bytes. WinMain's catch needs Application::Initialize to be able to throw
   (its stub does): without it LTCG drops the catch, aligns WinMain's frame (and esp,-8) and passes
   AboutToShutdown nothing (it loads appPtr itself), which costs RegisterWindowClass its 100.

   Against 2016, 2013's WndProc:
     - forwards only WM_KEYDOWN, WM_MOUSEMOVE, WM_MOUSELEAVE, WM_MOUSEWHEEL, WM_SETFOCUS,
       WM_KILLFOCUS, WM_ACTIVATE and WM_ACTIVATEAPP to the view (no WM_CHAR, no WM_INPUT; jump table
       0x00442834 and its byte index 0x00442848);
     - has a third command, 40003, which shows a MessageBox about video recording under OpenGL;
     - calls the view without null tests (Application::HandleWindowsMessage; the WM_SIZE and wiki
       code is written out here, see Application.h). */

#include "../Win/LogManager.h"
#include "FastLog.h"

#include "Application.h"

#include <atlbase.h>

LOGGROUP(HangDetection)
LOGGROUP(RobloxWndInit)

// Menu commands (2016 resource.h names; 40003 has no 2016 counterpart).
#define ID_UPLOADSESSIONLOGS 33042
#define ID_LOADWIKI 40002
#define ID_UNK_40003 40003

// GLOBAL: PLAYER 0x00e250a0
RBX::Application* appPtr;

// STUB: PLAYER 0x004426a0
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_TIMER:
		// Notify log manager that main thread is still responsive
		if (MainLogManager* manager = LogManager::getMainLogManager())
		{
			FASTLOG(FLog::HangDetection, "WindowsPlayer: timer event fired");
			manager->NotifyFGThreadAlive();
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_UPLOADSESSIONLOGS:
			appPtr->UploadSessionLogs();
			break;
		case ID_LOADWIKI:
			ShellExecute(appPtr->m_unk0x24->m_unk0x00->GetHWnd(), "open", "rundll32.exe",
				"url.dll,FileProtocolHandler http://wiki.roblox.com", NULL, SW_SHOWDEFAULT);
			break;
		case ID_UNK_40003:
			MessageBoxA(hWnd,
				"Video recording doesn't support OpenGL mode yet, please visit \r\n"
				"http://www.roblox.com/drivers for more information.",
				"Information", MB_OK);
			break;
		}
		break;
	case WM_GETMINMAXINFO:
		RBX::Application::OnGetMinMaxInfo((MINMAXINFO*)lParam);
		break;
	case WM_KEYDOWN:
	case WM_MOUSEMOVE:
	case WM_MOUSELEAVE:
	case WM_MOUSEWHEEL:
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
	case WM_ACTIVATE:
	case WM_ACTIVATEAPP:
		appPtr->HandleWindowsMessage(message, wParam, lParam);
		break;
	case WM_DESTROY:
		appPtr->AboutToShutdown();
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		appPtr->m_unk0x24->m_unk0x00->GetGfxView()->onResize(LOWORD(lParam), HIWORD(lParam));
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#define MAX_LOADSTRING 100
#define IDS_APP_TITLE 101
#define IDC_WINDOWSCLIENT 102
#define IDI_WINDOW_ICON 104

// GLOBAL: PLAYER 0x00e250a8
TCHAR szWindowClass[MAX_LOADSTRING];

// Registers the window class (LTCG: hInstance in eax).
// FUNCTION: PLAYER 0x00442870
ATOM RegisterWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOW_ICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_WINDOWSCLIENT);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_WINDOW_ICON));

	return RegisterClassEx(&wcex);
}

// Against 2016: no SSE2 test, no client settings fetch before the window, CreateWindow at
// (CW_USEDEFAULT, 0) sized 800x600, Initialize takes only the window, and no VirtualProtect before
// Shutdown. The catch is `const RBX::initialization_error&` (FuncInfo 0x00ce3e8c).
// STUB: PLAYER 0x00442900
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	RBX::Application app;
	appPtr = &app;
	CComModule comModule; // Needed for ActiveX hosting of IWebBrowser2

	if (!app.LoadAppSettings(hInstance))
		return FALSE;
	if (!app.ParseArguments(lpCmdLine))
		return FALSE;

	TCHAR szTitle[MAX_LOADSTRING];

	// initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINDOWSCLIENT, szWindowClass, MAX_LOADSTRING);

	RegisterWindowClass(hInstance);

	HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 800, 600, NULL,
		NULL, hInstance, NULL);
	if (!hWnd)
		return FALSE;

	try
	{
		if (!app.Initialize(hWnd))
			return FALSE;
	}
	catch (const RBX::initialization_error& e)
	{
		const char* const errorMessage = e.what();
		FASTLOGS(FLog::RobloxWndInit, "Error during initialization. User message = %s", errorMessage);
		MessageBoxA(hWnd, errorMessage, "ROBLOX", MB_OK);
		app.AboutToShutdown();
		app.Shutdown();
		return FALSE;
	}

	// keep-alive timer, see WM_TIMER
	SetTimer(hWnd, NULL, 10 * 1000, NULL);

	ShowWindow(hWnd, SW_HIDE);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	app.Shutdown();

	return (int)msg.wParam;
}
