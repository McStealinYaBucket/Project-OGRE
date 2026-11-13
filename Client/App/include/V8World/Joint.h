/* V8World/Joint.h -- October 2013 (physics joint base, 152B).
   Evidence: Joint C2(p0,p1,c0,c1) 0x577924 (Edge base + hooks zeroed + owner
     NULL + coord rule) / C2(void) 0x577C98 (identity asserts :77-78) /
     D2 0x577EF0 (:83 !owner + lifetime log) / getJointOwner 0x57822E (+52) /
     setJointOwner 0x5781A0 (:133 null-xor assert) / notifyMoved 0x57808E
     (prim+244 myOwner IMoving ping) / isBreakable 0x507080 (false) /
     getEdgeType 0x485F20 (0 = JOINT) / 24-slot primary vtable off_EEE638 /
     SpanningEdge secondary +32 (Thn32) / sizes new(0x98) (Weld/Snap),
     MultiJoint 0xD0/RotateP-V 0xDC/Glue 0x100 (follow-ups).
   JointType enum (13, matches 2016 order; base asserts "0" Joint.h:106).
   DELTAS (all marked): step/moving hooks skipped (types unmapped);
     isGeometryOrthogonal mapped by kind (TODO-verify 2013 set);
     getJointType/isAutoJoint/canJoin/builders with concrete types (follow-up);
     setPrimitive slot-4 override with JointInstance integration (follow-up);
     FastLog JointLifetime skipped (channel absent).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/Joint.h (path inferred) */

#pragma once

#include "G3D/CoordinateFrame.h"
#include "Util/NormalId.h"
#include "Util/SpanningEdge.h"
#include "V8World/Edge.h"

namespace RBX {

	class Primitive;
	class IMoving;
	class JointInstance;
	class Link;

	// Carried from March 2016 v8world/Joint.h (first-party; order verified
	// against all 13 binary getJointType bodies). Named (no object) so the
	// header stays definition-free across TUs.

	class Joint : public Edge, public SpanningEdge
	{
	public:
		enum JointType {
			ANCHOR_JOINT,
			WELD_JOINT,
			MANUAL_WELD_JOINT,
			SNAP_JOINT,
			MOTOR_1D_JOINT,
			MOTOR_6D_JOINT,
			ROTATE_JOINT,
			ROTATE_P_JOINT,
			ROTATE_V_JOINT,
			GLUE_JOINT,
			MANUAL_GLUE_JOINT,
			FREE_JOINT,
			KERNEL_JOINT,
			NO_JOINT
		};
		Joint();
		Joint(Primitive* p0, Primitive* p1,
			const G3D::CoordinateFrame& c0, const G3D::CoordinateFrame& c1);
		virtual ~Joint();

		// mac: 0x57822E / 0x5781A0.
		JointInstance* getJointOwner() const { return jointOwner; }
		void setJointOwner(JointInstance* owner);
		// mac: 0x57823A (176B, batch218): FastLog-gated world-notify relink.
		// Overrides Edge::setPrimitive (void here -- binary returns a byte
		// no in-tree caller uses; see Edge.h DELTA).
		virtual void setPrimitive(int i, Primitive* p);
		// mac: 0x57808E (0x5780AD interior): ping both prims' myOwner movers.
		void notifyMoved();
		// mac: 0x485F20: 0 (Edge::JOINT).
		// STUB: PLAYER 0x0096bde0
		virtual int getEdgeType() const { return JOINT; }
		// 2016 shape (inlined at every use in 2013 -- no standalone EA):
		// poor-man's Joint check behind Primitive::insertEdge's routing.
		static bool isJoint(const Edge* e) { return e->getEdgeType() == Edge::JOINT; }

