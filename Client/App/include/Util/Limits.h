/* Util/Limits.h -- October 2013 instance-count limiter (section 258d).
   Evidence: Counter::canAdd 0x149848 (54B) + getCurrentCount 0x1497E4
     (100B) + current() once/thread-specific machinery
     (safe_static_init_current 0x149B70 + do_get_current 0x149B7A) +
     countLimit/globallyEnforcedLimit/globallyEnforcedLimitExceeded BSS
     (0xFA5734/38/39, no writers DB-wide) (batches 517/542).
   Landed: countLimit + canAdd + getCurrentCount + current. Queued:
     C2/D2/add/Activator + globallyEnforced* + 'Count exceeded'
     (per-thread counting era; add() is the only writer/thrower).
   Original file: unknown (Util/ era). */

#pragma once

#include "rbx/threadsafe.h"
#include "boost/shared_ptr.hpp"
#include "boost/noncopyable.hpp"

namespace RBX { namespace Limits {

	class Countable;

	class Counter : boost::noncopyable
	{
		friend class Countable;
		long count;	// +0, read by getCurrentCount (never incremented
				// in-tree: add() queues with the counting era)
	public:
		Counter() : count(0) {}

		static int countLimit;	// BSS-zero: canAdd short-circuits true
		static bool globallyEnforcedLimit;

		static bool canAdd(int n);
		static int getCurrentCount();
		static boost::shared_ptr<Counter> current();

		// PLAYER 0x009b90a0 / STUDIO 0x00a6afd0: throws "Count exceeded" over the
		// limit, then RbxInterlockedIncrement(&count). Not in this tree yet.
		void add(Countable* theValue);
	};

	// 2013 layout (ctor 0x009b94d0, dtor 0x009b9160): one shared_ptr<Counter>, not
	// polymorphic. The ctor zeroes it, assigns Counter::current() and calls add(this);
	// the dtor decrements counter->count and releases. Instance places it at +0x2c,
	// after all its polymorphic bases. The Base tree's rbx/Limits.h (unused) declares
	// a virtual destructor, which 2013 does not have.
	// SIZE 0x8
	class Countable
	{
	public:
		Countable();
		~Countable();

	private:
		boost::shared_ptr<Counter> counter;	// 0x00
	};

} }
