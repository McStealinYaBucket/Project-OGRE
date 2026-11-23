/* Util/BinaryString.h -- October 2013 reconstruction.
   A std::string holding raw bytes, as a distinct type so properties of it
   serialize as binary rather than text. No 2013 trace (no RTTI or string in
   PLAYER, no symbol in iOS 10-29; header-only, so none is expected), and no
   TU in this tree includes it. Kept so the path exists. */
#pragma once

#include <string>

namespace RBX {

class BinaryString
{
	std::string bytes;

public:
	BinaryString() {}
	explicit BinaryString(const std::string& data) : bytes(data) {}

	const std::string& value() const { return bytes; }
	void set(const char* data, unsigned int length) { bytes.assign(data, length); }

	bool operator==(const BinaryString& rhs) const { return bytes == rhs.bytes; }
	bool operator!=(const BinaryString& rhs) const { return bytes != rhs.bytes; }
	bool operator<(const BinaryString& rhs) const { return bytes < rhs.bytes; }
};

} // namespace RBX
