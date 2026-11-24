/* App/util/Profiling.cpp -- October 2013 (2013 assert path Client\App\util\Profiling.cpp), the whole .obj
   from the 2013 bytes (PLAYER 0x00744fb0-0x00745572). The markTlsIndex TLS slot (0x00df728c) holds the
   innermost live Mark of the thread; profilingEnabled is 0x00df7290. DebugSettings+0x73 is the
   settings-side enable flag (2016 enableProfiling; unnamed in our DebugSettings.h). */

#include "stdafx.h"

#include "Util/Profiling.h"
#include "V8DataModel/DebugSettings.h"
#include "decomp.h"

static long markTlsIndex = 0;
static bool profilingEnabled = false;

namespace RBX {
	namespace Profiling {

		// FUNCTION: PLAYER 0x00744fb0
		// FUNCTION: STUDIO 0x007d7810
		void init(bool enabled)
		{
			profilingEnabled = enabled;
			markTlsIndex = TlsAlloc();
		}











		// FUNCTION: PLAYER 0x00744fd0
		// FUNCTION: STUDIO 0x007d7830
		void setEnabled(bool enabled)
		{
			profilingEnabled = enabled;
			RBXASSERT(markTlsIndex);
		}

		// FUNCTION: PLAYER 0x00745280
		// FUNCTION: STUDIO 0x007d7ae0
		bool isEnabled()
		{
			return profilingEnabled || DebugSettings::singleton().m_unk0x73;
		}

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

		// FUNCTION: PLAYER 0x00745070
		// FUNCTION: STUDIO 0x007d78d0
		void CodeProfiler::log(bool frameTick, double wallTimeElapsed)
		{
			Time time = Time::now<Time::Benchmark>();
			Time::Interval sampleTimeElapsed = time - lastSampleTime;
			if (sampleTimeElapsed >= bucketTimeSpan)
			{
				currentBucket++;
				currentBucket %= buckets.size();
				buckets[currentBucket].frames = frameTick ? 1 : 0;
				buckets[currentBucket].sampleTimeElapsed = sampleTimeElapsed.seconds();
				buckets[currentBucket].wallTimeSpan = wallTimeElapsed;
			}
			else
			{
				if (frameTick)
					buckets[currentBucket].frames++;
				buckets[currentBucket].sampleTimeElapsed += sampleTimeElapsed.seconds();
				buckets[currentBucket].wallTimeSpan += wallTimeElapsed;
			}
			lastSampleTime = time;
		}













		// FUNCTION: PLAYER 0x007452a0
		// FUNCTION: STUDIO 0x007d7b00
		Bucket Profiler::getWindow(double window) const
		{
			Bucket result;

			if (!isEnabled() && !DebugSettings::singleton().m_unk0x73)
				return result;

			const int firstBucket = currentBucket - 1 + buckets.size();

			const size_t count = std::min((size_t)std::max((int)(window / bucketTimeSpan.seconds()), 1), buckets.size() - 2);
			for (size_t i = 0; i < count; ++i)
			{
				int index = (firstBucket - i) % buckets.size();
				RBXASSERT(index>=0);
				result += buckets[index];
				if (result.sampleTimeElapsed >= window)
					break;
			}

			return result;
		}

		// FUNCTION: PLAYER 0x007453f0
		// FUNCTION: STUDIO 0x007d7c50
		Bucket Profiler::getFrames(int frames) const
		{
			Bucket result;

			if (!isEnabled() && !DebugSettings::singleton().m_unk0x73)
				return result;

			const int firstBucket = currentBucket - 1 + buckets.size();
			const size_t count = std::min((size_t)frames, buckets.size() - 2);
			for (size_t i = 0; i < count; ++i)
			{
				int index = (firstBucket - i) % buckets.size();
				RBXASSERT(index>=0);
				result += buckets[index];
			}

			return result;
		}

		// FUNCTION: PLAYER 0x00745020
		// FUNCTION: STUDIO 0x007d7880
		double Bucket::getActualFPS() const
		{
			return sampleTimeElapsed > 0.0 ? frames / sampleTimeElapsed : 0;
		}

		// FUNCTION: PLAYER 0x00745030
		// FUNCTION: STUDIO 0x007d7890
		double Bucket::getNominalFPS() const
		{
			return wallTimeSpan > 0.0 ? frames / wallTimeSpan : 0;
		}

		// FUNCTION: PLAYER 0x00745050
		// FUNCTION: STUDIO 0x007d78b0
		double Bucket::getNominalFramePeriod() const
		{
			return frames > 0 ? wallTimeSpan / frames : 0;
		}

		// FUNCTION: PLAYER 0x007454f0
		// FUNCTION: STUDIO 0x007d7d50
		Mark::Mark(CodeProfiler& section, bool frameTick, bool logInclusive)
			: section(section)
			, frameTick(frameTick)
			, enabled(profilingEnabled || DebugSettings::singleton().m_unk0x73)
			, logInclusive(logInclusive)
		{
			if (enabled)
			{
				startTime = Time::now<Time::Benchmark>();
				enclosingMark = (Mark*)TlsGetValue(markTlsIndex);
				TlsSetValue(markTlsIndex, this);
			}
		}

		// STUB: PLAYER 0x00745150
		// STUB: STUDIO 0x007d79b0
		Mark::~Mark()
		{
			if (enabled)
			{
				TlsSetValue(markTlsIndex, enclosingMark);
				Time::Interval elapsed = Time::now<Time::Benchmark>() - startTime;
				section.log(frameTick, logInclusive ? elapsed.seconds() : (elapsed - childrenElapsed).seconds());
				if (enclosingMark)
					enclosingMark->childrenElapsed += elapsed;
			}
		}

	} // namespace Profiling
} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::Profiling::CodeProfiler, 0xc040);
DECOMP_SIZE_ASSERT(RBX::Profiling::Mark, 0x30);
