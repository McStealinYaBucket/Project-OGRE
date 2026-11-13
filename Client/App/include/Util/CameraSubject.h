/* Util/CameraSubject.h -- October 2013.
   2013 vftable {for CameraSubject}: 10 slots (PLAYER PartInstance 0x00b4c6d8, ModelInstance group at
   +0x80 of 0x00b3f6bc). Slot names from iOS 10-29 (RBX::CameraSubject::onCameraWrapMouse, zoom,
   stepLocationAndFocus, getSelectionIgnorePrimitives, ...); the order is the iOS order with the
   Itanium D1/D0 pair collapsed onto MSVC's one destructor slot.
   Subobject size 0x14 (PartInstance +0x8c..+0xa0, ModelInstance +0x80..+0x94): vfptr, vbptr for the
   virtual IHasLocation, then three dwords the PartInstance ctor 0x007268d0 initialises (+0x08 byte,
   +0x10 float); their names are not known yet.
   2016 differences: 2016 drops onCameraWrapMouse, zoom and stepLocationAndFocus and the data.
   The trivial defaults are bodies /OPT:ICF shares with other functions, so they carry no marker here:
   onCameraWrapMouse PLAYER 0x00528720 (STUDIO 0x00606c90), onCameraHeartbeat 0x008c8450 (0x006125f0),
   getSelectionIgnorePrimitives 0x00575d00 (0x007aa330), zoom 0x005b7f70 (0x0065d860).
   Path from the 2016 tree: no 2013 __FILE__ names this header. */

#pragma once

#include <vector>
#include "rbx/Declarations.h"
#include "Util/IHasLocation.h"
#include "G3D/Vector3.h"
#include "G3D/CoordinateFrame.h"
#include "decomp.h"

namespace RBX {

	class Primitive;

	class RBXBaseClass CameraSubject : public virtual IHasLocation
	{
	public:
		virtual ~CameraSubject() {}  // vtable+0x00

		virtual void onCameraWrapMouse() {}  // vtable+0x04 (`ret`)
		virtual void onCameraHeartbeat(const G3D::Vector3& cameraLocation, const G3D::Vector3& focusPoint) {}  // vtable+0x08
		virtual G3D::CoordinateFrame getRenderLocation() = 0;  // vtable+0x0c
		virtual G3D::Vector3 getRenderSize() = 0;              // vtable+0x10
		virtual void onCameraNear(float distance) {}           // vtable+0x14
		virtual void getCameraIgnorePrimitives(std::vector<const Primitive*>& primitives) {}  // vtable+0x18
		virtual void getSelectionIgnorePrimitives(std::vector<const Primitive*>& primitives) {}  // vtable+0x1c (`ret 4`)
		virtual bool zoom(float distance, G3D::CoordinateFrame& cameraCoord, G3D::CoordinateFrame& focusCoord)  // vtable+0x20
		{
			return false;
		}
		// FUNCTION: PLAYER 0x006d2920
		// FUNCTION: STUDIO 0x00787c10
		virtual void stepLocationAndFocus(G3D::Vector3& location, G3D::CoordinateFrame& focus, double elapsed)  // vtable+0x24
		{
			focus = getRenderLocation();
		}

	private:
		undefined m_unk0x08[0x14 - 0x08];  // 0x08 (vbptr at 0x04)
	};

} // namespace RBX
