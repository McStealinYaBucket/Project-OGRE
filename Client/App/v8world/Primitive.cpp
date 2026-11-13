/* App/v8world/Primitive.cpp — October 2013 (ctor shell).
   Evidence: Primitive C2 0x58F2AA (defaults friction 0.0 / elasticity 0.75 /
     specificGravity 0.0 / unanchored / undragged).
   TODO: full decompile. */

#include "stdafx.h"

#include "V8World/Primitive.h"
#include "V8World/Assembly.h"
#include "V8World/Clump.h"
#include "V8World/Mechanism.h"
#include "V8World/Geometry.h"
#include "V8World/Joint.h"
#include "V8World/RigidJoint.h"
#include "V8World/Contact.h"
#include "V8World/IMoving.h"
#include "V8Kernel/Body.h"
#include "Util/Velocity.h"
#include "Util/SpanningEdge.h"
#include "Util/Math.h"
#include <cmath>
#include <limits>

namespace RBX {

	// STUB: PLAYER 0x00756a40
	// STUB: STUDIO 0x00895d10
	Primitive::Primitive(int geometryType)
		: geometryType(geometryType)
		, specificGravity(0.0f)
		, friction(defaultFriction())
		, elasticity(defaultElasticity())
		, anchoredProperty(false)
		, dragging(false)
		, preventCollide(false)
		, networkIsSleeping(false)
		, myOwner(NULL)
		, worldIndex(-1)
		, world(NULL)
		, guidSynced(false)
		, contacts(this)
		, joints(this)
		, partSize(4.0f, 1.0f, 2.0f)
		// Binary C2 defaults unrecovered for +0x74/+0x78; 0 matches the
		// setName default arm (anything-but-HRP/Torso -> 0) and the aux is
		// only ever zeroed, so 0/0 is the consistent choice.
		, sizeMultiplier(SIZE_MULTIPLIER_0)
		, sizeMultiplierAux(0)
		, sortSize(0)
		// The fuzzy-extents cache (binary +0xD0/+0xE8) must start DIRTY:
		// getFastFuzzyExtents only recomputes when the cached serial differs
		// from the body serial, so seed the cache with a value the body serial
		// cannot take.
		, bodyUpdateSerial(0)
		, cachedFuzzyExtentsSerial(1)
		, contactElasticity316(0.0f)
		, contactParamsDirty320(false)
		// Kernel body (binary +240; 2016 `body(new Body())`). D2 below
		// deletes it (2016 D2 shape; the !world/geometry asserts need
		// members not yet modeled -- queued with the full C2/D2).
		, body(new Body())
		, collideCount(0)
		, networkOwner()
	{
		for (int i = 0; i < 6; ++i)
			surfaces[i] = NO_SURFACE;
	}

	// D2: body teardown (see C2 note).
	Primitive::~Primitive()
	{
		delete body;
	}

	// mac: 0x5914E2 (batch257).
	// STUB: PLAYER 0x00753e10
	// STUB: STUDIO 0x008931c0
	bool Primitive::getCanThrottle() const
	{
		return body->getCanThrottle();
	}

	// Const mirror (2016 shape; symmetric to getAssembly).
	const Assembly* Primitive::getConstAssembly() const
	{
		return Assembly::getConstPrimitiveAssembly(this);
	}

	// Clump group lookup (mirrors the getAssembly pair).
	Clump* Primitive::getClump()
	{
		return Clump::getPrimitiveClump(this);
	}

	// Const mirror.
	const Clump* Primitive::getConstClump() const
	{
		return Clump::getConstPrimitiveClump(this);
	}

	// Batch261: change-gated flag (+100) + owner notify. NOTE SLOT:
	// the binary calls owner vtbl+12 with (owner) only; our IMoving
	// declares onNetworkIsSleepingChanged at MSVC +20, so the callee
	// may differ -- TODO-verify with PartInstance owners (unreachable
	// in-tree: test prims have NULL owners).
	void Primitive::setNetworkIsSleeping(bool value)
	{
		if (value != networkIsSleeping)
		{
			networkIsSleeping = value;
			myOwner->onNetworkIsSleepingChanged();
		}
	}

