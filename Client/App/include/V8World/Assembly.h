/* V8World/Assembly.h -- October 2013 (rigid-assembly shell).
   Evidence: Assembly C2 0x554424 (243B): IPipelined base (vptr + +4 = 0),
     IndexedMesh secondary base at +8 (vtable off_EEDE14, Thn8 batch273
     -- MI, same precedent as Joint/Primitive/TreeStage), plain inits
     (+48/+52 = 0, +56/+57 = 0,
     +60/+64 = 0, Edge/Joint arrays at +68/+84, +100 = 0, +104/+108 = -1,
     +112 = 6, +120 = 1, +124 = this, +128 = &computeAssemblyMaxRadius,
     +132 = 0); getPrimitiveAssembly 0x554C50 (50B) + getConstPrimitiveAssembly
     0x554C82 (50B, symmetric): two computedUpper hops from prim+8 with -8
     owner adjusts, NULL-safe; getPrimitiveAssemblyFast 0x554BC0 asserts the
     intermediate (queued -- assert-first shape for known-linked callers).
   DELTAS: +128 stores NULL here (binary stores &computeAssemblyMaxRadius;
     the method needs Body::getBranchCofmPos + visitPrimitivesImpl + the +40
     linkage -- all queued; nothing in-tree reads the slot).  getAssemblyPrimitive
     pair (+40 linkage), Fast, reset/wakeUp/sampleAndNotMoving/state queue
     behind linkage/Body. onLowersChanged real (0x554CB4, 12B: flag120=1).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/Assembly.h (path inferred) */

#pragma once

#include "V8World/IPipelined.h"
#include "V8World/Enum.h"
#include "Util/IndexedMesh.h"
#include "G3D/Array.h"

namespace RBX {

	class Edge;
	class Joint;
	class Primitive;
	class AssemblyHistory;
	class SimJob;

