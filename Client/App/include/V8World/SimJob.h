/* V8World/SimJob.h -- October 2013 (per-moving-root network job).
   Evidence: C1 0x59ABCC tail -> C2 0x59ABD6 / D1 0x59AC8E tail ->
     D2 0x59AC98 (batch778); owner flows from SendPhysics
     onMovingAssemblyRootAdded 0x59A096 (job new'd 0x1C here, first arm)
     and deleted in onMovingAssemblyRootRemoving 0x59A16C after
     destroySimJob.
   Members: std::vector<SimJobTracker*> trackers (+8; binary begin/end/
     cap at +8/+0xC/+0x10), Assembly* assembly (+0x14, the C2 ctor arg),
     int useCount (+0x18; ++ in 0x59A096, -- in 0x59A16C).  sizeof
     (SimJob) = 0x1C.
   C2 (0x59ABD6): zeroizes the trackers vector, stores the ctor arg at
     +0x14, useCount 0, then asserts "assembly->getConstSimJob() == NULL"
     (SimJob.cpp:100, assembly-param spell).  D2 (0x59AC98): asserts
     "useCount == 0" (:106) + "assembly->getConstSimJob() == NULL"
     (:107); no member deletes (the vector frees itself).  No vtable
     (non-virtual class; no D0 symbol -- the 0x59A16C teardown is the
     direct ~SimJob + operator delete pair).
   Ownership note: SendPhysics only LINKS the job (buildSimJob/
     destroySimJob touch trackers-unlinking); the job memory lives and
     dies with the moving-root assembly's send membership, deleted in
     onMovingAssemblyRootRemoving.
   No construction sites in-tree yet outside SendPhysics -- the rest
   queue with the Network era.
   Self-audit: fidelity 100% (every decoded cell is transcribed below;
     accessor names are the assert-string spellings); function ~80% for
     the unit in-tree: created/destroyed by the landed SimulateStage
     flows, but only Network consumers (RoundRobinPhysicsSender --
     batch776 symbol list) put the trackers to work.
   Original file: Client/App/v8world/SimJob.h (path inferred; the assert
     strings carry SimJob.cpp line cites). */

#pragma once

#include <vector>

namespace RBX {

	class Assembly;

	class SimJob
	{
		friend class SimJobTracker;
		friend class SendPhysics;

	public:
		SimJob(Assembly* assembly);
		~SimJob();  // non-virtual by evidence (no D0/D1 vtable family).

		// 2013 inline accessors -- names pinned by the assert strings
		// (SendPhysics.cpp :24 uses getConstAssembly/getConstSimJob,
		// :44 uses getAssembly/getSimJob); no out-of-line symbols.
		Assembly* getAssembly() { return assembly; }
		const Assembly* getConstAssembly() const { return assembly; }

	private:
		std::vector<SimJobTracker*> trackers;   // +8 (C2 empties; binary
		                                        // begin/end/cap at +8/..C/..10).
		Assembly* assembly;                     // +0x14 (C2 stores the ctor arg).
		int useCount;                           // +0x18 (0x59A096 ++ / 0x59A16C --).
	};

} // namespace RBX
