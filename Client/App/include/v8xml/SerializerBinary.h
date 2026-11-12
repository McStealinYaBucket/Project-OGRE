#pragma once

#include <istream>
#include <ostream>
#include <vector>

#include <v8tree/Instance.h>

#include "FastLog.h"

LOGGROUP(Serializer)

namespace RBX
{
	class MemoryOutputStream;
	class MemoryInputStream;

	// 1-byte BinaryPropertyFormat tags live in writePropertyValues/readPropertyValues
	// (0x700E3E/0x6FDC57 — TODO port); the checker below is already exact.
	void readFormatExpected(MemoryInputStream& in, unsigned char expectedFormat);  // 0x6FDBC1

	// Binary int-array codecs (BinCodecs_small.txt; binary-static, exposed here
	// for unit tests — behavior identical).
	// mac: 0x703E38: int array transposed by byte, MSB first.
	void writeInterleavedBytes(MemoryOutputStream& out, const std::vector<int>& values);
	// mac: 0x703C27: zigzag + transposed.
	void writeZigzagInterleaved(MemoryOutputStream& out, const std::vector<int>& values);
	// mac: 0x703F67: delta + 0x703C27.
	void writeDeltaValues(MemoryOutputStream& out, const std::vector<int>& values);
	// mac: 0x703D3F: float array in rotated-byte layout (>>23, >>15, >>7, ROL1).
	void writeFloatArray(MemoryOutputStream& out, const std::vector<float>& values);
	// Inverse of writeInterleavedBytes (0x700D0E-class readers).
	void readUIntArray(MemoryInputStream& in, std::vector<unsigned int>& values, size_t count);
	// Inverse of writeZigzagInterleaved.
	void readIntArray(MemoryInputStream& in, std::vector<int>& values, size_t count);
	// Inverse of writeFloatArray.
	void readFloatArray(MemoryInputStream& in, std::vector<float>& values, size_t count);
	// Inverse of writeDeltaValues: zigzag-decode then cumulative sum (0x700E10).
	void readIdArray(MemoryInputStream& in, std::vector<int>& values, size_t count);
	// Length-prefixed string (inverse of the string branch; 2016 readString).
	void readBinaryString(MemoryInputStream& in, std::string& value);
	// mac: 0x706425 — chunk framing (resolved call convention: out, chunk, tag,
	// flags). 16-byte header: tag[4] + u32 compressedLen + u32 uncompressedLen
	// + u32 reserved(0); payload raw iff flags & sfNoCompression (END always
	// raw). LZ4 via vendored 1.9.4 (block format is version-stable).
	// FLog::Serializer literals TODO.
	void writeChunk(std::ostream& out, MemoryOutputStream& chunk, const char* tag, unsigned int flags);

	// Binary wire tags (BinaryPropertyFormat) — values binary-exact; names ours.
	// 2013 assignments from writePropertyValues 0x700E3E (2016 renumbered CFrame
	// to 16/17 and added 20-25 — NOT backported). 16/17 unused in 2013.
	enum BinaryPropertyFormat
	{
		bpfCFrameChunk = 0,	// CFrame writes TWO tag-0 chunks (orientId, matrix)
		bpfString = 1,		// ProtectedString/std::string AND ContentId (all len+bytes)
		bpfBool = 2,		// raw bytes
		bpfInt = 3,			// zigzag + transposed (0x703C27)
		bpfFloat = 4,		// rotated layout (0x703D3F)
		bpfDouble = 5,		// raw 8-byte
		bpfUDim = 6,		// rotated scales + zigzag offsets (session 69)
		bpfUDim2 = 7,		// X/Y scales + X/Y zigzag offsets (session 69)
		bpfRay = 8,			// Origin/Direction floats (session 48)
		bpfFaces = 9,		// raw mask bytes (session 70)
		bpfAxes = 10,		// raw mask bytes (session 70)
		bpfBrickColor = 11,	// uints, raw transposed (session 47; tag from 0x700E3E)
		bpfColor3 = 12,		// 3x rotated float arrays (session 47; tag from 0x700E3E)
		bpfVector2 = 13,	// X, Y rotated float arrays (session 70)
		bpfVector3 = 14,	// TODO
		bpfVector2int16 = 15, // TODO
		bpfEnum = 18,		// uints, raw transposed (0x703E38)
		bpfRef = 19			// idMap ids, delta + zigzag (0x703F67)
	};

	// mac: 0x700E3E — per-type tag + value arrays. Branches for types with descriptors
	// in the tree are exact; UDim..CFrame fall through to the binary's
	// "Unknown property type for property %s" throw (unreachable until their
	// descriptors land). flags = SerializeFlags (CFrame-only, TODO).
	void writePropertyValues(MemoryOutputStream& out,
		const Reflection::PropertyDescriptor& desc,
		const std::vector<const Instance*>& instances,
		const std::map<const Instance*, int>& idMap,
		unsigned int flags);
	// mac: 0x6FDC57 — per-type tag check + value arrays. Symmetric inverse of
	// writePropertyValues: same branch order, tags, and asserts (isMemberOf
	// via ConstProperty where reads go through getters; setters assert
	// isValueType via Property::setValue). Instances are non-const (both
	// binaries take mutable instance lists on the read side). idMap is the
	// referent-index vector (2016 readPropertyValues 0xA42670 signature).
	// Unreachable-type fallthrough and flag handling TODO as on the write side.
	void readPropertyValues(MemoryInputStream& in,
		const Reflection::PropertyDescriptor& desc,
		const std::vector<Instance*>& instances,
		const std::vector<shared_ptr<Instance> >& idMap);

	namespace SerializerBinary
	{
		enum SerializeFlags
		{
			sfHighCompression = 1 << 0,
			sfNoCompression = 1 << 1,
			sfInexactCFrame = 1 << 2
		};
		
		static const char kMagicHeader[] = "<roblox!";
		
		void serialize(std::ostream& out, const Instance* root, unsigned int flags = 0, const Instance::SaveFilter saveFilter = Instance::SAVE_ALL);
		void serialize(std::ostream& out, const Instances& instances, unsigned int flags = 0);
		
		void deserialize(std::istream& in, Instance* root);
		void deserialize(std::istream& in, Instances& result);
	};
}
