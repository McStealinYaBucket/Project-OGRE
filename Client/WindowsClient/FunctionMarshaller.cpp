/* WindowsClient/FunctionMarshaller.cpp -- October 2013 client shell (PLAYER only; path from the 2016
   tree, see FunctionMarshaller.h). Written so far: Submit, from the 2013 bytes. */

#include "FunctionMarshaller.h"

#include "decomp.h"

namespace RBX {

	// STUB: PLAYER 0x00437180
	__declspec(noinline) void FunctionMarshaller::ReleaseWindow(FunctionMarshaller* marshaller)
	{
		STUB(0x00437180);
		::DestroyWindow(marshaller->m_hWnd);
	}

	// Left (74-75%): 2013 inlines function0's copy ctor (`[esi]=0; call assign_to_own` 0x00437a90) where
	// our LTCG inlines assign_to_own into an out-of-line copy ctor. The assign_to_own bodies are byte-
	// identical; a second LTCG caller (function0::operator=) does not change it, /Os does (but /Os is
	// not the shell's flag set). The same choice costs stopAction (assign_to 0x0043d110) and the
	// ScreenshotVerb ctor, so it is LTCG context, expected to follow the rest of the shell.
	// STUB: PLAYER 0x00437500
	void FunctionMarshaller::Submit(boost::function<void()> job)
	{
		boost::function<void()>* f = new boost::function<void()>(job);
		asyncCalls.push(f);
		if (postedAsyncMessage.swap(1) == 0)
			PostMessage(WM_ASYNCEVENT, 0, 0);
	}

} // namespace RBX
