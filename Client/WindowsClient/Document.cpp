/* WindowsClient/Document.cpp -- October 2013 client shell (PLAYER only; no 2013 __FILE__ string names
   it, path from the 2016 tree). Stubs for what Application::Shutdown (0x00404f10) calls. Their stand-in
   bodies touch a member so LTCG keeps `this`; __declspec(noinline) keeps them out of the caller. */

#include <stddef.h>

#include "Document.h"

namespace RBX {

	// STUB: PLAYER 0x0041d020
	__declspec(noinline) Document::~Document()
	{
		STUB(0x0041d020);
		m_unk0x1c = NULL;
	}

	// Removes the View's jobs (0x00466a30) and then shuts the Game's DataModel down.
	// STUB: PLAYER 0x0041d270
	__declspec(noinline) void Document::Shutdown()
	{
		STUB(0x0041d270);
		m_unk0x00 = NULL;
	}

} // namespace RBX
