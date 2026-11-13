/* V8World/AssemblyStage.h -- October 2013 (assembly stage node, 0x28).
   Evidence: C2 0x557E08 (149B, batch220: news 0x34 MovingAssemblyStage) /
     D1 0x557ECE / D0 0x557E9E (batch224: D1 calls EdgeBuffer::~EdgeBuffer
     -- AssemblyStage : EdgeBuffer) / getStageType 0x558126 -> 9
     (batches 213/220/224).
   onEdge pair INTERIM (documented in EdgeBuffer.h): binary slots hold
   EdgeBuffer::onEdgeAdded/Removing (0x572C2A/0x572D36, batch226 proof);
   inherited IWorldStage base bodies until the buffer flow lands.
   Layout: EdgeBuffer base (+4 upstream / +8 downstream=MovingAssemblyStage
     / +12 world) + assemblyEdges +20.
   Original file: Client/App/v8world/AssemblyStage.h (path inferred) */

#pragma once

#include "V8World/EdgeBuffer.h"

namespace RBX {

	class World;

	class AssemblyStage : public EdgeBuffer
	{
	public:
		AssemblyStage(IStage* upstream, World* world);
		// User-declared in 2013 (2016 too): the deleting destructor 0x008ea3c0 stores
		// AssemblyStage's vftable before calling ~EdgeBuffer 0x009994a0, so it does not fold
		// with EdgeBuffer's 0x008ea600.
		~AssemblyStage();

		// SYNTHETIC: PLAYER 0x008ea3c0
		// SYNTHETIC: STUDIO 0x00a085d0
		// RBX::AssemblyStage::`scalar deleting destructor'

		// mac: 0x558126 (10B): ASSEMBLY_STAGE.
		// FUNCTION: PLAYER 0x00918e20
		// FUNCTION: STUDIO 0x009427b0
		virtual StageType getStageType() const { return ASSEMBLY_STAGE; }
		// mac: 0x55807E (batch297: putInStage + afterAssemblyAdded).
		void onFixedAssemblyRootAdded(Assembly* assembly);
	};

} // namespace RBX
