/* WindowsClient/UserInput.h -- October 2013 client shell (PLAYER only). No 2013 __FILE__ string names
   UserInput.cpp; the path is the 2016 tree's. The game window's input (RTTI .?AVUserInput@RBX@@,
   vftable PLAYER 0x00be4d9c, base UserInputBase whose ctor is App's 0x0072af80).
   Layout from the ctor (PLAYER 0x0045e300; LTCG: `this` in esi, ret 0x10 for (HWND, View*,
   shared_ptr<Game>)) and PostUserInputMessage:
     +0x38  shared_ptr<UserInputSleepingJob>  setGame (0x0045e630) creates it (`new` 0x260,
            ctor 0x004627b0) and adds it to the TaskScheduler; PostUserInputMessage queues
            each message on it and wakes it. 2016 handles the message right away instead.
     +0x40  RobloxCriticalSection (ctor 0x0045e190, dtor 0x0045e5c0), +0xa4 diKeys[256],
     +0x2bc wnd, +0x2d4 the View, +0x2e4 hArrow, +0x2f4 hInvisibleCursor (all from the ctor);
     +0x2ec shared_ptr<Game> game  setGame assigns it; PostUserInputMessage calls getDataModel on it.
   Only what PostUserInputMessage uses is declared so far. */

#pragma once

#include <windows.h>
#include <atlbase.h>
#include <atlsync.h>

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "rbx/threadsafe.h"
#include "Util/UserInputBase.h"

#include "decomp.h"

// No DirectX SDK in this tree: the DirectInput 8 names UserInput uses (dinput.h, DIRECTINPUT_VERSION
// 0x0800). PLAYER: DirectInput8Create 0x009f09da, IID_IDirectInput8A 0x00bc3628.
struct IDirectInput8A : public IUnknown
{
};
struct IDirectInputDevice8A : public IUnknown
{
};
extern "C" const IID IID_IDirectInput8A;
extern "C" HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut,
	LPUNKNOWN punkOuter);

namespace RBX {

	class Game;
	class RunService;
	class View;

	// App's v8datamodel/SleepingJob.h is not in this tree yet: only the call UserInput makes.
	class SleepingJob
	{
	public:
		void wake(); // PLAYER 0x0072b220 (after SleepingJob::error)
	};

	// RTTI .?AVUserInputSleepingJob@RBX@@ (vftable 0x00be4e18, ctor 0x004627b0, SIZE 0x260).
	class UserInputSleepingJob : public SleepingJob
	{
	public:
		// One queued window message (12 bytes: PostUserInputMessage fills [esp+0x14..0x1c]).
		struct Message
		{
			UINT uMsg;
			WPARAM wParam;
			LPARAM lParam;
		};

	private:
		undefined m_unk0x00[0x238 - 0x00]; // 0x00

	public:
		// Pushed through the shell's safe_queue::push (lock at +0x258).
		rbx::safe_queue<Message> messages; // 0x238
		undefined4 m_unk0x25c;             // 0x25c the ctor's UserInput*
	};

	// clang-format off
	// TEMPLATE: PLAYER 0x00460ee0
	// ?push@?$safe_queue@UMessage@UserInputSleepingJob@RBX@@@rbx@@QAEXABUMessage@UserInputSleepingJob@RBX@@@Z

	// 2013 inlines queue::push here (`this` in eax, the value in ebx).
	// TEMPLATE: PLAYER 0x00461700
	// ?push_back@?$deque@UMessage@UserInputSleepingJob@RBX@@V?$allocator@UMessage@UserInputSleepingJob@RBX@@@std@@@std@@QAEXABUMessage@UserInputSleepingJob@RBX@@@Z
	// clang-format on

	// 2016's class; 2013's ctor is out of line (LTCG: `this` pushed, ret 4).
	// SIZE 0x54
	class RobloxCriticalSection
	{
	public:
		int callers;                    // 0x00
		std::string recentCaller;       // 0x04
		std::string olderCaller;        // 0x20
		ATL::CCriticalSection diSection; // 0x3c

