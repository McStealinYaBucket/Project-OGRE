/* Network/API.h -- October 2013 (network API surface, partial).
   Carried from March 2016 Network/include/network/api.h, trimmed to what
   this tree defines so far. 2013 evidence: isTrustedContent 0x8386C0
   (batch694; consumed by the AsyncHttpQueue URL screening helper --
   ContentProvider::isUrl + Boost to_lower + the identifier find + the
   14-suffix trusted list; "roblox.com/"@0xB947D5, ".robloxlabs.com/"
   0xB947E1, "ownership"@0xB94846, "placerolesets"@0xB94850). Neighbouring
   2013 symbols (isPlayerAuthenticationEnabled 0x8385BF, isNetworkClient
   0x8385D1, initWithoutSecurity 0x8385F0) are NOT yet ported -- their
   declarations land with their bodies (the exe link is the real check;
   never declare without a definition).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/Network/API.cpp */

#pragma once

#include <string>
#include <vector>

namespace RBX
{
	class Instance;
	class DataModel;
	namespace Network
	{
		bool isTrustedContent(const char* url);
	}
}
