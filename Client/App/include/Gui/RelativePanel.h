/* gui/RelativePanel.h -- October 2013 C++-only widget (NO reflection).
   Evidence: N (NonFactoryProduct base only); C2(Layout) 0x3A6E40 + C2(void) 0x431324; init(Layout) 0x5CDA58 memberwise copy TODO — member map unmapped.
   TODO: full decompile (member map + init/render/process bodies).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/RelativePanel.h */

#pragma once

#include "gui/TopMenuBar.h"
#include "gui/Layout.h"
#include "Util/Rect.h"
#include "G3D/Color4.h"

namespace RBX {

	class RelativePanel
		: public TopMenuBar
	{
	public:
		// C2(void) 0x431324 (default Layout) + C2(Layout) 0x3A6E40;
		// init(Layout) 0x5CDA58: xLoc/yLoc/offset stored +136..144,
		// style/backdrop OVERWRITE the inherited +128/+112 slots.
		RelativePanel()
			: xLocCode(Rect::LEFT), yLocCode(Rect::TOP), offset(0, 0)
		{
			style = Layout::HORIZONTAL;
			backdropColor = G3D::Color4::clear();
		}
		RelativePanel(const Layout& layout)
			: xLocCode(layout.xLocation), yLocCode(layout.yLocation), offset(layout.offset)
		{
			style = layout.layoutStyle;
			backdropColor = layout.backdropColor;
		}
	private:
		int xLocCode;  // +136 (Layout[0]).
		int yLocCode;  // +140 (Layout[1]).
		G3D::Vector2int16 offset;  // +144 (Layout[2]).
	};

} // namespace RBX

