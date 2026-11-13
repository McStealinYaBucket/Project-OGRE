/* Util/IHasLocation.h -- October 2013.
   One-slot interface, always inherited virtually (PartInstance and ModelInstance reach it through
   IHasPrimaryPart and CameraSubject; RTTI 0x00b4c784 / 0x00b3f6bc list it with pdisp 100, vdisp 4).
   2013 vftable {for IHasLocation}: one slot, getLocation (PLAYER PartInstance 0x00723590, a vtordisp
   thunk to 0x00723520). No virtual destructor in 2013 (2016 adds one; the 2013 vftable has no room).
   Path from the 2016 tree: no 2013 __FILE__ names this header. */

#pragma once

#include "rbx/Declarations.h"
#include "G3D/CoordinateFrame.h"

namespace RBX {

	class RBXInterface IHasLocation
	{
	public:
		virtual G3D::CoordinateFrame getLocation() = 0;  // vtable+0x00
	};

} // namespace RBX
