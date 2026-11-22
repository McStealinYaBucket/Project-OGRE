/* Copyright 2003-2006 ROBLOX Corporation, All Rights Reserved */

// Network/Streaming.cpp -- October 2013, partial (the .obj spans PLAYER 0x005a6670-0x005aab90+).
// Written from the 2013 bytes; asserts sit on their 2013 lines (39 41 50 51 59 60 70 71, 1107).
// 2013 differs from 2016 here: no enumSizeMSB parameter and no serializeEnumIndex helper (each
// function writes or reads the bits itself), read failures throw std::runtime_error (2016:
// network_stream_exception), and deserializeEnum asserts the index instead of clamping it.

#include "Streaming.h"

#include "StringCompressor.h"
#include "RbxFormat.h"
#include "Util/Math.h"
#include "Util/Quaternion.h"

#include <stdexcept>
#include <malloc.h>

#define MAX_STRING_SIZE 200000

namespace RBX {

using namespace Reflection;

namespace Network {

// The bits written are getEnumCountMSB()+1 wide; the index is read back into an int.
// serializeEnum writes the Item's value (+0x14), not its index (+0x18, EnumDesc::addPair 0x008217f0),
// although the name and assert say index: PLAYER 0x005a66cd reads [item+0x14].




// FUNCTION: PLAYER 0x005a6670
// FUNCTION: STUDIO 0x006160f0
void serializeEnum(const EnumDescriptor* enumDesc, const Variant& value, RakNet::BitStream& bitStream)
{
	const EnumDescriptor::Item* item = enumDesc->lookup(value);
	RBXASSERT(item);
	size_t valueIndex = item->value;
	RBXASSERT(valueIndex < enumDesc->getEnumCount());
	bitStream.WriteBits((const unsigned char*)&valueIndex, enumDesc->getEnumCountMSB() + 1);
}
// FUNCTION: PLAYER 0x005a6730
// FUNCTION: STUDIO 0x006161b0
void deserializeEnum(const EnumDescriptor* enumDesc, Variant& result, RakNet::BitStream& bitStream)
{
	int value = 0;
	if (!bitStream.ReadBits((unsigned char*)&value, enumDesc->getEnumCountMSB() + 1)) throw std::runtime_error("deserializeEnum failed");
	RBXASSERT(value>=0);
	RBXASSERT(value<(int)enumDesc->getEnumCount());
	if (!enumDesc->convertToValue(value, result)) throw std::runtime_error("deserializeEnum conversion failed");
}
// FUNCTION: PLAYER 0x005a68a0
// FUNCTION: STUDIO 0x00616320
void serializeEnumProperty(const ConstProperty& property, RakNet::BitStream& bitStream)
{
	const EnumPropertyDescriptor& enumDesc = static_cast<const EnumPropertyDescriptor&>(property.getDescriptor()); int value = (int)enumDesc.getIndexValue(property.getInstance());
	RBXASSERT(value>=0);
	RBXASSERT(value<(int)enumDesc.enumDescriptor.getEnumCount());
	bitStream.WriteBits((const unsigned char*)&value, enumDesc.enumDescriptor.getEnumCountMSB() + 1);
}
// FUNCTION: PLAYER 0x005a6960
// FUNCTION: STUDIO 0x006163e0
void deserializeEnumProperty(Property& property, RakNet::BitStream& bitStream)
{
	const EnumPropertyDescriptor& enumDesc = static_cast<const EnumPropertyDescriptor&>(property.getDescriptor());
	int value = 0;
	if (!bitStream.ReadBits((unsigned char*)&value, enumDesc.enumDescriptor.getEnumCountMSB() + 1)) throw std::runtime_error("deserializeEnumProperty failed");
	RBXASSERT(value>=0);
	RBXASSERT(value<(int)enumDesc.enumDescriptor.getEnumCount());
	if (property.getInstance())
		enumDesc.setIndexValue(property.getInstance(), value);
}

} // namespace Network

// The BitStream readers and writers (ios: RBX::operator>><T> and RBX::operator<<). Each reader has
// its own failure message, so each is an explicit specialization; RakNet::Time and size_t are the
// messages' spelling (iOS: unsigned long long and unsigned long). The 4-byte writers (int, long,
// unsigned int, float) have one body: PLAYER keeps it at 0x005a71f0 before >>long, STUDIO at
// 0x00616f90 before >>int, so those two carry the markers. iOS has no operator<<(char); the 8-bit
// writer at 0x005a74f0 is taken as its unsigned char.

// FUNCTION: PLAYER 0x005a6a90
// FUNCTION: STUDIO 0x00616510
RakNet::BitStream& operator<<(RakNet::BitStream& stream, bool value)
{
	stream.Write(value);
	return stream;
}

// FUNCTION: PLAYER 0x005a6ab0
// FUNCTION: STUDIO 0x00616530
template<>
RakNet::BitStream& operator>>(RakNet::BitStream& stream, bool& value)
{
	if (!stream.Read(value))
		throw std::runtime_error("BitStream >> bool failed");
	return stream;
}

// FUNCTION: PLAYER 0x005a6ee0
// FUNCTION: STUDIO 0x00616aa0
template<>
RakNet::BitStream& operator>>(RakNet::BitStream& stream, BrickColor& value)
{
	// 6 bits: an index into the 64-colour 2013 palette (colorPalette() is PLAYER 0x00791f80).
	int index = 0;
	if (!stream.ReadBits((unsigned char*)&index, 6))
		throw std::runtime_error("BitStream >> BrickColor failed");
	value = BrickColor::colorPalette()[index];
	return stream;
}

// FUNCTION: PLAYER 0x005a71f0
RakNet::BitStream& operator<<(RakNet::BitStream& stream, long value)
{
	stream.Write(value);
	return stream;
}

// FUNCTION: PLAYER 0x005a7250
// FUNCTION: STUDIO 0x00616e70
template<>
RakNet::BitStream& operator>>(RakNet::BitStream& stream, long& value)
{
	if (!stream.Read(value))
		throw std::runtime_error("BitStream >> long failed");
	return stream;
}

// FUNCTION: PLAYER 0x005a72d0
// FUNCTION: STUDIO 0x00616ef0
RakNet::BitStream& operator<<(RakNet::BitStream& stream, const unsigned long long& value)
{
	stream.Write(value);
	return stream;
}

// FUNCTION: PLAYER 0x005a72f0
// FUNCTION: STUDIO 0x00616f10
template<>
RakNet::BitStream& operator>>(RakNet::BitStream& stream, unsigned long long& value)
{
	if (!stream.Read(value))
		throw std::runtime_error("BitStream >> RakNet::Time failed");
	return stream;
}

// FUNCTION: STUDIO 0x00616f90
RakNet::BitStream& operator<<(RakNet::BitStream& stream, int value)
{
	stream.Write(value);
	return stream;
}

// FUNCTION: PLAYER 0x005a7370
// FUNCTION: STUDIO 0x00616ff0
template<>
RakNet::BitStream& operator>>(RakNet::BitStream& stream, int& value)
{
	if (!stream.Read(value))
		throw std::runtime_error("BitStream >> int failed");
	return stream;
}

RakNet::BitStream& operator<<(RakNet::BitStream& stream, unsigned int value)
{
	stream.Write(value);
	return stream;
}

// FUNCTION: PLAYER 0x005a73f0
// FUNCTION: STUDIO 0x00617070
template<>
RakNet::BitStream& operator>>(RakNet::BitStream& stream, size_t& value)
{
	if (!stream.Read(value))
		throw std::runtime_error("BitStream >> size_t failed");
	return stream;
}

// FUNCTION: PLAYER 0x005a7470
// FUNCTION: STUDIO 0x006170f0
template<>
RakNet::BitStream& operator>>(RakNet::BitStream& stream, char& value)
{
	if (!stream.Read(value))
		throw std::runtime_error("BitStream >> char failed");
	return stream;
}

// FUNCTION: PLAYER 0x005a74f0
// FUNCTION: STUDIO 0x00617170
RakNet::BitStream& operator<<(RakNet::BitStream& stream, unsigned char value)
{
	stream.Write(value);
	return stream;
}

// FUNCTION: PLAYER 0x005a7510
// FUNCTION: STUDIO 0x00617190
template<>
RakNet::BitStream& operator>>(RakNet::BitStream& stream, unsigned char& value)
{
	if (!stream.Read(value))
		throw std::runtime_error("BitStream >> unsigned char failed");
	return stream;
}

RakNet::BitStream& operator<<(RakNet::BitStream& stream, float value)
{
	stream.Write(value);
	return stream;
}

// FUNCTION: PLAYER 0x005a7590
// FUNCTION: STUDIO 0x00617210
template<>
RakNet::BitStream& operator>>(RakNet::BitStream& stream, float& value)
{
	if (!stream.Read(value))
		throw std::runtime_error("BitStream >> float failed");
	return stream;
}

// FUNCTION: PLAYER 0x005a7610
// FUNCTION: STUDIO 0x00617290
RakNet::BitStream& operator<<(RakNet::BitStream& stream, double value)
{
	stream.Write(value);
	return stream;
}

// FUNCTION: PLAYER 0x005a7670
// FUNCTION: STUDIO 0x006172f0
template<>
RakNet::BitStream& operator>>(RakNet::BitStream& stream, double& value)
{
	if (!stream.Read(value))
		throw std::runtime_error("BitStream >> double failed");
	return stream;
}

// FUNCTION: PLAYER 0x005a76f0
// FUNCTION: STUDIO 0x00617370
RakNet::BitStream& operator<<(RakNet::BitStream& stream, const std::string& value)
{
	uint32_t size = static_cast<uint32_t>(value.size());
	if (size > MAX_STRING_SIZE)
		throw RBX::runtime_error("BitStream string write: String too long: %d", size);
	stream.Write(size);
	RakNet::StringCompressor::Instance()->EncodeString(value.c_str(), static_cast<int>(size + 1), &stream);
	return stream;
}

// FUNCTION: PLAYER 0x005a77a0
// FUNCTION: STUDIO 0x00617420
template<>
RakNet::BitStream& operator>>(RakNet::BitStream& stream, std::string& value)
{
	uint32_t size;
	if (!stream.Read(size))
		throw std::runtime_error("BitStream >> std::string: failed to read length");
	if (size > MAX_STRING_SIZE)
		throw std::runtime_error("BitStream >> std::string: Bad string length");
	char* buffer = (char*)alloca(size + 1);
	RakNet::StringCompressor::Instance()->DecodeString(buffer, static_cast<int>(size + 1), &stream);
	value = buffer;
	return stream;
}

namespace Network {

// FUNCTION: PLAYER 0x005a7bd0
// FUNCTION: STUDIO 0x00617850
void readBrickVector(RakNet::BitStream& stream, G3D::Vector3& value)
{
	// Qualified calls: our staged RakNet still declares RakNet::operator>>/<< templates, which 2016's
	// BitStream.h (and so presumably 2013's) has under "#if 0"; plain stream >> x is ambiguous here.
	bool isBrickLocation;
	RBX::operator>>(stream, isBrickLocation);
	if (isBrickLocation)
	{
		short x = 0;
		unsigned short y = 0;
		short z = 0;
		if (!stream.ReadBits((unsigned char*)&x, 11))
			throw std::runtime_error("readBrickVector x failed");
		if (!stream.ReadBits((unsigned char*)&y, 11))
			throw std::runtime_error("readBrickVector y failed");
		if (!stream.ReadBits((unsigned char*)&z, 11))
			throw std::runtime_error("readBrickVector z failed");

		// Fill in the sign bits:
		if (x & 0x0400) x |= 0xFC00;
		if (z & 0x0400) z |= 0xFC00;

		value.x = float(x) / 2;
		value.y = float(y) / 10;
		value.z = float(z) / 2;
	}
	else
	{
		RBX::operator>>(stream, value.x);
		RBX::operator>>(stream, value.y);
		RBX::operator>>(stream, value.z);
	}
}

} // namespace Network

// FUNCTION: PLAYER 0x005a82b0
// FUNCTION: STUDIO 0x00617f30
template<>
RakNet::BitStream& operator>>(RakNet::BitStream& stream, G3D::CoordinateFrame& value)
{
	Network::readBrickVector(stream, value.translation);

	bool isAxisAligned;
	RBX::operator>>(stream, isAxisAligned);
	if (isAxisAligned)
	{
		int orientId = 0;
		if (!stream.ReadBits((unsigned char*)&orientId, 6))
			throw std::runtime_error("BitStream >> CoordinateFrame orientId failed");
		Math::idToMatrix3(orientId, value.rotation);
	}
	else
	{
		Quaternion q;
		stream.ReadFloat16(q.w, -1.0f, 1.0f);
		stream.ReadFloat16(q.x, -1.0f, 1.0f);
		stream.ReadFloat16(q.y, -1.0f, 1.0f);
		stream.ReadFloat16(q.z, -1.0f, 1.0f);
		q.toRotationMatrix(value.rotation);
		Math::orthonormalizeIfNecessary(value.rotation);
	}
	return stream;
}

} // namespace RBX
