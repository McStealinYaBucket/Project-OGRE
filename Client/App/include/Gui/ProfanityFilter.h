
#pragma once

#include "Util/FastFlags.h"
#include <string>

namespace RBX
{
	// Evidence: ProfanityFilter::ContainsProfanity 0x5D2104 (323B); the
	// FLog::Asserts body strings name
	// "/Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/
	// ProfanityFilter.cpp" line 85 -> this source file.  The
	// ChatOutput::onGameChatMessage 0x5C2DB8 gate (0x5C2DC6-0x5C2DE8)
	// reads FFlag::LocalTextFiltering (off_EB8418 deref -> 0xF88527)
	// before calling it.
	class ProfanityFilter
	{
	public:
		// mac: 0x5D2104: static (mangle __ZN3RBX15ProfanityFilter17-
		// ContainsProfanityERKSs).  The head gate returns false when
		// FFlag::LocalTextFiltering is off (0x5D2110 area), then
		// ScopedSingleton<ProfanityFilter>::getInstance() (with the
		// init-count ReleaseAssert boilerplate) and the blocklist walk
		// over a copy of the text.  Landed keeps the gate + the worker
		// route (the singleton machinery unported -- DELTA).
		static bool ContainsProfanity(const std::string& text);

	private:
		// The blocklist walk; landed DELTA stub (returns false) -- the
		// binary worker is the ScopedSingleton instance's method
		// (decompile: ContainsProfanityWorker(v3, v6)); landed routes it
		// static so the ContainsProfanity contract stays callable without
		// the unported singleton machinery.
		static bool ContainsProfanityWorker(const std::string& text);
	};
} // namespace RBX
