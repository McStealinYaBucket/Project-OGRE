/* App/v8world/Joint.cpp -- October 2013 (joint base bodies).
   See V8World/Joint.h for evidence. isGeometryOrthogonal mapped by
   GeometryType kind (legacy 6-sided vs generalized polyhedra, per the 2016
   comment; TODO-verify the exact 2013 set).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/Joint.cpp (path inferred) */

#include "stdafx.h"

#include "V8World/Joint.h"
#include "V8World/Primitive.h"
#include "V8World/World.h"
#include "V8World/IMoving.h"
#include "Util/Math.h"
#include "Util/Face.h"
#include "Util/FastFlags.h"
#include "rbx/Debug.h"

namespace RBX {

	// STUB: PLAYER 0x00821410
	// STUB: STUDIO 0x00914ae0
	Joint::Joint()
		: Edge(NULL, NULL)
		, jointOwner(NULL)
	{
		// C2(void) 0x577C98 Joint.cpp:77-78 (members default to identity).
		RBXASSERT(jointCoord0 == G3D::CoordinateFrame());
		RBXASSERT(jointCoord1 == G3D::CoordinateFrame());
	}

	// STUB: PLAYER 0x008215c0
	// STUB: STUDIO 0x00914c90
	Joint::Joint(Primitive* p0, Primitive* p1,
		const G3D::CoordinateFrame& c0, const G3D::CoordinateFrame& c1)
		: Edge(p0, p1)
		, jointOwner(NULL)
	{
		// TODO: FastLog JointLifetime "created, empty primitives" (channel absent).
		if (p0 != NULL && p1 != NULL &&
			(!p0->isGeometryOrthogonal() || !p1->isGeometryOrthogonal()))
		{
			jointCoord0 = c0;
			jointCoord1 = c1;
		}
		else
		{
			jointCoord0 = Math::snapToGrid(c0, 0.1f);
			jointCoord1 = Math::snapToGrid(c1, 0.1f);
			RBXASSERT(Math::isOrthonormal(jointCoord0.rotation));  // Joint.cpp:59-60
			RBXASSERT(Math::isOrthonormal(jointCoord1.rotation));
		}
	}

	// STUB: PLAYER 0x00821010
	// STUB: STUDIO 0x009146e0
	Joint::~Joint()
	{
		// D2 0x577EF0 Joint.cpp:83 (+ lifetime log TODO).
		RBXASSERT(jointOwner == NULL);
	}

	// STUB: PLAYER 0x0081fae0
	// STUB: STUDIO 0x00913180
	void Joint::setJointOwner(JointInstance* owner)
	{
		// mac: 0x5781A0 Joint.cpp:133 (exactly one side set).
		RBXASSERT((owner == NULL) != (jointOwner == NULL));
		jointOwner = owner;
	}

	// mac: 0x578C5C (batch238): prim+8 node or NULL (Primitive secondary is
	// SpanningNode at +8; static_cast spells the MSVC MI equivalent).
	// STUB: PLAYER 0x0081fd10
	// STUB: STUDIO 0x009133c0
	SpanningNode* Joint::getNode(int i)
	{
		Primitive* prim = getPrimitive(i);
		if (prim == NULL)
			return NULL;
		return static_cast<SpanningNode*>(prim);
	}

	// mac: 0x578C8C (batch238: const mirror).
	const SpanningNode* Joint::getConstNode(int i) const
	{
		const Primitive* prim = const_cast<Joint*>(this)->getPrimitive(i);
		if (prim == NULL)
			return NULL;
		return static_cast<const SpanningNode*>(prim);
	}

	bool Joint::isAutoJoint() const
	{
		// Filter verbatim (getEdgeType==0 required throughout).
		int e = getEdgeType();
		int t = getJointType();
		if (e == 0 && (t == 0 || t == 11))
			return false;
		if (e == 0 && t == 12)
			return false;
		return e == 0 && ((t & ~8) != 2);
	}

