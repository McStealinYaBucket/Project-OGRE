/* gui/Layout.h -- October 2013 (widget layout value).
   Carried from March 2016 App/include/gui/Layout.h (first-party; agent-
   verified 32B aggregate {xLoc,yLoc,offset,style,backdrop}; defaults
   LEFT/TOP/(0,0)/HORIZONTAL/clear match RelativePanel C2(void) 0x431324).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/Layout.h */

#pragma once

#include "Util/Rect.h"
#include "Util/G3DCore.h"

namespace RBX {

	class Layout 
	{
	public:
		enum Style	{HORIZONTAL = 0, VERTICAL = 1};

		Rect::Location		xLocation;
		Rect::Location		yLocation;
		Vector2int16		offset;
		Style				layoutStyle;
		Color4				backdropColor;

		Layout() 
			: xLocation(Rect::LEFT)
			, yLocation(Rect::TOP)
			, offset(Vector2int16(0,0))
			, layoutStyle(HORIZONTAL)
			, backdropColor(Color4::clear())
		{}
	};
}	// namespace
