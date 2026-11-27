/* WindowsClient/UserInput.cpp -- October 2013 client shell (PLAYER only; the path is the 2013 __FILE__
   of ~UserInput's asserts, lines 148-150). Written so far: PostUserInputMessage, which
   View::HandleWindowsMessage (0x00466510) calls for every message but WM_ACTIVATE, ~UserInput, and
   the DirectInput acquire/release code with its cursor helpers (0x0045edf0..0x0045f1aa and
   0x00460320..0x004607f7). Every assert sits on its 2013 line (148-150, 269, 300, 340, 775, 781, 804,
   825), so the file keeps blank gaps where the unwritten functions go. */

// App's headers first: they include winsock2.h ahead of windows.h.
#include "V8DataModel/DataModel.h"
#include "V8DataModel/Game.h"
#include "V8DataModel/OverlayDataModel.h"
#include "V8DataModel/RunService.h"
#include "Util/Math.h"

#include "UserInput.h"

#include <windowsx.h>

DECOMP_SIZE_ASSERT(RBX::RobloxCriticalSection, 0x54);

namespace RBX {

	// FUNCTION: PLAYER 0x0045e190
	RobloxCriticalSection::RobloxCriticalSection()
		: callers(0)
	{
	}

	// Slot 3; it keeps __thiscall only while the ctor emits the vftable.
	// FUNCTION: PLAYER 0x0045e200
	void UserInput::centerCursor()
	{
		wrapMousePosition = Vector2::zero();
	}

	// Against 2016: no SDL controller and no inputObjectMap; DirectInput failure throws (AtlThrow)
	// instead of a message box; the job at +0x38 comes from setGame.
	// STUB: PLAYER 0x0045e300
	UserInput::UserInput(HWND wnd, View* view, boost::shared_ptr<RBX::Game> game)
		: isMouseCaptured(false)
		, wrapping(false)
		, rightMouseDown(false)
		, autoMouseMove(true)
		, mouseButtonSwap(false)
		, externallyForcedKeyDown(0)
		, wnd(wnd)
		, parentView(view)
		, posToWrapTo(0, 0)
		, leftMouseButtonDown(false)
		, isMouseInside(false)
		, desireKeyboardAcquired(false)
		, isMouseAcquired(false)
		, isKeyboardAcquired(false)
	{
		hArrow = LoadCursor(NULL, IDC_ARROW);
		hInvisibleCursor = LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(105));

		setGame(game);

		memset(&diKeys, 0, sizeof(diKeys));

		HRESULT hr = DirectInput8Create(GetModuleHandle(NULL), 0x0800, IID_IDirectInput8A, (VOID**)&diPtr, NULL);
		if (FAILED(hr))
			AtlThrow(hr);

		// Left-handed mouse setting (2013: CRegKey::Open 0x0041aee0, QueryStringValue 0x00462350).
		CRegKey key;
		if (key.Open(HKEY_CURRENT_USER, "Control Panel\\Mouse", KEY_READ) == ERROR_SUCCESS)
		{
			TCHAR value[256];
			ULONG length = 256;
			if (key.QueryStringValue(_T("SwapMouseButtons"), value, &length) == ERROR_SUCCESS)
				mouseButtonSwap = (value[0] == '1');
		}

		createMouse();
		createKeyboard();
		createAccelerators();

