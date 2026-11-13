/* App/v8xml/SerializerBinary.cpp — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation
     (F:\2013Decompile\exports\decomp\SerializerV2_full.cpp.txt,
      BinHelpers.cpp.txt, WriteChildren.cpp.txt)
      - serialize 0x704012: collect direct children passing canWriteChild into a
        vector, then sub_704134(vec, flags). (sub_704134 = binary writer TODO.)
      - deserialize 0x7051EF: delegates to sub_70520C (binary reader TODO).
      - canWriteChild 0x706B22: FW+23 gate (false -> skip); SAVE_GAME(1) writes
        {StarterGuiService, StarterPackService, ServerScriptService};
        SAVE_WORLD(0) writes {Workspace, Lighting, SoundService, ServerStorage,
        ReplicatedStorage}; SAVE_ALL(2) writes everything. isA checks via
        ClassDescriptor::isA in the binary; fastDynamicCast here (equivalent for
        these hierarchies).
      - Instance::writeChildren 0x542444 filters with the SAME function (XML and
        binary share the filter) — see Instance.cpp.
   TODO(decompile): SaveFilter enum-value mapping (WORLD=0/GAME=1/ALL=2 carried
     from 2016 — the restrictive service lists make sense only if partial saves
     exclude subtrees; verify against save outputs); FW+23 flag identity (set =1
     by FWInstance ctor; cleared by ?); sub_704134/sub_70520C binary format;
     writeXml role-gating (CreatorRole 0 forbidden at Instance.cpp:239; roles 1/2
     check ClassDescriptor bitflags +476 bits 2/3).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8xml/SerializerBinary.cpp */

#include "stdafx.h"

#include "V8Xml/Serializer.h"
#include "V8Xml/SerializerBinary.h"
#include "V8Tree/Service.h"
#include "V8DataModel/StarterGuiService.h"
#include "V8DataModel/StarterPackService.h"
#include "V8DataModel/ServerScriptService.h"
#include "V8DataModel/ReplicatedStorage.h"
#include "V8DataModel/Workspace.h"
#include "V8DataModel/Lighting.h"
#include "Util/SoundService.h"
#include "V8DataModel/ServerStorage.h"
#include "Util/FeatherweightDict.h"

#include "reflection/property.h"
#include "reflection/Type.h"
#include "reflection/object.h"
#include "Util/ContentId.h"
#include "Util/UDim.h"
#include "Util/Faces.h"
#include "Util/Axes.h"
#include "G3D/Vector2.h"
#include "Util/BrickColor.h"
#include "G3D/Color3.h"
#include "G3D/Vector3.h"
#include "G3D/CoordinateFrame.h"
#include "RbxG3D/RbxRay.h"
#include "Util/Quaternion.h"
#include "Util/Math.h"
#include <cstdio>
#include <cmath>
#include "Util/ProtectedString.h"

// OGRE vc9: lz4 headers from upstream svn r94 (3rdparty/_downloads/lz4-r94), the revision the 2016
// tree's App/lz4 carries; PLAYER has the decoder (deserialize, "Malformed data (%d != %d)").
#include "lz4.h"
#include "lz4hc.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <stdexcept>
#include <cstddef>

/* MemoryOutputStream / MemoryInputStream -- 2013 defines both here (no MemoryStreams.h:
   the grow assert cites SerializerBinary.cpp:122; the prior tree had them in
   include/v8xml/MemoryStreams.h).
   V8Xml/MemoryStreams.h — October 2013 binary-serializer streams.
   Evidence: symbolled Studio 0.126 decompilation
     (F:\2013Decompile\exports\decomp\BinStreams.txt)
      - MemoryOutputStream::write 0x7074A6: {data +0, size +4, capacity +8};
        grow when size+n > capacity; memcpy at data+size; size += n.
      - MemoryOutputStream::grow 0x7074F2: ReleaseAssert "size > capacity"
        (Client/App/v8xml/SerializerBinary.cpp line 122 — also confirms the
        original TU); doubling loop from 16; new[] + memcpy + delete[].
      - MemoryInputStream::read 0x707954: {data +0, offset +4, size +8};
        bounds-check throws std::runtime_error
        "Read offset is out of bounds while reading %d bytes"; memcpy + advance.
   Member ORDER preserved (+0/+4/+8); MSVC sizes differ (Itanium note).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8xml/SerializerBinary.cpp */

namespace RBX {

	class MemoryOutputStream
	{
	public:
		MemoryOutputStream()
			: data(NULL)
			, size(0)
			, capacity(0)
		{}

		~MemoryOutputStream()
		{
			delete[] data;
		}

		// mac: 0x7074A6.
		void write(const void* src, size_t n)
		{
			if (size + n > capacity)
			{
				grow(size + n);
			}
			memcpy(data + size, src, n);
			size += n;
		}

		const char* getData() const { return data; }
		size_t getSize() const { return size; }

	private:
		// mac: 0x7074F2.
		// STUB: STUDIO 0x007416c0
		void grow(size_t needed)
		{
			// Binary asserts here iff grow was called unnecessarily
			// ("size > capacity", SerializerBinary.cpp:122).
			RBXASSERT(capacity < needed);
			size_t next = 16;
			while (next < needed)
				next *= 2;
			char* grown = new char[next];
			memcpy(grown, data, size);
			delete[] data;
			data = grown;
			capacity = next;
		}

		char* data;			// +0
		size_t size;		// +4
		size_t capacity;	// +8
	};

	class MemoryInputStream
	{
	public:
		MemoryInputStream(const char* data, size_t size)
			: data(data)
			, offset(0)
			, size(size)
		{}

		// mac: 0x707954.
		void read(void* dst, size_t n)
		{
			if (offset + n > size)
				throw std::runtime_error("Read offset is out of bounds while reading %d bytes");
			memcpy(dst, data + offset, n);
			offset += n;
		}

		size_t getOffset() const { return offset; }
		size_t getSize() const { return size; }

	private:
		const char* data;	// +0
		size_t offset;		// +4
		size_t size;		// +8
	};

} // namespace RBX

bool Serializer::canWriteChild(shared_ptr<RBX::Instance> child, RBX::Instance::SaveFilter saveFilter)
{
	// FW+23 inclusion flag (FWInstance ctor sets =1).
	if (!child || !child->fw() || !child->fw()->b23)
		return false;
	if (saveFilter == RBX::Instance::SAVE_GAME)
	{
		if (RBX::Instance::fastDynamicCast<RBX::StarterGuiService>(child.get()))
			return true;
		if (RBX::Instance::fastDynamicCast<RBX::StarterPackService>(child.get()))
			return true;
		return RBX::Instance::fastDynamicCast<RBX::ServerScriptService>(child.get()) != NULL;
	}
	if (saveFilter != RBX::Instance::SAVE_WORLD)
		return true;
	if (RBX::Instance::fastDynamicCast<RBX::Workspace>(child.get()))
		return true;
	if (RBX::Instance::fastDynamicCast<RBX::Lighting>(child.get()))
		return true;
	if (RBX::Instance::fastDynamicCast<RBX::Soundscape::SoundService>(child.get()))
		return true;
	if (RBX::Instance::fastDynamicCast<RBX::ServerStorage>(child.get()))
		return true;
	return RBX::Instance::fastDynamicCast<RBX::ReplicatedStorage>(child.get()) != NULL;
}

