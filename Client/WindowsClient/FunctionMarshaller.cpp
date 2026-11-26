/* WindowsClient/FunctionMarshaller.cpp -- October 2013 client shell (PLAYER only; path from the 2016
   tree, see FunctionMarshaller.h). Written so far: Submit, from the 2013 bytes. */

#include "FunctionMarshaller.h"

namespace RBX {

	// STUB: PLAYER 0x00437500
	void FunctionMarshaller::Submit(boost::function<void()> job)
	{
		boost::function<void()>* f = new boost::function<void()>(job);
		asyncCalls.push(f);
		if (postedAsyncMessage.swap(1) == 0)
			PostMessage(WM_ASYNCEVENT, 0, 0);
	}

} // namespace RBX
