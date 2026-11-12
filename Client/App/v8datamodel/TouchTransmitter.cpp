/* App/v8datamodel/TouchTransmitter.cpp -- October 2013 RBX::TouchTransmitter.
   See include/V8DataModel/TouchTransmitter.h for the disassembly evidence
   (ctor 0x4E2FAA, checkTouch 0x4E3296, checkUntouch 0x4E32C2).

   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/TouchTransmitter.cpp */

#include "stdafx.h"

#include "V8DataModel/TouchTransmitter.h"

namespace RBX {
	const char* const sTouchTransmitter = "TouchTransmitter";

	// mac: 0x4E2FAA.  The four vtable fixups, the reflection class-descriptor
	// touch and the Diagnostics::Countable increment are all done by
	// DescribedCreatable's own constructor, so they do not appear here.
	// STUB: PLAYER 0x0081c590
	// STUB: STUDIO 0x00911870
	TouchTransmitter::TouchTransmitter()
		: debouncer(new TouchDebouncer)
	{
		// mac: 0x4E313F: setName(std::string("TouchInterest")).  This is the
		// last thing the ctor does, and it is the DEFAULT NAME a brand new
		// TouchTransmitter carries.
		setName("TouchInterest");
	}

	// mac: 0x4E31DC.  Deletes the debouncer (the out-of-line
	// boost::scoped_ptr<TouchDebouncer>::~scoped_ptr at 0x4E3C66, which in
	// turn runs std::vector<Item>::~vector at 0x4E3CA0).
	TouchTransmitter::~TouchTransmitter()
	{
		delete debouncer;
	}

	// mac: 0x4E3296 (43 bytes) -- `mov eax,[this+5Ch]; ... mov [esp+8],0; jmp check`
	bool TouchTransmitter::checkTouch(const shared_ptr<PartInstance>& part)
	{
		return debouncer->check(part, TouchPair::Touched);
	}

	// mac: 0x4E32C2 (43 bytes) -- identical, with the immediate 1
	bool TouchTransmitter::checkUntouch(const shared_ptr<PartInstance>& part)
	{
		return debouncer->check(part, TouchPair::Untouched);
	}
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::TouchTransmitter);
}