		layout = GetKeyboardLayout(0);
	}

	// FUNCTION: PLAYER 0x0045e5c0
	RobloxCriticalSection::~RobloxCriticalSection()
	{
	}

	// Against 2016: the job runs on the overlay DataModel when there is one (Game::getOverlayDataModel,
	// the shell's copy 0x0041ce60).
	// STUB: PLAYER 0x0045e630
	void UserInput::setGame(boost::shared_ptr<RBX::Game> game)
	{
		this->game = game;
		runService = shared_from(ServiceProvider::create<RunService>(game->getDataModel().get()));

		shared_ptr<DataModel> dataModel = game->getDataModel();
		if (game->getOverlayDataModel())
			dataModel = game->getOverlayDataModel();

		m_unk0x38.reset(new UserInputSleepingJob(this, dataModel));
		TaskScheduler::singleton().add(m_unk0x38);
	}

	// FUNCTION: PLAYER 0x0045e900
	void UserInput::removeJobs()
	{
		if (m_unk0x38)
		{
			TaskScheduler::singleton().removeBlocking(m_unk0x38);
			m_unk0x38.reset();
		}
	}

	// Inline everywhere in 2013 (no out-of-line copy).
	inline void UserInput::unAcquireKeyboard()
	{
		diKeyboardPtr->Unacquire();
		isKeyboardAcquired = false;
	}

	inline void UserInput::updateKeyboard()
	{
		if (diKeyboardPtr.p == NULL)
			return;

		if (isKeyboardAcquired == desireKeyboardAcquired)
			return;

		if (desireKeyboardAcquired)
			acquireKeyboard();
		else
			unAcquireKeyboard();
	}

	inline G3D::Vector2 UserInput::getCursorPositionInternal()
	{
		return isMouseAcquired ? getGameCursorPositionInternal() : getWindowsCursorPositionInternal();
	}

	// Non-virtual (UserInputBase declares no virtual destructor); View::initializeInput (0x00466840) is
	// its caller, through scoped_ptr::reset. The asserts must stay on lines 148-150.
	// Left (93.5%): 2013's frame has one more dword (`push ecx`), so every stack offset is 4 higher.
	// STUB: PLAYER 0x0045e990
	UserInput::~UserInput()
	{
		removeJobs();

		RBXASSERT(diMousePtr);
		RBXASSERT(diKeyboardPtr);
		RBXASSERT(diPtr);

		if (diMousePtr)
		{
			diMousePtr->Unacquire();
			diMousePtr = NULL;
		}
		if (diKeyboardPtr)
		{
			diKeyboardPtr->Unacquire();
			diKeyboardPtr = NULL;
		}
		if (diPtr)
			diPtr = NULL;
	}

	// 100% when the DirectX SDK's dinput8.lib/dxguid.lib are linked and the five DirectInput data rows
	// are paired (measured with the November 2007 SDK); 97.8% while GUID_SysMouse and c_dfDIMouse2
	// stay unresolved.
	// STUB: PLAYER 0x0045ec60
	void UserInput::createMouse()
	{
		HRESULT hr = diPtr->CreateDevice(GUID_SysMouse, &diMousePtr, NULL);
		if (FAILED(hr))
			AtlThrow(hr);

		hr = diMousePtr->SetDataFormat(&c_dfDIMouse2);
		if (FAILED(hr))
			AtlThrow(hr);

		DIPROPDWORD dipdw;
		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = 0x800;
		hr = diMousePtr->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
		if (FAILED(hr))
			AtlThrow(hr);
	}

	// As createMouse: 100% with the SDK libraries, 97.8% without.
	// STUB: PLAYER 0x0045ecf0
	void UserInput::createKeyboard()
	{
		HRESULT hr = diPtr->CreateDevice(GUID_SysKeyboard, &diKeyboardPtr, NULL);
		if (FAILED(hr))
			AtlThrow(hr);

		hr = diKeyboardPtr->SetDataFormat(&c_dfDIKeyboard);
		if (FAILED(hr))
			AtlThrow(hr);

		DIPROPDWORD dipdw;
		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = 0x800;
		hr = diKeyboardPtr->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
		if (FAILED(hr))
			AtlThrow(hr);
	}

	// The window's accelerator table (resource 255), copied into accelerators.
	// STUB: PLAYER 0x0045ed80
	void UserInput::createAccelerators()
	{
		HACCEL hAccel = LoadAccelerators(GetModuleHandle(NULL), MAKEINTRESOURCE(255));
		if (hAccel)
		{
			int count = CopyAcceleratorTable(hAccel, NULL, 0);
			accelerators.resize(count, ACCEL());
			if (count)
				CopyAcceleratorTable(hAccel, &accelerators[0], count);
		}
	}

	// Its one caller (ProcessUserInputMessage, WM_SETFOCUS) passes true.
	// FUNCTION: PLAYER 0x0045edf0
	void UserInput::setKeyboardDesiredInternal(bool set)
	{
		desireKeyboardAcquired = set;
		updateKeyboard();
	}

	// FUNCTION: PLAYER 0x0045ee20
	void UserInput::setKeyboardDesired(bool set)
	{
		RobloxCritSecLoc lock(diSection, "setKeyboardDesired");
		setKeyboardDesiredInternal(set);
	}

	// STUB: PLAYER 0x0045eec0
	void UserInput::acquireMouseInternal()
	{
		Vector2 cp = getCursorPositionInternal();
		acquireMouseInternalBase(cp);
	}
















	// Against 2016: no diMousePtr test.
	// FUNCTION: PLAYER 0x0045ef20
	void UserInput::acquireMouseInternalBase(const Vector2& pos)
	{
		RBXASSERT(!isMouseAcquired);

		diMousePtr->Unacquire();

		HRESULT hr = diMousePtr->SetCooperativeLevel(wnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		if (hr == DI_OK)
		{
			hr = diMousePtr->Acquire();
			if (hr == DI_OK)
			{
				wrapMousePosition = Math::expandVector2(pos - getWindowRect().center(), -10);
				isMouseAcquired = true;
				isMouseCaptured = false;
				SetClassLong(wnd, GCL_HCURSOR, (LONG)hInvisibleCursor);
			}
		}
	}











	// FUNCTION: PLAYER 0x0045f050
	void UserInput::unAcquireMouse()
	{
		RBXASSERT(isMouseAcquired);

		diMousePtr->Unacquire();

		Vector2 pos = getGameCursorPositionExpandedInternal();
		POINT p;
		p.x = static_cast<int>(pos.x);
		p.y = static_cast<int>(pos.y);
		ClientToScreen(wnd, &p);

		SetCursorPos(p.x, p.y);

		SetClassLong(wnd, GCL_HCURSOR, (LONG)hArrow);
		isMouseAcquired = false;
		isMouseCaptured = false;
	}





















	// FUNCTION: PLAYER 0x0045f120
	void UserInput::acquireKeyboard()
	{
		RBXASSERT(!isKeyboardAcquired);

		diKeyboardPtr->Unacquire();

		HRESULT hr = diKeyboardPtr->SetCooperativeLevel(wnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
		if (hr == DI_OK)
		{
			if (SUCCEEDED(hr = diKeyboardPtr->Acquire()))
				isKeyboardAcquired = true;
		}
	}

	// Against 2016: the message is not handled here but queued on the UserInputSleepingJob, which is
	// then woken; the WM_MOUSEMOVE sample is the same.
	// FUNCTION: PLAYER 0x0045f1b0
	void UserInput::PostUserInputMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_MOUSEMOVE)
			if (shared_ptr<DataModel> dataModel = game->getDataModel())
				dataModel->mouseStats.osMouseMove.sample();

		if (m_unk0x38)
		{
			UserInputSleepingJob::Message message = {uMsg, wParam, lParam};
			m_unk0x38->messages.push(message);
			m_unk0x38->wake();
		}
	}

	inline void UserInput::updateMouse()
	{
		if (!diMousePtr)
		{
			isMouseAcquired = false;
			return;
		}

		if (isMouseAcquired == isMouseInside)
			return;

		if (isMouseInside)
			acquireMouseInternal();
		else
			unAcquireMouse();
	}

	inline void UserInput::onMouseLeave()
	{
		if (!isMouseInside)
			return;

		isMouseInside = false;

		if (!isMouseCaptured)
		{
			if (isMouseAcquired)
				unAcquireMouse();
		}
	}

	// The handler for one queued message (2016's name, the string it passes to the critical section's
	// caller record). LTCG: `this` in edx, uMsg in ecx, ret 8.
	// Against 2016: the events are App's UIEvent, not InputObjects; the wheel direction is the event
	// type (8 forward, 9 back). Not written: the WM_MOUSEMOVE event type (UserInputUtil 0x00463ed0) and
	// the focus events (0xd WM_SETFOCUS, 0xc WM_KILLFOCUS, sent through 0x00460210), which need App's
	// 2013 UIEvent (20 bytes: type, the cursor and the window size as Vector2int16, ..., 0 at +0x10).
	// STUB: PLAYER 0x0045f2a0
	void UserInput::ProcessUserInputMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		RobloxCritSecLoc lock(diSection, "ProcessUserInputMessage");

		if (uMsg == WM_MOUSEMOVE)
		{
			onMouseInside();

			if (!isMouseAcquired)
			{
				Vector2 pos((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
				sendMouseEvent(0, pos);
			}
			updateMouse();
			return;
		}

		if (uMsg == WM_MOUSELEAVE)
		{
			onMouseLeave();
			return;
		}

		if (uMsg == WM_MOUSEWHEEL)
		{
			if (!isMouseAcquired)
			{
				int zDelta = (int)wParam;
				Vector2 pos((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
				sendMouseEvent(zDelta > 0.0 ? 8 : 9, pos);
			}
			return;
		}

		if (uMsg == WM_SETFOCUS)
		{
			setKeyboardDesiredInternal(true);
			return;
		}
	}

	// Builds a UIEvent from the type, the position and the window size (0x00460370) and sends it
	// (0x00460210). Needs App's 2013 UIEvent.
	// STUB: PLAYER 0x004601c0
	__declspec(noinline) void UserInput::sendMouseEvent(int type, const Vector2& pos)
	{
		STUB(0x004601c0);
		externallyForcedKeyDown = type + (int)pos.x;
	}

	// Against 2016: no DFFlag::UserInputViewportSizeFixWindows branch.
	// FUNCTION: PLAYER 0x00460320
	G3D::Rect2D UserInput::getWindowRect() const
	{
		RECT rect;
		GetClientRect(wnd, &rect);
		return Rect2D::xyxy(rect.left, rect.top, rect.right, rect.bottom);
	}

	// STUB: PLAYER 0x004603e0
	TextureProxyBaseRef UserInput::getGameCursor(Adorn* adorn)
	{
		STUB(0x004603e0);
		return UserInputBase::getGameCursor(adorn);
	}












































































































































































































































































































	// FUNCTION: PLAYER 0x00460510
	G3D::Vector2 UserInput::getGameCursorPositionInternal()
	{
		RBXASSERT(isMouseAcquired);
		return getWindowRect().center() + wrapMousePosition;
	}
	// FUNCTION: PLAYER 0x004605f0
	G3D::Vector2 UserInput::getGameCursorPositionExpandedInternal()
	{
		RBXASSERT(isMouseAcquired);
		return getWindowRect().center() + Math::expandVector2(wrapMousePosition, 10);
	}

	// Called for every WM_MOUSEMOVE. Against 2016: no UserInputService mouse wrap mode.
	// FUNCTION: PLAYER 0x004606e0
	void UserInput::onMouseInside()
	{
		if (isMouseInside)
			return;

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = wnd;

		if (!TrackMouseEvent(&tme))
			return;

		isMouseInside = true;
		if (!isMouseCaptured)
			acquireMouseInternal();
		else
			RBXASSERT(isMouseAcquired);
	}
















	// FUNCTION: PLAYER 0x00460780
	G3D::Vector2 UserInput::getWindowsCursorPositionInternal()
	{
		RBXASSERT(!isMouseAcquired);

		POINT p;
		GetCursorPos(&p);
		ScreenToClient(wnd, &p);
		return G3D::Vector2(static_cast<float>(p.x), static_cast<float>(p.y));
	}

	// STUB: PLAYER 0x00460800
	G3D::Vector2 UserInput::getCursorPosition()
	{
		STUB(0x00460800);
		return wrapMousePosition;
	}

	// STUB: PLAYER 0x00460910
	bool UserInput::keyDown(KeyCode code) const
	{
		STUB(0x00460910);
		return diKeys[code] != 0;
	}

	// STUB: PLAYER 0x00460940
	void UserInput::setKeyState(KeyCode code, ModCode modCode, char modifiedKey, bool isDown)
	{
		STUB(0x00460940);
	}

	// 81.3%: 2013 keeps an EH slot for the by-value arbiter argument (8 more bytes of frame). With
	// setGame's written body as its caller (App accessor probe) it scores 98.9%, the EH table aside.
	// STUB: PLAYER 0x004627b0
	UserInputSleepingJob::UserInputSleepingJob(UserInput* userInput, shared_ptr<DataModelArbiter> arbiter)
		: SleepingJob("UserInputSleeping", DataModelJob::Write, false, arbiter, Time::Interval(0.01), 60.0)
		, userInput(userInput)
	{
	}

	// Handles the queued messages under a write request; a WM_MOUSEMOVE followed by another one is
	// dropped. LTCG: pop_if_present (0x00462a50) and peek_if_present (0x00462b00) stay out of line.
	// STUB: PLAYER 0x00462920
	TaskScheduler::StepResult UserInputSleepingJob::stepDataModelJob(const Stats& stats)
	{
		DataModel::scoped_write_request request(userInput->game->getDataModel().get());

		Message message;
		while (messages.pop_if_present(message))
		{
			if (message.uMsg == WM_MOUSEMOVE)
			{
				Message next;
				if (messages.peek_if_present(next) && next.uMsg == message.uMsg)
					continue;
			}
			userInput->ProcessUserInputMessage(message.uMsg, message.wParam, message.lParam);
		}

		sleep();
		return TaskScheduler::Stepped;
	}

} // namespace RBX
