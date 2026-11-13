/* V8World/EdgeBuffer.h -- October 2013 (assembly edge buffer base, 0x28).
   Evidence: AssemblyStage C2 0x557E08 (149B, batch220: +20 set-header
     init -- the buffer member) / D2 0x57255A (batch240: :20
     assemblyEdges.empty + multimap erase + delete +8) /
     EdgeBuffer::onEdgeAdded 0x572C2A / onEdgeRemoving 0x572D36 /
     pushEdgeIfOk / pushSpringOk 0x572B2E / pushKinematicOk 0x572A04 /
     assemblyPrimitiveAdded 0x5726EE / assemblyPrimitiveRemoved /
     beforeAssemblyRemoving 0x572792 (batches 234/235/240).
   Members: BiMultiMap<Assembly*,Edge*> assemblyEdges (+20, 2016 shape --
     BiMultiMap header carried, JointStage precedent builds green).
   onEdge pair REAL (batch240): add asserts !isGroundJoint (:149) +
   !isRigidJoint (:150), stages, pushes; removal forwards downstream
   first (mirrors IWorldStage order) then un stages. The INTERIM base
   forwarders are retired.
   pushEdgeIfOk/pushSpringOk/pushKinematicOk: 2016 shapes (first-party),
   disasm-verified (kinematic/motor :81, spring inStage :95, kinematic
   inStage :119 + isMotorJoint :120). pushKinematicOk needs
   Assembly::computeIsGrounded (Assembly gateway -- batch241).
   assemblyPrimitiveAdded: DeferredContacts flag arm (off by default;
   flag-on needs SpatialHash::onPrimitiveAssembled -- unported) + the
   contact/joint push loops. assemblyPrimitiveRemoved: downstream
   forward per edge (2013 shape -- direct, not removeEdgeIfDownstream,
   which has no 2013 symbol). beforeAssemblyRemoving + afterAssembly
   flow: Assembly::visitPrimitivesImpl gateway, deferred.
   Layout: IStage base (+4 upstream / +8 downstream=MovingAssemblyStage /
     +12 world) + buffer member.
   Original file: Client/App/v8world/EdgeBuffer.h (path from assert strings:
   EdgeBuffer.cpp:20/:81/:95/:119/:120/:149/:150) */

#pragma once

#include "V8World/IWorldStage.h"
#include "Util/BiMultiMap.h"

namespace RBX {

	class World;
	class Assembly;
	class Edge;
	class Primitive;

	class EdgeBuffer : public IWorldStage
	{
	public:
		virtual ~EdgeBuffer();

		// SYNTHETIC: PLAYER 0x008ea600
		// SYNTHETIC: STUDIO 0x00a08810
		// RBX::EdgeBuffer::`scalar deleting destructor'

		// mac: 0x572C2A (batch240: :149/:150 asserts + putInStage + push).
		virtual void onEdgeAdded(Edge* edge);
		// mac: 0x572D36 (batch240: downstream-first + removeFromStage).
		virtual void onEdgeRemoving(Edge* edge);

		// mac: 0x5728E2 (batch234: inStage gate + :81 assert + kinematic/
		// spring dispatch + generateDataForMovingAssemblyStage +
		// downstream forward; returns the push verdict).
		bool pushEdgeIfOk(Edge* edge);
		// mac: 0x572B2E (batch235: :95 assert + both-here + one-downstream +
		// distinct-assemblies triple).
		bool pushSpringOk(Edge* edge);
		// mac: 0x572A04 (batch235: :119/:120 asserts + grounded-or-downstream).
		bool pushKinematicOk(Edge* edge);

		// mac: 0x5726EE (batch240: DeferredContacts arm + push loops).
		void assemblyPrimitiveAdded(Primitive* prim);
		// (batch240: downstream forward per downstream edge).
		void assemblyPrimitiveRemoved(Primitive* prim);
		// 2016 shape (visitPrimitives + assemblyPrimitiveAdded; the
		// contactManager arm is DeferredContacts-gated (off) -- see
		// assemblyPrimitiveAdded's no-op arm).
		void afterAssemblyAdded(Assembly* assembly);

	protected:
		EdgeBuffer(IStage* upstream, IStage* downstream, World* world)
			: assemblyEdges()
		{
			this->upstream = upstream;
			this->downstream = downstream;
			this->world = world;
		}

		// 2016 shape (first-party; inlined in the 2013 pushSpringOk body
		// as the unknown04+inOrDownstream half -- unknown04 tracks the
		// pipeline flag, spelled here via inPipeline).
		bool assemblyIsHere(Assembly* assembly);

		BiMultiMap<Assembly*, Edge*> assemblyEdges;  // +20.
	};

} // namespace RBX
