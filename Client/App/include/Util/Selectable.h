/* Util/Selectable.h -- October 2013. One slot (vftable {for Selectable} at PartInstance+0xa0,
   ModelInstance+0x94). Path from the 2016 tree: no 2013 __FILE__ names this header. */

#pragma once

#include "rbx/Declarations.h"

namespace RBX {

	class RBXBaseClass Selectable
	{
	public:
		// PLAYER 0x006be550 / STUDIO 0x0078ad10, a `mov al,1; ret` body ICF shares: no marker.
		virtual bool isSelectable3d() { return true; }  // vtable+0x00
	};

} // namespace RBX
