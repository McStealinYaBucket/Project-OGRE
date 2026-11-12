/* App/util/LegacyContentTable.cpp — October 2013 MINIMAL (link-complete stubs).
   The real table (rbxasset:// sounds map + lowercase lookup, ~700 lines in the
   2016 App/util/LegacyContentTable.cpp) is TODO: port + verify 2013 contents.
   These minimal bodies keep ContentId linking; FindEntry passes through. */

#include "stdafx.h"

#include "Util/LegacyContentTable.h"

namespace RBX {

	LegacyContentTable::LegacyContentTable()
	{
	}

	const std::string& LegacyContentTable::FindEntry(const std::string& url) const
	{
		return url;
	}

} // namespace RBX
