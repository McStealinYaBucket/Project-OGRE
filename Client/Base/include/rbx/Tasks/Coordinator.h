// Base/include/rbx/Tasks/Coordinator.h: October 2013 reconstruction (Project OGRE).
// Evidence: PLAYER RTTI has exactly Coordinator, SequenceBase, Sequence and ExclusiveSequence in
// RBX::Tasks (plus sp_counted_impl_p of the last two); iOS 1029 names
// SequenceBase::{isInhibited, advance, onAdded, onRemoved}, Sequence::onPreStep,
// ExclusiveSequence::onPostStep and Coordinator's inline onPreStep/onPostStep. 2016's Exclusive and
// Barrier have no 2013 RTTI or body and are dropped. SequenceBase layout (Base/rbx/Tasks/
// Coordinator.cpp): nextJobIndex at 4, mutex at 8, jobs after it.
#pragma once

#include "rbx/TaskScheduler.h"
#include <map>

namespace RBX {
namespace Tasks {

// A job's coordinators can hold it back from stepping (TaskScheduler::Job::isInhibited) and are
// told around each step. They shape ordering, not parallelism. Callers may be on any thread, but
// never on two threads for the same job.
class RBXBaseClass Coordinator
{
public:
	virtual bool isInhibited(TaskScheduler::Job* job) = 0;
	virtual void onPreStep(TaskScheduler::Job* job) {}
	virtual void onPostStep(TaskScheduler::Job* job) {}
	virtual void onAdded(TaskScheduler::Job* job) {}
	virtual void onRemoved(TaskScheduler::Job* job) {}
};

// Lets the coordinated jobs start only in the order they were added. Subclasses decide when the
// turn moves on by calling advance().
class SequenceBase : public Coordinator
{
	unsigned int nextJobIndex;
	RBX::mutex mutex;
	std::vector<TaskScheduler::Job*> jobs;
protected:
	void advance();
public:
	SequenceBase() : nextJobIndex(0) {}
	virtual bool isInhibited(TaskScheduler::Job* job);
	virtual void onAdded(TaskScheduler::Job* job);
	virtual void onRemoved(TaskScheduler::Job* job);
};

// Turn passes when a job starts stepping, so steps may overlap.
class Sequence : public SequenceBase
{
public:
	virtual void onPreStep(TaskScheduler::Job* job) { advance(); }
};

// Turn passes when a job finishes stepping: one job at a time, in order.
class ExclusiveSequence : public SequenceBase
{
public:
	virtual void onPostStep(TaskScheduler::Job* job) { advance(); }
};

}
}
