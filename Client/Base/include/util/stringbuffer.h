// Base/include/util/stringbuffer.h: October 2013 reconstruction (Project OGRE).
// Evidence: assert "cur != buffer.end()" at util/stringbuffer.h line 18 (PLAYER and STUDIO);
// iOS 1029 RBX::StringReadBuffer::operator>>(unsigned char&), readCountValue<StringReadBuffer>,
// writeCountValue<StringWriteBuffer> (MegaCluster chunk streams).
#pragma once
#include "rbx/Debug.h"
#include <string>
namespace RBX {

// Byte reader over a string it does not own.
class StringReadBuffer
{
	std::string::const_iterator cur;
	const std::string& buffer;
public:
	StringReadBuffer(const std::string& str) : buffer(str) { cur = buffer.begin(); }
	StringReadBuffer& operator>>(unsigned char& value) {
		RBXASSERT(cur != buffer.end()); // line 18 in 2013
		if (cur == buffer.end())
			throw RBX::runtime_error("Reading past end of string");
		value = *cur++;
		return *this;
	}

	bool eof() { return cur == buffer.end(); }
};

// Byte writer that accumulates into its own string. Together with the reader it gives the
// MegaCluster (terrain) chunk encoder and decoder a stream interface over std::string: the 2013
// templates readCountValue/writeCountValue and decodeChunkDataFromStream/encodeChunkDataIntoStream
// are instantiated on exactly these two classes. Reading past the end asserts in debug builds and
// throws a std::runtime_error from RBX::runtime_error in every build, so a truncated chunk cannot
// read garbage.
class StringWriteBuffer
{
	std::string buffer;
public:
	StringWriteBuffer() : buffer(std::string()) {}

	StringWriteBuffer& operator<<(unsigned char value)
	{
		buffer.push_back(value);
		return *this;
	}

	const std::string& str() { return buffer; }
};

}
