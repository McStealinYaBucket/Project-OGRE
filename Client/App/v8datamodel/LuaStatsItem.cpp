/* App/v8datamodel/LuaStatsItem.cpp -- October 2013 (Lua stat subtree).
   C2 + init per batch581. TODO: full decompile (update 0x1ACB92). */

#include "stdafx.h"

#include "V8DataModel/LuaStatsItem.h"
#include "V8DataModel/ScriptContext.h"
#include "V8DataModel/TypedStatsItem.h"

#include <boost/bind.hpp>

namespace RBX {

	// mac: 0x1A8E5E: SC* at +0x68 + setName("Lua") (base stores via StatsItem()).
	LuaStatsItem::LuaStatsItem(ScriptContext* context)
		: context(context)
		, childAvgGcInterval(NULL)
		, childAvgGcTime(NULL)
		, childAvgMem(NULL)
		, childThreadsResumed(NULL)
		, childThreadsThrottled(NULL)
	{
		setName("Lua");
	}

	// mac: 0x1ACA88 (batch581 order: disabled-bool, threads-intfn,
	// ThreadsResumed, ThreadsThrottled-avg, AverageGcInterval/Time/Mem).
	// STUB: PLAYER 0x008ac940
	// STUB: STUDIO 0x009ee9c0
	void LuaStatsItem::init()
	{
		ScriptContext* sc = context;
		createBoundChildItem("disabled", sc->disabled);
		// Bound target: getThreadCount is the arity-0 int reader on SC;
		// the binary's bind target is truncated in evidence (LOUD TODO).
		boost::function<int()> threadsFn = boost::bind(&ScriptContext::getThreadCount, sc);
		createChildItem("threads", threadsFn);
		childThreadsResumed = createChildItem("ThreadsResumed");
		childThreadsThrottled = createBoundChildItem("ThreadsThrottled", sc->threadsThrottled);
		childAvgGcInterval = createChildItem("AverageGcInterval");
		childAvgGcTime = createChildItem("AverageGcTime");
		childAvgMem = createChildItem("AverageMemAlloc");
	}

	// mac: 0x1ACB92 (batch610: avgInterval "%.2f msec" + avgTime "%.4f msec"
	// + threshold "%.2f KB/gcFrequency" + resume-rate formatRate).
	// STUB: PLAYER 0x008abb70
	// STUB: STUDIO 0x009edc80
	void LuaStatsItem::update()
	{
		ScriptContext* sc = context;
		childAvgGcInterval->formatValue(sc->avgLuaGcInterval.value(), "%.2f msec",
			sc->avgLuaGcInterval.value());
		childAvgGcTime->formatValue(sc->avgLuaGcTime.value(), "%.4f msec",
			sc->avgLuaGcTime.value());
		childAvgMem->formatValue(sc->gcStepThreshold, "%.2f KB/gcFrequency",
			sc->gcStepThreshold);
		childThreadsResumed->formatRate(sc->threadsResumedRate);
	}

} // namespace RBX
