/* App/v8world/Joint.cpp -- October 2013 (joint base bodies). Laid out on the 2013 assert lines:
   Joint(p0,p1,c0,c1) :59-60, Joint() :77-78, ~Joint :83, getConstJoint :120, setJointOwner :133, the
   isHeavierThan Guid checks :318-321. FLog::JointLifetime is PLAYER 0x00de3f56 (initializer 0x00a65480,
   registered from v8datamodel/FastLogSettings.cpp in 2013). */

#include "stdafx.h"

#include "V8World/Joint.h"
#include "V8World/Primitive.h"
#include "V8World/World.h"
#include "V8World/IMoving.h"
#include "Util/Math.h"
#include "Util/Face.h"
#include "Util/FastFlags.h"
#include "rbx/Debug.h"
#include "FastLog.h"

LOGGROUP(JointLifetime)
using G3D::CoordinateFrame;	// "jointCoord0 == CoordinateFrame()" (:77/:78)
namespace RBX {





















	// FUNCTION: PLAYER 0x008215c0
	// FUNCTION: STUDIO 0x00914c90
	Joint::Joint(Primitive* p0, Primitive* p1,
		const G3D::CoordinateFrame& c0, const G3D::CoordinateFrame& c1)
		: Edge(p0, p1)
		, jointOwner(NULL)
	{
		FASTLOG3(FLog::JointLifetime, "Joint %p created, prim0: %p, prim1: %p", this, p0, p1);
		if (p0 && p1 && (!p0->isGeometryOrthogonal() || !p1->isGeometryOrthogonal()))
		{
			jointCoord0 = c0;
			jointCoord1 = c1;
		}
		else
		{
			jointCoord0 = Math::snapToGrid(c0, 0.1f);
			jointCoord1 = Math::snapToGrid(c1, 0.1f);
			RBXASSERT(Math::isOrthonormal(jointCoord0.rotation));
			RBXASSERT(Math::isOrthonormal(jointCoord1.rotation));
		}
	}







	// STUB: PLAYER 0x00821410
	// STUB: STUDIO 0x00914ae0
	Joint::Joint()
		: Edge(NULL, NULL)
		, jointOwner(NULL)
	{
		FASTLOG1(FLog::JointLifetime, "Joint %p created, empty primitives", this);
		RBXASSERT(jointCoord0 == CoordinateFrame());
		RBXASSERT(jointCoord1 == CoordinateFrame());
	}
	// FUNCTION: PLAYER 0x00821010
	// FUNCTION: STUDIO 0x009146e0
	Joint::~Joint() {
		RBXASSERT(!jointOwner);
		FASTLOG1(FLog::JointLifetime, "Joint %p destroyed", this);
	}















	// FUNCTION: PLAYER 0x00820c80
	// FUNCTION: STUDIO 0x00914330
	const Joint* Joint::findConstJoint(const Primitive* p, JointType type)
	{
		int num = p->getNumJoints();
		for (int i = 0; i < num; ++i) {
			const Joint* joint = p->getConstJoint(i);
			if (joint->getJointType() == type) {
				return joint;
			}
		}
		return NULL;
	}

	// FUNCTION: PLAYER 0x00820cc0
	// FUNCTION: STUDIO 0x00914370
	const Joint* Joint::getConstJoint(const Primitive* p, JointType type)
	{
		const Joint* answer = findConstJoint(p, type);
		RBXASSERT(answer);
		return answer;
	}
	// FUNCTION: PLAYER 0x00820d20
	// FUNCTION: STUDIO 0x009143d0
	Joint* Joint::getJoint(Primitive* p, JointType type)
	{
		return const_cast<Joint*>(getConstJoint(p, type));
	}
	// FUNCTION: PLAYER 0x0081fae0
	// FUNCTION: STUDIO 0x00913180
	void Joint::setJointOwner(IJointOwner* value)
	{
		RBXASSERT((value == NULL) != (jointOwner == NULL));
		jointOwner = value;
	}
	// FUNCTION: PLAYER 0x00501530
	// FUNCTION: STUDIO 0x009131f0
	IJointOwner* Joint::getJointOwner() const { return jointOwner; }
	// FUNCTION: PLAYER 0x0081fd10
	// FUNCTION: STUDIO 0x009133c0
	SpanningNode* Joint::getNode(int i)
	{
		return getPrimitive(i);


	}

	const SpanningNode* Joint::getConstNode(int i) const
	{
		return const_cast<Joint*>(this)->getPrimitive(i);  // folded with getNode in 2013 (both slots 0x0081fd10)



	}










	// STUB: PLAYER 0x0081faa0
	// STUB: STUDIO 0x00913140
	void Joint::notifyMoved()
	{
		for (int i = 0; i < 2; ++i)
		{
			Primitive* prim = (i == 0) ? getPrim0() : getPrim1();
			IMoving* mover = (prim != NULL) ? prim->getOwner() : NULL;
			if (mover != NULL)
				mover->notifyMoved();
		}
	}

	// STUB: PLAYER 0x00820fb0
	// STUB: STUDIO 0x00914660
	bool Joint::FacesOverlapped(const Primitive* p0, unsigned long n0,
		const Primitive* p1, unsigned long n1, float tolerance)
	{
		(void)p0; (void)n0; (void)p1; (void)n1; (void)tolerance;
		return false;
	}

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








	static unsigned int getPrimitiveSize2(const Primitive* p)
	{
		return const_cast<Primitive*>(p)->getSortSize();
	}

	unsigned int getJointSize2(const Joint* j)
	{
		unsigned int answer = getPrimitiveSize2(j->getPrim0());
		if (j->getPrim1()) {
			answer = std::max(answer, getPrimitiveSize2(j->getPrim1()));
		}
		return answer;
	}

