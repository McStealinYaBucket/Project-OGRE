/* V8World/Primitive.h — October 2013 (physics primitive shell).
   Evidence: Primitive C2 0x58F2AA (F:\2013Decompile\exports\decomp\
     PrimCtor.txt): IndexedMesh base +8, extents init, NetworkOwner::
     Unassigned at +23/+24, Guid +104, EdgeLists +128/+164, -1 worldIndex
     (+50), Body +60, friction 0.0 (+65), elasticity 0.75 (+66), surfaces
     zeroed +67.., PrimitiveLifetime log; PartInstance+43 holds the Primitive*.
     Defaults friction 0.0 / elasticity 0.75 match March 2016
     (defaultFriction/defaultElasticity inlines).
     GeometryType enum from 2016 Geometry.h (joinAll 0x5AB34C skips MEGACLUSTER=10).
   Data layout: Windows 2013 (see the data block); EdgeList asserts sit on Primitive.h:53-54. */

#pragma once

#include "G3D/Vector3.h"
#include "G3D/CoordinateFrame.h"
#include "Util/NormalId.h"
#include "Util/SystemAddress.h"
#include "Util/SurfaceType.h"
#include "Util/Extents.h"
#include "Util/Face.h"
#include "Util/Guid.h"
#include "Util/IndexedMesh.h"
#include "Util/SpanningNode.h"
#include "Util/PV.h"
#include "G3D/Array.h"
#include "V8World/Edge.h"
#include "rbx/Debug.h"
#include "V8World/IPipelined.h"
#include "V8World/BasicSpatialHashPrimitive.h"
#include "Util/ConcurrencyValidator.h"
#include "V8Kernel/BodyPvSetter.h"
#include "decomp.h"

namespace RBX {

	class Primitive;
	// EdgeList, October 2013: parallel G3D::Array<T,10,32> edges +4 / others +20 (36 B); out-of-line members in v8world/Primitive.cpp.
	class EdgeList
	{
	public:
		explicit EdgeList(Primitive* owner) : owner(owner) {}
		// FUNCTION: PLAYER 0x00755400
		// FUNCTION: STUDIO 0x00894720
		~EdgeList()
		{
			RBXASSERT(edges.size() == 0);
			RBXASSERT(others.size() == 0);
		}
		// FUNCTION: PLAYER 0x00754560
		// FUNCTION: STUDIO 0x00893910
		Edge* getEdge(int i) const
		{
			RBXASSERT(i < size());
			RBXASSERT(edges[i]->otherPrimitive(owner) == others[i]);
			return edges[i]; }
		// FUNCTION: PLAYER 0x0071b6f0
		// FUNCTION: STUDIO 0x007b7170
		Primitive* getOther(int i) const {
			RBXASSERT(i < size());
			RBXASSERT(edges[i]->otherPrimitive(owner) == others[i]);
			return others[i];
		}
		Edge* getNext(const Primitive* p, Edge* e) const;  // Primitive.cpp (iOS 10-29 parameter order)
		void insertEdge(Edge* e);  // Primitive.cpp
		void removeEdge(Edge* e);  // Primitive.cpp
		// Bring-up size query (binary reads sizes directly; not a method there).
		int size() const { return edges.size(); }

	private:
		Primitive* owner;                     // +0
		G3D::Array<Edge*,10,32> edges;        // +4
		G3D::Array<Primitive*,10,32> others;  // +20
	};

	class Joint;
	class RigidJoint;
	class Geometry;
	class Contact;
	class Mechanism;
	class Assembly;
	class World;
	class Body;
	class Clump;

	class IMoving;