	// Batch261: assembly-root-or-unassembled prims take zero velocity.
	void Primitive::zeroVelocity()
	{
		Assembly* assembly = getAssembly();
		bool assemblyRoot = (assembly != NULL) && (assembly->getAssemblyPrimitive() == this);
		if (assemblyRoot || assembly == NULL)
			getBody()->setVelocity(Velocity::zero());
	}

	void Primitive::setSurfaceType(NormalId normalId, SurfaceType surfaceType)
	{
		if (normalId >= 0 && normalId < 6)
			surfaces[normalId] = surfaceType;
	}

	int Primitive::countNumAutoJoints() const
	{
		int total = 0;
		for (int i = 0; i < joints.size(); ++i)
		{
			const Joint* j = static_cast<const Joint*>(joints.getEdge(i));
			if (j->isAutoJoint())
				++total;
		}
		return total;
	}

	// EdgeList::getNext: out of line here as in 2013 (Primitive.cpp:226-227).
	// mac: 0x590190.
	// STUB: PLAYER 0x007546b0
	// STUB: STUDIO 0x008939e0
	Edge* EdgeList::getNext(Edge* e, Primitive* p) const
	{
		RBXASSERT(e != NULL);
		RBXASSERT(e->links(p));
		int idx = e->getIndex(p) + 1;
		return idx < edges.size() ? edges[idx] : NULL;
	}

	// mac: 0x590916. p0 is `this`; p1 arrives as the argument.
	// STUB: PLAYER 0x00754960
	// STUB: STUDIO 0x00893c90
	Joint* Primitive::getJoint(Primitive* p1, int n)
	{
		// Fewer-edged prim list; nth links(p0,p1) match (NULL if exhausted).
		Primitive* p0 = this;
		Primitive* least = (p0->joints.size() <= p1->joints.size()) ? p0 : p1;
		int found = 0;
		for (int i = 0; i < least->joints.size(); ++i)
		{
			Edge* e = least->joints.getEdge(i);
			if (e->links(p0) && e->links(p1) && found++ == n)
				return static_cast<Joint*>(e);
		}
		return NULL;
	}

	// mac: 0x5904B0.
	Edge* Primitive::getJoint(int i)
	{
		return joints.getEdge(i);
	}

	// mac: 0x59084A (101B, batch223): first joints-list edge whose type is
	// WELD(1)/UNIVERSAL(5)/MANUAL_WELD(2), i.e. (t&~4)==1 || t==2 with
	// t = edgeType ? 13 : jointType. Static per the mangling.
	Edge* Primitive::getFirstRigidAt(Joint* start)
	{
		Edge* edge = start;
		if (edge == NULL)
			return NULL;
		while (true)
		{
			int type = 13;
			if (edge->getEdgeType() == Edge::JOINT)
				type = rbx_static_cast<Joint*>(edge)->getJointType();
			if ((type & ~4) == 1 || type == 2)
				break;
			edge = joints.getNext(edge, this);
			if (edge == NULL)
				return NULL;
		}
		return edge;
	}

	// mac: 0x5908B0 (batch223).
	RigidJoint* Primitive::getFirstRigid()
	{
		return static_cast<RigidJoint*>(getFirstRigidAt(getFirstJoint()));
	}

	// mac: 0x5908E0 (batch223).
	RigidJoint* Primitive::getNextRigid(RigidJoint* prev)
	{
		return static_cast<RigidJoint*>(getFirstRigidAt(getNextJoint(prev)));
	}

	// Const twin of getJoint(int) (binary Primitive::getConstJoint).
	const Joint* Primitive::getConstJoint(int i) const
	{
		return static_cast<const Joint*>(joints.getEdge(i));
	}

	// mac: 0x5907C4 (29B, batch218).
	Joint* Primitive::getFirstJoint()
	{
		if (joints.size() <= 0)
			return NULL;
		return static_cast<Joint*>(joints.getEdge(0));
	}

	// mac: 0x5907E2 (38B, batch218): EdgeList::getNext over the joints list.
	Joint* Primitive::getNextJoint(Joint* prev)
	{
		return static_cast<Joint*>(joints.getNext(prev, this));
	}

