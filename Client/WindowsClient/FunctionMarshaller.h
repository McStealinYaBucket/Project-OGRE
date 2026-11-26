/* WindowsClient/FunctionMarshaller.h -- October 2013 client shell (PLAYER only). No 2013 __FILE__ string
   names FunctionMarshaller.cpp; the path is the 2016 tree's. Marshals a function onto a window's thread.
   Declared so far: what Submit (PLAYER 0x00437500) uses. Layout from Submit:
     m_hWnd +0x04 (CWindow, after CMessageMap's vptr), asyncCalls +0x24, postedAsyncMessage +0x48
     (swap goes through RbxInterlockedExchange). LTCG: Submit takes `this` on the stack (ret 0x24). */

#pragma once

#include <atlbase.h>
#include <atlwin.h>

#include <boost/function.hpp>

#include "rbx/threadsafe.h"
#include "rbx/atomic.h"

namespace RBX {

	class FunctionMarshaller : public ATL::CWindowImpl<FunctionMarshaller>
	{
		static const int WM_ASYNCEVENT = WM_USER + 102; // 0x466

		rbx::safe_queue<boost::function<void()>*> asyncCalls; // 0x24
		rbx::atomic<LONG> postedAsyncMessage;                 // 0x48

	public:
		// Submits a function to be executed by the window's thread.
		void Submit(boost::function<void()> job);
	};

} // namespace RBX
