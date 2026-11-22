/* App/v8kernel/Connector.cpp -- October 2013 (connector base).
   Body: computeCanThrottle 0x526464 (batch257). */

#include "stdafx.h"

#include "V8Kernel/Connector.h"
#include "V8Kernel/Body.h"

namespace RBX {

	// mac: 0x526464 (batch257: getBody(0/1) via vtbl+28, AND of Body+56).
	// FUNCTION: PLAYER 0x008b9d80
	// FUNCTION: STUDIO 0x009c3ad0
	bool Connector::computeCanThrottle()
	{
		return (getBody(body0)->getCanThrottle() && getBody(body1)->getCanThrottle());


	}

} // namespace RBX