	// EdgeList's out-of-line insert/remove (2013 Primitive.cpp:236-251; getNext is above, for
	// PLAYER address order).
	// mac: 0x590356 (Primitive.cpp:250 "removeIndex >= 0" / :251
	// "edges[removeIndex] == e"; unconditional swap-hole on both arrays).
	// STUB: PLAYER 0x00756040
	// STUB: STUDIO 0x00895360
	void EdgeList::removeEdge(Edge* e)
	{
		int idx = e->getIndex(owner);
		RBXASSERT(idx >= 0);  // Primitive.cpp:250 "removeIndex >= 0"
		RBXASSERT(edges[idx] == e);  // Primitive.cpp:251 "edges[removeIndex] == e"
		int last = edges.size() - 1;
		edges[idx] = edges[last];
		others[idx] = others[last];
		edges[idx]->setIndex(owner, idx);
		edges.resize(last);
		others.resize(last);
		e->setIndex(owner, -1);
	}

	// mac: 0x59027E (Primitive.cpp:236 "e->getIndex(owner) == -1").
	// STUB: PLAYER 0x007563f0
	// STUB: STUDIO 0x00895710
	void EdgeList::insertEdge(Edge* e)
	{
		RBXASSERT(e->getIndex(owner) == -1);  // Primitive.cpp:236
		e->setIndex(owner, edges.size());
		edges.append(e);
		others.append(e->otherPrimitive(owner));
	}

	// mac: 0x590516 (259B). Static-style: p0 = e+16, p1 = e+20 (getPrim0/1
	// inlines); getEdgeType (vtbl+20) routes foreign edges to both prims'
	// contacts lists (+128) and real joints to the joints lists (+164).
	// Single-prim joints (p1 NULL) take the :301 Anchor/Free assert arm.
	// STUB: PLAYER 0x007564b0
	// STUB: STUDIO 0x008957d0
	void Primitive::insertEdge(Edge* e)
	{
		Primitive* p0 = e->getPrim0();
		Primitive* p1 = e->getPrim1();
		if (!Joint::isJoint(e))
		{
			p0->contacts.insertEdge(e);
			p1->contacts.insertEdge(e);
		}
		else
		{
			p0->joints.insertEdge(e);
			if (p1 != NULL)
				p1->joints.insertEdge(e);
			else
				// Primitive.cpp:301, binary-verbatim (classes landed 118).
				RBXASSERT(AnchorJoint::isAnchorJoint(rbx_static_cast<Joint*>(e))
					|| FreeJoint::isFreeJoint(rbx_static_cast<Joint*>(e)));  // :301
		}
	}

	// mac: 0x59061A (259B). Mirror of insertEdge (:326 assert arm).
	// STUB: PLAYER 0x00756160
	// STUB: STUDIO 0x00895480
	void Primitive::removeEdge(Edge* e)
	{
		Primitive* p0 = e->getPrim0();
		Primitive* p1 = e->getPrim1();
		if (!Joint::isJoint(e))
		{
			p0->contacts.removeEdge(e);
			p1->contacts.removeEdge(e);
		}
		else
		{
			p0->joints.removeEdge(e);
			if (p1 != NULL)
				p1->joints.removeEdge(e);
			else
				// Primitive.cpp:326, binary-verbatim (classes landed 118).
				RBXASSERT(AnchorJoint::isAnchorJoint(rbx_static_cast<Joint*>(e))
					|| FreeJoint::isFreeJoint(rbx_static_cast<Joint*>(e)));  // :326
		}
	}

	// mac: 0x5921E0 (32B, batch229).
	// STUB: PLAYER 0x007553e0
	// STUB: STUDIO 0x00894700
	unsigned int Primitive::getSortSize()
	{
		if (sortSize == 0)
			calculateSortSize();
		return sortSize;
	}

	// mac: 0x592200 (341B, batch230).
	// STUB: PLAYER 0x00754e40
	// STUB: STUDIO 0x00894170
	void Primitive::calculateSortSize()
	{
		float v2 = partSize.x;
		float v3 = partSize.y;
		float v4 = partSize.z;
		float planar;
		if (v4 <= v3)
			planar = v3 * (v2 > v4 ? v2 : v4);
		else
			planar = v4 * (v2 > v3 ? v2 : v3);
		RBXASSERT(planar * 1000.0f + 1 < (float)std::numeric_limits<unsigned int>::max());  // :1010
		RBXASSERT(planar > 0.0f);  // :1011
		// lrint -> G3D::iRound (VS2012 has no lrintf; integral input).
		int result = G3D::iRound(floorf(planar * 50.0f)) * getSizeMultiplier() + 1;
		sortSize = (unsigned int)result;
	}

