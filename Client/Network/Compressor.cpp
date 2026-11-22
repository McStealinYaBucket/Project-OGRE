/* Copyright 2003-2008 ROBLOX Corporation, All Rights Reserved */
// Network/Compressor.cpp -- October 2013, partial. Asserts on their 2013 lines (56 writeRotation,
// 103 writeTranslation, 156 readRotation, 201 readTranslation). writeRotation (PLAYER 0x005f3ef0) and readRotation (0x005ed9b0) are not written yet: readRotation's HEAVILY_COMPRESSED case tests an FFlag (getter 0x005a6660) and then calls a static reader at 0x005ed8a0 ("Failed to read rotation quaternion"), which 2016 does not have. The boost iostreams gzip bodies between them come from a compression helper in this .obj that is not identified yet. Address order is VC9's (callees first): readCompressionType 0x005ece30, readRotation, writeTranslation 0x005ee1d0, readTranslation 0x005ee3d0, ..., writeRotation 0x005f3ef0.
#include "Compressor.h"
#include "Streaming.h"
#include <stdexcept>
using namespace RBX;
using namespace RBX::Network;
enum
{
	translationBits0 = 15,
	translationBits1 = 14,
	translationBits2 = 15
};
// GLOBAL: PLAYER 0x00b26b90
// GLOBAL: STUDIO 0x00bdea60
static const float translationMin[] = { -1024, -512, -1024 };
// GLOBAL: PLAYER 0x00b26b9c
// GLOBAL: STUDIO 0x00bdea6c
static const float translationMax[] = { 1024, 512, 1024 };











































// FUNCTION: PLAYER 0x005ee1d0
// FUNCTION: STUDIO 0x00686400
void Compressor::writeTranslation(RakNet::BitStream& bitStream, const Vector3& translation, CompressionType compressionType)
{
	if ((compressionType == Compressor::HEAVILY_COMPRESSED) && !canHeavilyCompressTranslation(translation)) {
		compressionType = Compressor::RAKNET_COMPRESSED;
	}
	writeCompressionType(bitStream, compressionType);
	switch (compressionType)
	{
	case Compressor::UNCOMPRESSED:
		{
			RBX::operator<<(bitStream, translation.x);
			RBX::operator<<(bitStream, translation.y);
			RBX::operator<<(bitStream, translation.z);
			break;
		}
	case Compressor::RAKNET_COMPRESSED:
		{
			bitStream.WriteVector(translation.x, translation.y, translation.z);
			break;
		}
	case Compressor::HEAVILY_COMPRESSED:
		{
			// p = (p_i - min)*shift/(max-min)
			unsigned short x = (unsigned short)((translation.x - translationMin[0])*(1 << translationBits0)/(translationMax[0]-translationMin[0]));
			unsigned short y = (unsigned short)((translation.y - translationMin[1])*(1 << translationBits1)/(translationMax[1]-translationMin[1]));
			unsigned short z = (unsigned short)((translation.z - translationMin[2])*(1 << translationBits2)/(translationMax[2]-translationMin[2]));
			// Check for overflow (underflow shouldn't be possible)
			if (x >= 1 << translationBits0) x = 0xFFFF;
			if (y >= 1 << translationBits1) y = 0xFFFF;
			if (z >= 1 << translationBits2) z = 0xFFFF;
			bitStream.WriteBits((const unsigned char*) &x, translationBits0);
			bitStream.WriteBits((const unsigned char*) &y, translationBits1);
			bitStream.WriteBits((const unsigned char*) &z, translationBits2);
			break;
		}
	default:
		{
			RBXASSERT(0);
		}
	}
}
bool Compressor::canHeavilyCompressTranslation(const Vector3& translation)
{
	return !(	translation.x>translationMax[0] || translation.x<translationMin[0] ||
				translation.y>translationMax[1] || translation.y<translationMin[1] ||
				translation.z>translationMax[2] || translation.z<translationMin[2]	);
}



















































// FUNCTION: PLAYER 0x005ee3d0
// FUNCTION: STUDIO 0x00686600
void Compressor::readTranslation(RakNet::BitStream& bitStream, Vector3& translation)
{
	CompressionType compressionType = readCompressionType(bitStream);
	switch (compressionType)
	{
	case UNCOMPRESSED:
		{
			RBX::operator>>(bitStream, translation.x);
			RBX::operator>>(bitStream, translation.y);
			RBX::operator>>(bitStream, translation.z);
			break;
		}
	case RAKNET_COMPRESSED:
		{
			if (!bitStream.ReadVector(translation.x, translation.y, translation.z))
				throw std::runtime_error("Compressor::readTranslation failed reading vector");
			break;
		}
	case HEAVILY_COMPRESSED:
		{
			unsigned short x,y,z = 0;
			if (!bitStream.ReadBits((unsigned char*) &x, translationBits0))
				throw std::runtime_error("Compressor::readTranslation failed reading x");
			if (!bitStream.ReadBits((unsigned char*) &y, translationBits1))
				throw std::runtime_error("Compressor::readTranslation failed reading y");
			if (!bitStream.ReadBits((unsigned char*) &z, translationBits2))
				throw std::runtime_error("Compressor::readTranslation failed reading z");
			// p_i = p/shift * (max-min) + min
			translation.x = (float)x * (1.0f/(1 << translationBits0) * (translationMax[0]-translationMin[0])) + translationMin[0];
			translation.y = (float)y * (1.0f/(1 << translationBits1) * (translationMax[1]-translationMin[1])) + translationMin[1];
			translation.z = (float)z * (1.0f/(1 << translationBits2) * (translationMax[2]-translationMin[2])) + translationMin[2];
			break;
		}
	default:
		{
			RBXASSERT(0);
		}
	}
}

void Compressor::writeCompressionType(RakNet::BitStream& bitStream, CompressionType compressionType)
{
	unsigned short temp = compressionType;
	bitStream.WriteBits((const unsigned char*) &temp, 2);
}

// FUNCTION: PLAYER 0x005ece30
// FUNCTION: STUDIO 0x00684e60
Compressor::CompressionType Compressor::readCompressionType(RakNet::BitStream& bitStream)
{
	unsigned short temp = 0;
	if (!bitStream.ReadBits((unsigned char*) &temp, 2))
		throw std::runtime_error("Compressor::readTranslation failed reading compressionType");

	CompressionType answer = static_cast<CompressionType>(temp);
	return answer;
}
