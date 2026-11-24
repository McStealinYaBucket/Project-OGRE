/* App/v8world/Primitive.cpp -- October 2013.  Asserts at the 2013 lines; size 0x170 (World ctor). */

#include "stdafx.h"

#include "V8World/Primitive.h"
#include "V8World/World.h"
#include "V8World/Assembly.h"
#include "V8World/Clump.h"
#include "V8World/Mechanism.h"
#include "V8World/Geometry.h"
#include "V8World/Joint.h"
#include "V8World/RigidJoint.h"
#include "V8World/Contact.h"
#include "V8World/IMoving.h"
#include "V8Kernel/Body.h"
#include "Network/NetworkOwner.h"
#include "V8Kernel/Constants.h"
#include "Util/Velocity.h"
#include "Util/SpanningEdge.h"
#include "Util/Math.h"
#include <cmath>
#include <limits>
#include "FastLog.h"

namespace FLog {
	// GLOBAL: PLAYER 0x00de3f55
	// GLOBAL: STUDIO 0x00f84e2d
	Channel PrimitiveLifetime = 0;
	int PrimitiveLifetimeIniter = RegisterLogGroup("PrimitiveLifetime", &PrimitiveLifetime, NULL, FASTVARTYPE_STATIC);
}

namespace RBX {

	// STUB: PLAYER 0x00756a40
	// STUB: STUDIO 0x00895d10
	Primitive::Primitive(int geometryType)
		: networkOwner(Network::NetworkOwner::Unassigned())
		, m_unk0x7c(false)
		, guidSetExternally(false)
		, sizeMultiplier(SIZE_MULTIPLIER_0)
		, sortSize(0)
		, collideCount(0)
		, contacts(this)
		, joints(this)
		, worldIndex(-1)
		, world(NULL)
		, fuzzyExtentsStateId(static_cast<unsigned int>(-2))
		, geometry(newGeometry(geometryType))
		, body(new Body())
		, myOwner(NULL)
		, engineType(0)
		, dragging(false)
		, anchoredProperty(false)
		, preventCollide(false)
		, friction(defaultFriction())
		, elasticity(defaultElasticity())
		, jointK(this, &Primitive::computeJointK)
	{
		FASTLOG1(FLog::PrimitiveLifetime, "Primitive %p created", this);
		for (int i = 0; i < 6; ++i) {
			surfaces[i] = NO_SURFACE;
			surfaceData[i] = NULL;
		}
	}





	// The 2013 dtor asserts the primitive left its world and still owns its geometry and body,
	// then releases them and the six surface records (PLAYER 0x00755540).
	// FUNCTION: PLAYER 0x00755540
	// FUNCTION: STUDIO 0x00894860
	Primitive::~Primitive()
	{
		RBXASSERT(!world);
		RBXASSERT(geometry);
		RBXASSERT(body);

		delete geometry;
		delete body;
		geometry = NULL;
		body = NULL;
		for (int i = 0; i < 6; ++i)
		{
			delete surfaceData[i];
			surfaces[i] = NO_SURFACE;
			surfaceData[i] = NULL;
		}

		FASTLOG1(FLog::PrimitiveLifetime, "Primitive %p destroyed", this);
	}

	// FUNCTION: PLAYER 0x00753e10
	// FUNCTION: STUDIO 0x008931c0
	bool Primitive::getCanThrottle() const
	{
		return body->getCanThrottle();
	}

	const Assembly* Primitive::getConstAssembly() const
	{
		return Assembly::getConstPrimitiveAssembly(this);
	}

	Clump* Primitive::getClump()
	{
		return Clump::getPrimitiveClump(this);
	}

	const Clump* Primitive::getConstClump() const
	{
		return Clump::getConstPrimitiveClump(this);
	}

	void Primitive::setNetworkIsSleeping(bool value)
	{
		if (value != networkIsSleeping)
		{
			networkIsSleeping = value;
			myOwner->onNetworkIsSleepingChanged();
		}
	}

	void Primitive::zeroVelocity()
	{
		Assembly* assembly = getAssembly();
		bool assemblyRoot = (assembly != NULL) && (assembly->getAssemblyPrimitive() == this);
		if (assemblyRoot || assembly == NULL)
			getBody()->setVelocity(Velocity::zero());
	}

	// FUNCTION: PLAYER 0x007542a0
	void Primitive::setSurfaceType(NormalId normalId, SurfaceType surfaceType)
	{
		if (surfaces[normalId] != surfaceType) {
			surfaces[normalId] = surfaceType;
		}
	}