namespace RBX {

	// mac: 0x6FDBC1 — read one byte, throw "Unexpected format %d (expected %d)" unless
	// it equals the expected BinaryPropertyFormat tag.
	// STUB: PLAYER 0x008cef20
	// STUB: STUDIO 0x00741910
	void readFormatExpected(MemoryInputStream& in, unsigned char expectedFormat)
	{
		unsigned char format = 0;
		in.read(&format, 1);
		if (format != expectedFormat)
			throw std::runtime_error("Unexpected format %d (expected %d)");
	}

	// Zigzag: binary computes (v >> 31) ^ (2 * v) with arithmetic shift
	// (0x703C27 passes). Bit-identical, without signed-overflow UB.
	unsigned int zigzagEncode(int value)
	{
		unsigned int u = static_cast<unsigned int>(value);
		unsigned int sign = static_cast<unsigned int>(value >> 31);
		return sign ^ (u * 2u);
	}

	// mac: 0x703E38 — write an int array transposed by byte, MSB first (all byte3s,
	// then byte2s, byte1s, byte0s). Empty arrays write nothing.
	void writeInterleavedBytes(MemoryOutputStream& out, const std::vector<int>& values)
	{
		for (int shift = 24; shift >= 0; shift -= 8)
		{
			for (size_t i = 0; i < values.size(); ++i)
			{
				unsigned char byte = static_cast<unsigned char>(
					(static_cast<unsigned int>(values[i]) >> shift) & 0xFFu);
				out.write(&byte, 1);
			}
		}
	}

	// mac: 0x703C27 — zigzag each int, then the 0x703E38 transposed layout. Proven
	// byte-identical: the binary's per-pass expressions
	// ((v>>31)^(2v))>>24, >>16, ((v>>31)^(2·low16(v)))>>8, (v>>31)^(2·low8(v))
	// are exactly bytes 3..0 of zigzag(v).
	void writeZigzagInterleaved(MemoryOutputStream& out, const std::vector<int>& values)
	{
		std::vector<int> encoded(values.size());
		for (size_t i = 0; i < values.size(); ++i)
			encoded[i] = static_cast<int>(zigzagEncode(values[i]));
		writeInterleavedBytes(out, encoded);
	}

	// mac: 0x703F67 — delta-encode (first - 0, then successive differences), then
	// mac: 0x703C27. (The decompile drops the vector argument to the 0x703C27 call;
	// the delta buffer has no other consumer, so the call target is certain.)
	void writeDeltaValues(MemoryOutputStream& out, const std::vector<int>& values)
	{
		std::vector<int> deltas(values.size());
		int previous = 0;
		for (size_t i = 0; i < values.size(); ++i)
		{
			deltas[i] = values[i] - previous;
			previous = values[i];
		}
		writeZigzagInterleaved(out, deltas);
	}

	// mac: 0x703F29 referent-intern lookup (BinCodecs_small.txt): the binary walks a
	// sorted table and returns the id or -1. The 0x700E3E signature takes
	// std::map<const Instance*, int> directly, so the port uses idMap.find()
	// (exact-match-or--1, identical semantics); no adapter needed.

	// mac: 0x703D3F — float array in rotated-byte layout. Proven byte-identical to
	// the binary (passes: v>>23, v>>15, v>>7, ROL(v,1)&0xFF); 2016's
	// writeFloatVector expresses the same bytes via ROL(v,1)>>24/16/8.
	void writeFloatArray(MemoryOutputStream& out, const std::vector<float>& values)
	{
		for (int pass = 0; pass < 4; ++pass)
		{
			for (size_t i = 0; i < values.size(); ++i)
			{
				unsigned int bits = 0;
				memcpy(&bits, &values[i], sizeof(bits));
				unsigned char byte = 0;
				switch (pass)
				{
				case 0: byte = static_cast<unsigned char>((bits >> 23) & 0xFFu); break;
				case 1: byte = static_cast<unsigned char>((bits >> 15) & 0xFFu); break;
				case 2: byte = static_cast<unsigned char>((bits >> 7) & 0xFFu); break;
				default: byte = static_cast<unsigned char>(((bits << 1) | (bits >> 31)) & 0xFFu); break;
				}
				out.write(&byte, 1);
			}
		}
	}

	static void writeBinaryString(MemoryOutputStream& out, const std::string& value)
	{
		// u32 length + raw bytes (2013 string branch; 2016 writeString 0xA47260).
		unsigned int size = static_cast<unsigned int>(value.size());
		out.write(&size, sizeof(size));
		if (!value.empty())
			out.write(value.data(), value.size());
	}

	/* mac: 0x700E3E — evidence: wprop2013.txt (2013) diffed against 2016
	   writePropertyValues 0xA45D40 (BinCodecs_2016.txt, PDB-named).
	   Branch order + tags + asserts mirror the binary: per-instance
	   isMemberOf (property.h:255) + isValueType (property.h:289) checks come
	   free via ConstProperty (ctor asserts the former, getValue<V> the latter).
	   2013 tag map (BinaryPropertyFormat): string/ContentId share tag 1;
	   CFrame writes two tag-0 chunks (TODO — no CFrame descriptors in tree);
	   16/17 unused in 2013 (2016: CFrame 16/17, Rect2D 24, PhysicalProperties
	   25, Vector3int16 20, NumberSequence 21, ColorSequence 22, NumberRange 23).
	   int = zigzag WITHOUT delta (delta applies to ref ids only); float =
	   rotated layout; double = raw; enums (bIsEnum) = raw transposed uints;
	   refs = idMap ids (miss/null -> -1, 2016 getInstanceId 0xA41A00) +
	   delta/zigzag.
	   TODO(decompile): UDim..CFrame branches (no such descriptors in tree yet —
	   unreachable; fall through to the binary's throw); ProtectedString
	   singleton (Type::getSingleton<ProtectedString> 0x234BAC pending —
	   string branch covers std::string only for now). */
	/* mac: 0x700E3E read: FFlag::SerializerBinaryForceExactCFrame forces the matrix
	   (exact) CFrame path. Static fast flag (2013 HAS static FFlags; only the
	   DYNAMIC system is absent). Default false TODO-verify (false preserves the
	   long-tested quaternion default). */
	FASTFLAGVARIABLE(SerializerBinaryForceExactCFrame, false)

