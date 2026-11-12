/* V8DataModel/StatsService.h -- October 2013 (StatsService shell).
   Evidence: ScriptContext::stats 0x18E89C (batch579) calls
     ServiceProvider::create<RBX::Stats::StatsService>(Instance*) +
     shared_from + SharedPtrBridge<Instance>::push, returning 1 --
     so the class exists in namespace RBX::Stats and is service-
     creatable. NO C2/D2/vtable symbols DB-wide (implicit trivial
     type -- same shape as the other thin services here).
   className "StatsService" is inferred by in-tree convention
   (s-name == unqualified C++ name, cf. sJointsService) -- TODO:
   confirm against the binary string table.
   Functionality (stat items/children) queues with the Stats era
   (LuaStatsItem + Stats::Item machinery).
   Original file: (2013 path TODO). */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "reflection/Type.h"

namespace RBX {
namespace Stats {

	extern const char* const sStatsService;

	class StatsService
		: public DescribedNonCreatable<StatsService, Instance, sStatsService, Reflection::ClassDescriptor::RUNTIME_LOCAL>
		, public Service
	{
	public:
		StatsService() {}

		// ScriptContext::reportError 0x196093 calls the 3-arg overload
		// (mangled __ZN3RBX5Stats12StatsService6reportESsN5boost10shared_ptrIKNS2_9unordered13unordered_mapISsNS_10Reflection7VariantENS2_4hashISsEESt8equal_toISsESaISt4pairIKSsS7_EEEEEEi
		// @0x4A1974, batch718): report(std::string, shared_ptr<const
		// ValueTable>, int).  2-arg overload 0x49F572 exists too.
		// DELTA: the binary body (throttle + checkLastReport +
		// addHeader/addCategoryAndTable + the postReport network send)
		// queues with the Stats-report era -- no-op here, so the call is
		// exercised but nothing is sent (tests must never hit network).
		// STUB: PLAYER 0x00741320
		// STUB: STUDIO 0x007c9230
		void report(std::string name,
			boost::shared_ptr<const Reflection::ValueTable> values,
			int throttlePercentage)
		{
			(void)name; (void)values; (void)throttlePercentage;
		}
	};

} // namespace Stats
} // namespace RBX
