/* WindowsClient/UserInput.cpp -- October 2013 client shell (PLAYER only; the path is the 2013 __FILE__
   of ~UserInput's asserts, lines 148-150). Written so far: PostUserInputMessage, which
   View::HandleWindowsMessage (0x00466510) calls for every message but WM_ACTIVATE, and ~UserInput. */

// App's headers first: they include winsock2.h ahead of windows.h.
#include "V8DataModel/DataModel.h"
#include "V8DataModel/Game.h"
#include "V8DataModel/OverlayDataModel.h"
#include "V8DataModel/RunService.h"

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

	// FUNCTION: PLAYER 0x0045e5c0
	RobloxCriticalSection::~RobloxCriticalSection()
	{
	}

	// Against 2016: the job runs on the overlay DataModel when there is one. The written body needs
	// Game::getOverlayDataModel (App, the shell's copy 0x0041ce60); until App declares it, the stand-in
	// below is compiled (it must use `this`, or LTCG drops it).
	// STUB: PLAYER 0x0045e630
	void UserInput::setGame(boost::shared_ptr<RBX::Game> game)
	{
#ifdef OGRE_APP_INPUT_ACCESSORS
		this->game = game;
		runService = shared_from(ServiceProvider::create<RunService>(game->getDataModel().get()));

		shared_ptr<DataModel> dataModel = game->getDataModel();
		if (game->getOverlayDataModel())
			dataModel = game->getOverlayDataModel();

		m_unk0x38.reset(new UserInputSleepingJob(this, dataModel));
		TaskScheduler::singleton().add(m_unk0x38);
#else
		STUB(0x0045e630);
		this->game = game;
		m_unk0x38.reset();
#endif
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
			accelerators.resize(count);
			if (count)
				CopyAcceleratorTable(hAccel, &accelerators[0], count);
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

	// 81.3%: 2013 keeps an EH slot for the by-value arbiter argument (8 more bytes of frame). With
	// setGame's written body as its caller (App accessor probe) it scores 98.9%, the EH table aside.
	// STUB: PLAYER 0x004627b0
	UserInputSleepingJob::UserInputSleepingJob(UserInput* userInput, shared_ptr<DataModelArbiter> arbiter)
		: SleepingJob("UserInputSleeping", DataModelJob::Write, false, arbiter, Time::Interval(0.01), 60.0)
		, userInput(userInput)
	{
	}

	// Handles the queued messages on the DataModel's write job.
	// STUB: PLAYER 0x00462920
	TaskScheduler::StepResult UserInputSleepingJob::stepDataModelJob(const Stats& stats)
	{
		STUB(0x00462920);
		messages.clear();
		return TaskScheduler::Stepped;
	}

} // namespace RBX