	// FUNCTION: PLAYER 0x00753bb0
	const Guid& Primitive::getGuid() const
	{
		RBXASSERT(guidSetExternally);
		return guid;
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

	// mac: 0x590916. p0 is `this`; p1 arrives as the argument.
	// STUB: PLAYER 0x00754960
	// STUB: STUDIO 0x00893c90
	Joint* Primitive::getJoint(Primitive* p0, Primitive* p1, int n)
	{
		// Fewer-edged prim list; nth links(p0,p1) match (NULL if exhausted).
		// (static in 2013: p0 is pushed like p1)
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

	Edge* Primitive::getJoint(int i)
	{
		return joints.getEdge(i);
	}

	// FUNCTION: PLAYER 0x007548b0
	Edge* Primitive::getFirstRigidAt(Joint* start)
	{
		for (Edge* e = start; e != NULL; e = joints.getNext(this, e)) {
			if (Joint::isRigidJoint(e)) {
				return e;
			}
		}
		return NULL;
	}

	// STUB: PLAYER 0x00754910
	RigidJoint* Primitive::getFirstRigid()
	{
		return static_cast<RigidJoint*>(getFirstRigidAt(getFirstJoint()));
	}

	// FUNCTION: PLAYER 0x00754940
	RigidJoint* Primitive::getNextRigid(RigidJoint* prev)
	{
		return static_cast<RigidJoint*>(getFirstRigidAt(getNextJoint(prev)));
	}

	const Joint* Primitive::getConstJoint(int i) const
	{
		return static_cast<const Joint*>(joints.getEdge(i));
	}

	Joint* Primitive::getFirstJoint()
	{
		if (joints.size() <= 0)
			return NULL;
		return static_cast<Joint*>(joints.getEdge(0));
	}

	Joint* Primitive::getNextJoint(Joint* prev)
	{
		return static_cast<Joint*>(joints.getNext(this, prev));
	}
	// FUNCTION: PLAYER 0x007546b0
	// FUNCTION: STUDIO 0x008939e0
	Edge* EdgeList::getNext(const Primitive* p, Edge* e) const
	{
		RBXASSERT(e);
		RBXASSERT(e->links(p));
		int idx = e->getIndex(p) + 1;
		return idx < edges.size() ? edges[idx] : NULL;
	}
	// mac: 0x59027E.
	// FUNCTION: PLAYER 0x007563f0
	// FUNCTION: STUDIO 0x00895710
	void EdgeList::insertEdge(Edge* e)
	{
		RBXASSERT(e->getIndex(owner) == -1);
		e->setIndex(owner, edges.size());
		edges.append(e);
		others.append(e->otherPrimitive(owner));
	}



	// mac: 0x590356: unconditional swap-hole on both arrays.
	// STUB: PLAYER 0x00756040
	// STUB: STUDIO 0x00895360
	void EdgeList::removeEdge(Edge* e)
	{
		int removeIndex = e->getIndex(owner);
		RBXASSERT(removeIndex >= 0);
		RBXASSERT(edges[removeIndex] == e);
		int last = edges.size() - 1;
		edges[removeIndex] = edges[last];
		others[removeIndex] = others[last];
		edges[removeIndex]->setIndex(owner, removeIndex);
		edges.resize(last);
		others.resize(last);
		e->setIndex(owner, -1);
	}
























	// mac: 0x590516 (259B). Static-style: p0 = e+16, p1 = e+20 (getPrim0/1
	// inlines); getEdgeType (vtbl+20) routes foreign edges to both prims'
	// contacts lists (+128) and real joints to the joints lists (+164).
	// Single-prim joints (p1 NULL) take the :301 Anchor/Free assert arm.
	// FUNCTION: PLAYER 0x007564b0
	// FUNCTION: STUDIO 0x008957d0
	void Primitive::insertEdge(Edge* e)
	{
		Primitive* p0 = e->getPrim0();
		Primitive* p1 = e->getPrim1();
		if (Joint::isJoint(e))
		{
			p0->joints.insertEdge(e);
			if (p1 != NULL)
				p1->joints.insertEdge(e);
			else
				RBXASSERT(AnchorJoint::isAnchorJoint(rbx_static_cast<Joint*>(e))
					|| FreeJoint::isFreeJoint(rbx_static_cast<Joint*>(e)));  // :301
		}
		else
		{
			p0->contacts.insertEdge(e);
			p1->contacts.insertEdge(e);
		}

	}


	// mac: 0x59061A (259B). Mirror of insertEdge (:326 assert arm).
	// FUNCTION: PLAYER 0x00756160
	// FUNCTION: STUDIO 0x00895480
	void Primitive::removeEdge(Edge* e)
	{
		Primitive* p0 = e->getPrim0();
		Primitive* p1 = e->getPrim1();
		if (Joint::isJoint(e))
		{
			p0->joints.removeEdge(e);
			if (p1 != NULL)
				p1->joints.removeEdge(e);
			else
				RBXASSERT(AnchorJoint::isAnchorJoint(rbx_static_cast<Joint*>(e))
					|| FreeJoint::isFreeJoint(rbx_static_cast<Joint*>(e)));  // :326
		}
		else
		{
			p0->contacts.removeEdge(e);
			p1->contacts.removeEdge(e);
		}

	}

	// mac: 0x5921E0 (32B, batch229).
	// FUNCTION: PLAYER 0x007553e0
	// FUNCTION: STUDIO 0x00894700
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
		float v2 = getSize().x;
		float v3 = getSize().y;
		float v4 = getSize().z;
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
		if (clipped != getSize())
		{
			geometry->setSizeNonVirtual(clipped);

			sortSize = 0;
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
		guidSetExternally = true;
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
	// FUNCTION: PLAYER 0x00753c90
	// FUNCTION: STUDIO 0x00893040
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
				sortSize = 0;
			}
		}
	}

