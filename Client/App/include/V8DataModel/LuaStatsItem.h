/* V8DataModel/LuaStatsItem.h -- October 2013 (Lua stat subtree root).
   Evidence: C2 0x1A8E5E (batch581: Instance::C2 + Item-pattern member
     stores + own vtable block + SC* at +0x68 + setName("Lua")) +
     init 0x1ACA88 (6 child creates). DERIVES Stats::Item content
     (base stores identical; no base-C2 call in binary -- inlined --
     ours chains to StatsItem() for the same stores). className
     inherits "StatsItem" (no LuaStatsItem typeinfo/descriptor in DB;
     LOUD TODO: binary descriptor unverified). Creator unevidenced
     (no factory symbol) -- direct construction only.
   update 0x1ACB92 + display/format* queue (no bodies in hand).
   TODO: full decompile. */

#pragma once

#include "V8DataModel/StatsItem.h"

namespace RBX {

	class ScriptContext;

	class LuaStatsItem : public StatsItem
	{
	public:
		explicit LuaStatsItem(ScriptContext* context);
		// mac: 0x1ACA88: build the 7 children (batch581 order).
		void init();
		// mac: 0x1ACB92 (batch610): format the 5 cached children from the
		// SC samplers (avg interval/time/threshold + resume rate).
		void update();
	private:
		ScriptContext* context;  // +0x68 role
		StatsItem* childAvgGcInterval;  // +0x6C role
		StatsItem* childAvgGcTime;      // +0x70 role
		StatsItem* childAvgMem;         // +0x74 role
		StatsItem* childThreadsResumed; // +0x78 role
		StatsItem* childThreadsThrottled;  // +0x7C role
	};

} // namespace RBX