		RobloxCriticalSection(); // PLAYER 0x0045e190
	};

	// VTABLE: PLAYER 0x00be4d9c
	// SIZE 0x2f8
	class UserInput : public UserInputBase
	{
		// Names are 2016's where the ctor initialises the same slot the same way. 2013 has no
		// steppedConnection, inputObjectMap, sdlGameController or previousCursorPosFraction.
		boost::shared_ptr<UserInputSleepingJob> m_unk0x38;      // 0x38
		mutable RobloxCriticalSection diSection;               // 0x40 ctor 0x0045e190, dtor 0x0045e5c0
		bool isMouseCaptured;                                  // 0x94
		G3D::Vector2 wrapMousePosition;                        // 0x98 centerCursor
		bool wrapping;                                         // 0xa0
		bool rightMouseDown;                                   // 0xa1
		bool autoMouseMove;                                    // 0xa2
		bool mouseButtonSwap;                                  // 0xa3
		BYTE diKeys[256];                                      // 0xa4
		int externallyForcedKeyDown;                           // 0x1a4
		HKL layout;                                            // 0x1a8
		BYTE keyboardState[256];                               // 0x1ac
		std::vector<ACCEL> accelerators;                       // 0x2ac
		HWND wnd;                                              // 0x2bc
		CComPtr<IDirectInput8A> diPtr;                         // 0x2c0
		CComPtr<IDirectInputDevice8A> diMousePtr;              // 0x2c4
		CComPtr<IDirectInputDevice8A> diKeyboardPtr;           // 0x2c8
		boost::shared_ptr<RunService> runService;              // 0x2cc setGame: shared_from(RunService)
		View* parentView;                                      // 0x2d4
		G3D::Vector2 posToWrapTo;                              // 0x2d8
		bool leftMouseButtonDown;                              // 0x2e0
		HCURSOR hArrow;                                        // 0x2e4
		bool isMouseInside;                                    // 0x2e8
		bool desireKeyboardAcquired;                           // 0x2e9
		bool isMouseAcquired;                                  // 0x2ea
		bool isKeyboardAcquired;                               // 0x2eb

	public:
		boost::shared_ptr<RBX::Game> game; // 0x2ec
		HCURSOR hInvisibleCursor;          // 0x2f4

		// LTCG: `this` in esi, ret 0x10.
		UserInput(HWND wnd, View* view, boost::shared_ptr<RBX::Game> game); // PLAYER 0x0045e300

		// UserInputBase's slots; 2013 has no destructor slot (UserInputBase declares none).
		/*implement*/ G3D::Vector2 getCursorPosition();                // vtable+0x00 PLAYER 0x00460800
		/*override*/ TextureProxyBaseRef getGameCursor(Adorn* adorn);   // vtable+0x04 PLAYER 0x004603e0
		/*override*/ void removeJobs();                                 // vtable+0x08 PLAYER 0x0045e900
		/*implement*/ void centerCursor();                              // vtable+0x0c PLAYER 0x0045e200
		/*implement*/ bool keyDown(KeyCode code) const;                 // vtable+0x10 PLAYER 0x00460910
		/*implement*/ void setKeyState(KeyCode code, ModCode modCode, char modifiedKey, bool isDown); // vtable+0x14 PLAYER 0x00460940

		void setGame(boost::shared_ptr<RBX::Game> game); // PLAYER 0x0045e630 (LTCG: `this` pushed)

		// LTCG: `this` on the stack, uMsg in ecx, ret 0xc.
		void PostUserInputMessage(UINT uMsg, WPARAM wParam, LPARAM lParam); // PLAYER 0x0045f1b0

	private:
		void createMouse();        // PLAYER 0x0045ec60
		void createKeyboard();     // PLAYER 0x0045ecf0
		void createAccelerators(); // PLAYER 0x0045ed80 (LTCG: `this` pushed)
	};

} // namespace RBX