	class Assembly
		: public IPipelined
		, public IndexedMesh
	{
	public:
		// FilterPhase: 2013 values (binary-pinned: onFixedAssemblyRootAdded
		// passes to=5/from=6; changePhase lists `<= 3` with assemblies[4];
		// moveInto dispatches 0,1->Simulate / 2->SendPhysics /
		// 3->NoSimulate / 4->inert / 5->Fixed / 6->unlisted).
		// 2016 carries two more phases (BufferZone/Anims) that 2013
		// lacks; 0-3 reuse the 2016 names (filterAssembly, which would
		// confirm them, queues with SendPhysics).
		typedef enum { Sim_SendIfSim, Sim_BufferZone, NoSim_Send,
			NoSim_SendIfSim, NUM_PHASES, Fixed, NOT_ASSIGNED } FilterPhase;
	private:
		int unknown04;  // +4 (C2 0).
		// IndexedMesh secondary base at +8 (vtable off_EEDE14, Thn8 thunk
		// batch273 -- MI, same precedent as Joint/Primitive/TreeStage).
		// (No member: the base IS the mesh; getMesh() adjusts.)
		int unknown48;  // +48 (C2 0).
		int unknown52;  // +52 (C2 0).
		bool flag56;  // +56 (C2 false; TODO identity).
		bool flag57;  // +57 (C2 false; TODO identity).
		AssemblyHistory* history;  // +60 (C2 NULL; D2 :36 asserts NULL).
		// mac: 0x554ABE (128B, batch259: returns this word; :94 assert
		// (ANCHORED || stage >= HUMANOID_STAGE)). D2 :37 checks this
		// same word for ANCHORED -- the old unknown64 name was wrong.
		Sim::AssemblyState assemblyState;  // +64 (C2 0 = ANCHORED).
	public:
		Sim::AssemblyState getAssemblyState() const;
		// 2016 shape (SleepStage::changeAssemblyState stores the new
		// state; binary writes +64 directly).
		void setAssemblyState(Sim::AssemblyState value) { assemblyState = value; }
		// Batch260 (0x554A04: assembly-prim network-wake + history wake;
		// history arm dead while +60 stays NULL -- link-covered, runs
		// with staged assemblies in 162+).
		void wakeUp();
		// mac: 0x555420 (batch299: assembly-prim assert + moved fan).
		void notifyMovedFromExternal();
		// Batch257: throttle predicates over the assembly prims
		// (getConstAssemblyPrimitive asserts on fresh assemblies --
		// link-covered, runs with staged assemblies in 162+).
		bool getCanThrottle() const;
		// Batch257 (static in spirit: no this-use; member per mangling).
		// Batch257 (static in spirit -- no this-use; 2016 declares it
		// static and the binary calls it without an instance).
		static bool computeCanThrottle(Edge* edge);
	private:
		G3D::Array<Edge*, 10, 32> edges;  // +68.
		G3D::Array<Joint*, 10, 32> joints;  // +84.
	public:
		// SimJob backref (+100/0x64; C2 NULL). unknown100 retired with
		// the SendPhysics era: SimJob C2/D2 and SendPhysics build/
		// destroy touch this word directly in the binary (accessor
		// access is all inlined -- no setter symbol; public per the
		// IStage-member precedent).
		// D2 :38 asserts "simJob == NULL" (See Assembly.cpp).
		SimJob* simJob;  // +100 (C2 0).

		// Inline accessors -- names pinned by the assert strings
		// (SimJob.cpp :100/:107 + SendPhysics.cpp :24 spell
		// assembly->getConstSimJob(); SendPhysics.cpp :44 spells
		// assembly->getSimJob(); no out-of-line symbols).
		SimJob* getSimJob() { return simJob; }
		const SimJob* getConstSimJob() const { return simJob; }
	private:
		int unknown104;  // +104 (C2 -1).
		int unknown108;  // +108 (C2 -1).
		// +112 filter phase (C2 6; onAssemblyRootRemoving asserts it is
		// NOT_ASSIGNED when stageless -- binary reads +112, so this IS
		// the phase word despite the 2016 enum placing NOT_ASSIGNED
		// elsewhere: enum order DELTA recorded, values verified only
		// for the C2/6 case).
		int filterPhase;  // +112 (C2 6).
	public:
		int getFilterPhase() const { return filterPhase; }
		// 2016 shape (changePhase writes it; int-typed here).
		void setFilterPhase(int value) { filterPhase = value; }
	private:
		// highVelocityNewTouch radius cache (0x59F7A2, batch259: computed
		// through +128 on the flag arm, stored here, reused when the flag
		// is clear). Fills our +116..+120 padding -- no layout shift.
		// C2 value unrecovered (binary C2 never stores +116; first read
		// follows the compute-arm store) -- 0.0f + loud TODO.
		float maxRadius;  // +116 (C2 0.0f; TODO exact C2).
		bool flag120;  // +120 (C2 true; TODO identity).
		Assembly* self124;  // +124 (C2 this).
		void* radiusCallback;  // +128 (see header DELTA).
		int unknown132;  // +132 (C2 0).
		// Child-mesh array (binary +20 array/+24 count; group-build
		// fills it; the onMechanismAdded/Removing + sendClumpChangedMessage
		// walks skip when empty). MODEL-RELATIVE tail slot (see body note
		// for the binary addresses).
		G3D::Array<IndexedMesh*> childMeshes;
	public:
		Assembly();
		// D2 (batch241: :36 history NULL / :37 state ANCHORED / :38
		// simJob NULL / :39 recursiveDepth -1 + member teardown).
		~Assembly();

		// mac: 0x554C50 (50B): two-hop null-safe lookup (prim+8 mesh x2, -8
		// adjust to the owning Assembly).  Static in spirit (no this-use;
		// member per the mangling).
		static Assembly* getPrimitiveAssembly(Primitive* prim);
		// mac: 0x554C82 (50B): const mirror (verified symmetric: same size,
		// same shape, const calls).
		static const Assembly* getConstPrimitiveAssembly(const Primitive* prim);

		// +8 mesh access for the Mechanism lookups (same member the binary
		// adjusts to/from with +-8).
		IndexedMesh* getMesh() { return static_cast<IndexedMesh*>(this); }
		const IndexedMesh* getMesh() const { return static_cast<const IndexedMesh*>(this); }
		// mac: 0x554CB4 (12B, batch273: flag120 = 1, invalidating the
		// maxRadius cache highVelocityNewTouch recomputes).
		/*override*/ virtual void onLowersChanged();
		// 2016 shapes (maxRadius engine for highVelocityNewTouch;
		// binary funnels through the +128 callback -- modeled direct
		// with the indirection as DELTA).
		float computeAssemblyMaxRadius();
		float getMaxRadius() const { return maxRadius; }
		// Flag-arm accessors for highVelocityNewTouch (+120 test/refill).
		bool needsMaxRadiusRecompute() const { return flag120; }
		void storeMaxRadius(float value) { maxRadius = value; flag120 = false; }

		// mac: 0x555572 (151B): the assembly on the far side of edge e
		// (a0 != a1 assert Assembly.cpp:262; returns whichever end isn't this).
		Assembly* otherAssembly(Edge* e);
		// 2016 shape (clump-upper + assembly-upper chain test; assert
		// cross-checks the primitive walk. Link-covered; runs with
		// staged assemblies -- loose prims have no uppers).
		static bool isAssemblyRootPrimitive(const Primitive* prim);

		// EdgeBuffer gateway (batch235/242: const member per the
		// vtbl+24-adjacent call in pushKinematicOk).
		bool computeIsGrounded() const;
		// mac: 0x554A3C (batch241/242: mesh computedUpper walk, -8 container
		// adjust + :102 answer assert -- same lattice idiom as
		// getPrimitiveAssembly; NULL-guarded, never reached unbuilt).
		Primitive* getAssemblyPrimitive();
		const Primitive* getConstAssemblyPrimitive() const;

		// 2016 shape (header templates there too -- no standalone EA;
		// rides getAssemblyPrimitive + isAssemblyRootPrimitive).
		template<class Func>
		void visitAssemblies(Func func) {
			this->visitMeAndChildren<Assembly, Func>(func);
		}
		template<class Func>
		void visitPrimitives(Func func) {
			Primitive* p = getAssemblyPrimitive();
			RBXASSERT(p != NULL);
			visitPrimitivesImpl<Func>(func, p);
		}

	private:
		template<class Func>
		void visitPrimitivesImpl(Func func, Primitive* p) {
			func(p);
			for (int i = 0; i < p->numChildren(); ++i) {
				Primitive* child = p->getTypedChild<Primitive>(i);
				if (!Assembly::isAssemblyRootPrimitive(child)) {
					visitPrimitivesImpl<Func>(func, child);
				}
			}
		}

	public:

}; // class Assembly

} // namespace RBX