	// mac: 0x58FC9E (151B).
	// STUB: PLAYER 0x00753ac0
	// STUB: STUDIO 0x00892e70
	int Primitive::getSizeMultiplier() const
	{
		switch (sizeMultiplier)
		{
		case SIZE_MULTIPLIER_0: return 1;
		case SIZE_MULTIPLIER_1: return 5;
		case SIZE_MULTIPLIER_2: return 10;
		case SIZE_MULTIPLIER_3: return 20;
		default: break;
		}
		RBXASSERT(false);  // Primitive.cpp:120 "0"
		return 1;
	}

	// mac: 0x5911DE (176B).  minss/maxss spelled as ternaries: `x<y?x:y` matches
	// minss in every case including NaN (unordered falls to y, i.e. the
	// bound -- verified against the SSE truth table).
	G3D::Vector3 Primitive::clipToSafeSize(const G3D::Vector3& size)
	{
		float cx = size.x < 2048.0f ? size.x : 2048.0f;
		float cy = size.y < 2048.0f ? size.y : 2048.0f;
		float cz = size.z < 2048.0f ? size.z : 2048.0f;
		G3D::Vector3 out(
			cx > 0.0f ? cx : 0.0f,
			cy > 0.0f ? cy : 0.0f,
			cz > 0.0f ? cz : 0.0f);
		if (out.x * out.y * out.z > 4.096e15f)
			out.y = floorf(1000000.0f / (out.x * out.z));
		return out;
	}

	// mac: 0x591124 (186B) PARTIAL (see header).
	void Primitive::setSize(const G3D::Vector3& value)
	{
		G3D::Vector3 clipped = clipToSafeSize(value);
		if (clipped != partSize)
		{
			partSize = clipped;
			++bodyUpdateSerial;
			sizeMultiplierAux = 0;
		}
	}

	// mac: 0x58FE46 (153B).  Guid::assign spelled via copyDataFrom (extract +
	// assign -- same bytes, no pointer surgery).
	// STUB: PLAYER 0x00753c10
	// STUB: STUDIO 0x00892fc0
	void Primitive::setGuid(const Guid& value)
	{
		RBXASSERT(world == NULL);  // Primitive.cpp:152 "!world"
		guid.copyDataFrom(value);
		guidSynced = true;
	}

	// getMechanism 0x590F4A / getConstMechanism 0x590F54: forward to the
	// Mechanism statics (real walks; NULL for unlinked prims).
	Mechanism* Primitive::getMechanism()
	{
		return Mechanism::getPrimitiveMechanism(this);
	}

	const Mechanism* Primitive::getConstMechanism() const
	{
		return Mechanism::getConstPrimitiveMechanism(this);
	}

	// getAssembly: forwards to the Assembly static (mirrors the getMechanism
	// shape). 0x590F36 (5B tail-jump, batch260: bare jmp to
	// Assembly::getPrimitiveAssembly -- the decompiler's second arg is
	// phantom; disasm shows (this) only).
	Assembly* Primitive::getAssembly()
	{
		return Assembly::getPrimitiveAssembly(this);
	}

	// mac: 0x58FD48 (135B).
	// STUB: PLAYER 0x00753b40
	// STUB: STUDIO 0x00892ef0
	void Primitive::setSizeMultiplier(SizeMultiplier value)
	{
		if (sizeMultiplier != value)
		{
			// Primitive.cpp:134 "0" -- refuse while owned.
			RBXASSERT(myOwner == NULL);
			if (myOwner == NULL)
			{
				sizeMultiplier = value;
				sizeMultiplierAux = 0;
			}
		}
	}

	// mac: 0x44B778 (149B).
	// STUB: PLAYER 0x0057fd90
	// STUB: STUDIO 0x00621f50
	Extents Primitive::getExtentsWorld() const
	{
		G3D::Vector3 half = partSize * 0.5f;
		Extents local(-half, half);
		return local.toWorldSpace(getCoordinateFrame());
	}

