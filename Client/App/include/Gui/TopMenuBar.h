/* gui/TopMenuBar.h -- October 2013 C++-only widget (NO reflection).
   Evidence: N (zero create<> hits; C2(void) 0x464A00; GuiItem base); init (style/backdrop/visible) TODO — member map unmapped.
   TODO: full decompile (member map + init/render/process bodies).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/TopMenuBar.h */

#pragma once

#include "V8DataModel/Item.h"
#include "G3D/Color4.h"

namespace RBX {

	class TopMenuBar
		: public GuiItem
	{
	public:
		// GuiBuilder writes visible/style/backdrop (panel birth-hidden + restyle).
		friend class GuiBuilder;
		// C2(void) 0x464A00; init 0x5CDCA6 (backdrop clear, style 0, visible 1).
		TopMenuBar()
			: backdropColor(G3D::Color4::clear()), style(0), visible(true)
		{}
		virtual bool isVisible() const { return visible; }
	protected:
		G3D::Color4 backdropColor;  // +112..127 (clear).
		int style;  // +128 (0; getSize/getChildPosition branch — bodies TODO).
		bool visible;  // +132.
	};

} // namespace RBX

