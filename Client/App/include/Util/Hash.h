
#pragma once

#include <string>
#include <stddef.h>

namespace RBX
{
	// Evidence: 0x204BF4 Hash::hash(raw asm, 51B); the PlayerChatLine ctor
	// mac: 0x5BEFFA calls it at 0x5BF106 (Hash::hash(playerName) & 7 ->
	// colorFromIndex8 for the name-color arm).
	class Hash
	{
	public:
		// mac: 0x204BF4: h = 0; mul = 0xF8C9 (63753, immediate at 0x204C08);
		// for every character: h = h * mul + (unsigned char)*s++
		// (0x204C0C-0x204C12), then mul = mul * 0x5C6B7 (377527,
		// immediate at 0x204C16) BEFORE the next pass; an empty string
		// returns 0 (the 0x204C05 jz); the 0x7FFFFFFF mask (0x204C1E)
		// applies only on the non-empty path.
		static int hash(const std::string& text);
	};
} // namespace RBX
