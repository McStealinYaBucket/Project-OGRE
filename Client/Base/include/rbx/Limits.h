/* rbx/Limits.h — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation (F:\2013Decompile\exports\decomp\Rbx_Limits.cpp.txt)
     - Countable ctor 0x1495FA: registers with thread-specific current Counter, then Counter::add
     - Counter::add 0x1496B2: throws std::runtime_error("Count exceeded") when
       globallyEnforcedLimit && countLimit > 0 && count > countLimit; sets globallyEnforcedLimitExceeded
     - Countable dtor 0x149794: interlocked decrement + sp_counted_base release
     - getCurrentCount 0x1497E4: reads thread-current counter's count, 0 if unset
   Members (x86): Countable { int* count; sp_counted_base* holder; } */

#pragma once

#include "rbx/boost.hpp"
#include <boost/thread.hpp>
#include <stdexcept>
#include <string>

namespace RBX {

	namespace Limits {

		class Countable;

		class Counter {
		public:
			static int countLimit;						// global per-thread count limit
			static bool globallyEnforcedLimit;			// enforce check on add
			static bool globallyEnforcedLimitExceeded;	// set when the limit trips

			static boost::shared_ptr<Counter> current();	// thread-specific current counter

			static bool canAdd(int count)				// 0x149848
			{
				return !globallyEnforcedLimit || countLimit <= 0 || count <= countLimit;
			}

			int getCurrentCount()						// 0x1497E4
			{
				boost::shared_ptr<Counter> c = current();
				return c ? c->count : 0;
			}

			int count;

			void add(Countable* theValue);				// 0x1496B2

			class Activator {
			public:
				Activator(boost::shared_ptr<Counter> counter);	// 0x14987E: sets thread-current
				~Activator();									// 0x149956: restores

			private:
				boost::shared_ptr<Counter> newCounter;
				boost::shared_ptr<Counter> oldCounter;
			};

		private:
			static boost::thread_specific_ptr<boost::shared_ptr<Counter> >& threadCurrent();
		};

		class Countable {
		public:
			Countable();									// 0x1495FA
			virtual ~Countable();							// 0x149794

			int getCount() const { return count ? *count : 0; }

		protected:
			int* count;										// points at current Counter::count

		private:
			void* holder;									// sp_counted_base* keeping the Counter alive
		};

	} // namespace Limits

} // namespace RBX
