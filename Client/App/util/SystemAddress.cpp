/* App/util/SystemAddress.cpp — October 2013 (comparison operators).
   Carried from March 2016 App/util/SystemAddress.cpp (first-party).
   TODO: verify against 2013. */

#include "stdafx.h"

#include "Util/SystemAddress.h"

namespace RBX {

bool SystemAddress::operator==( const RBX::SystemAddress& right ) const
{
	return binaryAddress == right.binaryAddress && port == right.port;
}

bool SystemAddress::operator!=( const RBX::SystemAddress& right ) const
{
	return binaryAddress != right.binaryAddress || port != right.port;
}

bool SystemAddress::operator>( const RBX::SystemAddress& right ) const
{
	return ( ( binaryAddress > right.binaryAddress ) || ( ( binaryAddress == right.binaryAddress ) && ( port > right.port ) ) );
}

bool SystemAddress::operator<( const RBX::SystemAddress& right ) const
{
	return ( ( binaryAddress < right.binaryAddress ) || ( ( binaryAddress == right.binaryAddress ) && ( port < right.port ) ) );
}

}	// namespace