	// Pair search over the contacts list (see header note for the static
	// signature evidence).  All entries are Contacts by construction.
	Contact* Primitive::getContact(Primitive* p0, Primitive* p1)
	{
		for (int i = 0; i < p0->contacts.size(); ++i)
		{
			Edge* edge = p0->contacts.getEdge(i);
			if (edge->links(p0) && edge->links(p1))
				return static_cast<Contact*>(edge);
		}
		return NULL;
	}

	// mac: 0x590808.
	Contact* Primitive::getFirstContact()
	{
		if (contacts.size() <= 0)
			return NULL;
		return static_cast<Contact*>(contacts.getEdge(0));
	}

	// mac: 0x590826.
	Contact* Primitive::getNextContact(Contact* c)
	{
		return static_cast<Contact*>(contacts.getNext(c, this));
	}
	Geometry* Primitive::newGeometry(int type)
	{
		switch (type)
		{
		case 1: return new BallGeometry();
		case 4: return new WedgePolyGeometry();
		case 5: return new PrismPolyGeometry();
		case 6: return new PyramidPolyGeometry();
		case 7: return new ParallelRampPolyGeometry();
		case 8: return new RightAngleRampPolyGeometry();
		case 9: return new CornerWedgePolyGeometry();
		case 10: return new MegaClusterPolyGeometry();
		default: return new BlockGeometry();
		}
	}
	void Primitive::deleteGeometry(Geometry* g)
	{
		delete g;
	}
	G3D::CoordinateFrame Primitive::getFaceCoordInObject(NormalId normalId) const
	{
		G3D::Vector3 axis = normalIdToVector3(normalId);
		G3D::Vector3 half = G3D::Vector3(0.5f * axis.x * partSize.x,
			0.5f * axis.y * partSize.y, 0.5f * axis.z * partSize.z);
		return G3D::CoordinateFrame(normalIdToMatrix3(normalId), half);
	}

	// mac: 0x5918BA -- the object-space face quad.  See the header note for the
	// -0.5f/+0.5f rodata pair and for why the three size floats live on
	// the geometry object rather than on the primitive.
	// STUB: PLAYER 0x00754070
	// STUB: STUDIO 0x00893420
	Face Primitive::getFaceInObject(NormalId normalId) const
	{
		Extents objectExtents(partSize * -0.5f, partSize * 0.5f);
		return Face::fromExtentsSide(objectExtents, normalId);
	}

	// mac: 0x59194A -- object-space face lifted into world space.  The binary
	// reads the world frame out of the Body at this+0xF0 (offset 0xA4)
	// after refreshing it; the primitive's own coordinate frame is the
	// same quantity in this model.
	// STUB: PLAYER 0x00755d40
	// STUB: STUDIO 0x00895060
	Face Primitive::getFaceInWorld(NormalId normalId) const
	{
		return getFaceInObject(normalId).toWorldSpace(getCoordinateFrame());
	}

	Edge* Primitive::getContact(int i)
	{
		return contacts.getEdge(i);
	}

	SurfaceType Primitive::getSurfaceType(NormalId normalId) const
	{
		if (normalId >= 0 && normalId < 6)
			return surfaces[normalId];
		return NO_SURFACE;
	}

	// mac: 0x591E02 -- the body's world CoordinateFrame with its translation
	// pushed back by half the grid size along the rotated axes.  The 3x3 is
	// copied verbatim (G3D::Matrix3 copy ctor 0x759B0C) and each output
	// component is  sum_i cf.translation[i] + sum_j R[i][j]*half[j], negated.
	G3D::CoordinateFrame Primitive::getGridCorner() const
	{
		const G3D::CoordinateFrame& cf = getCoordinateFrame();
		const G3D::Vector3 half = getSize() * 0.5f;
		G3D::CoordinateFrame result;
		result.rotation = cf.rotation;
		result.translation = cf.translation
			- cf.rotation.column(0) * half.x
			- cf.rotation.column(1) * half.y
			- cf.rotation.column(2) * half.z;
		return result;
	}