	// FUNCTION: PLAYER 0x00820e70
	// FUNCTION: STUDIO 0x00914520
	static int biggerJointSize2(const Joint* j0, const Joint* j1)
	{
		unsigned int s0 = getJointSize2(j0);
		unsigned int s1 = getJointSize2(j1);
		if (s0 > s1) {
			return 1;
		}
		else if (s1 > s0) {
			return -1;
		}
		else {
			return 0;
		}
	}

	// FUNCTION: PLAYER 0x0081fb50
	// FUNCTION: STUDIO 0x00913200
	static int biggerJointGuid2(const Joint* j0, const Joint* j1)
	{
		const Guid* j00 = &j0->getPrim0()->getGuid();
		const Guid* j10 = &j1->getPrim0()->getGuid();

		const Guid* j01 = j0->getPrim1() ? &j0->getPrim1()->getGuid() : NULL;
		const Guid* j11 = j1->getPrim1() ? &j1->getPrim1()->getGuid() : NULL;

		int answer = Guid::compare(j00, j01, j10, j11);
		RBXASSERT(answer == Guid::compare(j01, j00, j10, j11));
		RBXASSERT(answer == Guid::compare(j00, j01, j11, j10));
		RBXASSERT(answer == -Guid::compare(j11, j10, j00, j01));
		RBXASSERT(answer == -Guid::compare(j10, j11, j01, j00));
		return answer;
	}

	// FUNCTION: PLAYER 0x00820ef0
	// FUNCTION: STUDIO 0x009145a0
	bool Joint::isHeavierThan(const SpanningEdge* other) const
	{
		const Joint* const j0 = this;
		const Joint* j1 = rbx_static_cast<const Joint*>(other);

		if (j0 == j1) {
			return false;
		}
		else {
			Joint::JointType jt0 = (Joint::JointType)j0->getJointType();
			Joint::JointType jt1 = (Joint::JointType)j1->getJointType();
			if (jt0 != jt1) {
				return (jt0 < jt1);
			}
			else {
				int i = biggerJointSize2(j0, j1);
				if (i > 0) {
					return true;
				}
				else if (i < 0) {
					return false;
				}
				else {
					i = biggerJointGuid2(j0, j1);
					if (i > 0) {
						return true;
					}
					else if (i < 0) {
						return false;
					}
					else {
						return (j0 < j1);
					}
				}
			}
		}
	}

	// mac: 0x578BBC (40B, batch226): the far-end node. Binary spells the
	// prim-lattice conversion as raw +8/-8 MI arithmetic; static_cast is
	// the MSVC-correct equivalent (never raw offsets across the MI).
	// FUNCTION: PLAYER 0x00820f80
	// FUNCTION: STUDIO 0x00914630
	SpanningNode* Joint::otherNode(SpanningNode* node)
	{
		return otherPrimitive(rbx_static_cast<Primitive*>(node));






	}

	// mac: 0x578C0C (batch226): const mirror with a parallel body. The 2013 linker
	// folded it with otherNode: both SpanningEdge slots +0x08/+0x0c hold 0x00820f80.
	const SpanningNode* Joint::otherConstNode(const SpanningNode* node) const
	{
		return otherPrimitive(rbx_static_cast<const Primitive*>(node));  // folded with otherNode (0x00820f80)






	}

	// mac: 0x57823A (176B, batch219a): FastLog-gated world-notify relink.
	// DELTA (return): binary returns a byte no in-tree caller uses; void
	// here (overrides Edge::setPrimitive -- see its DELTA).
	// FUNCTION: PLAYER 0x00820d30
	// FUNCTION: STUDIO 0x009143e0
	void Joint::setPrimitive(int i, Primitive* p)
	{
		FASTLOG3(FLog::JointLifetime, "Joint %p, setting primitive %u to %p", this, i, p);
		if (p != getPrimitive(i)) {
			World* world = findWorld();
			if (world && getPrimitive(i)) {
				world->onJointPrimitiveNulling(this, getPrimitive(i));
			}
			Edge::setPrimitive(i, p);
			if (world && p) {
				world->onJointPrimitiveSet(this, p);
			}
		}
	}

} // namespace RBX

// EnumDesc<T> ctors, 2013 bytes (registration order, values and legacy names); they sit
// at the end of the file so no 2013 assert line above them moves.
#include "reflection/EnumConverter.h"

namespace RBX {
namespace Reflection {

// FUNCTION: PLAYER 0x00821b30
// FUNCTION: STUDIO 0x00915200
template<>
EnumDesc<Joint::JointType>::EnumDesc()
	: EnumDescriptor("JointType", typeid(Joint::JointType))
{
	addPair(Joint::NO_JOINT, "None");
	addPair(Joint::ROTATE_JOINT, "Rotate");
	addPair(Joint::ROTATE_P_JOINT, "RotateP");
	addPair(Joint::ROTATE_V_JOINT, "RotateV");
	addPair(Joint::GLUE_JOINT, "Glue");
	addPair(Joint::WELD_JOINT, "Weld");
	addPair(Joint::SNAP_JOINT, "Snap");
}

template<>
const Type& Type::getSingleton<Joint::JointType>()
{
	return EnumDesc<Joint::JointType>::singleton();
}

template<> EnumRegistrar<Joint::JointType> EnumRegistrar<Joint::JointType>::registrar(0);
template<> TypeRegistrar<Joint::JointType> TypeRegistrar<Joint::JointType>::registrar(0);

} // namespace Reflection
} // namespace RBX
