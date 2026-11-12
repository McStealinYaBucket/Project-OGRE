/* V8World/MultiJoint.h -- October 2013 (multi-connector joint base + Rotate/Glue).
   Evidence: MultiJoint C2(int) 0x5878DA / C2(p0,p1,c0,c1,maxConn) 0x58794A /
     D2 0x587A2A (:41 connector[0]==NULL, :42 numConnector==0, MutilJoint.cpp
     filename typo original); size 0xD0: +152 numConnector=0, +188 maxConn,
     +192..207 connector[4]=NULL; RotateP/V 0xDC, Glue 0x100 (+208..255 F0-F3);
     getJointType Rotate 6 / RotateP 7 / RotateV 8 / Glue 9 (10B const bodies);
     Glue isBreakable true 0x574A80 (only override in DB besides base false);
     RotateP/V C2 arg maxConn=2, Glue=4 (canBuildJoint construction sites).
   Hierarchy: Glue/Rotate -> MultiJoint -> Joint; Weld/Snap -> RigidJoint.
   DELTAS: connectors modeled as void* (exact connector types at integration);
     RotateP/V +208 angle/+212 connector/+216 int unmapped (follow-up);
     Glue F0-F3 default zero (builder computes them — follow-up);
     isBroken/putInKernel/stepper slots with stepper module (follow-up).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/MultiJoint.h (path inferred) */

#pragma once

#include "V8World/Joint.h"
#include "Util/SurfaceType.h"

namespace RBX {

	class MultiJoint : public Joint
	{
	public:
		explicit MultiJoint(int maxConnectors)
			: Joint()
			, numConnector(0)
			, maxConnector(maxConnectors)
		{
			for (int i = 0; i < 4; ++i)
				connectors[i] = NULL;
		}
		MultiJoint(Primitive* p0, Primitive* p1,
			const G3D::CoordinateFrame& c0, const G3D::CoordinateFrame& c1,
			int maxConnectors)
			: Joint(p0, p1, c0, c1)
			, numConnector(0)
			, maxConnector(maxConnectors)
		{
			for (int i = 0; i < 4; ++i)
				connectors[i] = NULL;
		}
		// STUB: PLAYER 0x009741c0
		// STUB: STUDIO 0x009bf3a0
		virtual ~MultiJoint()
		{
			// D2 0x587A2A MutilJoint.cpp:41-42 (+ Joint D2 via base).
			RBXASSERT(connectors[0] == NULL);
			RBXASSERT(numConnector == 0);
		}

	protected:
		int numConnector;      // +152
		int maxConnector;      // +188
		void* connectors[4];   // +192..207 (exact types at integration)
	};

	class RotateJoint : public MultiJoint
	{
	public:
		RotateJoint()
			: MultiJoint(2)
		{}
		RotateJoint(Primitive* p0, Primitive* p1,
			const G3D::CoordinateFrame& c0, const G3D::CoordinateFrame& c1)
			: MultiJoint(p0, p1, c0, c1, 2)
		{}

		virtual int getJointType() const { return ROTATE_JOINT; }

		// mac: 0x59833E (542B).  STATIC factory (no `this` in the mangling):
		// ROTATE -> RotateJoint, ROTATE_V -> RotateVJoint (+208 = the st0
		// return of RotateConnector::computeJointAngle, +212/+216 = 0),
		// ROTATE_P -> RotatePJoint (same), anything else -> RBXASSERT(false)
		// (RotateJoint.cpp:75) and NULL.  See v8world/RotateJoint.cpp.
		static Joint* surfaceTypeToJoint(
			SurfaceType surfaceType,
			Primitive* p0,
			Primitive* p1,
			const G3D::CoordinateFrame& c0,
			const G3D::CoordinateFrame& c1);

		// mac: 0x59855C (2196B).  STATIC factory returning Joint* (NULL when the
		// pair cannot carry a rotate joint): guard (either surface IsRotate),
		// near-opposite world normals (angle <= 0.025), face overlap (0.35)
		// within planes (0.05), extrusion containment with the either/
		// contains0 swap rule, then two jointFramesCoincide attempts
		// (grid-snapped, then raw) feeding surfaceTypeToJoint.
		// See v8world/RotateJoint.cpp.
		static Joint* canBuildJoint(
			Primitive* p0,
			Primitive* p1,
			NormalId n0,
			NormalId n1);
	};

