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
#include "Network/Client.h"
#include "Network/Server.h"
#include "Network/Player.h"
#include "Network/Players.h"
#include "Peer.h"
#include "Replicator.h"
#include "GuidRegistryService.h"
#include "V8DataModel/ContentProvider.h"
#include "V8DataModel/GlobalAdvancedSettings.h"
#include "V8DataModel/NetworkSettings.h"

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

// 2013 API.obj registers the Network classes: its COMDAT copies of their classDescriptor()s
// (PLAYER 0x00525da0-0x005268b0) read the s-names from memory, as a TU that sees them only as
// externs does.
RBX_REGISTER_CLASS(RBX::Network::Client);
RBX_REGISTER_CLASS(RBX::Network::Server);
RBX_REGISTER_CLASS(RBX::Network::Player);
RBX_REGISTER_CLASS(RBX::Network::Players);
RBX_REGISTER_CLASS(RBX::NetworkSettings);
RBX_REGISTER_CLASS(RBX::Network::Peer);
// 2013 order: Marker (not written yet), then Replicator (initializer PLAYER 0x00a5ee00), ServerReplicator.
namespace RBX { namespace Reflection {
// GLOBAL: PLAYER 0x00dd8bac
// GLOBAL: STUDIO 0x00f5f98c
template<> ClassRegistrar<Network::Replicator> ClassRegistrar<Network::Replicator>::registrar(0);
} }
RBX_REGISTER_CLASS(RBX::Network::GuidRegistryService);

// ios: 0x00957740. Holds a VMProtect marker region in both builds (PLAYER 0x00526c6a, 137 B), so it stays a STUB; kept for
// what it instantiates here: GlobalAdvancedSettings::classDescriptor (PLAYER 0x005268b0) and
// GlobalAdvancedSettingsItem<NetworkSettings>::singleton (0x00521d60) come from API.obj in 2013.
// STUB: PLAYER 0x00526c50
// STUB: STUDIO 0x006079c0
void RBX::Network::initWithoutSecurity()
{
	Client::classDescriptor();
	Server::classDescriptor();
	Player::classDescriptor();
	Players::classDescriptor();
	GlobalAdvancedSettings::classDescriptor();
	NetworkSettings::classDescriptor();
	NetworkSettings::singleton();
}

// The RakNet incoming password Server::onCreateRakPeer sets (2016 name). Default-constructed by the
// API.obj initializer PLAYER 0x00a5ece0.
// GLOBAL: PLAYER 0x00dd6670
// GLOBAL: STUDIO 0x00f5e690
namespace RBX { namespace Network { std::string versionB; } }
