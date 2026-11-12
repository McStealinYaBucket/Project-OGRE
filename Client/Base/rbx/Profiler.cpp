/* Base/rbx/Profiler.cpp — October 2013 (diagnostic stubs for link debts).
   The 2016 Profiler wraps MicroProfile (third-party, absent from this tree),
   so only the referenced subset is defined here as no-ops. All callers use
   these for diagnostics only (RBXPROFILER_SCOPE etc. degrade to token 0).
   TODO: vendor MicroProfile + port the full module (render/gpu/mouse/frame).
   Original file: (2013 TU layout TODO; 2016 equivalent: Client/Base/rbx/Profiler.cpp) */

#include "rbx/Profiler.h"

namespace RBX {
	namespace Profiler {

		Token getToken(const char* group, const char* name, int color)
		{
			(void)group; (void)name; (void)color;
			return 0;
		}

		uint64_t enterRegion(Token token)
		{
			(void)token;
			return 0;
		}

		void leaveRegion(Token token, uint64_t enterTimestamp)
		{
			(void)token; (void)enterTimestamp;
		}

		void onThreadCreate(const char* name)
		{
			(void)name;
		}

		void onThreadExit()
		{
		}

	} // namespace Profiler
} // namespace RBX
