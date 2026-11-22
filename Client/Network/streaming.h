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
