// Base/include/rbx/CEvent.h: October 2013 reconstruction (Project OGRE).
// Evidence: Base/rbx/CEvent.cpp is a 2013 __FILE__ (PLAYER); RTTI sp_counted_impl_p<RBX::CEvent>
// (TaskScheduler::remove takes a shared_ptr<CEvent>); iOS 1029 symbol
// RBX::CEvent::WaitForSingleObject(RBX::CEvent&, int). On Windows the object is one kernel event
// handle (4 bytes); elsewhere it is a boost condition variable plus mutex.
#pragma once

#include "boost/noncopyable.hpp"
#include "RbxFormat.h"
#include "rbx/RbxTime.h"

#include <boost/thread.hpp>

#if !defined(_WIN32)
#define RBX_CEVENT_BOOST
#endif

namespace RBX {

// An ATL::CEvent look-alike: manual- or auto-reset, set by one thread and waited on by others.
// Manual reset: once Set, every waiter passes until the event is recreated (there is no Reset).
// Auto reset: Set releases one waiter and the event clears itself. The TaskScheduler uses these to
// let a caller block until a job has been removed (TaskScheduler::remove's shared_ptr<CEvent>),
// and ScriptContext keeps one too. Not copyable: it owns its kernel object.
class CEvent : public boost::noncopyable
{
#if defined(RBX_CEVENT_BOOST)
	const bool manualReset;
	volatile bool isSet;
	boost::condition_variable cond;
	boost::mutex mut;
#elif defined(_WIN32)
	void* m_h; // HANDLE from CreateEvent
#endif

public:
	explicit CEvent(bool bManualReset);
	~CEvent() throw();

	void Set() throw();
	void Wait(); // blocks without a time limit
	// true when signalled, false on timeout
	bool Wait(int milliseconds);
	bool Wait(RBX::Time::Interval interval)
	{
		return Wait((int)(1000.0 * interval.seconds()));
	}

private:
	// Win32 values, so the portable build can return the same codes.
	static const int cWAIT_OBJECT_0 = 0;
	static const int cWAIT_TIMEOUT = 258;
	static const int cINFINITE = 0xFFFFFFFF;
	static int WaitForSingleObject(CEvent& event, int milliseconds);
};

}