	void writePropertyValues(MemoryOutputStream& out,
		const Reflection::PropertyDescriptor& desc,
		const std::vector<const Instance*>& instances,
		const std::map<const Instance*, int>& idMap,
		unsigned int flags)
	{
		// flags bit 2 forces exact CFrame (matrix path); see the CFrame branch below.
		using namespace Reflection;
		if (desc.type == Type::singleton<std::string>())
		{
			unsigned char tag = bpfString;
			out.write(&tag, sizeof(tag));
			for (size_t i = 0; i < instances.size(); ++i)
				writeBinaryString(out, desc.getStringValue(instances[i]));
			return;
		}
		if (desc.type == Type::singleton<bool>())
		{
			unsigned char tag = bpfBool;
			out.write(&tag, sizeof(tag));
			for (size_t i = 0; i < instances.size(); ++i)
			{
				unsigned char value = ConstProperty(desc, instances[i]).getValue<bool>() ? 1 : 0;
				out.write(&value, sizeof(value));
			}
			return;
		}
		if (desc.type == Type::singleton<int>())
		{
			std::vector<int> values(instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
				values[i] = ConstProperty(desc, instances[i]).getValue<int>();
			unsigned char tag = bpfInt;
			out.write(&tag, sizeof(tag));
			writeZigzagInterleaved(out, values);
			return;
		}
		if (desc.type == Type::singleton<float>())
		{
			std::vector<float> values(instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
				values[i] = ConstProperty(desc, instances[i]).getValue<float>();
			unsigned char tag = bpfFloat;
			out.write(&tag, sizeof(tag));
			writeFloatArray(out, values);
			return;
		}
		if (desc.type == Type::singleton<double>())
		{
			unsigned char tag = bpfDouble;
			out.write(&tag, sizeof(tag));
			for (size_t i = 0; i < instances.size(); ++i)
			{
				double value = ConstProperty(desc, instances[i]).getValue<double>();
				out.write(&value, sizeof(value));
			}
			return;
		}
		if (desc.bIsEnum)
		{
			const EnumPropertyDescriptor& enumDesc = static_cast<const EnumPropertyDescriptor&>(desc);
			std::vector<int> values(instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
				values[i] = enumDesc.getEnumValue(instances[i]);
			unsigned char tag = bpfEnum;
			out.write(&tag, sizeof(tag));
			writeInterleavedBytes(out, values);
			return;
		}
		if (RefPropertyDescriptor::isRefPropertyDescriptor(desc))
		{
			const RefPropertyDescriptor& refDesc = static_cast<const RefPropertyDescriptor&>(desc);
			std::vector<int> ids(instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
			{
				int id = -1;
				if (const Instance* target = dynamic_cast<const Instance*>(refDesc.getRefValue(instances[i])))
				{
					std::map<const Instance*, int>::const_iterator it = idMap.find(target);
					if (it != idMap.end())
						id = it->second;
				}
				ids[i] = id;
			}
			unsigned char tag = bpfRef;
			out.write(&tag, sizeof(tag));
			writeDeltaValues(out, ids);
			return;
		}
		if (desc.type == Type::singleton<BrickColor>())
		{
			std::vector<int> values(instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
				values[i] = ConstProperty(desc, instances[i]).getValue<BrickColor>().asInt();
			unsigned char tag = bpfBrickColor;
			out.write(&tag, sizeof(tag));
			writeInterleavedBytes(out, values);
			return;
		}
		if (desc.type == Type::singleton<G3D::Color3>())
		{
			std::vector<float> r(instances.size()), g(instances.size()), b(instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
			{
				G3D::Color3 value = ConstProperty(desc, instances[i]).getValue<G3D::Color3>();
				r[i] = value.r; g[i] = value.g; b[i] = value.b;
			}
			unsigned char tag = bpfColor3;
			out.write(&tag, sizeof(tag));
			writeFloatArray(out, r);
			writeFloatArray(out, g);
			writeFloatArray(out, b);
			return;
		}
		if (desc.type == Type::singleton<G3D::Vector2>())
		{
			// mac: 0x700E3E Vector2 branch: tag 13 + X, Y rotated float arrays.
			std::vector<float> xs(instances.size()), ys(instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
			{
				G3D::Vector2 value = ConstProperty(desc, instances[i]).getValue<G3D::Vector2>();
				xs[i] = value.x; ys[i] = value.y;
			}
			unsigned char tag = bpfVector2;
			out.write(&tag, sizeof(tag));
			writeFloatArray(out, xs);
			writeFloatArray(out, ys);
			return;
		}
		if (desc.type == Type::singleton<ContentId>())
		{
			unsigned char tag = bpfString;  // shared with strings (2013 + 2016)
			out.write(&tag, sizeof(tag));
			for (size_t i = 0; i < instances.size(); ++i)
				writeBinaryString(out, ConstProperty(desc, instances[i]).getValue<ContentId>().toString());
			return;
		}
		if (desc.type == Type::singleton<G3D::Vector3>())
		{
			// mac: 0x700E3E Vector3 branch: tag 14 + 3x rotated float arrays (x, y, z).
			std::vector<float> x(instances.size()), y(instances.size()), z(instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
			{
				G3D::Vector3 value = ConstProperty(desc, instances[i]).getValue<G3D::Vector3>();
				x[i] = value.x; y[i] = value.y; z[i] = value.z;
			}
			unsigned char tag = bpfVector3;
			out.write(&tag, sizeof(tag));
			writeFloatArray(out, x);
			writeFloatArray(out, y);
			writeFloatArray(out, z);
			return;
		}
		if (desc.type == Type::singleton<UDim>())
		{
			// mac: 0x700E3E UDim branch: tag 6 + rotated scale floats + zigzag offsets.
			std::vector<float> scales(instances.size());
			std::vector<int> offsets(instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
			{
				UDim value = ConstProperty(desc, instances[i]).getValue<UDim>();
				scales[i] = value.scale; offsets[i] = value.offset;
			}
			unsigned char tag = bpfUDim;
			out.write(&tag, sizeof(tag));
			writeFloatArray(out, scales);
			writeZigzagInterleaved(out, offsets);
			return;
		}
		if (desc.type == Type::singleton<UDim2>())
		{
			// mac: 0x700E3E UDim2 branch: tag 7 + X.scale, Y.scale floats + X.offset, Y.offset zigzags.
			std::vector<float> xscales(instances.size()), yscales(instances.size());
			std::vector<int> xoffsets(instances.size()), yoffsets(instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
			{
				UDim2 value = ConstProperty(desc, instances[i]).getValue<UDim2>();
				xscales[i] = value.x.scale; yscales[i] = value.y.scale;
				xoffsets[i] = value.x.offset; yoffsets[i] = value.y.offset;
			}
			unsigned char tag = bpfUDim2;
			out.write(&tag, sizeof(tag));
			writeFloatArray(out, xscales);
			writeFloatArray(out, yscales);
			writeZigzagInterleaved(out, xoffsets);
			writeZigzagInterleaved(out, yoffsets);
			return;
		}
		if (desc.type == Type::singleton<RbxRay>())
		{
			// mac: 0x700E3E Ray branch: tag 8 + RAW 12-byte origin + 12-byte direction per instance.
			unsigned char tag = bpfRay;
			out.write(&tag, sizeof(tag));
			for (size_t i = 0; i < instances.size(); ++i)
			{
				RbxRay value = ConstProperty(desc, instances[i]).getValue<RbxRay>();
				out.write(&value.origin().x, sizeof(float));
				out.write(&value.origin().y, sizeof(float));
				out.write(&value.origin().z, sizeof(float));
				out.write(&value.direction().x, sizeof(float));
				out.write(&value.direction().y, sizeof(float));
				out.write(&value.direction().z, sizeof(float));
			}
			return;
		}
		if (desc.type == Type::singleton<Faces>())
		{
			// mac: 0x700E3E Faces branch: tag 9 + raw mask bytes (one per instance).
			unsigned char tag = bpfFaces;
			out.write(&tag, sizeof(tag));
			for (size_t i = 0; i < instances.size(); ++i)
			{
				Faces value = ConstProperty(desc, instances[i]).getValue<Faces>();
				unsigned char byte = (unsigned char)(value.normalIdMask & 0xFF);
				out.write(&byte, sizeof(byte));
			}
			return;
		}
		if (desc.type == Type::singleton<Axes>())
		{
			// mac: 0x700E3E Axes branch: tag 10 + raw mask bytes (one per instance).
			unsigned char tag = bpfAxes;
			out.write(&tag, sizeof(tag));
			for (size_t i = 0; i < instances.size(); ++i)
			{
				Axes value = ConstProperty(desc, instances[i]).getValue<Axes>();
				unsigned char byte = (unsigned char)(value.axisMask & 0xFF);
				out.write(&byte, sizeof(byte));
			}
			return;
		}
		if (desc.type == Type::singleton<G3D::CoordinateFrame>())
		{
			// mac: 0x700E3E CFrame branch (NO bpf tag: chunk starts with 0x10/0x11).
			// exact (matrix, lossless) iff FFlag set or flags bit 2; else quaternion.
			// NOTE: 2013 bit 2 means force-EXACT here (the shared sfInexactCFrame
			// name was carried from 2016 and is NOT used for this test).
				std::vector<G3D::Matrix3> matrices(instances.size());
			std::vector<float> x(instances.size()), y(instances.size()), z(instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
			{
				G3D::CoordinateFrame value = ConstProperty(desc, instances[i]).getValue<G3D::CoordinateFrame>();
				matrices[i] = value.rotation;
				x[i] = value.translation.x; y[i] = value.translation.y; z[i] = value.translation.z;
			}
			bool exact = FFlag::SerializerBinaryForceExactCFrame || (flags & 4) != 0;
			unsigned char tag = exact ? 0x10 : 0x11;
			out.write(&tag, sizeof(tag));
			for (size_t i = 0; i < instances.size(); ++i)
			{
				if (Math::isAxisAligned(matrices[i]))
				{
					unsigned char orientId = (unsigned char)(Math::getOrientId(matrices[i]) + 1);
					out.write(&orientId, sizeof(orientId));
				}
				else if (exact)
				{
					unsigned char zero = 0;
					out.write(&zero, sizeof(zero));
					out.write(&matrices[i][0][0], 36);
				}
				else
				{
					Quaternion q(matrices[i]);
					// Binary parenthesization mirrored bit-exactly (float add is not associative).
					float len = sqrtf(q.w * q.w + (q.z * q.z + (q.x * q.x + q.y * q.y)));
					float inv = 1.0f / len;
					q.x *= inv; q.y *= inv; q.z *= inv; q.w *= inv;
					unsigned char zero = 0;
					out.write(&zero, sizeof(zero));
					out.write(&q.x, 16);
				}
			}
			writeFloatArray(out, x);
			writeFloatArray(out, y);
			writeFloatArray(out, z);
			return;
		}
		throw std::runtime_error("Unknown property type for property %s");
	}

	// Inverse array readers (verified against 2013 sub_700AFB/700C08/700D0E/
	// 700E10 call shapes + 2016 readUIntVector/readFloatVector/readIdVector/
	// readString bodies).
	static unsigned int zigzagDecode(unsigned int z)
	{
		return (z >> 1) ^ (0u - (z & 1u));
	}

	void readUIntArray(MemoryInputStream& in, std::vector<unsigned int>& values, size_t count)
	{
		values.resize(count);
		for (int shift = 24; shift >= 0; shift -= 8)
		{
			for (size_t i = 0; i < count; ++i)
			{
				unsigned char byte = 0;
				in.read(&byte, 1);
				values[i] |= (static_cast<unsigned int>(byte) << shift);
			}
		}
	}

	void readIntArray(MemoryInputStream& in, std::vector<int>& values, size_t count)
	{
		std::vector<unsigned int> raw(count, 0);
		for (int shift = 24; shift >= 0; shift -= 8)
		{
			for (size_t i = 0; i < count; ++i)
			{
				unsigned char byte = 0;
				in.read(&byte, 1);
				raw[i] |= (static_cast<unsigned int>(byte) << shift);
			}
		}
		values.resize(count);
		for (size_t i = 0; i < count; ++i)
			values[i] = static_cast<int>(zigzagDecode(raw[i]));
	}

	void readFloatArray(MemoryInputStream& in, std::vector<float>& values, size_t count)
	{
		// Inverse rotation: pass p holds byte p of every value
		// (b0=v>>23, b1=v>>15, b2=v>>7, b3=ROL(v,1)&0xFF).
		std::vector<unsigned char> passes(4 * count);
		for (size_t p = 0; p < 4 * count; ++p)
			in.read(&passes[p], 1);
		values.resize(count);
		for (size_t i = 0; i < count; ++i)
		{
			unsigned int b0 = passes[i];
			unsigned int b1 = passes[count + i];
			unsigned int b2 = passes[2 * count + i];
			unsigned int b3 = passes[3 * count + i];
			unsigned int bits = (b0 << 23) | (b1 << 15) | (b2 << 7)
				| ((b3 >> 1) & 0x7Fu) | ((b3 & 1u) << 31);
			float value = 0;
			memcpy(&value, &bits, sizeof(value));
			values[i] = value;
		}
	}

	void readIdArray(MemoryInputStream& in, std::vector<int>& values, size_t count)
	{
		// Inverse of writeDeltaValues: zigzag-decode, then cumulative sum.
		readIntArray(in, values, count);
		int previous = 0;
		for (size_t i = 0; i < values.size(); ++i)
		{
			values[i] += previous;
			previous = values[i];
		}
	}

	void readBinaryString(MemoryInputStream& in, std::string& value)
	{
		// Inverse of the string branch (2013 sub_700A92; 2016 readString).
		unsigned int size = 0;
		in.read(&size, sizeof(size));
		value.resize(size);
		if (size != 0)
			in.read(&value[0], size);
	}

	// mac: 0x706425. Header layout verified against the 0x70520C reader
	// (tag u32 switch, compressed/uncompressed lens, "Malformed data..."
	// mismatch throw) and 2016 kChunk* names (INST/PROP/PRNT/END).
	void writeChunk(std::ostream& out, MemoryOutputStream& chunk, const char* tag, unsigned int flags)
	{
		const size_t size = chunk.getSize();
		const bool raw = (flags & SerializerBinary::sfNoCompression) != 0 || size == 0;
		std::vector<char> compressed;
		int compressedSize = 0;
		if (!raw)
		{
			compressed.resize(static_cast<size_t>(LZ4_compressBound(static_cast<int>(size))));
			if ((flags & SerializerBinary::sfHighCompression) != 0)
				// OGRE vc9: LZ4_compress_HC/LZ4_compress_default (with a level) are lz4 r129+ names; the
				// lz4 Roblox vendored (App/lz4, upstream svn r94) spells them *_limitedOutput.
				compressedSize = LZ4_compressHC_limitedOutput(chunk.getData(), &compressed[0],
					static_cast<int>(size), static_cast<int>(compressed.size()));
			else
				compressedSize = LZ4_compress_limitedOutput(chunk.getData(), &compressed[0],
					static_cast<int>(size), static_cast<int>(compressed.size()));
			if (compressedSize <= 0)
				throw std::runtime_error("LZ4 compression failed");
		}
		char header[16];
		memcpy(header, tag, 4);
		unsigned int compressedLen = raw ? 0 : static_cast<unsigned int>(compressedSize);
		unsigned int uncompressedLen = static_cast<unsigned int>(size);
		unsigned int reserved = 0;
		memcpy(header + 4, &compressedLen, sizeof(compressedLen));
		memcpy(header + 8, &uncompressedLen, sizeof(uncompressedLen));
		memcpy(header + 12, &reserved, sizeof(reserved));
		out.write(header, sizeof(header));
		if (raw)
		{
			if (size != 0)
				out.write(chunk.getData(), size);
		}
		else
		{
			out.write(&compressed[0], compressedSize);
		}
		// TODO: FLog::Serializer chunk log (exact literals unrecovered).
	}

	/* mac: 0x6FDC57 — evidence: rprop2013.txt diffed against 2016
	   readPropertyValues 0xA42670 (BinCodecs_2016.txt, PDB-named).
	   Symmetric inverse of writePropertyValues: same branch order and tags
	   (string/ContentId share bpfString on read too — 2016 confirms), same
	   asserts. Setters: setStringValue for strings; Property::setValue for
	   bool/int/float/double (isValueType assert at property.h:341, mirroring
	   the binary); EnumPropertyDescriptor::setEnumValue for enums (symmetric
	   with the getEnumValue the writer stores (value semantics PROVED: getEnumValue
	   returns (int)getValue, i.e. raw values -- Wood=512/Sand=1296 round-trip through
	   the interleave codec, parsetest 23) — TODO-verify index-vs-value);
	   RefPropertyDescriptor::setRefValue for refs (id -1 or unmapped -> NULL;
	   out-of-range id throws "Invalid id %d" per 2016 0xA42670).
	   Instances are mutable (both binaries); idMap is the referent-index
	   vector of shared owners (keeps referents alive during the read).
	   TODO: UDim..CFrame branches (no descriptors in tree); enum
	   value-vs-index verification. */
	// STUB: PLAYER 0x008cfd60
	// STUB: STUDIO 0x00743f70
	void readPropertyValues(MemoryInputStream& in,
		const Reflection::PropertyDescriptor& desc,
		const std::vector<Instance*>& instances,
		const std::vector<shared_ptr<Instance> >& idMap)
	{
		using namespace Reflection;

		if (desc.type == Type::singleton<std::string>())
		{
			readFormatExpected(in, bpfString);
			for (size_t i = 0; i < instances.size(); ++i)
			{
				std::string value;
				readBinaryString(in, value);
				desc.setStringValue(instances[i], value);
			}
			return;
		}
		if (desc.type == Type::singleton<bool>())
		{
			readFormatExpected(in, bpfBool);
			for (size_t i = 0; i < instances.size(); ++i)
			{
				unsigned char byte = 0;
				in.read(&byte, 1);
				Property(desc, instances[i]).setValue<bool>(byte != 0);
			}
			return;
		}
		if (desc.type == Type::singleton<int>())
		{
			readFormatExpected(in, bpfInt);
			std::vector<int> values;
			readIntArray(in, values, instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
				Property(desc, instances[i]).setValue<int>(values[i]);
			return;
		}
		if (desc.type == Type::singleton<float>())
		{
			readFormatExpected(in, bpfFloat);
			std::vector<float> values;
			readFloatArray(in, values, instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
				Property(desc, instances[i]).setValue<float>(values[i]);
			return;
		}
		if (desc.type == Type::singleton<double>())
		{
			readFormatExpected(in, bpfDouble);
			for (size_t i = 0; i < instances.size(); ++i)
			{
				double value = 0;
				in.read(&value, sizeof(value));
				Property(desc, instances[i]).setValue<double>(value);
			}
			return;
		}
		if (desc.bIsEnum)
		{
			readFormatExpected(in, bpfEnum);
			const EnumPropertyDescriptor& enumDesc = static_cast<const EnumPropertyDescriptor&>(desc);
			std::vector<unsigned int> values;
			readUIntArray(in, values, instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
				enumDesc.setEnumValue(instances[i], static_cast<int>(values[i]));
			return;
		}
		if (RefPropertyDescriptor::isRefPropertyDescriptor(desc))
		{
			readFormatExpected(in, bpfRef);
			const RefPropertyDescriptor& refDesc = static_cast<const RefPropertyDescriptor&>(desc);
			std::vector<int> ids;
			readIdArray(in, ids, instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
			{
				DescribedBase* target = NULL;
				if (ids[i] != -1)
				{
					if (ids[i] < 0 || static_cast<size_t>(ids[i]) >= idMap.size())
						throw std::runtime_error("Invalid id %d");
					target = idMap[static_cast<size_t>(ids[i])].get();
				}
				refDesc.setRefValue(instances[i], target);
			}
			return;
		}
		if (desc.type == Type::singleton<BrickColor>())
		{
			readFormatExpected(in, bpfBrickColor);
			std::vector<unsigned int> values;
			readUIntArray(in, values, instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
				Property(desc, instances[i]).setValue<BrickColor>(BrickColor(static_cast<int>(values[i])));
			return;
		}
		if (desc.type == Type::singleton<G3D::Color3>())
		{
			readFormatExpected(in, bpfColor3);
			std::vector<float> r, g, b;
			readFloatArray(in, r, instances.size());
			readFloatArray(in, g, instances.size());
			readFloatArray(in, b, instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
				Property(desc, instances[i]).setValue<G3D::Color3>(G3D::Color3(r[i], g[i], b[i]));
			return;
		}
		if (desc.type == Type::singleton<G3D::Vector2>())
		{
			readFormatExpected(in, bpfVector2);
			std::vector<float> xs, ys;
			readFloatArray(in, xs, instances.size());
			readFloatArray(in, ys, instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
			{
				G3D::Vector2 v;
				v.x = xs[i]; v.y = ys[i];
				Property(desc, instances[i]).setValue<G3D::Vector2>(v);
			}
			return;
		}
		if (desc.type == Type::singleton<ContentId>())
		{
			readFormatExpected(in, bpfString);
			for (size_t i = 0; i < instances.size(); ++i)
			{
				std::string value;
				readBinaryString(in, value);
				desc.setStringValue(instances[i], value);
			}
			return;
		}
				if (desc.type == Type::singleton<G3D::Vector3>())
		{
			readFormatExpected(in, bpfVector3);
			std::vector<float> x, y, z;
			readFloatArray(in, x, instances.size());
			readFloatArray(in, y, instances.size());
			readFloatArray(in, z, instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
				Property(desc, instances[i]).setValue<G3D::Vector3>(G3D::Vector3(x[i], y[i], z[i]));
			return;
		}
		if (desc.type == Type::singleton<UDim>())
		{
			readFormatExpected(in, bpfUDim);
			std::vector<float> scales;
			std::vector<int> offsets;
			readFloatArray(in, scales, instances.size());
			readIntArray(in, offsets, instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
				Property(desc, instances[i]).setValue<UDim>(UDim(scales[i], offsets[i]));
			return;
		}
		if (desc.type == Type::singleton<UDim2>())
		{
			readFormatExpected(in, bpfUDim2);
			std::vector<float> xscales, yscales;
			std::vector<int> xoffsets, yoffsets;
			readFloatArray(in, xscales, instances.size());
			readFloatArray(in, yscales, instances.size());
			readIntArray(in, xoffsets, instances.size());
			readIntArray(in, yoffsets, instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
			{
				UDim2 v(xscales[i], xoffsets[i], yscales[i], yoffsets[i]);
				Property(desc, instances[i]).setValue<UDim2>(v);
			}
			return;
		}
		if (desc.type == Type::singleton<RbxRay>())
		{
			readFormatExpected(in, bpfRay);
			for (size_t i = 0; i < instances.size(); ++i)
			{
				float o[3], d[3];
				in.read(&o[0], 12);
				in.read(&d[0], 12);
				Property(desc, instances[i]).setValue<RbxRay>(RbxRay(G3D::Vector3(o[0], o[1], o[2]), G3D::Vector3(d[0], d[1], d[2])));
			}
			return;
		}
		if (desc.type == Type::singleton<Faces>())
		{
			readFormatExpected(in, bpfFaces);
			for (size_t i = 0; i < instances.size(); ++i)
			{
				unsigned char byte = 0;
				in.read(&byte, 1);
				Property(desc, instances[i]).setValue<Faces>(Faces(byte));
			}
			return;
		}
		if (desc.type == Type::singleton<Axes>())
		{
			readFormatExpected(in, bpfAxes);
			for (size_t i = 0; i < instances.size(); ++i)
			{
				unsigned char byte = 0;
				in.read(&byte, 1);
				Property(desc, instances[i]).setValue<Axes>(Axes(byte));
			}
			return;
		}
		if (desc.type == Type::singleton<G3D::CoordinateFrame>())
		{
			// mac: 0x6FDC57 CFrame branch (no format tag: 0x10 matrix / 0x11 quaternion).
			unsigned char format = 0;
			in.read(&format, 1);
			if (format != 0x10 && format != 0x11)
			{
				char msg[64];
				sprintf(msg, "Unexpected cframe format %d", (int)format);
				throw std::runtime_error(msg);
			}
			std::vector<G3D::Matrix3> matrices(instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
			{
				unsigned char b = 0;
				in.read(&b, 1);
				if (b != 0)
					Math::idToMatrix3((int)b - 1, matrices[i]);
				else if (format == 0x10)
					in.read(&matrices[i][0][0], 36);
				else
				{
					float q[4];
					in.read(&q[0], 16);
					Quaternion(q[0], q[1], q[2], q[3]).toRotationMatrix(matrices[i]);
				}
			}
			std::vector<float> x, y, z;
			readFloatArray(in, x, instances.size());
			readFloatArray(in, y, instances.size());
			readFloatArray(in, z, instances.size());
			for (size_t i = 0; i < instances.size(); ++i)
				Property(desc, instances[i]).setValue<G3D::CoordinateFrame>(G3D::CoordinateFrame(matrices[i], G3D::Vector3(x[i], y[i], z[i])));
			return;
		}
	throw std::runtime_error("Unknown property type for property %s");	}

	namespace SerializerBinary
	{
		// Chunk tags (2016 kChunk* names; 2013 bytes verified via reader switch).
		static const char kChunkInstances[] = "INST";
		static const char kChunkProperty[] = "PROP";
		static const char kChunkParents[] = "PRNT";
		static const char kChunkEnd[] = "END";

		// mac: 0x70520C read helpers.
		static void readExact(std::istream& in, char* dst, size_t size)
		{
			// Short reads throw (binary's "Unexpected end of..." — literal TODO).
			in.read(dst, size);
			if (static_cast<size_t>(in.gcount()) != size)
				throw std::runtime_error("Unexpected end of file");
		}

		static shared_ptr<Instance> createServiceInstance(Instance* root, const RBX::Name& name)
		{
			// 2016 createServiceInstance shape; 2013 calls root's vtable+44
			// (ServiceProvider::createChild override: existing-or-new service).
			shared_ptr<Instance> result = root->createChild(name, SerializationCreator);
			if (result)
				result->setParent(root);
			return result;
		}

		// mac: 0x70520C (via 2016 deserializeImpl 0xA41140 cross-check).
		static void deserializeImpl(std::istream& in, Instance* root, Instances* result)
		{
			// File header (32 bytes).
			char magic[8];
			readExact(in, magic, sizeof(magic));
			if (memcmp(magic, "<roblox!", sizeof(magic)) != 0)
				throw std::runtime_error("Unrecognized format");
			// 2013 order: u16(2586) + u32(168689545). (2016 reordered these
			// six bytes to 89 FF 0D 0A 1A 0A + separate version — NOT backported.)
			char signature[6];
			readExact(in, signature, sizeof(signature));
			static const char expectedSignature[6] = { '\x1A', '\x0A', '\x89', '\xFF', '\x0D', '\x0A' };
			if (memcmp(signature, expectedSignature, sizeof(signature)) != 0)
				throw std::runtime_error("The file header is corrupted");
			unsigned short version = 0;
			{
				char buf[2];
				readExact(in, buf, sizeof(buf));
				memcpy(&version, buf, sizeof(version));
			}
			if (version != 0)
				throw std::runtime_error("Unrecognized version %d");
			unsigned int classCount = 0, instanceCount = 0, reserved = 0;
			{
				char buf[16];
				readExact(in, buf, sizeof(buf));
				memcpy(&classCount, buf, sizeof(classCount));
				memcpy(&instanceCount, buf + 4, sizeof(instanceCount));
			}

			std::vector<const Reflection::ClassDescriptor*> classes(classCount, NULL);
			std::vector<shared_ptr<Instance> > idMap(instanceCount);
			std::vector<std::vector<Instance*> > classInstances(classCount);

			for (;;)
			{
				char tag[4];
				unsigned int compressedLen = 0, uncompressedLen = 0, reserved2 = 0;
				{
					char header[16];
					readExact(in, header, sizeof(header));
					memcpy(tag, header, sizeof(tag));
					memcpy(&compressedLen, header + 4, sizeof(compressedLen));
					memcpy(&uncompressedLen, header + 8, sizeof(uncompressedLen));
				}
			std::vector<char> payload(uncompressedLen);
				if (compressedLen != 0)
				{
					std::vector<char> compressed(compressedLen);
					readExact(in, &compressed[0], compressedLen);
					if (!payload.empty())
					{
						int decoded = LZ4_decompress_safe(&compressed[0], &payload[0],
							static_cast<int>(compressedLen), static_cast<int>(uncompressedLen));
						if (decoded != static_cast<int>(uncompressedLen))
							throw std::runtime_error("Malformed data in chunk");
					}
				}
				else if (uncompressedLen != 0)
				{
					readExact(in, &payload[0], uncompressedLen);
				}
				MemoryInputStream chunk(payload.empty() ? NULL : &payload[0], payload.size());

				if (memcmp(tag, kChunkInstances, sizeof(tag)) == 0)
				{
					unsigned int classIdx = 0;
					chunk.read(&classIdx, sizeof(classIdx));
					if (classIdx >= classes.size())
						throw std::runtime_error("Type index out of range");
					if (classes[classIdx] != NULL)
						throw std::runtime_error("Duplicate type index");
					std::string className;
					readBinaryString(chunk, className);
					unsigned char serviceByte = 0;
					chunk.read(&serviceByte, sizeof(serviceByte));
					if (serviceByte >= 2)
						throw std::runtime_error("Unrecognized object format");
					unsigned int count = 0;
					chunk.read(&count, sizeof(count));
					std::vector<int> ids;
					readIdArray(chunk, ids, count);
					std::vector<char> serviceRooted;
					if (serviceByte == 1)
					{
						serviceRooted.resize(ids.size());
						for (size_t k = 0; k < ids.size(); ++k)
						{
							unsigned char value = 0;
							chunk.read(&value, sizeof(value));
							serviceRooted[k] = value;
						}
					}
					bool rootIsProvider = root != NULL
						&& dynamic_cast<ServiceProvider*>(root) != NULL;
					const RBX::Name& typeName = *RBX::Name::lookup(className);
					const RBX::ICreator* creator = Creatable<Instance>::getCreator(typeName);
					const Reflection::ClassDescriptor* filledDesc = NULL;
					for (size_t k = 0; k < ids.size(); ++k)
					{
						if (ids[k] < 0 || static_cast<size_t>(ids[k]) >= idMap.size())
							throw std::runtime_error("Invalid id");
						shared_ptr<Instance> object;
						if (serviceByte == 1 && rootIsProvider && serviceRooted[k] != 0)
							object = createServiceInstance(root, typeName);
						else if (creator != NULL)
							object = boost::dynamic_pointer_cast<Instance, Reflection::DescribedBase>(creator->create());
					if (object)
						{
							if (idMap[static_cast<size_t>(ids[k])])
								throw std::runtime_error("Duplicate id");
							idMap[static_cast<size_t>(ids[k])] = object;
							classInstances[classIdx].push_back(object.get());
							filledDesc = &object->getDescriptor();
						}
					}
					classes[classIdx] = filledDesc;
				}
				else if (memcmp(tag, kChunkProperty, sizeof(tag)) == 0)
				{
					unsigned int classIdx = 0;
					chunk.read(&classIdx, sizeof(classIdx));
					if (classIdx >= classes.size())
						throw std::runtime_error("Type index out of range");
					if (classes[classIdx] != NULL)
					{
						std::string fullName;
						readBinaryString(chunk, fullName);
						// Full "Class-Prop" match (2013 writer emits className +
						// '-' + propName; separator byte 0x2D verified in IDA).
						const Reflection::PropertyDescriptor* found = NULL;
						std::string prefix = classes[classIdx]->name->str + "-";
						typedef Reflection::MemberDescriptorContainer<Reflection::PropertyDescriptor> PropContainer;
						const PropContainer* propsOf = classes[classIdx];
						for (PropContainer::Collection::const_iterator
							p = propsOf->descriptors_begin(); p != propsOf->descriptors_end(); ++p)
						{
							if (prefix + (*p)->name->str == fullName)
							{
								found = *p;
								break;
							}
						}
						// No canXmlRead gate in 2013 (2016 added one): null match only.
						if (found != NULL)
							readPropertyValues(chunk, *found, classInstances[classIdx], idMap);
					}
				}
				else if (memcmp(tag, kChunkParents, sizeof(tag)) == 0)
				{
					unsigned char format = 0;
					chunk.read(&format, sizeof(format));
					if (format != 0)
						throw std::runtime_error("Unrecognized parent link format");
					unsigned int count = 0;
					chunk.read(&count, sizeof(count));
					std::vector<int> childIds, parentIds;
					readIdArray(chunk, childIds, count);
					readIdArray(chunk, parentIds, count);
					for (size_t k = 0; k < childIds.size(); ++k)
					{
						if (childIds[k] < 0 || static_cast<size_t>(childIds[k]) >= idMap.size())
							throw std::runtime_error("Invalid id");
						if (shared_ptr<Instance> child = idMap[static_cast<size_t>(childIds[k])])
						{
							if (parentIds[k] == -1)
							{
								if (root != NULL)
									child->setParent(root);
								if (result != NULL)
									result->push_back(child);
							}
							else
							{
								if (parentIds[k] < 0 || static_cast<size_t>(parentIds[k]) >= idMap.size())
									throw std::runtime_error("Invalid id");
								child->setParent(idMap[static_cast<size_t>(parentIds[k])].get());
							}
						}
					}
				}
				else if (memcmp(tag, kChunkEnd, sizeof(tag)) == 0)
				{
					break;
				}
				// else: unknown chunk — payload already consumed; ignore.
			}
		}
		// Pre-order recursive collector (sub_706389): fw()->b23 gate +
		// canXmlWrite gate (desc+476 bit 0x4 == isSerializable, verified).
		// a2-output (pre-order) feeds idMap/classMap; the binary also keeps a
		// post-order list for PRNT iteration — order-irrelevant there (pairs
		// decode positionally), so one list serves both (TODO if byte-exact
		// PRNT order ever matters).
		struct InstanceCollector
		{
			std::vector<const Instance*>& out;
			explicit InstanceCollector(std::vector<const Instance*>& o) : out(o) {}
			void operator()(shared_ptr<Instance> child) const
			{
				const Instance* inst = child.get();
				if (inst == NULL || inst->fw() == NULL || !inst->fw()->b23)
					return;
				if (!inst->getDescriptor().isSerializable())
					return;
				out.push_back(inst);
				inst->visitChildren(InstanceCollector(out));
			}
		};

		// DescriptorNameComparator equivalent (binary orders classes by name).
		struct ClassNameLess
		{
			bool operator()(const Reflection::ClassDescriptor* a, const Reflection::ClassDescriptor* b) const
			{
				return a->name->str < b->name->str;
			}
		};

		struct PropNameLess
		{
			bool operator()(const Reflection::PropertyDescriptor* a, const Reflection::PropertyDescriptor* b) const
			{
				return a->name->str < b->name->str;
			}
		};

		// SaveFilter gate for the 0x704012 collect step.
		struct ChildFilter
		{
			std::vector<shared_ptr<Instance> >& out;
			Instance::SaveFilter filter;
			ChildFilter(std::vector<shared_ptr<Instance> >& o, Instance::SaveFilter f) : out(o), filter(f) {}
			void operator()(shared_ptr<Instance> child) const
			{
				if (::Serializer::canWriteChild(child, filter))
					out.push_back(child);
			}
		};

		// mac: 0x704134 (via 2016 serializeImpl 0xA44370 cross-check).
		static void writeImpl(std::ostream& out, const Instance* root,
			const std::vector<shared_ptr<Instance> >& roots, unsigned int flags)
		{
			// Gather + id assignment (pre-order).
			std::vector<const Instance*> all;
			InstanceCollector collector(all);
			for (size_t i = 0; i < roots.size(); ++i)
				collector(roots[i]);
			std::map<const Instance*, int> idMap;
			for (size_t i = 0; i < all.size(); ++i)
				idMap[all[i]] = static_cast<int>(i);

			// Group by class (DescriptorNameComparator order).
			std::map<const Reflection::ClassDescriptor*, std::vector<const Instance*>, ClassNameLess> classes;
			for (size_t i = 0; i < all.size(); ++i)
				classes[&all[i]->getDescriptor()].push_back(all[i]);

			// File header (32 bytes, raw).
			{
				char header[32];
				memcpy(header, "<roblox!", 8);
				unsigned short v10 = 2586;			// 0x0A1A
				unsigned int v11 = 168689545u;		// binary-exact (bytes 89 FF 0D 0A)
				unsigned short v12 = 0;
				unsigned int classCount = static_cast<unsigned int>(classes.size());
				unsigned int instanceCount = static_cast<unsigned int>(all.size());
				unsigned int reserved = 0;
				memcpy(header + 8, &v10, sizeof(v10));
				memcpy(header + 10, &v11, sizeof(v11));
				memcpy(header + 14, &v12, sizeof(v12));
				memcpy(header + 16, &classCount, sizeof(classCount));
				memcpy(header + 20, &instanceCount, sizeof(instanceCount));
				memcpy(header + 24, &reserved, sizeof(reserved));
				memcpy(header + 28, &reserved, sizeof(reserved));
				out.write(header, sizeof(header));
			}

			// INST chunks.
			{
				unsigned int classIndex = 0;
				for (std::map<const Reflection::ClassDescriptor*, std::vector<const Instance*>, ClassNameLess>::const_iterator
					it = classes.begin(); it != classes.end(); ++it, ++classIndex)
				{
					const std::vector<const Instance*>& members = it->second;
					MemoryOutputStream chunk;
					chunk.write(&classIndex, sizeof(classIndex));
					std::string className = it->first->name->str;
					unsigned int nameLen = static_cast<unsigned int>(className.size());
					chunk.write(&nameLen, sizeof(nameLen));
					if (!className.empty())
						chunk.write(className.data(), className.size());
					unsigned char isService = dynamic_cast<const Service*>(members[0]) != NULL ? 1 : 0;
					chunk.write(&isService, sizeof(isService));
					std::vector<int> ids(members.size());
					for (size_t k = 0; k < members.size(); ++k)
						ids[k] = idMap[members[k]];
					unsigned int idCount = static_cast<unsigned int>(ids.size());
					chunk.write(&idCount, sizeof(idCount));
					writeDeltaValues(chunk, ids);
					if (isService)
					{
						for (size_t k = 0; k < members.size(); ++k)
						{
							unsigned char directService = members[k]->getParent() == root ? 1 : 0;
							chunk.write(&directService, sizeof(directService));
						}
					}
					writeChunk(out, chunk, "INST", flags);
				}
			}

			// PROP chunks.
			{
				unsigned int classIndex = 0;
				for (std::map<const Reflection::ClassDescriptor*, std::vector<const Instance*>, ClassNameLess>::const_iterator
					it = classes.begin(); it != classes.end(); ++it, ++classIndex)
				{
					// Writable props (!isWriteOnly && canXmlWrite — the latter
					// carries the binary's property.h:98 assert).
					std::vector<const Reflection::PropertyDescriptor*> props;
					typedef Reflection::MemberDescriptorContainer<Reflection::PropertyDescriptor> PropContainer;
					const PropContainer* propSource = it->first;
					for (PropContainer::Collection::const_iterator
						p = propSource->descriptors_begin(); p != propSource->descriptors_end(); ++p)
					{
						if (!(*p)->isWriteOnly() && (*p)->canXmlWrite())
							props.push_back(*p);
					}
					std::sort(props.begin(), props.end(), PropNameLess());
					for (size_t k = 0; k < props.size(); ++k)
					{
						MemoryOutputStream chunk;
						chunk.write(&classIndex, sizeof(classIndex));
						std::string propName = it->first->name->str + "-" + props[k]->name->str;
						unsigned int nameLen = static_cast<unsigned int>(propName.size());
						chunk.write(&nameLen, sizeof(nameLen));
						if (!propName.empty())
							chunk.write(propName.data(), propName.size());
						writePropertyValues(chunk, *props[k], it->second, idMap, flags);
						writeChunk(out, chunk, "PROP", flags);
					}
				}
			}

			// PRNT chunk: byte 0 + count + delta(childIds) + delta(parentIds).
			// Missing parents (e.g. the unwritten root) encode as -1 (703F29 miss).
			{
				MemoryOutputStream chunk;
				unsigned char zero = 0;
				chunk.write(&zero, sizeof(zero));
				unsigned int count = static_cast<unsigned int>(all.size());
				chunk.write(&count, sizeof(count));
				std::vector<int> childIds(all.size());
				std::vector<int> parentIds(all.size());
				for (size_t i = 0; i < all.size(); ++i)
				{
					childIds[i] = idMap[all[i]];
					const Instance* parent = all[i]->getParent();
					std::map<const Instance*, int>::const_iterator found = idMap.find(parent);
					parentIds[i] = (found == idMap.end()) ? -1 : found->second;
				}
				writeDeltaValues(chunk, childIds);
				writeDeltaValues(chunk, parentIds);
				writeChunk(out, chunk, "PRNT", flags);
			}

			// END chunk ("</roblox>", raw).
			{
				MemoryOutputStream chunk;
				static const char endMark[] = "</roblox>";
				chunk.write(endMark, 9);
				writeChunk(out, chunk, "END", SerializerBinary::sfNoCompression);
			}
		}

		void serialize(std::ostream& out, const Instance* root, unsigned int flags, Instance::SaveFilter saveFilter)
		{
			// mac: 0x704012: filter direct children, then write.
			std::vector<shared_ptr<Instance> > filtered;
			if (root != NULL)
				root->visitChildren(ChildFilter(filtered, saveFilter));
			writeImpl(out, root, filtered, flags);
		}

		void serialize(std::ostream& out, const Instances& instances, unsigned int flags)
		{
			// Vector form: no context root (service-attach bools all false).
			writeImpl(out, NULL, instances, flags);
		}

		void deserialize(std::istream& in, Instance* root)
		{
			// mac: 0x7051EF -> sub_70520C.
			deserializeImpl(in, root, NULL);
		}

		void deserialize(std::istream& in, Instances& result)
		{
			// Vector form (2016 shape).
			deserializeImpl(in, NULL, &result);
		}
	}

} // namespace RBX
