/* App/util/Profiling.cpp -- October 2013 (2013 assert path Client\App\util\Profiling.cpp).
   Only the two constructors so far; the rest of the .obj (init 0x00744fb0, setEnabled 0x00744fd0,
   the Bucket rates 0x00745020.., log/unlog, isEnabled 0x00745280, getWindow 0x007452a0) is unwritten. */

#include "stdafx.h"

#include "Util/Profiling.h"
#include "decomp.h"

namespace RBX {
	namespace Profiling {

		// GLOBAL: PLAYER 0x00d5cec0
		// GLOBAL: STUDIO 0x00ed5248
		double Profiler::profilingWindow = 3.0;

		// FUNCTION: PLAYER 0x007451d0
		// FUNCTION: STUDIO 0x007d7a30
		Profiler::Profiler(const char* name)
			: bucketTimeSpan(profilingWindow * 0.003)
			, currentBucket(0)
			, name(name)
		{
		}

		// FUNCTION: PLAYER 0x00745230
		// FUNCTION: STUDIO 0x007d7a90
		CodeProfiler::CodeProfiler(const char* name)
			: Profiler(name)
		{
		}

	} // namespace Profiling
} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::Profiling::CodeProfiler, 0xc040);
