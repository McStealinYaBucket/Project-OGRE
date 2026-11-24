/* Util/Profiling.h -- October 2013.
   Profiler and CodeProfiler from the 2013 bytes: Profiler ctor PLAYER 0x007451d0 (vftable 0x00b526d4),
   CodeProfiler ctor 0x00745230 (vftable 0x00b52790), both one slot holding the shared deleting
   destructor 0x00745250; every user news 0xc040.
     0x08 bucketTimeSpan = profilingWindow (0x00d5cec0, 3.0) * 0.003
     0x10 currentBucket, 0x18 buckets (0x800 Buckets of 0x18: two doubles and an int),
     0xc018 lastSampleTime, 0xc020 name (std::string); size 0xc040 by the double alignment.
   Mark is still a no-op shell (the 2013 Mark takes a CodeProfiler&; not rebuilt yet).
   Original file: Client/App/include/Util/Profiling.h (the .cpp's 2013 assert path is
   Client\App\util\Profiling.cpp). */

#pragma once

#include "rbx/rbxtime.h"
#include "boost/array.hpp"
#include <string>

namespace RBX {
	namespace Profiling {

		// SIZE 0x18
		struct Bucket
		{
			double sampleTimeElapsed;	// 0x00
			double wallTimeSpan;		// 0x08
			int frames;					// 0x10

			Bucket()
				: sampleTimeElapsed(0)
				, wallTimeSpan(0)
				, frames(0)
			{
			}
		};

		// VTABLE: PLAYER 0x00b526d4
		// VTABLE: STUDIO 0x00c0be58
		// SIZE 0xc040
		class Profiler
		{
		protected:
			const Time::Interval bucketTimeSpan;	// 0x08
			int currentBucket;						// 0x10
			boost::array<Bucket, 0x800> buckets;	// 0x18
			Time lastSampleTime;					// 0xc018

		public:
			const std::string name;					// 0xc020

			Profiler(const char* name);
			virtual ~Profiler() {}

			static double profilingWindow;
		};

		// VTABLE: PLAYER 0x00b52790
		// VTABLE: STUDIO 0x00c0bf18
		// SIZE 0xc040
		class CodeProfiler : public Profiler
		{
		public:
			CodeProfiler(const char* name);

			// Both vftables hold this one body (Profiler's is identical and folded into it).
			// SYNTHETIC: PLAYER 0x00745250
			// SYNTHETIC: STUDIO 0x007d7ab0
			// RBX::Profiling::CodeProfiler::`scalar deleting destructor'

		};

		// Binary takes CodeProfiler& (never NULL there); ours tolerates
		// NULL -- same net timing state (unobserved), no crash.
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
