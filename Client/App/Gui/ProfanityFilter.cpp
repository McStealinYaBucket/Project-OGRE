// ProfanityFilter.cpp -- reconstructed from October 2013 binary dbq24.
// Source of truth: symbolled Mach-O Studio binary (0.126.0.39503), EAs noted per function.
// Self-audit: ContainsProfanity landed from the decompile (0x5D2104,
// 323B).  DELTAS: (1) the ScopedSingleton<ProfanityFilter>::getInstance
// machinery (FLog::Asserts + ReleaseAssert "instance->getInitCount() <=
// 1 file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/
// ProfanityFilter.cpp line: 85") is unported -- the singleton has no
// observable effect on the bool contract until the worker lands; (2)
// ContainsProfanityWorker (the blocklist walk right after 0x5D2104) is
// undecoded -- landed stub returns false, so with LocalTextFiltering on
// (the shipped default, flag byte 0x01 @0xF88527) the onGameChatMessage
// gate never screens text yet.

#include "stdafx.h"

#include "Gui/ProfanityFilter.h"
#include "decomp.h"

namespace RBX
{
	// STUB: PLAYER 0x00668e90
	// STUB: STUDIO 0x008a8420
	bool ProfanityFilter::ContainsProfanityWorker(const std::string& text)
	{
		// DELTA stub -- see the file header.  The 0x5D2104 tail passes a
		// shared_ptr<ProfanityFilter> + a string copy into the worker;
		// returning false keeps the caller-side verdict false.
		STUB(0x00668e90);
		return false;
	}

	// mac: 0x5D2104: byte gate on FFlag::LocalTextFiltering first (read again
	// inside the function -- the ChatOutput::onGameChatMessage caller
	// checks the same flag before calling, so the double read is in both
	// call frames); then the worker over the text copy.
	// STUB: PLAYER 0x006690f0
	// STUB: STUDIO 0x008a8680
	bool ProfanityFilter::ContainsProfanity(const std::string& text)
	{
		if (!FFlag::LocalTextFiltering)
			return false;
		return ContainsProfanityWorker(text);
	}
} // namespace RBX
