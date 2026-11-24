/* Util/Profiling.h -- October 2013.
   Profiler and CodeProfiler from the 2013 bytes: Profiler ctor PLAYER 0x007451d0 (vftable 0x00b526d4),
   CodeProfiler ctor 0x00745230 (vftable 0x00b52790), both one slot holding the shared deleting
   destructor 0x00745250; every user news 0xc040.
     0x08 bucketTimeSpan = profilingWindow (0x00d5cec0, 3.0) * 0.003
     0x10 currentBucket, 0x18 buckets (0x800 Buckets of 0x18: two doubles and an int),
     0xc018 lastSampleTime, 0xc020 name (std::string); size 0xc040 by the double alignment.
   Mark (0x30): ctor 0x007454f0 and dtor 0x00745150 are out of line in Profiling.obj.
   Original file: Client/App/include/Util/Profiling.h (the .cpp's 2013 assert path is
   Client\App\util\Profiling.cpp). */

#pragma once

#include "rbx/rbxtime.h"
#include "boost/array.hpp"
#include <string>

namespace RBX {
	namespace Profiling {

		void init(bool enabled);
		void setEnabled(bool enabled);
		bool isEnabled();

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

			Bucket& operator+=(const Bucket& b)
			{
				sampleTimeElapsed += b.sampleTimeElapsed;
				wallTimeSpan += b.wallTimeSpan;
				frames += b.frames;
				return *this;
			}

			double getActualFPS() const;
			double getNominalFPS() const;
			double getNominalFramePeriod() const;
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

			Bucket getWindow(double window) const;
			Bucket getFrames(int frames) const;

			static double profilingWindow;
		};

		// VTABLE: PLAYER 0x00b52790
		// VTABLE: STUDIO 0x00c0bf18
		// SIZE 0xc040
		class CodeProfiler : public Profiler
		{
		public:
			CodeProfiler(const char* name);

		private:
			friend class Mark;
			void log(bool frameTick, double wallTimeElapsed);
		public:

			// Both vftables hold this one body (Profiler's is identical and folded into it).
			// SYNTHETIC: PLAYER 0x00745250
			// SYNTHETIC: STUDIO 0x007d7ab0
			// RBX::Profiling::CodeProfiler::`scalar deleting destructor'

		};

		// SIZE 0x30
		class Mark
		{
			CodeProfiler& section;				// 0x00
			Mark* enclosingMark;				// 0x04 (set only when enabled)
			bool frameTick;						// 0x08
			Time startTime;						// 0x10
			const bool enabled;					// 0x18
			Time::Interval childrenElapsed;		// 0x20
			const bool logInclusive;			// 0x28

		public:
			Mark(CodeProfiler& section, bool frameTick, bool logInclusive = false);
			~Mark();
		};

	} // namespace Profiling
} // namespace RBX
