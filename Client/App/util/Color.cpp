// Color.cpp -- reconstructed from October 2013 binary dbq24.
// Source of truth: symbolled Mach-O Studio binary (0.126.0.39503), EAs noted per function.
// Self-audit: both functions landed from raw asm (see EAs); the 16-entry
// palette is the byte-verified fill at 0x1F37B7-0x1F399D (each decimal
// literal round-trips to the binary's dword bit pattern -- hex comments
// carry the source dwords).  DELTAS: member names per the mangled symbols
// (RBX::Color); the landed table is init-once instead of the lazy
// cxa_guard fill (value-identical, thread-safe at module init).

#include "Util/Color.h"

namespace RBX
{
	// Entries in fill order: the stores at 0x1F37B7 (table[0] all zero)
	// through 0x1F398D (table[15] all 1.0f).  d_FADFD0/byte_FADF09 guard
	// one fill only; landed static-init is equivalent.
	static const G3D::Color3 sColorTable[16] =
	{
        G3D::Color3(0.0f, 0.0f, 0.0f),  // 0x00000000 0x00000000 0x00000000
        G3D::Color3(0.34117648f, 0.34117648f, 0.34117648f),  // 0x3EAEAEAF 0x3EAEAEAF 0x3EAEAEAF
        G3D::Color3(0.6784314f, 0.137254909f, 0.137254909f),  // 0x3F2DADAE 0x3E0C8C8D 0x3E0C8C8D
        G3D::Color3(0.164705887f, 0.294117659f, 0.843137264f),  // 0x3E28A8A9 0x3E969697 0x3F57D7D8
        G3D::Color3(0.113725491f, 0.4117647f, 0.0784313753f),  // 0x3DE8E8E9 0x3ED2D2D3 0x3DA0A0A1
        G3D::Color3(0.5058824f, 0.2901961f, 0.09803922f),  // 0x3F018182 0x3E949495 0x3DC8C8C9
        G3D::Color3(0.5058824f, 0.149019614f, 0.7529412f),  // 0x3F018182 0x3E189899 0x3F40C0C1
        G3D::Color3(0.627451f, 0.627451f, 0.627451f),  // 0x3F20A0A1 0x3F20A0A1 0x3F20A0A1
        G3D::Color3(0.5058824f, 0.772549033f, 0.08627451f),  // 0x3F018182 0x3F45C5C6 0x3DB0B0B1
        G3D::Color3(0.6156863f, 0.6862745f, 1.0f),  // 0x3F1D9D9E 0x3F2FAFB0 0x3F800000
        G3D::Color3(0.160784319f, 0.8156863f, 0.8156863f),  // 0x3E24A4A5 0x3F50D0D1 0x3F50D0D1
        G3D::Color3(1.0f, 0.572549045f, 0.2f),  // 0x3F800000 0x3F129293 0x3E4CCCCD
        G3D::Color3(1.0f, 0.933333337f, 0.2f),  // 0x3F800000 0x3F6EEEEF 0x3E4CCCCD
        G3D::Color3(0.9137255f, 0.870588243f, 0.733333349f),  // 0x3F69E9EA 0x3F5EDEDF 0x3F3BBBBC
        G3D::Color3(1.0f, 0.8039216f, 0.9529412f),  // 0x3F800000 0x3F4DCDCE 0x3F73F3F4
        G3D::Color3(1.0f, 1.0f, 1.0f),  // 0x3F800000 0x3F800000 0x3F800000
	};;

	// mac: 0x1F3774.
	const G3D::Color3* Color::getColorByIndex(int index)
	{
		// mac: 0x1F399E-A0: index % 16 == index - (index % 16 rounded via the
		// sar 31 / shr 28 / add / and ~15 trick), then imul 12 (0x1F39AD)
		// for the table stride.
		int slot = index % 16;
		return &sColorTable[slot];
	}

	// mac: 0x1F39BE.
	const G3D::Color3* Color::colorFromIndex8(int index)
	{
		int slot;
		switch (index)
		{
			case 0: slot = 2; break;
			case 1: slot = 3; break;
			case 2: slot = 4; break;
			case 3: slot = 6; break;
			case 4: slot = 11; break;
			case 5: slot = 12; break;
			case 6: slot = 14; break;
			default: slot = 13; break;
		}
		return getColorByIndex(slot);
	}
} // namespace RBX
