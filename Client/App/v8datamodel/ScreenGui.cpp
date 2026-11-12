/* App/v8datamodel/ScreenGui.cpp — October 2013 (ctor pours: the +0xB0..+0xBC
   windowRect defaults pour per rarc15/16 -- default ctor {0,0,800,600},
   name-ctor zero-filled).
   Evidence: ScreenGui C2s 0x470D36/0x470EAA.
   TODO: full decompile (display surface). */

#include "stdafx.h"

#include "V8DataModel/ScreenGui.h"

namespace RBX {
	const char* const sScreenGui = "ScreenGui";
}

using namespace RBX;

REFLECTION_BEGIN();
REFLECTION_END();

// Class registrar (see Instance.cpp/Service.cpp note).
namespace RBX {
RBX_REGISTER_CLASS(RBX::ScreenGui);
}

namespace RBX {

	typedef DescribedCreatable<ScreenGui, GuiLayerCollector, sScreenGui> ScreenGuiSuper;

	// mac: 0x470D36 (372B; rarc15): GuiLayerCollector("ScreenGui") + the
	// pinned member block: +0xAC = 0; the +0xB0..+0xBC window =
	// {0, 0, 800.0f, 600.0f} (C2 dword 46/47 = 0x44480000/0x44160000,
	// raw 470E05-family); dwords 49..51 (+0xC4..+0xCC) zero; word
	// +0xC0 = 0; the +0xD4..+0xE4 region zero with +0xDC/+0xE0
	// self-pointing at +0xD4.
	ScreenGui::ScreenGui()
		: ScreenGuiSuper("ScreenGui")
		, windowRect(G3D::Rect2D::xyxy(0.0f, 0.0f, 800.0f, 600.0f))
	{
	}

	// mac: 0x470EAA (364B; rarc16): same member block, but the window is
	// ZERO-FILLED (raw 470FB2..470FD0 all four dwords, word +0xC0 = 0
	// at 470FDA) -- the name-ctor ScreenGui starts with a zero rect,
	// unlike the default ctor's 800x600 preset.
	ScreenGui::ScreenGui(const char* name)
		: ScreenGuiSuper(name)
		, windowRect(G3D::Rect2D::xyxy(0.0f, 0.0f, 0.0f, 0.0f))
	{
	}

} // namespace RBX
