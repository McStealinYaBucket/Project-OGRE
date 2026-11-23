// Log/include/FastLogStream.h: October 2013 reconstruction (Project OGRE).
// Evidence: the 2013 Log.dll (bin/player) exports these three, at 0x10009880 (file),
// 0x100098d0 (TCP client) and 0x100099c0 (TCP server), and the Rich header counts
// FastLogStream.cpp among its two LTCG objects.
#pragma once

#include "FastLog.h"

namespace FLog {

bool LOGAPI SetLogStreamFile(const char* path);
bool LOGAPI SetLogStreamNetworkClient(const char* address, unsigned int port);
bool LOGAPI SetLogStreamNetworkServer(unsigned int port);

}
