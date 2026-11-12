/* Util/SystemAddress.h — October 2013 (peer address value type).
   Carried verbatim from March 2016 App/include/util/SystemAddress.h
   (first-party, header-only part; RakNet-compatible layout).
   TODO: verify against 2013 (member EA pending; operator== has a 2013
   out-of-line instantiation per updatePhysicsInstructions 0x2D9E72).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/include/util/SystemAddress.h */

#pragma once

// RakNet doesn't namespace this - ouch
namespace RBX {

class SystemAddress
{
public:
	unsigned int binaryAddress;
	unsigned short port;

	SystemAddress()
		: binaryAddress(0xFFFFFFFF)
		, port(0xFFFF)
	{}

	SystemAddress(unsigned int binaryAddress, unsigned short port)
		: binaryAddress(binaryAddress)
		, port(port)
	{}

	bool empty() const
	{
		return binaryAddress == 0xFFFFFFFF && port == 0xFFFF;
	}

	void clear()
	{
		binaryAddress = 0xFFFFFFFF;
		port = 0xFFFF;
	}

	unsigned int getAddress() const { return binaryAddress; }
	unsigned short getPort() const { return port; }

	bool operator == ( const SystemAddress& right ) const;
	bool operator != ( const SystemAddress& right ) const;
	bool operator > ( const SystemAddress& right ) const;
	bool operator < ( const SystemAddress& right ) const;
};

}	// namespace