	class Primitive
		: public IPipelined
		, public SpanningNode
		, public BodyPvSetter
		, public BasicSpatialHashPrimitive
	{
	public:
		typedef enum {
			GEOMETRY_UNDEFINED = 0,
			GEOMETRY_BALL,
			GEOMETRY_BLOCK,
			GEOMETRY_CYLINDER,
			GEOMETRY_WEDGE,
			GEOMETRY_PRISM,
			GEOMETRY_PYRAMID,
			GEOMETRY_PARALLELRAMP,
			GEOMETRY_RIGHTANGLERAMP,
			GEOMETRY_CORNERWEDGE,
			GEOMETRY_MEGACLUSTER,
			GEOMETRY_SMOOTHCLUSTER,
			GEOMETRY_TRI_MESH
		} GeometryType;

		static float defaultFriction() { return 0.0f; }
		static float defaultElasticity() { return 0.75f; }

		// SizeMultiplier (binary +0x74/29th dword; values observed through
		// PartInstance::setName 0x4441B4: HumanoidRootPart->2, Torso->1,
		// anything else->0; a 4th value 3 exists per the getter switch).
		// Enumerator NAMES unrecovered -- mechanical placeholders.
		typedef enum {
			SIZE_MULTIPLIER_0 = 0,
			SIZE_MULTIPLIER_1 = 1,
			SIZE_MULTIPLIER_2 = 2,
			SIZE_MULTIPLIER_3 = 3
		} SizeMultiplier;

		Primitive(int geometryType = GEOMETRY_BLOCK);
		// D2 deletes the kernel body (see Primitive.cpp C2 note).
		~Primitive();

		// PartInstance-facing live state (EAs in PartInstance.h header
		// comment; offsets: specificGravity +256/friction +260/
		// elasticity +264/anchored +253/dragging +252).
		int getGeometryType() const;	// 0x00753cb0

		// Primitive::getCollideType (called by ContactManager::createContact
		// mac: 0x563ABE).  The binary forwards to its Geometry object (virtual
		// per-kind: Ball 1, Block 2, Poly 3).  Our Primitive carries only
		// the type int, so this mirrors newGeometry's switch exactly:
		// 1->1 (Ball), 4..10->3 (Poly), everything else->2 (Block).
		int getCollideType() const;	// 0x00753d20







		void setFriction(float value);  // 0x00753ff0
		float getFriction() const { return friction; }
		void setElasticity(float value);  // 0x00754030
		float getElasticity() const { return elasticity; }
		void setSpecificGravity(float value) { specificGravity = value; }
		float getSpecificGravity() const { return specificGravity; }
		void setAnchoredProperty(bool value);  // 0x00754cd0
		bool getAnchoredProperty() const { return anchoredProperty; }
		void setDragging(bool value);  // 0x00754cb0
		bool getDragging() const { return dragging; }
		void setPreventCollide(bool value);  // 0x00753fc0
		void setFixed(bool anchored, bool dragging);  // 0x00753f40
		void setEngineType(int value);  int getEngineType() const { return engineType; }  enum EngineType { DYNAMICS_ENGINE, HUMANOID_ENGINE };  // set 0x00753ed0
		bool getPreventCollide() const { return preventCollide; }
		// mac: 0x58FC9E (151B): 0->1, 1->5, 2->10, 3->20, else assert "0" (:120)
		// + return 1.
		int getSizeMultiplier() const;
		// mac: 0x58FD48 (135B): on change, refuse while owned (assert "0" :134
		// on myOwner != NULL), else store + zero the +0x78 slot.
		void setSizeMultiplier(SizeMultiplier value);
		// mac: 0x44B778 (149B): +/-half-size extents around the origin, pushed
		// through the coordinate frame (binary reads the geometry size at
		// +0xEC; partSize is our geometry size).
		Extents getExtentsWorld() const;
		// getConstMechanism 0x590F54 (9B) / getMechanism 0x590F4A: forward to
		// Mechanism::getConstPrimitiveMechanism / getPrimitiveMechanism, which
		// walk the +8 IndexedMesh link (batch190: mesh, mesh, -8 adjust).
		// Unlinked prims answer NULL through the real walk (exact today;
		// the old NULL-tripwire is retired now the walk is ported).
		const Mechanism* getConstMechanism() const;
		Mechanism* getMechanism();
		// Assembly linkage (same tripwire family as the Mechanism pair:
		// getAssembly is read by insert/removePrimitive asserts :857/:891).
		// No assembly linkage exists in-tree; replace with the +8-mesh walk
		// (batch190 EAs) when Assembly lands.
		Assembly* getAssembly();
		void setOwner(IMoving* owner) { myOwner = owner; }
		IMoving* getOwner() const { return myOwner; }

		// mac: 0x5921C0 (31 bytes) is a ONE-LINE forwarder:
		//   mov eax,[this+0ECh]        ; the Geometry* (the polymorphic
		//   mov ecx,[eax]              ;  pointer that getGridCorner and
		//   mov [esp],eax              ;  computeFuzzyExtents also use)
		//   call dword ptr [ecx+48h]    ; vtable slot [18]
		//   movzx eax,al                ; -> Geometry::isGeometryOrthogonal
		// Geometry::isGeometryOrthogonal 0x55AF1E is 10 bytes and is
		// `mov eax,1; ret` -- the base returns TRUE, and each Poly subclass
		// that is not axis-aligned overrides slot [18] (WedgePoly 0x5A6EA0,
		// PrismPoly 0x593DAC, PyramidPoly 0x592940, ParallelRampPoly
		// mac: 0x5886C8, RightAngleRampPoly 0x596B20, CornerWedgePoly 0x571250,
		// MegaClusterPoly 0x592870).  The switch below is the resolved form
		// of that dispatch (the geometry kinds whose override is non-true).
		bool isGeometryOrthogonal() const
		{
			switch (getGeometryType())
			{
			case GEOMETRY_WEDGE:
			case GEOMETRY_PRISM:
			case GEOMETRY_PYRAMID:
			case GEOMETRY_PARALLELRAMP:
			case GEOMETRY_RIGHTANGLERAMP:
			case GEOMETRY_CORNERWEDGE:
				return false;
			default:
				return true;
			}
		}

		// mac: 0x4437AE: NORM_Y/STUDS + NORM_Y_NEG/INLET at construction.
		void setSurfaceType(NormalId normalId, SurfaceType surfaceType);
		SurfaceType getSurfaceType(NormalId normalId) const;

		// Edge adjacency: contacts +0x98, joints +0xbc (data block below; public).

		// mac: 0x5904B0: joints.getEdge(i).
		Edge* getJoint(int i);
		// mac: 0x5921E0 (32B, batch229): cached sort size (computes on dirty).
		unsigned int getSortSize();
		// mac: 0x592200 (341B, batch230): two-largest-dims product x50,
		// floored, x size-multiplier + 1. DELTA (source): binary reads
		// the Geometry object (+236); partSize is our geometry size
		// (same values per the header note). fmaxf->ternary (finite
		// inputs only -- sizes clamp 0..2048), lrint->iRound.
		void calculateSortSize();
		// mac: 0x59084A (101B) / getFirstRigid 0x5908B0 / getNextRigid 0x5908E0
		// (batch223): rigid-joint walk (type math inline, no Body).
		Edge* getFirstRigidAt(Joint* start);
		RigidJoint* getFirstRigid();
		RigidJoint* getNextRigid(RigidJoint* prev);
		// Const twin (binary Primitive::getConstJoint; mirrors getJoint(int)).
		const Joint* getConstJoint(int i) const;
		// mac: 0x5907C4 (29B) / 0x5907E2 (38B, batch218): joints-list head/next
		// (JointStage::onPrimitiveRemoving walks them static-style).
		Joint* getFirstJoint();
		Joint* getNextJoint(Joint* prev);
		// mac: 0x5904F4: contacts.getEdge(i) (bonus EA from edgelist agent).
		Edge* getContact(int i);
		// Pair search used by the broadphase contact arm (addNode 0x569A0B,
		// addContactFromChildren 0x572F47): both call sites pass exactly two
		// stack words, and the mangling carries two Primitive* params, so
		// this is STATIC (p0, p1) -- not a 1-param member (which the
		// mangling would show with one param).  Behavior is identical
		// either way; the static spelling matches the evidence.
		// Returns the contact Edge linking both, or NULL.
		static Contact* getContact(Primitive* p0, Primitive* p1);
		// mac: 0x590808 (29B): first contact Edge in the contacts list, or NULL
		// when empty (binary reads a dedicated count at +136; ours IS the
		// list size -- same value).  Entries are Contacts by construction.
		Contact* getFirstContact();
		// mac: 0x590826 (36B): next Edge after c in the contacts list, or NULL.
		// EdgeList::getNext does the index walk (0x590190, in-tree).
		Contact* getNextContact(Contact* c);
		// mac: 0x590516 / 0x59061A (259B each): STATIC (Edge* e) -- the binary
		// call sites (EdgeStage::onEdgeAdded 0x573518 / onEdgeRemoving
		// mac: 0x573542) pass exactly one stack word (disasm setup wins over the
		// decompiler's phantom second arg); the bodies never touch a `this`
		// prim (p0/p1 come from e+16/e+20). DELTA vs 2016 (non-static
		// member, same name/signature otherwise): static matches the binary.
		static void insertEdge(Edge* e);
		static void removeEdge(Edge* e);
		// mac: 0x592388 (batch242: anchored prims short-circuit false;
		// otherwise consult the assembly, if any).
		bool computeIsGrounded() const;

		// SpanningNode overrides (Primitive is a node: secondary at +8,
		// Itanium offset -8 -- batch227 vtable proof). Public: the binary
		// calls them from SpanningTree/TreeStage code (non-members), so
		// the overrides widen the protected base access.
	public:
		// mac: 0x5920FE (47B, batch238): head joint (or NULL) through the filter.
		virtual SpanningEdge* getFirstSpanningEdge();
		// mac: 0x592160 (batch238): joints-list successor through the filter.
		virtual SpanningEdge* getNextSpanningEdge(SpanningEdge* edge);

	private:
		// mac: 0x5920B2 (batch239): skip non-spanning joints, return edge+32
		// (binary MI secondary) or NULL.
		SpanningEdge* nextSpanningEdgeFromJoint(Joint* start);

	public:

		// mac: 0x58F616 factory (flat switch; 3->Block; no Cylinder class).
		static Geometry* newGeometry(int type);
		// Lib-side destroyer (CRT rule session 72: newGeometry allocates in lib TU).
		static void deleteGeometry(Geometry* g);
		// mac: 0x59181A: CF{normalIdToMatrix3, 0.5*axis*size} (size=partSize
		// alias; single-store propagation TODO).  The binary's 0x5918BA
		// counterpart is the CONST `getFaceInObject(NormalId)`, which is
		// what getFaceInWorld 0x59194A calls; both are the same
		// computation, so this single body serves both (const as of
		// session 105 so getFaceInWorld can call it).
		G3D::CoordinateFrame getFaceCoordInObject(NormalId normalId) const;

		// mac: 0x5918BA (144 bytes, const member, sret).  The three float reads
		// are `movss xmm2,[geom+4] / xmm0,[geom+8] / xmm1,[geom+0Ch]`
		// where geom = *(this+0xEC) -- the same three size floats
		// getGridCorner 0x591E02 reads from the same object (+0 is never
		// touched).  They are scaled by the ADJACENT rodata pair
		// 0xC1E4B8 = -0.5f and 0xC1E4BC = +0.5f, so the Extents built is
		// the part's own box centred on the object origin, and the Face
		// comes straight out of Face::fromExtentsSide -> 
		// Extents::getFaceCorners 0x1FFE9E.
		Face getFaceInObject(NormalId normalId) const;

		// mac: 0x59194A (84 bytes, const member, sret):
		//   Face f = getFaceInObject(n);                    // 0x5918BA
		//   body->updatePV();                               // *(this+0xF0)
		//   return f.toWorldSpace(*(CoordinateFrame*)(body + 0xA4));
		// Body+0xA4 is the body's cached WORLD CoordinateFrame -- the same
		// value getGridCorner 0x591E02 fetches from Body::getPV_Spin_Lock.
		// PARTIAL MODEL: Body is not ported, so the body's cached world
		// CoordinateFrame is taken from the primitive's own frame (the
		// two are the same quantity for a primitive that is not inside
		// a moving assembly).
		Face getFaceInWorld(NormalId normalId) const;
		// mac: 0x5900EC: auto-joints among getJoint()s.
		int countNumAutoJoints() const;
		// mac: 0x590916: nth joints-list match linking (p0=this, p1) (assert text
		// :454/:463 unmapped; loop selection rule + match order exact).
		// Signature: the mangled 0x590916 is getJoint(Primitive*,Primitive*,int)
		// counting `this`, i.e. p0->getJoint(p1, n) -- World calls it that way
		// at 0x5AAD76.
		static Joint* getJoint(Primitive* p0, Primitive* p1, int n);	// static: ContactStage::onEdgeAdded 0x009aa93c pushes p0
		const G3D::Vector3& getSize() const;	// 0x00753d80
		// mac: 0x5911DE (176B): per-component clamp to [0, 2048], then if the
		// volume exceeds 4.096e15, y = floor(1e6 / (x*z)).  Pure (ignores
		// this; non-const member per the mangling).  NaN flows through
		// fminf/fmaxf like minss/maxss (second-operand-wins).
		G3D::Vector3 clipToSafeSize(const G3D::Vector3& size);
		// mac: 0x591124 (186B) PARTIAL: clip + store + serial bump + aux zero.
		// DELTAS: geometry vtbl+16 write (== partSize store here),
		// mass/inertia (Body module), world extents+geometry notifications
		// (no world backref member yet), +0x140 flag (identity TODO).
		void setSize(const G3D::Vector3& value);

		// mac: 0x4419EC/0x447A86: physics-space coordinate frame (setCoordinateFrame
		// writes through here via PartInstance::setPhysics equivalent).
		// 2013: out of line, the body's PV position (0x00755f00 tail-jumps to Body::getPV_Spin_Lock).
		const G3D::CoordinateFrame& getCoordinateFrame() const;
		void setCoordinateFrame(const G3D::CoordinateFrame& value);	// through the body's PV (Primitive.cpp)

		// mac: 0x591E02 / 0x59199E.  Both read the GRID size from *(this+0xEC)
		// (floats at +4/+8/+0xC of that object; the +0 dword is never read --
		// see header note) and Body::getPV_Spin_Lock() from *(this+0xF0) for
		// the body's world CoordinateFrame.  Pure translation shift:
		//   getGridCorner:  out.translation = cf.translation - R*(size/2)
		//   setGridCorner:  out.translation = cf.translation + R*(size/2)
		// The rotation is copied through unchanged.  They are exact inverses.
		// PARTIAL MODEL: we read `partSize` (a Vector3 member) where the binary
		// dereferences the +0xEC pointer; the arithmetic is verbatim.
		G3D::CoordinateFrame getGridCorner() const;
		void setGridCorner(const G3D::CoordinateFrame& value);

		// ---- Spatial-hash state (broadphase membership) ----
		// Binary +52..+79 (see SpatialHashPrimitive.h): minCell/maxCell/level.
		// Model-relative placement here (our Primitive is smaller than the
		// binary's 0x150); the template reaches it through these accessors,
		// mirroring BasicSpatialHashPrimitive::getSpatialNodeLevel().
		// Level starts at -1 (asserted on insert, :585).
		const Vector3int32& getSpatialMinCell() const { return spatialExtents.low; }
		const Vector3int32& getSpatialMaxCell() const { return spatialExtents.high; }
		void setSpatialCells(const Vector3int32& minCell, const Vector3int32& maxCell, int level)
		{
			spatialExtents.low = minCell;
			spatialExtents.high = maxCell;
			spatialNodeLevel = level;
		}

		// ---- Fuzzy extents (the auto-joint broadphase input) ----
		// mac: 0x58FEE0 computeFuzzyExtents: asserts
		//   !Math::isNanInfVector3(getBody()->getPos())   (Primitive.cpp:169)
		// then Extents::fromCenterCorner(bodyPos, geometryHalfExtents), and
		// pads by -0.01f on `low` and +0.01f on `high` (all six lanes; the
		// constants are 0x3D4CCCCD/0x3C83126F).
		// mac: 0x59003A getFastFuzzyExtents: the cached wrapper.  It calls
		// Body::updatePV(), compares the cached dword at +0xE8 against
		// Body+0xA0, recomputes into the 24-byte Extents at +0xD0 on a miss,
		// re-runs updatePV() and re-reads the serial; then returns this+0xD0.
		// Returns a reference into the object, so the result is only valid
		// until the next mutation -- exactly as in the binary.
		Extents computeFuzzyExtents();
		const Extents& getFastFuzzyExtents();
		// Bring-up model for Body+0xA0 (`bodyUpdateSerial`): the binary
		// derives it inside Body::updatePV(), which needs the physics Body
		// (not ported).  Here it is bumped by every mutation that would
		// invalidate the cached fuzzy extents, which preserves the observable
		// caching behaviour of getFastFuzzyExtents.
		unsigned int getBodyUpdateSerial() const;	// the body's state index (Body+0xa0)

		// ---- World membership (insert/removePrimitive 0x5AB4E0/0x5AB856) ----
		// Upper lattice (Clump/Assembly/Mechanism walks): the binary
		// links child+8 (the SpanningNode-base IndexedMesh, whose
		// parent socket IS the spanning parent -- rigid children climb
		// through it, cf. TreeStage :215). The `mesh` member below is
		// vestigial (never linked; kept for layout stability).
		IndexedMesh* getMesh() {
			return static_cast<IndexedMesh*>(static_cast<SpanningNode*>(this));
		}
		// PV hand-out for the velocity accessors (PartInstance 0x441A7C+).
		PV* getPV();
		const PV* getPV() const;
		const IndexedMesh* getMesh() const {
			return static_cast<const IndexedMesh*>(static_cast<const SpanningNode*>(this));
		}
		// Binary slot +4 (putInPipeline/removeFromPipeline/putInStage/
		// removeFromStage maintain it; see IPipelined.h). inPipeline() ==
		// (currentStage != NULL) retires the bool flag (section 116; World
		// :858/:876/:892 verified).
		bool inPipeline() const { return IPipelined::inPipeline(); }
		int getWorldIndex() const { return worldIndex; }
		World* getWorld() const { return world; }
		// World's IndexArray key (out of line in World.obj, PLAYER 0x00745580: lea eax,[ecx+0xe0]).
		int& worldIndexFunc() { return worldIndex; }
		// Writers: World insert/remove own world/worldIndex; the stage owns
		// the inPipeline entry/exit effect (CleanStage).
		void setWorldIndex(int value) { worldIndex = value; }
		void setWorld(World* value) { world = value; }
		// Edge counts for the :860/:889 asserts (contacts + joints).
		int getNumEdges() const { return joints.size() + contacts.size(); }
		// List counts (JointStage :116/:141 read the raw sizes; same sums).
		int getNumContacts() const { return contacts.size(); }
		int getNumJoints() const { return joints.size(); }
		// First edge for the :859/:888 asserts. Fresh prims have none;
		// order between the lists is unmapped (TODO) -- joints-first here.
		Edge* getFirstEdge()
		{
			if (joints.size() > 0)
				return joints.getEdge(0);
			if (contacts.size() > 0)
				return contacts.getEdge(0);
			return NULL;
		}
		// Kernel body (binary +240; 2016 Primitive C2 `body(new Body())`).
		// MODEL-RELATIVE slot (full Primitive layout audit is Next #2):
		// behavior is exact (real Body with SimBody), address moves to
		// +240 when the audit lands.
		Body* getBody() { return body; }
		const Body* getConstBody() const { return body; }
		// Network owner address (binary +92; batch284: getNetworkOwner
		// reads it with a :1615 non-null-prim assert). C2 Unassigned
		// (deterministic; binary leaves fresh-heap garbage, which no
		// green path may depend on -- loud DELTA).
		const SystemAddress& getNetworkOwner() const { return networkOwner; }
		// ios: 0x00753da0; JointK's compute function (C2 stores &computeJointK at +0x168).
		float computeJointK();
		// mac: 0x5914E2 (batch257: *(Body+56) canThrottle via prim+240).
		bool getCanThrottle() const;
		// mac: 0x590F36 (5B tail-jump, batch260: forwards to
		// Assembly::getPrimitiveAssembly -- the decompiler's second
		// arg is phantom; disasm shows a bare jmp with (this) only).
		const Assembly* getConstAssembly() const;
		// Clump group lookup (Clump::getPrimitiveClump single-hop;
		// member per the getAssembly-pair pattern). NULL-safe
		// (unlinked meshes walk NULL) -- updatePrimitiveState 0x4456C0
		// relies on exactly this shape for its :749 assert.
		Clump* getClump();
		const Clump* getConstClump() const;
		// Batch261: change-gated network-sleep flag (+100) with owner
		// notify (see the slot note at the definition -- TODO-verify
		// with PartInstance owners; unreachable in-tree today).
		void setNetworkIsSleeping(bool value);
		// Batch261: assembly-root-or-unassembled prims take zero body
		// velocity (Body::setVelocity with Velocity::zero()).
		void zeroVelocity();
		// mac: 0x591D58 (batch266: PV snapshot under the body spinlock,
		// velocity swapped in, setPV stores). DEF deferred: needs setPV
		// (batch267 body banked -- needs Body::setPv + notifyMovedFromExternal
		// + onAssemblyExtentsChanged + ticklePrimitive).
		void setVelocity(const Velocity& vel);
		// 2016-verbatim one-liner (dragging || anchoredProperty); the
		// binary onAssemblyAdded :36 assert ("!requestFixed()") reads
		// the same two members (prim+252/+253).
		bool requestFixed() const { return dragging || anchoredProperty; }
		// Binary +124 store spelled as a mutator (no binary accessor;
		// World::onPrimitiveCollided owns the only writer).
		void bumpCollideCount() { collideCount = (collideCount & 0xF0) | ((collideCount + 1) & 0x0F); }
		// Bring-up view of the collide count (no binary getter recovered).
		unsigned char getCollideCount() const { return collideCount; }
		// mac: 0x58FE46 (153B): assert !world (:152), Guid::assign at +104,
		// flag +112.  Identity of +112 TODO (set on sync).
		void setGuid(const Guid& value);
		// Bring-up view of the +104 guid (no binary getter recovered).
		const Guid& getGuid() const;  // 0x00753bb0 (assert "guidSetExternally", Primitive.cpp:144)
		// Contact-override elasticity (+316, C2 0.0) + dirty flag (+320,
		// C2 false). Read by Contact::onPrimitiveContactParametersChanged
		// (0x55E06E, batch248): flag set only through the custom-physical-
		// properties path (unported); the callback triple (+324/+328/+332)
		// queues with it. Fresh prims take the else arms (0.0 defaults).

		// ---- Data, in the 2013 Windows layout (Primitive C2 0x00756a40, D2 0x00755540; operator new
		// 0x170 at 0x0074aecb). Bases: IPipelined +0, SpanningNode +8, BodyPvSetter and
		// BasicSpatialHashPrimitive both at +0x34 (RTTI BCA 0x00c250b4). MSVC keeps declaration order
		// across access labels, so the public lists sit in sequence here. Bring-up members that 2013
		// does not have (it keeps them in Geometry and Body) follow after +0x170.
	private:
		ConcurrencyValidator concurrencyValidator;  // 0x50 (D2 ~ConcurrencyValidator(+0x50))
		SystemAddress networkOwner;  // 0x74 (C2 NetworkOwner::Unassigned())
		bool m_unk0x7c;              // 0x7c (C2 false)
		Guid guid;                   // 0x80 (getGuid 0x00753bb0 returns this+0x80)
		bool guidSetExternally;      // 0x88 (getGuid assert "guidSetExternally")
		SizeMultiplier sizeMultiplier;  // 0x8c (getSizeMultiplier 0x00753ac0)
		unsigned int sortSize;       // 0x90 (getSortSize 0x007553e0; 0 = dirty)
		unsigned char collideCount : 4;  // 0x94 (C2 `and byte ptr [esi+0x94],0xf0`)
	public:
		EdgeList contacts;           // 0x98
		EdgeList joints;             // 0xbc (getConstJoint 0x00754780)
	private:
		int worldIndex;              // 0xe0 (C2 -1)
		World* world;                // 0xe4 (setSizeMultiplier, setFixed read it)
		Extents fuzzyExtents;        // 0xe8 (C2 maxFinite / -maxFinite)
		unsigned int fuzzyExtentsStateId;  // 0x100 (C2 -2)
		Geometry* geometry;          // 0x104 (C2 newGeometry 0x00755970; getRadius slot 6)
		Body* body;                  // 0x108 (C2 new Body, 0xec bytes)
		IMoving* myOwner;            // 0x10c (setOwner 0x00753f10)
		int engineType;              // 0x110 (setEngineType 0x00753ed0)
		bool dragging;               // 0x114 (setDragging 0x00754cb0: setFixed(+0x115, value))
		bool anchoredProperty;       // 0x115
		bool preventCollide;         // 0x116 (setPreventCollide 0x00753fc0)
		bool networkIsSleeping;      // 0x117 (2016 order; not touched by C2)
		float specificGravity;       // 0x118 (setter 0x007542d0; 2016 order)
		float friction;              // 0x11c (setFriction 0x00753ff0; C2 0)
		float elasticity;            // 0x120 (setElasticity 0x00754030; C2 0.75)
		SurfaceType surfaces[6];     // 0x124 (setter 0x007542a0)
		void* surfaceData[6];        // 0x13c (D2 deletes each; setSurfaceData 0x00754170)
		undefined4 m_unk0x154;       // 0x154
		undefined4 m_unk0x158;       // 0x158
		bool jointKDirty;            // 0x15c (C2 true; setMassInertia sets it)
		Primitive* jointKOwner;      // 0x160 (C2 this)
		float jointK;                // 0x164
		float (Primitive::*jointKCompute)();  // 0x168 (C2 &computeJointK; 8 bytes under MI)

	};

} // namespace RBX
