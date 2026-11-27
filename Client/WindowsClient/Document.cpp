/* WindowsClient/Document.cpp -- October 2013 client shell (PLAYER only; no 2013 __FILE__ string names
   it, path from the 2016 tree). Written so far: the destructor. Shutdown (0x0041d270) is a stub: under
   a write lock it removes the View's jobs and shuts the DataModel down, taking 2013's
   OverlayDataModel::DualLegacyLock (ctor 0x0068c1f0, the shell's dtor copy 0x0041d550; App does not
   declare it yet) when there is an overlay DataModel and more than one TaskScheduler thread. */

#include "Document.h"

#include "GameVerbs.h"
#include "View.h"

namespace RBX {

	// Every member destroys itself, the View last (~View 0x00465b90, then operator delete).
	// FUNCTION: PLAYER 0x0041d020
	Document::~Document()
	{
	}

	// STUB: PLAYER 0x0041d270
	__declspec(noinline) void Document::Shutdown()
	{
		STUB(0x0041d270);
		m_unk0x00.reset();
	}

} // namespace RBX
