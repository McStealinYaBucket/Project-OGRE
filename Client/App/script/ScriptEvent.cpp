/* script/ScriptEvent.cpp -- October 2013 (first import, section 249).
   Evidence: YieldingThreads C1/C2 0x1AB266/0x1AB270 + queueWaiter pair
     0x1AB2D4/0x1AB302 + waiterCount 0x1AB3E2 + WaitingThread C2 0x1AC1B6
     (batches 472-473/475). queueWaiter's yieldCaptured tripwire carries
     "Client/App/script/ScriptEvent.cpp" line 29 (batch474) -- this TU is
     the canonical home. wait/delay/spawn/version bodies live in
     ScriptContext.cpp (their assert literals pin "script/ScriptContext.cpp",
     batch476).
   resume() (0x1AB3FA) + the +620 average member queue behind 250. */

#include "stdafx.h"

#include "script/ScriptEvent.h"
#include "V8DataModel/ScriptContext.h"
#include "Util/FastFlags.h"
#include "RbxAssert.h"
#include "rbx/atomic.h"
#include "lua.h"

LOGGROUP(ScriptContext);

namespace RBX { namespace Lua {

// mac: 0x1AB302 (224B, batch473): deadline waiter; asserts the thread isn't already yield-captured.
// FUNCTION: PLAYER 0x008a9ed0
// FUNCTION: STUDIO 0x009edaf0
void YieldingThreads::queueWaiter(lua_State* L, double timeout)
{
	RBXASSERT(!RobloxExtraSpace::get(L)->yieldCaptured);
	RobloxExtraSpace::get(L)->yieldCaptured = true;
	waiters.push(WaitingThread(L, Time::Interval(timeout)));
}

// mac: 0x1AB266/0x1AB270 (C1 9B delegates to C2 100B, batch473/475):
// context at +0, default-constructed queue at +4.
YieldingThreads::YieldingThreads(ScriptContext* context)
	: context(context)
{
}

// mac: 0x1AC1B6 (146B, batch473): heap WeakThreadRef + now/+deadline pair.
// The intrusive_ptr member shares ownership across queue copies; the
// binary's manual addref/release dance is observably identical.
YieldingThreads::WaitingThread::WaitingThread(lua_State* thread, Time::Interval timeout)
	: thread(new WeakThreadRef(thread))
	, startTime(Time::now<Time::Precise>().timestampSeconds())
	, resumeTime(startTime + timeout.seconds())
{
}

// mac: 0x1AB2D4 (45B, batch473): immediate waiter.
void YieldingThreads::queueWaiter(lua_State* thread)
{
	queueWaiter(thread, 0);
}

// mac: 0x1AB3E2 (23B, batch473): queue size.
size_t YieldingThreads::waiterCount() const
{
	return waiters.size();
}

// mac: 0x1AB3FA (652B, batch473): pop every due waiter, push (elapsed, now),
// and run it via ScriptContext::resume#2 (2 results). Stops early with
// maxed=true past the limit.
// DELTA: the +620 running-average sample is skipped (pure telemetry;
// the member queues behind the layout audit).
// STUB: PLAYER 0x008a9bb0
// STUB: STUDIO 0x009ed7d0
void YieldingThreads::resume(double now, Time limit, bool& maxed)
{
	if (FLog::ScriptContext != 0)
		FLog::FastLog(FLog::ScriptContext, "Resuming waiting threads", NULL);
	while (!waiters.empty())
	{
		Time current = Time::now<Time::Precise>();
		const WaitingThread& top = waiters.top();
		if (top.resumeTime > current.timestampSeconds())
			break;
		double elapsed = current.timestampSeconds() - top.startTime;
		WaitingThread due = top;
		waiters.pop();
		Lua::ThreadRef thread = due.thread->lock();
		lua_State* live = thread.get();
		if (live != NULL)
		{
			int outerTop = lua_gettop(live);
			lua_pushnumber(live, elapsed);
			lua_pushnumber(live, now);
			context->resume(thread, 2);
			lua_settop(live, outerTop);
		}
		if (current.timestampSeconds() > limit.timestampSeconds())
		{
			maxed = true;
			return;
		}
	}
}

} }
