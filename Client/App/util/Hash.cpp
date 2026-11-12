// Hash.cpp -- reconstructed from October 2013 binary dbq24.
// Source of truth: symbolled Mach-O Studio binary (0.126.0.39503), EAs noted per function.
// Self-audit: Hash::hash landed from raw bytes (0x204BF4..0x204C26); the
// two immediates 0xF8C9 / 0x5C6B7 were confirmed from the encoded bytes
// (BE C9 F8 00 00 / 69 F6 B7 C6 05 00).  DELTAS: none -- the landed loop
// is the binary's loop; landing-side size_t drives the len walk (the
// binary re-uses the GCC COW len at [data-12], value-identical).

#include "Util/Hash.h"

namespace RBX
{
	// mac: 0x204BF4.
	// STUB: PLAYER 0x008e5210
	// STUB: STUDIO 0x00829fb0
	int Hash::hash(const std::string& text)
	{
		unsigned int h = 0;
		unsigned int mul = 63753u;          // 0xF8C9
		size_t length = text.length();      // binary reads the COW len
		if (length != 0)
		{
			const char* s = text.data();
			while (length != 0)
			{
				h = h * mul + (unsigned int)(unsigned char)*s;
				++s;
				mul = mul * 377527u;        // 0x5C6B7
				--length;
			}
			h &= 0x7FFFFFFFu;
		}
		return (int)h;
	}
} // namespace RBX