	class RotatePJoint : public MultiJoint
	{
	public:
		// +208 (0xD0) float angle, +212 (0xD4) int, +216 (0xD8) int.
		// Pinned by RotateJoint::surfaceTypeToJoint 0x59833E: after the
		// MultiJoint(p0,p1,c0,c1,2) ctor it does `movss [esi+0D0h], angle`
		// (the st0 return of RotateConnector::computeJointAngle, popped by
		// `fstp` at 0x5983F2), then `mov [esi+0D4h], 0` and
		// `mov [esi+0D8h], 0`.  The 2016 source calls +208 `angle`; the
		// +212/+216 split is NOT recoverable from two zero stores (they
		// could be a nulled pointer plus an int, or two ints), so both are
		// plain ints until a reader of either appears.  Total 0xDC.
		float angle;      // +208
		int int212;       // +212 (always 0 at the construction site)
		int int216;       // +216 (always 0 at the construction site)

		RotatePJoint()
			: MultiJoint(2)
			, angle(0.0f)
			, int212(0)
			, int216(0)
		{}
		RotatePJoint(Primitive* p0, Primitive* p1,
			const G3D::CoordinateFrame& c0, const G3D::CoordinateFrame& c1)
			: MultiJoint(p0, p1, c0, c1, 2)
			, angle(0.0f)
			, int212(0)
			, int216(0)
		{}

		virtual int getJointType() const { return ROTATE_P_JOINT; }
	};

	class RotateVJoint : public MultiJoint
	{
	public:
		// Same tail as RotatePJoint (0x598440: identical three stores).
		float angle;      // +208
		int int212;       // +212 (always 0 at the construction site)
		int int216;       // +216 (always 0 at the construction site)

		RotateVJoint()
			: MultiJoint(2)
			, angle(0.0f)
			, int212(0)
			, int216(0)
		{}
		RotateVJoint(Primitive* p0, Primitive* p1,
			const G3D::CoordinateFrame& c0, const G3D::CoordinateFrame& c1)
			: MultiJoint(p0, p1, c0, c1, 2)
			, angle(0.0f)
			, int212(0)
			, int216(0)
		{}

		virtual int getJointType() const { return ROTATE_V_JOINT; }
	};

	class GlueJoint : public MultiJoint
	{
	public:
		GlueJoint()
			: MultiJoint(4)
			, f0(G3D::Vector3::zero())
			, f1(G3D::Vector3::zero())
			, f2(G3D::Vector3::zero())
			, f3(G3D::Vector3::zero())
		{}
		GlueJoint(Primitive* p0, Primitive* p1,
			const G3D::CoordinateFrame& c0, const G3D::CoordinateFrame& c1)
			: MultiJoint(p0, p1, c0, c1, 4)
			, f0(G3D::Vector3::zero())
			, f1(G3D::Vector3::zero())
			, f2(G3D::Vector3::zero())
			, f3(G3D::Vector3::zero())
		{}

		virtual int getJointType() const { return GLUE_JOINT; }
		// FUNCTION: PLAYER 0x006be550 FOLDED
		// FUNCTION: STUDIO 0x0078ad10 FOLDED
		virtual bool isBreakable() const { return true; }

		// mac: 0x574988  GlueJoint::setFacePoint(int, Vector3 const&).  Evidenced
		// by symbol only; no caller in the tree yet and its body is not
		// decoded, so it stays UNDEFINED (non-virtual -- no vtable impact).
		void setFacePoint(int index, const G3D::Vector3& point);

		// Bring-up helper (not in binary): read back the footprint points
		// canBuildJoint computes.  Tests need to observe f0..f3.
		const G3D::Vector3& getFacePoint(int index) const
		{
			return (&f0)[index];
		}

		// mac: 0x57382E (1775B).  STATIC factory returning Joint*: gate
		// (GLUE on either face) + canBuildJointLoose, project/overlap the
		// faces into both object spaces, snap (0.1) and realign in world,
		// cornersAligned (0.05) with a snapped-then-raw second attempt,
		// then new GlueJoint(p0,p1,c0,c1-snapped) with the overlap footprint
		// (in c0's frame) stored to f0..f3.  See v8world/GlueJoint.cpp.
		static Joint* canBuildJoint(
			Primitive* p0,
			Primitive* p1,
			NormalId n0,
			NormalId n1);

	private:
		// +208/+220/+232/+244 (computed by the builder — follow-up).
		G3D::Vector3 f0;
		G3D::Vector3 f1;
		G3D::Vector3 f2;
		G3D::Vector3 f3;
	};

} // namespace RBX
