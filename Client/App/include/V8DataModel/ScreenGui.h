/* V8DataModel/ScreenGui.h — October 2013 (screen gui shell).
   Evidence: ScreenGui C2s 0x470D36/0x470EAA (GuiLayerCollector(name) with
     "ScreenGui" default; DescribedCreatable<ScreenGui, GuiLayerCollector,
     sScreenGui>); CoreGuiService::createRobloxScreenGui 0x45E19E
     (create + "RobloxGui" + lock + parent).
      Member pins (rarc15/rarc16 C2s): the +0xB0..+0xBC four-dword window =
      our windowRect below (GuiObject::getWindowRect 0x3ABA9A copies it
      VERBATIM into its Rect2D sret, raw 3ABBA9..3ABBD2; the default ctor
      0x470D36 writes {0, 0, 800.0f, 600.0f} -- 0x44480000/0x44160000, the
      legacy default client viewport -- while the name-ctor 0x470EAA
      zero-fills the whole rect, 470FB2..470FDA); both ctors zero dword 43
      (+0xAC) after the +0x30 descriptor/registrar work, carry a 7th vptr
      slot at +0xA8 (dword 42, single-base DELTA = not declared), zero
      dwords 49..51 (+0xC4..+0xCC) and word +0xC0 = 0, and end with dwords
      55/56 self-pointing at +0xD4 (an empty list head identity).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ScreenGui.h */

#pragma once

#include "V8DataModel/GuiLayerCollector.h"

namespace RBX {

	extern const char* const sScreenGui;

	class ScreenGui
		: public DescribedCreatable<ScreenGui, GuiLayerCollector, sScreenGui>
		// Restored TRUE parent (was Instance standing in).
	{
	public:
		ScreenGui();
		ScreenGui(const char* name);

		// +0xB0..+0xBC (rarc15/16 C2 = verbatim dword evidence): the
		// ScreenGui window -- GuiObject::getWindowRect 0x3ABA9A copies
		// these four dwords VERBATIM into its Rect2D sret when the
		// ancestor walk finds a ScreenGui (raw 3ABBA9..3ABBD2).
		// Corner-form (x0, y0, x1, y1): the default ctor = {0,0,800,600}
		// (so the landed GuiBase2d::recalculateAbsoluteSizeAndPosition
		// reads size (800,600) = the legacy default client viewport),
		// the name-ctor = all-zero.  DELTA: pos/corner identities exact
		// as the four verbatim dwords; no reflection prop binds here
		// (the rarc15 ScreenGui PropDescriptor<ScreenGui, Vector2int16>
		// pair binds the GuiBase2d abs-props, not this block).
		G3D::Rect2D windowRect;
	};

} // namespace RBX