		// clang-format off
		// Predicate family (2016 header-inline shapes; binary names confirmed
		// via EdgeBuffer assert strings :149/:150/:81. DELTA: type ranges from
		// 2016 (order verified) -- Asserts-gated uses only in Release builds.
		static int getJointType(const Edge* e) {
			return isJoint(e) ? rbx_static_cast<const Joint*>(e)->getJointType() : (int)NO_JOINT;
		}
		static bool isGroundJoint(const Edge* e) {
			int jt = getJointType(e);
			return jt == FREE_JOINT || jt == ANCHOR_JOINT;
		}
		static bool isRigidJoint(const Edge* e) {
			int jt = getJointType(e);
			return jt == WELD_JOINT || jt == SNAP_JOINT || jt == MANUAL_WELD_JOINT;
		}
		static bool isKinematicJoint(const Edge* e) {
			int jt = getJointType(e);
			return jt >= WELD_JOINT && jt <= MOTOR_6D_JOINT;
		}
		// ---- Joint's own virtuals, in 2013 vftable order (PLAYER 0x00b7252c,
		// STUDIO the same 18 slots; iOS 10-29 names; 2016 declaration order).
		// +0x18 getJointType: base asserts "0" at Joint.h:106 (PLAYER 0x008210d0); concrete types override.
		// +0x1c isBreakable: false (Glue overrides true); PLAYER 0x00927cc0 `xor al,al; ret`, folded.
		// +0x20 isBroken: Joint itself never breaks; World::doBreakJoints dispatches this slot.
		// +0x24 joinsFace: PLAYER 0x00927840 (`xor al,al; ret 8`, folded). +0x28 isAligned: 0x006be550, folded.
		// +0x2c align: asserts "0" at Joint.h:111 and returns an identity frame (ret 0xc).
		// The 2013 assert strings (both targets) pin getJointType to line 106 and align to
		// line 111, so these six are packed onto lines 106-111.
		// FUNCTION: PLAYER 0x008210d0
		// FUNCTION: STUDIO 0x009147a0
		virtual int getJointType() const { RBXASSERT(0); return NO_JOINT; }
		virtual bool isBreakable() const { return false; }  virtual bool isBroken() const { return false; }
		virtual bool joinsFace(Primitive* p, NormalId faceId) const { return false; }  virtual bool isAligned() { return true; }
		// FUNCTION: PLAYER 0x006d5230
		// FUNCTION: STUDIO 0x007a0650
		virtual G3D::CoordinateFrame align(Primitive* pMove, Primitive* pStay) { RBXASSERT(0); return G3D::CoordinateFrame(); }
		// clang-format on
		// vtable+0x30. PLAYER 0x00528720 (empty, folded).
		virtual void setPhysics() {}
		// vtable+0x34/+0x38. mac: 0x57A260 / 0x575C40 (7B each, batch231):
		// base answers false (stepper families override).
		// STUB: PLAYER 0x00927cc0
		// STUB: STUDIO 0x008db930
		virtual bool canStepWorld() const { return false; }
		// STUB: PLAYER 0x00927cc0
		// STUB: STUDIO 0x008db930
		virtual bool canStepUi() const { return false; }
		// vtable+0x3c. PLAYER 0x00528720 (empty, folded).
		virtual void stepWorld() {}
		// vtable+0x40. PLAYER 0x00927840 (`xor al,al; ret 8`, folded).
		virtual bool stepUi(double distributedGameTime) { return false; }
		static bool isSpringJoint(const Edge* e) {
			int jt = getJointType(e);
			return (jt >= ROTATE_JOINT && jt <= GLUE_JOINT) || jt == MANUAL_GLUE_JOINT;
		}
		static bool isMotorJoint(const Edge* e) {
			int jt = getJointType(e);
			return jt >= MOTOR_1D_JOINT && jt <= MOTOR_6D_JOINT;
		}
		static bool isKernelJoint(const Edge* e) {
			return getJointType(e) == KERNEL_JOINT;
		}
		static bool isManualJoint(const Edge* e) {
			int jt = getJointType(e);
			return jt == MANUAL_WELD_JOINT || jt == MANUAL_GLUE_JOINT;
		}
		// mac: 0x5780CE (70B, batch218): first joint of type on prim (NULL if none).
		static const Joint* findConstJoint(const Primitive* p, JointType type);
		// mac: 0x578114 (129B, batch219a): find + :120 "answer" assert.
		static const Joint* getConstJoint(const Primitive* p, JointType type);
		// mac: 0x578196 (9B, batch219a): non-const twin.
		static Joint* getJoint(Primitive* p, JointType type);
		// Auto-joint filter (uses [5]=getEdgeType, [7]=getJointType).
		bool isAutoJoint() const;
		// mac: 0x4103C8 (115B, batch216): spanning-tree membership -- the auto
		// families (types 1..10) plus anchor/free. Static per the mangling
		// (Edge const* only, no this).
		static bool isSpanningTreeJoint(const Edge* e);
		// mac: 0x578BBC (40B, batch226) / 0x578C0C (const mirror): the far-end
		// node (raw +8/-8 arithmetic on the prim lattice -- binary MI
		// layout; SpanningNode forward-declared, no members touched).
		// Virtual per the secondary-vtable thunks (binary slot differs by
		// ABI; dispatch is by name here).
		virtual SpanningNode* otherNode(SpanningNode* node);
		virtual const SpanningNode* otherConstNode(const SpanningNode* node) const;
		// mac: 0x578C5C / 0x578C8C (batch238: prim+8 node or NULL).
		virtual SpanningNode* getNode(int i);
		virtual const SpanningNode* getConstNode(int i) const;
		// mac: 0x5788AA (749B, batch226): weight order (type, size, guid,
		// address). Virtual per the secondary-vtable thunks (binary slot
		// differs by ABI; dispatch is by name here). The SpanningEdge*
		// back-adjusts -32 to the sibling joint (binary MI layout).
		virtual bool isHeavierThan(const SpanningEdge* other) const;
		// Header-inline in binary (0 links-hits; body from Primitive::getJoint
		// use sites + "links(p0, p1)"/"!links(p0, p1)" asserts :454/:463).
		bool links(const Primitive* p0, const Primitive* p1) const
		{
			const Primitive* a0 = getPrim0();
			const Primitive* a1 = getPrim1();
			return (a0 == p0 && a1 == p1) || (a0 == p1 && a1 == p0);
		}
		const G3D::CoordinateFrame& getJointCoord0() const { return jointCoord0; }
		const G3D::CoordinateFrame& getJointCoord1() const { return jointCoord1; }
		// vtable+0x44. mac: 0x3D73A8 (batch271: base asserts !"Not Implemented"
		// (Joint.h:183) and returns NULL; motor joints override).
		// FUNCTION: PLAYER 0x006d5280
		// FUNCTION: STUDIO 0x007a0570
		virtual Link* resetLink() { RBXASSERT(!"Not Implemented"); return NULL; }


