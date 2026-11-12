/* App/util/Limits.cpp -- October 2013 (section 258d; see
   include/Util/Limits.h for evidence). */

#include "stdafx.h"

#include "Util/Limits.h"

#include "rbx/atomic.h"
#include <boost/thread/once.hpp>

namespace RBX { namespace Limits {

	int Counter::countLimit = 0;
	bool Counter::globallyEnforcedLimit = false;

	namespace {
		rbx::thread_specific_shared_ptr<Counter>& currentValue()
		{
			static rbx::thread_specific_shared_ptr<Counter> value;
			return value;
		}
		void safe_static_init_current()
		{
		}
	}

	// mac: 0x149848 (54B, batch542): no limit (<= 0) admits everything.
	bool Counter::canAdd(int n)
	{
		if (countLimit <= 0)
			return true;
		return getCurrentCount() + n <= countLimit;
	}

	// getCurrentCount 0x1497E4 (100B, batch542): once-guarded
	// thread-specific counter value, 0 when unset.
	int Counter::getCurrentCount()
	{
		static boost::once_flag once = BOOST_ONCE_INIT;
		boost::call_once(&safe_static_init_current, once);
		boost::shared_ptr<Counter> current = currentValue();
		return current.get() != NULL ? current->count : 0;
	}

	boost::shared_ptr<Counter> Counter::current()
	{
		static boost::once_flag once = BOOST_ONCE_INIT;
		boost::call_once(&safe_static_init_current, once);
		return currentValue();
	}

	// FUNCTION: PLAYER 0x009b9160
	// FUNCTION: STUDIO 0x00a6b090
	Countable::~Countable()
	{
		if (counter)
			RbxInterlockedDecrement(&counter->count);
	}

	// STUB: PLAYER 0x009b94d0
	// STUB: STUDIO 0x00a6b420
	Countable::Countable()
	{
		counter = Counter::current();
		if (counter)
			counter->add(this);
	}

} }
