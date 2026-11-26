/* WindowsClient/UserInput.cpp -- October 2013 client shell (PLAYER only; path from the 2016 tree, see
   UserInput.h). Written so far: PostUserInputMessage, which View::HandleWindowsMessage (0x00466510)
   calls for every message but WM_ACTIVATE. */

// App's headers first: they include winsock2.h ahead of windows.h.
#include "V8DataModel/DataModel.h"
#include "V8DataModel/Game.h"

#include "UserInput.h"

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

	// Creates the UserInputSleepingJob (0x260 bytes, ctor 0x004627b0) into +0x38 and adds it to the
	// TaskScheduler. The body here is a stand-in (it must use `this`, or LTCG drops it).
	// STUB: PLAYER 0x0045e630
	__declspec(noinline) void UserInput::setGame(boost::shared_ptr<RBX::Game> game)
	{
		STUB(0x0045e630);
		this->game = game;
	}

	// STUB: PLAYER 0x0045e900
	void UserInput::removeJobs()
	{
		STUB(0x0045e900);
		m_unk0x38.reset();
	}

	// Stand-in bodies below: each touches a member so LTCG keeps `this`.
	// STUB: PLAYER 0x0045ec60
	__declspec(noinline) void UserInput::createMouse()
	{
		STUB(0x0045ec60);
		isMouseAcquired = false;
	}

	// STUB: PLAYER 0x0045ecf0
	__declspec(noinline) void UserInput::createKeyboard()
	{
		STUB(0x0045ecf0);
		isKeyboardAcquired = false;
	}

	// STUB: PLAYER 0x0045ed80
	__declspec(noinline) void UserInput::createAccelerators()
	{
		STUB(0x0045ed80);
		accelerators.clear();
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

	// STUB: PLAYER 0x004603e0
	TextureProxyBaseRef UserInput::getGameCursor(Adorn* adorn)
	{
		STUB(0x004603e0);
		return UserInputBase::getGameCursor(adorn);
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

} // namespace RBX
