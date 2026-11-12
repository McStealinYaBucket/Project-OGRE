/* script/ScriptEvent.h -- October 2013 (first import, section 249).
   Evidence: YieldingThreads C1/C2 0x1AB266/0x1AB270 + queueWaiter pair
     0x1AB2D4/0x1AB302 + waiterCount 0x1AB3E2 + resume 0x1AB3FA +
     WaitingThread C2 0x1AC1B6 (batches 472-473) + priority_queue<
     WaitingThread> push/pop/heap symbols + scoped_ptr<YieldingThreads>
     dtor 0x1A1D62. Assert literals pin the file: queueWaiter's
     yieldCaptured tripwire carries "Client/App/script/ScriptEvent.cpp"
     line 29 (batch474).
   Layout (binary-pinned): YieldingThreads = {context +0,
     priority_queue<WaitingThread> +4} (C2 stores context, constructs the
     queue; operator new(20) = 4 + 16 for MSVC's container+compare).
     WaitingThread = {thread +0 (intrusive_ptr<WeakThreadRef>, 4B),
     start +4/+12 (double pair: queued-at, resume-at)} = 20B (C2
     news the WeakThreadRef on the heap; copies share it).
     The queue is deadline-ordered: std::less throughout (push_heap/
     pop_heap symbols) with an INVERTED operator< (earliest deadline
     reads "largest"), so the top is always the next waiter due.
   Queued (250): resume() (needs ScriptContext::resume#2 + the +620
     running-average member) + ypcall/Continuations/scheduleResume. */

#pragma once

#include "script/ThreadRef.h"
#include "rbx/rbxtime.h"

#include <queue>
#include <vector>

namespace RBX {

	class ScriptContext;

	namespace Lua {

		class YieldingThreads
		{
		public:
			struct WaitingThread
			{
				boost::intrusive_ptr<WeakThreadRef> thread;
				double startTime;
				double resumeTime;
				WaitingThread(lua_State* thread, Time::Interval timeout);
				// Inverted: the priority_queue (std::less) pops the
				// earliest deadline first.
				bool operator<(const WaitingThread& other) const
				{
					return resumeTime > other.resumeTime;
				}
			};

			explicit YieldingThreads(ScriptContext* context);
			void queueWaiter(lua_State* thread);
			void queueWaiter(lua_State* thread, double timeout);
			size_t waiterCount() const;
			// mac: 0x1AB3FA (652B, batch473): pops due waiters and resumes
			// them via ScriptContext::resume#2. Queues behind 250
			// (resume#2 + the +620 average member).
			void resume(double elapsed, Time now, bool& maxed);

		private:
			ScriptContext* context;
			std::priority_queue<WaitingThread> waiters;
		};

	}
}
