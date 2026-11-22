/* Base/rbx/Tasks/Coordinator.cpp -- October 2013. Only SequenceBase has bodies in the 2013 binaries
   (PLAYER 0x009bb710-0x009bba70; the vector<Job*>::_Insert_n at 0x009bb830 is push_back's helper).
   Barrier and Exclusive have no 2013 body. Layout from the PLAYER bytes: nextJobIndex @4, mutex @8,
   jobs @0x20 (_Myfirst read at +0x24). Path from the 2016 tree (no 2013 __FILE__ string). */

#include "rbx/Tasks/Coordinator.h"

using namespace RBX;
using namespace Tasks;

// FUNCTION: PLAYER 0x009bb710
// FUNCTION: STUDIO 0x00a72670
bool SequenceBase::isInhibited(TaskScheduler::Job* job)
{
	RBX::mutex::scoped_lock lock(mutex);

	if (nextJobIndex < jobs.size())
		return jobs[nextJobIndex] != job;
	else
		return true;
}

// FUNCTION: PLAYER 0x009bb760
// FUNCTION: STUDIO 0x00a726c0
void SequenceBase::advance()
{
	RBX::mutex::scoped_lock lock(mutex);

	if (++nextJobIndex == jobs.size())
		nextJobIndex = 0;
}

// FUNCTION: PLAYER 0x009bb7a0
// FUNCTION: STUDIO 0x00a72700
void SequenceBase::onRemoved(TaskScheduler::Job* job)
{
	RBX::mutex::scoped_lock lock(mutex);

	for (size_t i=0; i<jobs.size(); ++i)
		if (jobs[i] == job)
		{
			jobs.erase(jobs.begin() + i);
			if (nextJobIndex > i)
				--nextJobIndex;
			break;
		}
}

// STUB: PLAYER 0x009bb9e0
// STUB: STUDIO 0x00a72940
void SequenceBase::onAdded(TaskScheduler::Job* job)
{
	RBX::mutex::scoped_lock lock(mutex);

	jobs.push_back(job);
}