	// mac: 0x44B778 (149B).
	// STUB: PLAYER 0x0057fd90
	// STUB: STUDIO 0x00621f50
	Extents Primitive::getExtentsWorld() const
	{
		G3D::Vector3 half = getSize() * 0.5f;
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
		return static_cast<Contact*>(contacts.getNext(this, c));
	}
	// STUB: PLAYER 0x00755970
	// STUB: STUDIO 0x00894c90
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
		G3D::Vector3 half = G3D::Vector3(0.5f * axis.x * getSize().x,
			0.5f * axis.y * getSize().y, 0.5f * axis.z * getSize().z);
		return G3D::CoordinateFrame(normalIdToMatrix3(normalId), half);
	}

	// mac: 0x5918BA -- the object-space face quad.  See the header note for the
	// -0.5f/+0.5f rodata pair and for why the three size floats live on
	// the geometry object rather than on the primitive.
	// STUB: PLAYER 0x00754070
	// STUB: STUDIO 0x00893420
	Face Primitive::getFaceInObject(NormalId normalId) const
	{
		Extents objectExtents(getSize() * -0.5f, getSize() * 0.5f);
		return Face::fromExtentsSide(objectExtents, normalId);
	}

	// mac: 0x59194A -- object-space face lifted into world space.  The binary
	// reads the world frame out of the Body at this+0xF0 (offset 0xA4)
	// after refreshing it; the primitive's own coordinate frame is the
	// same quantity in this model.
	// FUNCTION: PLAYER 0x00755d40
	// FUNCTION: STUDIO 0x00895060
	Face Primitive::getFaceInWorld(NormalId normalId) const
	{
		return getFaceInObject(normalId).toWorldSpace(body->getCoordinateFrame());
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








	// FUNCTION: PLAYER 0x00753cb0
	// FUNCTION: STUDIO 0x00893060
	int Primitive::getGeometryType() const
	{
		RBXASSERT(geometry);
		return geometry ? geometry->getGeometryType() : GEOMETRY_UNDEFINED;
	}

	// FUNCTION: PLAYER 0x00753d20
	// FUNCTION: STUDIO 0x008930d0
	int Primitive::getCollideType() const
	{
		RBXASSERT(geometry);
		return geometry->getCollideType();
	}

	// FUNCTION: PLAYER 0x00753d80
	// FUNCTION: STUDIO 0x00893130
	const G3D::Vector3& Primitive::getSize() const
	{
		return geometry->getSize();
	}

	// mac: 0x58FEE0 -- Primitive.cpp:169 "!Math::isNanInfVector3(getBody()->getPos())"; the 2013 body
	// takes the centre from the body and the half-corner from a geometry vftable slot (not modelled).
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
	// FUNCTION: PLAYER 0x007558e0
	// FUNCTION: STUDIO 0x00894c00
	const Extents& Primitive::getFastFuzzyExtents()
	{
		if (fuzzyExtentsStateId != body->getStateIndex())
		{
			fuzzyExtents = computeFuzzyExtents();
			fuzzyExtentsStateId = body->getStateIndex();
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
			: joints.getNext(this, joint);
		return nextSpanningEdgeFromJoint(static_cast<Joint*>(next));
	}

	// mac: 0x5920B2 (batch239): skip non-spanning joints, return the
	// SpanningEdge secondary (binary +32; static_cast here) or NULL.
	SpanningEdge* Primitive::nextSpanningEdgeFromJoint(Joint* start)
	{
		for (Edge* edge = start; edge != NULL; edge = joints.getNext(this, edge))
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

	// ---- 2013 bodies written after the assert-pinned functions (no asserts; lines free) ----
	// FUNCTION: PLAYER 0x00753da0
	// FUNCTION: STUDIO 0x00893150
	float Primitive::computeJointK()
	{
		G3D::Vector3 size = (getGeometryType() == GEOMETRY_MEGACLUSTER) ? G3D::Vector3(4.0f, 4.0f, 4.0f) : getSize();
		return Constants::getJointK(size, getGeometryType() == GEOMETRY_BALL);
	}

	// FUNCTION: PLAYER 0x00753ed0
	// FUNCTION: STUDIO 0x00893280
	void Primitive::setEngineType(int value)
	{
		if (engineType != value) {
			Assembly* a = NULL;
			if (world) {
				a = world->onPrimitiveEngineChanging(this);
			}
			engineType = value;
			if (a) {
				world->onPrimitiveEngineChanged(a);
			}
		}
	}

	// FUNCTION: PLAYER 0x00753f40
	// FUNCTION: STUDIO 0x008932f0
	void Primitive::setFixed(bool newAnchored, bool newDragging)
	{
		bool wasFixed = (anchoredProperty || dragging);
		bool willBeFixed = (newAnchored || newDragging);
		bool changed = (wasFixed != willBeFixed);
		if (changed && world) {
			world->onPrimitiveFixedChanging(this);
		}
		anchoredProperty = newAnchored;
		dragging = newDragging;
		if (changed && world) {
			world->onPrimitiveFixedChanged(this);
		}
	}

	// FUNCTION: PLAYER 0x00753fc0
	// FUNCTION: STUDIO 0x00893370
	void Primitive::setPreventCollide(bool value)
	{
		if (value != preventCollide) {
			preventCollide = value;
			if (world) {
				world->onPrimitivePreventCollideChanged(this);
			}
		}
	}

	// FUNCTION: PLAYER 0x00753ff0
	// FUNCTION: STUDIO 0x008933a0
	void Primitive::setFriction(float value)
	{
		if (value != friction) {
			friction = value;
			if (world) {
				world->onPrimitiveContactParametersChanged(this);
			}
		}
	}

	// FUNCTION: PLAYER 0x00754030
	// FUNCTION: STUDIO 0x008933e0
	void Primitive::setElasticity(float value)
	{
		if (value != elasticity) {
			elasticity = value;
			if (world) {
				world->onPrimitiveContactParametersChanged(this);
			}
		}
	}

	// FUNCTION: PLAYER 0x00754cb0
	// FUNCTION: STUDIO 0x00893fe0
	void Primitive::setDragging(bool value)
	{
		setFixed(anchoredProperty, value);
	}

	// FUNCTION: PLAYER 0x00754cd0
	// FUNCTION: STUDIO 0x00894000
	void Primitive::setAnchoredProperty(bool value)
	{
		setFixed(value, dragging);
	}

	// FUNCTION: PLAYER 0x00755f00
	// FUNCTION: STUDIO 0x00895220
	const G3D::CoordinateFrame& Primitive::getCoordinateFrame() const
	{
		return body->getPV_Spin_Lock()->position;
	}

	// Our accessors over the body (2013 callers read the body directly; not 2013 functions).
	void Primitive::setCoordinateFrame(const G3D::CoordinateFrame& value)
	{
		PV pv = *body->getPV_Spin_Lock();
		pv.position = value;
		body->setPv(pv, *this);
	}
	unsigned int Primitive::getBodyUpdateSerial() const
	{
		return body->getStateIndex();
	}
	PV* Primitive::getPV()
	{
		return body->getPV_Spin_Lock();
	}
	const PV* Primitive::getPV() const
	{
		return body->getPV_Spin_Lock();
	}

} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::Primitive, 0x170);
