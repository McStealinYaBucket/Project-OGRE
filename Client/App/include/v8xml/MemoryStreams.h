/* V8Xml/MemoryStreams.h — October 2013 binary-serializer streams.
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

#pragma once

#include <cstddef>
#include <cstring>
#include <stdexcept>

#include "rbx/Debug.h"

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
