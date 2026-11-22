/* V8World/SimJob.h -- October 2013 (per-moving-root network job and its round-robin tracker).
   Layout, Windows 2013 (SimJob C2 PLAYER 0x00822390): boost::noncopyable takes +0 (MSVC gives an
   empty first base 4 bytes when another base follows), the SimJobHook node +4/+8 (safe_link: the
   ctor zeroes both), std::vector<SimJobTracker*> trackers +0xc (begin +0x10), Assembly* assembly
   +0x1c, int useCount +0x20; operator new(0x24) in SendPhysics::onMovingAssemblyRootAdded 0x008e604c.
   SendPhysics keeps the jobs in a SimJobList (size +0, root +4/+8: buildSimJob 0x008e5ec0).
   2016 SimJob.h has the same shape. */

#pragma once

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/intrusive/list.hpp>

namespace RBX {

	class Assembly;
	class Primitive;
	class SimJob;

	typedef boost::intrusive::list_base_hook< boost::intrusive::tag<SimJob> > SimJobHook;
	typedef boost::intrusive::list<SimJob, boost::intrusive::base_hook<SimJobHook> > SimJobList;

	// SIZE 0x4
	class SimJobTracker
	{
	private:
		SimJob* simJob;  // 0x00

		bool containedBy(SimJob* s);
	public:
		void stopTracking();

		SimJobTracker() : simJob(NULL) {}

		bool tracking();

		void setSimJob(SimJob* s);

		SimJob* getSimJob();

		static void transferTrackers(SimJob* from, SimJob* to);
	};

	// SIZE 0x24
	class SimJob
		: public boost::noncopyable
		, public SimJobHook
	{
		friend class SimJobTracker;

	private:
		std::vector<SimJobTracker*> trackers;  // 0x0c
		Assembly* assembly;                    // 0x1c

	public:
		int useCount;                          // 0x20

		SimJob(Assembly* _assembly);

		~SimJob();

		Assembly* getAssembly() { return assembly; }
		const Assembly* getConstAssembly() const { return assembly; }

		static SimJob* getSimJobFromPrimitive(Primitive* primitive);
		static const SimJob* getConstSimJobFromPrimitive(const Primitive* primitive);
	};

} // namespace RBX
