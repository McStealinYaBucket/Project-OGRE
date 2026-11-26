/* Copyright 2003-2006 ROBLOX Corporation, All Rights Reserved */

// Network/Streaming.h -- October 2013, partial. Included as "Streaming.h": PLAYER's __FILE__ for it is
// c:\buildagent\work\407e17641b466b38\client\network\Streaming.h (found next to its includer).
// Its line 49 holds RBXASSERT(iter != descToId.end()) in a descriptor-dictionary send (PLAYER
// 0x00573920, a ServerReplicator.obj copy); that template is not written yet, so nothing here may
// move onto line 49 with an assert until it is.

#pragma once

#include "BitStream.h"
#include "reflection/object.h"
#include "reflection/EnumConverter.h"
#include "reflection/property.h"
#include "Util/BrickColor.h"
#include "V8Tree/Instance.h"
#include "Util/Guid.h"
#include "Dictionary.h"
#include <map>
#include <vector>

namespace RBX {
namespace Network {

// ios: names and parameter lists (2013 has no enumSizeMSB parameter; 2016 added it).
void serializeEnum(const Reflection::EnumDescriptor* enumDesc, const Reflection::Variant& value, RakNet::BitStream& bitStream);
void deserializeEnum(const Reflection::EnumDescriptor* enumDesc, Reflection::Variant& result, RakNet::BitStream& bitStream);
void serializeEnumProperty(const Reflection::ConstProperty& property, RakNet::BitStream& bitStream);
void deserializeEnumProperty(Reflection::Property& property, RakNet::BitStream& bitStream);

} // namespace Network

RakNet::BitStream& operator<<(RakNet::BitStream& stream, bool value);
RakNet::BitStream& operator<<(RakNet::BitStream& stream, long value);
RakNet::BitStream& operator<<(RakNet::BitStream& stream, const unsigned long long& value);
RakNet::BitStream& operator<<(RakNet::BitStream& stream, int value);
RakNet::BitStream& operator<<(RakNet::BitStream& stream, unsigned int value);
RakNet::BitStream& operator<<(RakNet::BitStream& stream, unsigned char value);
RakNet::BitStream& operator<<(RakNet::BitStream& stream, float value);
RakNet::BitStream& operator<<(RakNet::BitStream& stream, double value);
RakNet::BitStream& operator<<(RakNet::BitStream& stream, const std::string& value);

// Reads one value or throws std::runtime_error; only the explicit specializations in Streaming.cpp exist.
template<class T>
RakNet::BitStream& operator>>(RakNet::BitStream& stream, T& value);

} // namespace RBX

namespace RBX {
namespace Network {

// RTTI: IdSerializer : Instance (vftable PLAYER 0x00b1d4dc, 31 slots: Instance's 29 plus setRefValue
// and setMaxGuidIndexBit). Layout from the ctor 0x005aafd0 and the implicit dtor 0x005a53a0; size
// 0x1094. 2013 has no serverScope and no waitItemsMutex (2016 added both), and holds the registry in
// a shared_ptr (iOS: shared_ptr<GuidItem<Instance>::Registry>), not an intrusive_ptr.
class IdSerializer : public Instance
{
	typedef Instance Super;

protected:
	SharedScopeDictionary scopeNames;                                  // 0x60
	shared_ptr<GuidItem<Instance>::Registry> guidRegistry;            // 0x107c

	// iOS: IdSerializer::WaitItem; 0xc bytes (_Destroy 0x0058ce50 releases +8).
	struct WaitItem
	{
		const Reflection::RefPropertyDescriptor* desc;
		shared_ptr<Instance> instance;
	};
	// iOS: std::map<Guid::Data, std::vector<WaitItem> > (node isnil at +0x25).
	typedef std::map<RBX::Guid::Data, std::vector<WaitItem> > WaitItemMap;
	WaitItemMap waitItems;                                             // 0x1084
	int guidIndexBits;                                                 // 0x1090 ctor 0x20; setMaxGuidIndexBit

	virtual void setRefValue(WaitItem& wi, Instance* instance);        // vtable+0x70

	// Drops the registry; a new provider's GuidRegistryService supplies it (create 0x00563c50).
	/*override*/ void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider); // vtable+0x20

public:
	IdSerializer();

	virtual void setMaxGuidIndexBit(int bits);                         // vtable+0x74
};

} // namespace Network
} // namespace RBX
