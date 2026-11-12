
#pragma once

#include "G3D/Color3.h"

namespace RBX
{
	// The flat player-name color palette source (a 16-entry float table at
	// 0xFADF0C that getColorByIndex lazily fills once under the cxa guards
	// at 0xFADFD0/byte 0xFADF09; landed as a file-scope static const with
	// the same 16 values -- the guards exist only for thread-safe lazy
	// fill, not for the reads).
	// Evidence: 0x1F3774 getColorByIndex (578B), 0x1F39BE colorFromIndex8
	// (109B); the PlayerChatLine ctor 0x5BEFFA calls both for the name
	// color arms.
	class Color
	{
	public:
		// mac: 0x1F3774: returns &table[index % 16] -- the signed modulo is
		// the sar/shr(28)/add/and(~15)/sub sequence at 0x1F399E-0x1F39AC
		// (table stride 12B; entry 0 = black, 15 = white).  The binary
		// returns a const pointer; the ctor copies the 3 floats out.
		static const G3D::Color3* getColorByIndex(int index);

		// mac: 0x1F39BE: the switch maps 0->2, 1->3, 2->4, 3->6, 4->11, 
		// 5->12, 6->14 and the >6 default->13 (0x1F39EC-0x1F3A22), then
		// tail-calls getColorByIndex.
		static const G3D::Color3* colorFromIndex8(int index);
	};
} // namespace RBX
