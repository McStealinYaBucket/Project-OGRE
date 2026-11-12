/* V8World/SimJobTracker.h -- October 2013 (round-robin send cursor).
   Evidence (batch778/779): setSimJob 0x59A8B6 / tracking 0x59A810 /
     getSimJob 0x59A9F4 / stopTracking 0x59A6D2 / transferTrackers
     0x59AAE6 (static, batch778). The remover inside stopTracking is
     fastRemoveShort<SimJobTracker*> 0x59AD96 (utility template -- see
     the cpp).
   Members: SimJob* simJob (+0). The ctor is NOT an emitted symbol
     (zero-init inlined at the construction sites -- Network-era owners,
     e.g. RoundRobinPhysicsSender's JobSender); there is NO vtable (the
     five member fns are the only symbols -- non-virtual class).
   No construction sites in-tree until the Network era; the class is
     carried now because SendPhysics's landed bodies call the members.
   Self-audit: fidelity ~99% (all five body shapes + the exact assert
     strings transcribed; fastRemoveShort internals LANDED batch780 in
     its binary home include/Util/StlExtra.h -- asserts :47/:48/:49
     byte-matched); function ~80% for the unit
     (only SendPhysics drives it until Network lands).
   Original file: Client/App/v8world/SimJobTracker.h (path inferred). */

#pragma once

namespace RBX {

	class SimJob;

	class SimJobTracker
	{
	public:
		// Inlined at every construction site in the binary (no
		// out-of-line symbol); zero-init only.
		SimJobTracker() : simJob(NULL) {}

		// mac: 0x59A8B6 (batch779): stopTracking, then (m != NULL) the
		// :66/:70 assert-pair around trackers.push_back. The binary's
		// NULL arm returns stale eax; unconsumed by both callers
		// (setTrackerSimJob 0x8D6FCC, transferTrackers) -- port
		// returns this on both arms.
		SimJobTracker* setSimJob(SimJob* m);
		// mac: 0x59A810 (batch779): tracked -> assert :53 (contained) + true.
		bool tracking();
		// mac: 0x59A9F4 (batch779): asserts :78 (simJob) + :79 (contained);
		// the :79 arm dereferences the vector -- the :78 assert must
		// stop the flow (ReleaseAssert is fatal in the binary too).
		SimJob* getSimJob();
		// mac: 0x59A6D2 (batch779): asserts :40 (contained), fastRemoveShort,
		// asserts :44 (not contained), simJob = NULL.
		void stopTracking();
		// mac: 0x59AAE6 (batch778): static (from, to); asserts :85 (from) /
		// :86 (from != to), then the back-pop loop re-tracks each
		// tracker onto to (each stopTracking removes it from from).
		static void transferTrackers(SimJob* from, SimJob* to);
		// Inline in the binary (all the :40/:53/:66/:70/:79 assert
		// strings spell "this->containedBy(...)"); landed out-of-line
		// -- the body needs the complete SimJob (vector walk).
		bool containedBy(SimJob* m);

	private:
		SimJob* simJob;   // +0 (the one member).
	};

} // namespace RBX
