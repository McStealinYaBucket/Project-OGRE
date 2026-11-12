/* Network/API.cpp -- October 2013 (isTrustedContent; rest of TU pending).
   Body carried verbatim from March 2016 Network/API.cpp:201-248; the 2013
   binary 0x8386C0 matches the carried text (isRobloxLabsUrl flag, both
   identifier finds, sizeof(...)-1 iterator seeding, the whitespace skip,
   then the 14-suffix ||-chain; all needle strings PE-verified). The two
   identifier #defines sat at the top of this TU in 2016 (lines 34-35);
   kept here since this TU is their only user.
   The remainder of the 2016 TU (replication peers, security init,
   setSecurityVersions, versionB/securityKey, marker class, debugged-check
   thread) is NOT ported yet -- those bodies queue behind the
   NetworkClient/Server port (see QUEUE).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/Network/API.cpp */

#include "stdafx.h"

#include "Network/API.h"
#include "V8DataModel/ContentProvider.h"

#include <boost/algorithm/string.hpp>

#define ROBLOX_URL_IDENTIFIER "roblox.com/"
#define ROBLOXLABS_URL_IDENTIFIER ".robloxlabs.com/"

// FUNCTION: PLAYER 0x00521620
// FUNCTION: STUDIO 0x00605950
bool RBX::Network::isTrustedContent(const char* url)
{
	if(!RBX::ContentProvider::isUrl(url))
		return false;

	std::string urlString(url);
	boost::algorithm::to_lower(urlString);

    bool isRobloxLabsUrl = false;
	size_t foundPos = urlString.find(ROBLOX_URL_IDENTIFIER);
	if(foundPos == std::string::npos)
    {
        foundPos = urlString.find(ROBLOXLABS_URL_IDENTIFIER);
        if(foundPos == std::string::npos)
            return false;
        
        isRobloxLabsUrl = true;
    }
    
	urlString = urlString.substr(foundPos,std::string::npos); // remove all of string before URL_IDENTIFIER
    
     // put our iterator at end of URL_IDENTIFIER
    if(isRobloxLabsUrl)
        foundPos = sizeof(ROBLOXLABS_URL_IDENTIFIER) - 1;
    else
        foundPos = sizeof(ROBLOX_URL_IDENTIFIER) - 1;

	while(foundPos < urlString.size() && (urlString[foundPos] == '\\' || urlString[foundPos] == '/'))
		++foundPos;

	if(foundPos >= urlString.size())
		return false;

	return	urlString.substr(foundPos,5)	== "asset"			|| 
		urlString.substr(foundPos,4)		== "game"			|| 
		urlString.substr(foundPos,9)		== "analytics"		||
		urlString.substr(foundPos,3)		== "ide"			|| 
		urlString.substr(foundPos,6)		== "images"			||
		urlString.substr(foundPos,6)		== "thumbs"			||
		urlString.substr(foundPos,2)		== "ui"				||
		urlString.substr(foundPos,11)		== "persistence"	||
		urlString.substr(foundPos,8)		== "rolesets"		||
		urlString.substr(foundPos,4)		== "auth"			||
		urlString.substr(foundPos,8)		== "currency"		||
		urlString.substr(foundPos,11)		== "marketplace"	||
		urlString.substr(foundPos,9)		== "ownership"		||
		urlString.substr(foundPos,13)		== "placerolesets";
}