	// mac: 0x58FEE0 -- Primitive.cpp:169 assert text recovered verbatim:
	//   "!Math::isNanInfVector3(getBody()->getPos())"
	// The binary then calls Body::updatePV() twice around the reads, takes
	// the centre from Body+0xC8 (getPos()) and the half-corner from the
	// geometry pointer at this+0xEC through vtable slot [23] (+0x5C), passing
	// Body+0xA4 as the argument, and pads the result.
	// PARTIAL MODEL: Body and Geometry are not ported, so the centre is the
	// primitive's own coordinate-frame translation and the half-corner is
	// size/2.  For the box primitives we model those are the same values
	// (Body+0xC8 is the body world position, which is the frame translation,
	// and the box geometry's fuzzy half extents are size/2); the padding,
	// the extents construction and the assert are verbatim.
	// STUB: PLAYER 0x007557c0
	// STUB: STUDIO 0x00894ae0
	Extents Primitive::computeFuzzyExtents()
	{
		RBXASSERT(!Math::isNanInfVector3(getCoordinateFrame().translation));
		Extents result = Extents::fromCenterCorner(
			getCoordinateFrame().translation, getSize() * 0.5f);
		G3D::Vector3 pad(0.01f, 0.01f, 0.01f);
		result.min() -= pad;
		result.max() += pad;
		return result;
	}

	// mac: 0x59003A -- the cached front end used by JointBuilder::canJoin (0x57B164)
	// and the broadphase.  Order of operations in the disasm:
	//   Body::updatePV();
	//   if (cached /*+0xE8*/ != body->serial /*+0xA0*/) {
	//       fuzzyExtents = computeFuzzyExtents();   // 24-byte copy to +0xD0
	//       Body::updatePV();
	//       cached = body->serial;
	//   }
	//   return &this->fuzzyExtents;                 // this + 0xD0
	// STUB: PLAYER 0x007558e0
	// STUB: STUDIO 0x00894c00
	const Extents& Primitive::getFastFuzzyExtents()
	{
		if (cachedFuzzyExtentsSerial != bodyUpdateSerial)
		{
			fuzzyExtents = computeFuzzyExtents();
			cachedFuzzyExtentsSerial = bodyUpdateSerial;
		}
		return fuzzyExtents;
	}

	// mac: 0x59199E -- exact inverse of getGridCorner(); ends in
	// Primitive::setCoordinateFrame 0x591AD2.
	void Primitive::setGridCorner(const G3D::CoordinateFrame& value)
	{
		const G3D::Vector3 half = getSize() * 0.5f;
		G3D::CoordinateFrame frame;
		frame.rotation = value.rotation;
		frame.translation = value.translation
			+ value.rotation.column(0) * half.x
			+ value.rotation.column(1) * half.y
			+ value.rotation.column(2) * half.z;
		setCoordinateFrame(frame);
	}

	// mac: 0x5920FE (47B, batch238): head joint (or NULL) through the filter.
	SpanningEdge* Primitive::getFirstSpanningEdge()
	{
		Joint* first = joints.size() > 0
			? static_cast<Joint*>(joints.getEdge(0))
			: NULL;
		return nextSpanningEdgeFromJoint(first);
	}

	// mac: 0x592160 (batch238): joints-list successor through the filter
	// (binary back-adjusts edge-32 to the sibling joint -- our MI
	// downcast spells the same object).
	SpanningEdge* Primitive::getNextSpanningEdge(SpanningEdge* edge)
	{
		Joint* joint = (edge == NULL) ? NULL : static_cast<Joint*>(edge);
		Edge* next = (joint == NULL)
			? NULL
			: joints.getNext(joint, this);
		return nextSpanningEdgeFromJoint(static_cast<Joint*>(next));
	}

	// mac: 0x5920B2 (batch239): skip non-spanning joints, return the
	// SpanningEdge secondary (binary +32; static_cast here) or NULL.
	SpanningEdge* Primitive::nextSpanningEdgeFromJoint(Joint* start)
	{
		for (Edge* edge = start; edge != NULL; edge = joints.getNext(edge, this))
		{
			if (Joint::isSpanningTreeJoint(edge))
				return static_cast<SpanningEdge*>(static_cast<Joint*>(edge));
		}
		return NULL;
	}

	// mac: 0x592388 (batch242).
	bool Primitive::computeIsGrounded() const
	{
		if (!anchoredProperty)
		{
			const Assembly* assembly = Assembly::getConstPrimitiveAssembly(this);
			if (assembly != NULL)
				return assembly->computeIsGrounded();
		}
		return false;
	}

} // namespace RBX