	void Joint::notifyMoved()
	{
		// mac: 0x57808E: ping both prims' myOwner movers (prim+244).
		for (int i = 0; i < 2; ++i)
		{
			Primitive* prim = (i == 0) ? getPrim0() : getPrim1();
			IMoving* mover = (prim != NULL) ? prim->getOwner() : NULL;
			if (mover != NULL)
				mover->notifyMoved();
		}
	}

	// mac: 0x57878E (105 bytes) -- a two-stage OR, no `this` use at all:
	//   al = 1;
	//   if (FaceVerticesOverlapped(p0,n0,p1,n1,tol) == 0)
	//       al = FaceEdgesOverlapped(p0,n0,p1,n1,tol);
	//   return al;
	// DELTA (documented): FaceVerticesOverlapped / FaceEdgesOverlapped are
	// the non-orthogonal (Poly/Ball) exact-polygon tests and are not
	// ported -- they need the Geometry subclasses.  Only the OR shape and
	// the call order are reproduced; the pair always reports "no overlap",
	// which gates Joint::canBuildJoint's non-orthogonal branch to false
	// exactly as the chain above does when both helpers are unavailable.
	// STUB: PLAYER 0x00820fb0
	// STUB: STUDIO 0x00914660
	bool Joint::FacesOverlapped(const Primitive* p0, unsigned long n0,
		const Primitive* p1, unsigned long n1, float tolerance)
	{
		(void)p0; (void)n0; (void)p1; (void)n1; (void)tolerance;
		return false;
	}

	// mac: 0x57840E (896 bytes).  Control flow, verbatim:
	//
	//   if (p0->isGeometryOrthogonal() && p1->isGeometryOrthogonal())
	//   {
	//       if (Math::fuzzyAxisAligned(cf0.rotation, cf1.rotation, angTol))
	//       {
	//           Face f0 = p0->getFaceInWorld(n0);
	//           Face f1 = p1->getFaceInWorld(n1);
	//           // 0x5784C7: FFlag branch picks the hasOverlap threshold.
	//           if (Face::hasOverlap(f0, f1, flag ? 0.1f : 0.35f) &&
	//               Face::overlapWithinPlanes(f0, f1, planeTol))
	//               return true;
	//       }
	//   }
	//   else
	//   {
	//       // geometry-driven path, 0x5784E8; see the DELTA note below.
	//   }
	//   return false;
	// STUB: PLAYER 0x00821120
	// STUB: STUDIO 0x009147f0
	bool Joint::canBuildJoint(Primitive* p0, Primitive* p1, NormalId n0, NormalId n1,
		float fuzzyAngleTolerance, float planeTolerance)
	{
		if (p0->isGeometryOrthogonal() && p1->isGeometryOrthogonal())
		{
			if (Math::fuzzyAxisAligned(p0->getCoordinateFrame().rotation,
					p1->getCoordinateFrame().rotation, fuzzyAngleTolerance))
			{
				Face f0 = p0->getFaceInWorld(n0);
				Face f1 = p1->getFaceInWorld(n1);
				// mac: 0x5784DB 3DCCCCCDh (0.1f) when the FFlag is set,
				// mac: 0x57872E 3EB33333h (0.35f) when it is not.
				float overlapTolerance =
					FFlag::PhysicsAllowAutoJointsWithSmallParts_DE6056 ? 0.1f : 0.35f;
				if (Face::hasOverlap(f0, f1, overlapTolerance) &&
					Face::overlapWithinPlanes(f0, f1, planeTolerance))
				{
					return true;
				}
			}
			return false;
		}

		// Non-orthogonal branch (0x5784E8).  The exact binary sequence is:
		//   int i0 = (*(p0->geom))->vtable[0x44](n0);      // slot [17]
		//   int i1 = (*(p1->geom))->vtable[0x44](n1);
		//   if (i0 != -1 && i1 != -1)
		//   {
		//       Matrix3 m0 = cf0.rotation * p0->geom->vtable[0x28]();  // slot [10]
		//       Matrix3 m1 = cf1.rotation * p1->geom->vtable[0x28]();
		//       if (Math::fuzzyAxisAligned(m0, m1, fuzzyAngleTolerance) &&
		//           FacesOverlapped(p0, i0, p1, i1, 0.005f))        // 0x3F7D70A4
		//       {
		//           // 0x5785xx: project the two face centres onto the shared
		//           // axis and accept when |delta| <= 2 * planeTolerance.
		//           return fabsf( dot(delta, axis) ) <= 2.0f * planeTolerance;
		//       }
		//   }
		//   return false;
		// DELTA: the two Geometry polymorphic slots (the this+0xEC pointer
		// used by getGridCorner / computeFuzzyExtents / isGeometryOrthogonal)
		// are not ported, and the FacesOverlapped body above is a
		// documented no-op, so this branch reports false.  Every geometry
		// kind PartInstance actually builds (GEOMETRY_BLOCK and the mesh
		// kinds) answers isGeometryOrthogonal() == true, so the ported
		// branch above is the live one.
		return false;
	}