		// ---- The shared auto-joint overlap test (all three T/J sections) ----
		// mac: 0x57878E FacesOverlapped(const Primitive*, unsigned long, const
		// Primitive*, unsigned long, float) -- NON-virtual STATIC (the
		// mangled name carries no `this`), and the face ids are unsigned
		// long, not NormalId.  It forwards to Poly::FacesOverlapped
		// (0x58A72C) / Ball::FacesOverlapped (0x592A1C) for the
		// non-orthogonal geometry kinds; those Geometry subclasses are not
		// ported, so the body is a documented no-op returning false.
		static bool FacesOverlapped(const Primitive* p0, unsigned long n0,
			const Primitive* p1, unsigned long n1, float tolerance);

		// mac: 0x57840E canBuildJoint(p0,p1,n0,n1,fuzzyAngleTolerance,planeTolerance).
		// Both tolerance arguments are checked by the callers below: the
		// 5th goes to Math::fuzzyAxisAligned, the 6th to
		// Face::overlapWithinPlanes.  Returns bool.
		static bool canBuildJoint(Primitive* p0, Primitive* p1, NormalId n0, NormalId n1,
			float fuzzyAngleTolerance, float planeTolerance);

		// mac: 0x5787F8 canBuildJointTight: `mov [esp+14h], 3C23D70Ah` /
		// `mov [esp+10h], 3C23D70Ah`  ->  both tolerances are 0.01f.
		static bool canBuildJointTight(Primitive* p0, Primitive* p1, NormalId n0, NormalId n1)
		{
			return canBuildJoint(p0, p1, n0, n1, 0.01f, 0.01f);
		}
		// mac: 0x578836 canBuildJointLoose: 3D4CCCCDh twice -> 0.05f / 0.05f.
		static bool canBuildJointLoose(Primitive* p0, Primitive* p1, NormalId n0, NormalId n1)
		{
			return canBuildJoint(p0, p1, n0, n1, 0.05f, 0.05f);
		}

	private:
		// StepJointsStageHook + MovingAssemblyStageHook skipped (types unmapped).
		JointInstance* jointOwner;             // +52 (NULL init)
		G3D::CoordinateFrame jointCoord0;      // +56 (48B)
		G3D::CoordinateFrame jointCoord1;      // +104 (48B)

	};

	// mac: 0x578875 (53B, batch228): max sort size across both prims.
	// Free function per the mangling (not a member).
	unsigned int getJointSize2(const Joint* joint);

	class ManualWeldJoint : public Joint
	{
	public:
		ManualWeldJoint()
			: Joint()
		{}
		ManualWeldJoint(Primitive* p0, Primitive* p1,
			const G3D::CoordinateFrame& c0, const G3D::CoordinateFrame& c1)
			: Joint(p0, p1, c0, c1)
		{}

		virtual int getJointType() const { return MANUAL_WELD_JOINT; }
	};

	// mac: 0x575CA0 / 0x575B7C (112B each, batch218): single-prim joints,
	// Joint(prim, NULL, identity, identity). DELTA: binary C2s also set the
	// +32 secondary (SpanningEdge) vptr -- MI secondary unmapped in-tree
	// (same precedent as Assembly/Mechanism secondaries).
	class AnchorJoint : public Joint
	{
	public:
		explicit AnchorJoint(Primitive* prim);

		virtual int getJointType() const { return ANCHOR_JOINT; }
		static bool isAnchorJoint(const Joint* j) {
			return j->getJointType() == ANCHOR_JOINT;
		}
	};

	class FreeJoint : public Joint
	{
	public:
		explicit FreeJoint(Primitive* prim);

		virtual int getJointType() const { return FREE_JOINT; }
		static bool isFreeJoint(const Joint* j) {
			return j->getJointType() == FREE_JOINT;
		}
	};

} // namespace RBX
