// Base/include/util/SafeToLower.h: October 2013 reconstruction (Project OGRE).
// Evidence: PLAYER 0x00827550 ?safeToLower@RBX@@YAXAAV?$basic_string...@Z (WinInet.cpp's copy);
// iOS 1029 has it as a local (inline) RBX::safeToLower(std::string&). The 2013 body tests isupper
// before calling tolower, one character at a time.
#pragma once

namespace RBX {

inline void safeToLower(std::string& text)
{
	for (unsigned n = 0; n < text.size(); ++n)
		if (isupper(text[n]))
			text[n] = tolower(text[n]);
}

}