	// mac: 0x4103C8 (115B, batch216). Unsigned range tests, exactly as decoded:
	// types 1..5 and 6..10 short-circuit true; otherwise anchor/free only.
	bool Joint::isSpanningTreeJoint(const Edge* e)
	{
		if (e->getEdgeType() != Edge::JOINT
			|| (unsigned int)(rbx_static_cast<const Joint*>(e)->getJointType() - 1) >= 5)
		{
			if (e->getEdgeType() != Edge::JOINT
				|| (unsigned int)(rbx_static_cast<const Joint*>(e)->getJointType() - 6) >= 5)
			{
				int type = 13;
				if (e->getEdgeType() == Edge::JOINT)
					type = rbx_static_cast<const Joint*>(e)->getJointType();
				return type == Joint::ANCHOR_JOINT || type == Joint::FREE_JOINT;
			}
		}
		return true;
	}

	// mac: 0x575CA0 / 0x575B7C (112B each, batch218).
	AnchorJoint::AnchorJoint(Primitive* prim)
		: Joint(prim, NULL, G3D::CoordinateFrame(), G3D::CoordinateFrame())
	{
	}

	FreeJoint::FreeJoint(Primitive* prim)
		: Joint(prim, NULL, G3D::CoordinateFrame(), G3D::CoordinateFrame())
	{
	}

	// mac: 0x5780CE (70B, batch218): binary loops the joints-list count (+172).
	// STUB: PLAYER 0x00820c80
	// STUB: STUDIO 0x00914330
	const Joint* Joint::findConstJoint(const Primitive* p, JointType type)
	{
		for (int i = 0; i < p->joints.size(); ++i)
		{
			const Joint* joint = p->getConstJoint(i);
			if (joint->getJointType() == type)
				return joint;
		}
		return NULL;
	}

	// mac: 0x578114 (129B, batch219a).
	// STUB: PLAYER 0x00820cc0
	// STUB: STUDIO 0x00914370
	const Joint* Joint::getConstJoint(const Primitive* p, JointType type)
	{
		const Joint* answer = findConstJoint(p, type);
		RBXASSERT(answer != NULL);  // Joint.cpp:120 "answer"
		return answer;
	}

	// mac: 0x578196 (9B, batch219a).
	// FUNCTION: PLAYER 0x00820d20
	// FUNCTION: STUDIO 0x009143d0
	Joint* Joint::getJoint(Primitive* p, JointType type)
	{
		return const_cast<Joint*>(getConstJoint(p, type));
	}

	// mac: 0x578875 (53B, batch228).
	// STUB: PLAYER 0x00820e70
	// STUB: STUDIO 0x00914520
	unsigned int getJointSize2(const Joint* joint)
	{
		unsigned int size = joint->getPrim0()->getSortSize();
		if (joint->getPrim1() != NULL)
		{
			unsigned int other = joint->getPrim1()->getSortSize();
			if (size < other)
				return other;
		}
		return size;
	}

