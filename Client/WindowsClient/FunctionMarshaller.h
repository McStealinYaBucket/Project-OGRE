/* WindowsClient/FunctionMarshaller.h -- October 2013 client shell (PLAYER only). No 2013 __FILE__ string
   names FunctionMarshaller.cpp; the path is the 2016 tree's. Marshals a function onto a window's thread.
   Declared so far: what Submit (PLAYER 0x00437500) and ReleaseWindow (0x00437180) use. Layout:
     m_hWnd +0x04 (CWindow, after CMessageMap's vptr), asyncCalls +0x24, postedAsyncMessage +0x48
     (swap goes through RbxInterlockedExchange), refCount +0x4c, threadID +0x50 (ReleaseWindow). LTCG: Submit takes `this` on the stack (ret 0x24). */

#pragma once

#include <atlbase.h>
#include <atlwin.h>

#include <map>

#include <boost/function.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include "rbx/threadsafe.h"
#include "rbx/atomic.h"

namespace RBX {

	class FunctionMarshaller : public ATL::CWindowImpl<FunctionMarshaller>
	{
		static const int WM_ASYNCEVENT = WM_USER + 102; // 0x466

		// One marshaller per thread. The SAFE_STATIC accessor is 0x00436c70 (value at 0x00e25010, the
		// mutex at +0xc), its call_once initializer 0x00436ce0.
		struct StaticData
		{
			std::map<DWORD, FunctionMarshaller*> windows;      // 0x00
			boost::recursive_mutex windowsCriticalSection;     // 0x0c
			~StaticData();
		};
		SAFE_STATIC(StaticData, staticData)

		rbx::safe_queue<boost::function<void()>*> asyncCalls; // 0x24
		rbx::atomic<LONG> postedAsyncMessage;                 // 0x48
		int refCount;                                         // 0x4c ReleaseWindow
		DWORD threadID;                                       // 0x50 the windows key ReleaseWindow erases

	public:
		// Drops a reference to the window and destroys it with the last one. LTCG: window in esi.
		static void ReleaseWindow(FunctionMarshaller* window); // PLAYER 0x00437180

		// Submits a function to be executed by the window's thread.
		void Submit(boost::function<void()> job);
	};

} // namespace RBX
