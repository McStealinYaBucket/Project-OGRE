// Reconstructed from October 2013 binary (symbolled Mach-O Studio).
// 2013 layout differs from the 2016 reference: this version holds a single
// source std::string plus a scoped_ptr<std::string> hash. There is NO
// bytecode field in 2013 (copy ctor @0x234578 copies only two strings:
// the source and a freshly allocated hash string).

#pragma once

#include <string>
#include <boost/scoped_ptr.hpp>

namespace RBX {

	class ProtectedString
	{
	public:
		static const ProtectedString emptyString;

		// mac: 0x2345E2 - default constructs then assigns stringRef and recalculates hash
		static ProtectedString fromTrustedSource(const std::string& stringRef);

		ProtectedString();                                          // 0x234476
		ProtectedString(const ProtectedString& other);              // 0x234578

		~ProtectedString();                                         // 0x235356

		// mac: 0x23484E - returns the underlying source string (this==&source in 2013)
		const std::string& getStringForImmediateUse() const { return source; }

		// mac: 0x234856 - returns *hash (the scoped_ptr target)
		const std::string& getOriginalHash() const;

		// mac: 0x234658 - MD5(salt + source) written into *out; empty source -> empty hash
		void calculateHash(std::string* out) const;

		// mac: 0x234862
		static const char* getSalt();

		// mac: 0x234874 - bounds-checked single-character read (used by obfuscation
		// checks without exposing the whole string)
		bool readUnprotectedChar(int index, char* outChar) const;

		bool operator==(const ProtectedString& other) const;        // 0x23489A
		bool operator!=(const ProtectedString& other) const;

		ProtectedString& operator=(const ProtectedString& other);   // 0x2348C0

	private:
		std::string source;

		// Need to keep a pointer to hash to keep the size of this object in
		// line with other lua-bridged types.
		boost::scoped_ptr<std::string> hash;
	};

	// boost::hash<ProtectedString> hashes the source bytes directly
	// (0x161368 - hash ^= byte + (h >> 2) + (h << 6) - 1640531527)
	size_t hash_value(const ProtectedString& str);

}
