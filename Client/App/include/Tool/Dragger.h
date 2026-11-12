/* Tool/Dragger.h -- October 2013 (drag-safe placement helpers).
   Evidence: the whole class is a set of STATIC members (no vtable, no ctor).
   Confirmed bodies/EAs in the symbolled binary (all take
   G3D::Array<RBX::Primitive*,10,32> or std::vector<RBX::Primitive*>):
     0x1C150A computeExtents(std::vector<Primitive*> const&)
     0x1C1670 computeExtents(G3D::Array<Primitive*,10,32> const&)
     0x1C17CE intersectingWorldOrOthers(PartInstance&, ContactManager&, float, float)
     0x1C1852 intersectingWorldOrOthers(Array const&, ContactManager&, float, float)
     0x1C18FA intersectingGroundPlane(Array const&, float)
     0x1C196E movePrimitivesGoal(Array const&, Vector3 const&, Vector3&)
     0x1C1A24 movePrimitives(Array const&, Vector3 const&)
     0x1C1AE8 movePrimitivesDelta(Array const&, Vector3 const&, Vector3&)
     0x1C1BF2 searchUpFine(Array const&, Vector3&, ContactManager&, float)
     0x1C1D3C searchDownFine(Array const&, Vector3&, ContactManager&, float)
     0x1C1E9C searchUpGross(Array const&, Vector3&, ContactManager&, float)
     0x1C2016 searchDownGross(Array const&, Vector3&, ContactManager&, float)
     0x1C21E6 safePlaceAlongLine(Array const&, Vector3 const&, Vector3 const&, Vector3&,
                                  ContactManager&)
     0x1C2622 safeMoveAlongLine(Array const&, Vector3 const&, ContactManager&, float)
     0x1C28F6 safeMoveYDrop(Array const&, Vector3 const&, ContactManager&, float)
     0x1C2A62 safeMoveYDrop_EXT(Array const&, Vector3 const&, ContactManager&, float)
     0x1C300A safeMoveNoDrop(Array const&, Vector3 const&, ContactManager&)
     0x1C3162 safeRotate(Array const&, Matrix3 const&, ContactManager&)
     0x1C348C safeRotate2(Array const&, Matrix3 const&, ContactManager&)
     0x1C3552 intersectingWorldOrOthers_EXT(vector<Extents>&, Array const&,
                  unordered_set<Primitive const*>&, ContactManager&, float, Vector3 const&)
     0x1C39B0 searchUpGross_EXT / 0x1C3B26 searchDownGross_EXT /
     0x1C3C9C searchDownFine_EXT / 0x1C3E22 searchUpFine_EXT (same shape)
     0x1C3F66 intersectingGroundPlane_EXT(vector<Extents> const&, Array const&, float,
                  Vector3 const&)
     0x1C407E isIntersecting(Primitive const*, CoordinateFrame const&,
                  Primitive const*, CoordinateFrame const&)
     0x1C4122 checkBallBallIntersection / 0x1C427A checkBallPolyIntersection /
     0x1C491E checkPolyPolyIntersection (same 4-arg shape)
     0x1C57AC moveExtents(vector<Extents>&, Vector3 const&)
   Only safeMoveNoDrop is modelled (PartInstance::safeMove 0x44787C calls it);
   every other body is TODO.  The body here is a BRING-UP NO-OP -- the real one
   scans upward/downward for a free spot and needs the ContactManager
   broadphase, which is not ported.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/tool/Dragger.h (path inferred) */

#pragma once

#include "G3D/Array.h"
#include "G3D/CoordinateFrame.h"
#include "G3D/Matrix3.h"
#include "G3D/Vector3.h"

namespace RBX {

	class ContactManager;
	class Primitive;

	class Dragger
	{
	public:
		// mac: 0x1C300A.
		static void safeMoveNoDrop(const G3D::Array<Primitive*, 10, 32>& prims,
			const G3D::Vector3& delta, ContactManager& contactManager);
	};

} // namespace RBX
