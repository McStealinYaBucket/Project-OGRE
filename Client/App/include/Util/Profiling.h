/* Util/Profiling.h -- October 2013 (profiler shell).
   The profiling module is absent (binary news CodeProfilers; ours are
   always NULL). Mark takes a nullable pointer and no-ops on NULL, so
   the kernel stepping flow links and runs; timing is unobserved.
   Original file: Client/App/include/Util/Profiling.h (path inferred). */

#pragma once

namespace RBX {
	namespace Profiling {

		class CodeProfiler;

		// Binary takes CodeProfiler& (never NULL there); ours tolerates
		// NULL (our profilers are always NULL) -- same net timing state
		// (unobserved), no crash.
		class Mark
		{
		public:
			Mark(CodeProfiler* section, bool frameTick, bool logInclusive = false)
			{
				(void)section;
				(void)frameTick;
				(void)logInclusive;
			}
			~Mark() {}
		};

	} // namespace Profiling
} // namespace RBX