	// mac: 0x5788AA (749B, batch226): weight order (type, size, guid,
	// address). The SpanningEdge*-to-Joint back-adjust is binary -32 MI
	// arithmetic; static_cast is the MSVC-correct equivalent.
	// STUB: PLAYER 0x00820ef0
	// STUB: STUDIO 0x009145a0
	bool Joint::isHeavierThan(const SpanningEdge* other) const
	{
		const Joint* o = NULL;
		if (other != NULL)
			o = static_cast<const Joint*>(other);
		if (o == this)
			return false;
		int t0 = getJointType();
		int t1 = o->getJointType();
		if (t0 != t1)
			return t0 < t1;
		unsigned int s0 = getJointSize2(this);
		unsigned int s1 = getJointSize2(o);
		if (s0 > s1)
			return true;
		if (s1 != s0)
			return false;
		const Guid* g00 = &getPrim0()->getGuid();
		const Guid* g01 = NULL;
		if (getPrim1() != NULL)
			g01 = &getPrim1()->getGuid();
		const Guid* g10 = &o->getPrim0()->getGuid();
		const Guid* g11 = NULL;
		if (o->getPrim1() != NULL)
			g11 = &o->getPrim1()->getGuid();
		int answer = Guid::compare(g00, g01, g10, g11);
		RBXASSERT(answer == Guid::compare(g01, g00, g10, g11));  // :318
		RBXASSERT(answer == Guid::compare(g00, g01, g11, g10));  // :319
		RBXASSERT(answer == -Guid::compare(g11, g10, g00, g01));  // :320
		RBXASSERT(answer == -Guid::compare(g10, g11, g01, g00));  // :321
		if (answer > 0)
			return true;
		return (this < o) && (answer >= 0);
	}

	// mac: 0x578BBC (40B, batch226): the far-end node. Binary spells the
	// prim-lattice conversion as raw +8/-8 MI arithmetic; static_cast is
	// the MSVC-correct equivalent (never raw offsets across the MI).
	// STUB: PLAYER 0x00820f80
	// STUB: STUDIO 0x00914630
	SpanningNode* Joint::otherNode(SpanningNode* node)
	{
		Primitive* base = NULL;
		if (node != NULL)
			base = static_cast<Primitive*>(node);
		Primitive* other = getPrim0() == base ? getPrim1() : getPrim0();
		if (other == NULL)
			return NULL;
		return static_cast<SpanningNode*>(other);
	}

	// mac: 0x578C0C (batch226): const mirror with a parallel body. The 2013 linker
	// folded it with otherNode: both SpanningEdge slots +0x08/+0x0c hold 0x00820f80.
	const SpanningNode* Joint::otherConstNode(const SpanningNode* node) const
	{
		const Primitive* base = NULL;
		if (node != NULL)
			base = static_cast<const Primitive*>(node);
		const Primitive* other = getPrim0() == base ? getPrim1() : getPrim0();
		if (other == NULL)
			return NULL;
		return static_cast<const SpanningNode*>(other);
	}

	// mac: 0x57823A (176B, batch219a): FastLog-gated world-notify relink.
	// DELTA (return): binary returns a byte no in-tree caller uses; void
	// here (overrides Edge::setPrimitive -- see its DELTA).
	// STUB: PLAYER 0x00820d30
	// STUB: STUDIO 0x009143e0
	void Joint::setPrimitive(int i, Primitive* p)
	{
		// FastLog JointLifetime "Joint %p, setting primitive %u to %p"
		// skipped (channel absent -- same precedent as Joint C2).
		if (getPrimitive(i) == p)
			return;
		World* world = findWorld();
		if (world == NULL)
		{
			Edge::setPrimitive(i, p);
			return;
		}
		if (getPrimitive(i) != NULL)
			world->onJointPrimitiveNulling(this, getPrimitive(i));
		Edge::setPrimitive(i, p);
		if (p != NULL)
			world->onJointPrimitiveSet(this, p);
	}

} // namespace RBX
