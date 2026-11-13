/* Base/rbx/Time.cpp -- October 2013, Windows.
   The 2013 Windows bodies, from PLAYER 0x009bb050-0x009bb707 (STUDIO 0x00a71ea0-0x00a72536; STUDIO drops isSpeedCheater/isDebugged), in
   their .obj order: checkSpeedHack 0x009bb050, getStart 0x009bb130, directCallback 0x009bb180,
   interpolatedCallback 0x009bb230, now<Precise> 0x009bb400, isSpeedCheater 0x009bb490,
   isDebugged 0x009bb4a0, nowFast 0x009bb4b0, nowFastSec 0x009bb4d0, operator- 0x009bb4f0,
   startMMTimer 0x009bb510, now<Fast> 0x009bb680, now<Benchmark> 0x009bb6e0.
   Where 2013 disagrees with 2016: no tick_resolution() static (every read divides by
   QueryPerformanceFrequency), the speed-hack constants are literals (1000/300/5, no FInt),
   recguard and inited go through RbxInterlockedExchange 0x009b8f20 (no rbx::atomic), and there
   is no now<Multimedia>, now(SampleMethod) or Interval::sleep in this object. The file-scope
   variables have external linkage here only so their GLOBAL markers pair (reccmp matches a
   namespace-scope static by its bare name).
   The multimedia-timer callbacks and startMMTimer are written from the bytes but not measured yet.
   Original file: Client/Base/rbx/Time.cpp */

// This prevent inclusion of winsock.h in windows.h, which prevents windows redifinition errors
#ifdef _WIN32
#define _WINSOCKAPI_
#endif

#include "rbx/rbxtime.h"
#include "rbx/debug.h"
#include "rbx/atomic.h"
#include <stdexcept>
#include "FastLog.h"

#include "Mmsystem.h"
#pragma comment (lib, "Winmm.lib")

namespace RBX
{
	// GLOBAL: PLAYER 0x00e1b620
	// GLOBAL: STUDIO 0x00f9e808
	volatile double currentSeconds = 0;
	// GLOBAL: PLAYER 0x00e1b628
	// GLOBAL: STUDIO 0x00f9e810
	volatile bool cheater = false;
	// GLOBAL: PLAYER 0x00e1b629
	// GLOBAL: STUDIO 0x00f9e811
	volatile bool isDebuggedValue = false;

	static const int correctionsPerSecond = 10;	// used for interpolation
	static const double secondsPerCorrection = 1.0 / correctionsPerSecond;

	// GLOBAL: PLAYER 0x00e1b610
	// GLOBAL: STUDIO 0x00f9e7f8
	double millisecondsPerTick;
	// GLOBAL: PLAYER 0x00e1b608
	// GLOBAL: STUDIO 0x00f9e7f0
	long ticksPerCorrection;

	// stuff to prevent GameCheat Speed hack
	// GLOBAL: PLAYER 0x00e1b62c
	// GLOBAL: STUDIO 0x00f9e814
	long lastSpeedCheckCounter = 0;
	// GLOBAL: PLAYER 0x00e1b630
	// GLOBAL: STUDIO 0x00f9e818
	long violationsCount = 0;
	// GLOBAL: PLAYER 0x00e1b618
	// GLOBAL: STUDIO 0x00f9e800
	LARGE_INTEGER prevSpeedHackCheckpointTime;
	// GLOBAL: PLAYER 0x00e1b634
	// GLOBAL: STUDIO 0x00f9e81c
	long prevTickTime = 0;

	// GLOBAL: PLAYER 0x00e1b638
	// GLOBAL: STUDIO 0x00f9e820
	volatile long recguard;

	template<> Time Time::now<Time::Precise>();
	template<> Time Time::now<Time::Fast>();

	static LARGE_INTEGER getSysTime()
	{
		SYSTEMTIME stime;
		GetSystemTime(&stime);
		FILETIME ftime;
		SystemTimeToFileTime(&stime, &ftime);
		LARGE_INTEGER r;
		r.HighPart = ftime.dwHighDateTime;
		r.LowPart = ftime.dwLowDateTime;
		return r;
	}

	// FUNCTION: PLAYER 0x009bb050
	// FUNCTION: STUDIO 0x00a71ea0
	static void checkSpeedHack()
	{
		lastSpeedCheckCounter ++;
		if (lastSpeedCheckCounter == 1000)
		{
			LARGE_INTEGER curTime = getSysTime();
			LARGE_INTEGER delta;
			delta.QuadPart = curTime.QuadPart - prevSpeedHackCheckpointTime.QuadPart;
			FILETIME ftime;
			ftime.dwHighDateTime = delta.HighPart;
			ftime.dwLowDateTime = delta.LowPart;
			SYSTEMTIME stime;
			FileTimeToSystemTime(&ftime, &stime);

			long curTickTime = timeGetTime();
			long td = curTickTime - prevTickTime;
			long sd = 1000*stime.wSecond + stime.wMilliseconds;

			if (abs(td - sd) > 300)
			{
				violationsCount ++;
			}
			else
			{
				violationsCount = 0;
			}

			if (violationsCount >= 5)
			{
				cheater = true;
			}

			prevSpeedHackCheckpointTime.QuadPart = curTime.QuadPart;
			prevTickTime = curTickTime;
			lastSpeedCheckCounter = 0;
		}
	}

	static inline void checkDbg()
	{
		DWORD dw = 0;
		__asm
		{
			push eax    // Preserve the registers
			push ecx
			mov eax, fs:[0x18]  // Get the TIB's linear address
			mov eax, dword ptr [eax + 0x30]
			mov ecx, dword ptr [eax]    // Get the whole DWORD
			mov dw, ecx // Save it
			pop ecx // Restore the registers
			pop eax
		}

		// The 3rd byte is the byte we really need to check for the
		// presence of a debugger.
		if (dw & 0x00010000)
		{
			isDebuggedValue = true;
		}
	}

	long long Time::getTickCount()
	{
		LARGE_INTEGER ticks;
		QueryPerformanceCounter(&ticks);
		return ticks.QuadPart;
	}

	static inline double tickFrequency()
	{
		LARGE_INTEGER tickFreq;
		QueryPerformanceFrequency(&tickFreq);
		return (double)tickFreq.QuadPart;
	}

	// Returns the address of the start tick count (0x00e1b640, guard 0x00e1b648).
	// STUB: PLAYER 0x009bb130
	// STUB: STUDIO 0x00a71f80
	long long* getStart()
	{
		static const long long start = Time::getTickCount();
		return const_cast<long long*>(&start);
	}

	// STUB: PLAYER 0x009bb180
	// STUB: STUDIO 0x00a71fd0
	void CALLBACK directCallback(UINT, UINT, DWORD, DWORD, DWORD)
	{
		if (RbxInterlockedExchange(&recguard, 1) == 1)
			return; // we are re-entering, ignore this call.

		double t = (Time::getTickCount() - *getStart()) / tickFrequency();
		if (t>currentSeconds)
			currentSeconds = t;

		checkSpeedHack();
		checkDbg();

		RbxInterlockedExchange(&recguard, 0);
	}

	// STUB: PLAYER 0x009bb230
	// STUB: STUDIO 0x00a72080
	void CALLBACK interpolatedCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
	{
		if (RbxInterlockedExchange(&recguard, 1) == 1)
			return; // we are re-entering, ignore this call.

		static long long last = *getStart();
		static long mmticks = 0;
		static double secsPerTick = millisecondsPerTick / 1000.0;

		int ticksSinceLastCorrection = mmticks % ticksPerCorrection;

		double t;
		if (ticksSinceLastCorrection == 0)
		{
			long long current = Time::getTickCount();
			double elapsedTime = (current - last) / tickFrequency();

			// Make sure that elapsedTime is reasonable.
			// If it isn't then we may have a bad sample.
			if (elapsedTime > 1.05 * secondsPerCorrection)
				{}
			else if (elapsedTime < 0.95 * secondsPerCorrection)
				{}
			else
				secsPerTick = elapsedTime / (double) ticksPerCorrection;

			last = current;
			t = (current - *getStart()) / tickFrequency();
		}
		else
		{
			t = (last - *getStart()) / tickFrequency() + secsPerTick * (double) ticksSinceLastCorrection;
		}

		if (t>currentSeconds)
			currentSeconds = t;

		mmticks++;

		checkSpeedHack();
		checkDbg();

		RbxInterlockedExchange(&recguard, 0);
	}

	// STUB: PLAYER 0x009bb400
	// STUB: STUDIO 0x00a72250
	template<>
	Time Time::now<Time::Precise>()
	{
		Time result;
		result.sec = (getTickCount() - *RBX::getStart()) / tickFrequency();
		return result;
	}

	// FUNCTION: PLAYER 0x009bb490
	bool Time::isSpeedCheater()
	{
		return cheater;
	}

	// FUNCTION: PLAYER 0x009bb4a0
	bool Time::isDebugged()
	{
		return isDebuggedValue;
	}

	// FUNCTION: PLAYER 0x009bb4b0
	// FUNCTION: STUDIO 0x00a722e0
	Time Time::nowFast()
	{
		return now<Time::Fast>();
	}

	// FUNCTION: PLAYER 0x009bb4d0
	// FUNCTION: STUDIO 0x00a72300
	double Time::nowFastSec()
	{
		return Time::nowFast().timestampSeconds();
	}

	// FUNCTION: PLAYER 0x009bb4f0
	// FUNCTION: STUDIO 0x00a72320
	Time::Interval operator-( const Time& t1, const Time& t0 )
	{
		const double seconds = t1.sec - t0.sec;
		return Time::Interval(seconds);
	}

	// FUNCTION: PLAYER 0x009bb510
	// FUNCTION: STUDIO 0x00a72340
	void startMMTimer()
	{
		const int targetResolution = 1;         // 1-millisecond target resolution

		TIMECAPS tc;
		if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
			throw std::runtime_error("Failed timeGetDevCaps");

		UINT timerRes = std::min<UINT>(std::max<UINT>(tc.wPeriodMin, targetResolution), tc.wPeriodMax);
		millisecondsPerTick = timerRes;
		ticksPerCorrection = 1000 / (correctionsPerSecond * timerRes);

		prevSpeedHackCheckpointTime = getSysTime();
		prevTickTime = timeGetTime();

		timeBeginPeriod(timerRes);

		MMRESULT result = timeSetEvent(
			timerRes, // delay
			timerRes, // resolution (global variable)
			ticksPerCorrection>=2 ? interpolatedCallback : directCallback,               // callback function
			NULL, // user data
			TIME_PERIODIC );  // single timer event
		if (!result)
			throw std::runtime_error("Failed timeSetEvent");
	}

	// GLOBAL: PLAYER 0x00db4b44
	// GLOBAL: STUDIO 0x00f3048c
	Time::SampleMethod Time::preciseOverride = Time::Precise;

	// FUNCTION: PLAYER 0x009bb680
	// FUNCTION: STUDIO 0x00a724b0
	template<>
	Time Time::now<Time::Fast>()
	{
		if (preciseOverride <= Fast)
			return now<Precise>();

		static long inited;	// 0x00e1b664
		if (inited==0)
			if (RbxInterlockedExchange(&inited, 1) == 0)
			{
				startMMTimer();
				FLog::Init(nowFastSec);
			}

		return Time(currentSeconds);
	}

	// FUNCTION: PLAYER 0x009bb6e0
	// FUNCTION: STUDIO 0x00a72510
	template<>
	Time Time::now<Time::Benchmark>()
	{
		if (preciseOverride <= Benchmark)
			return now<Precise>();
		else
			return now<Fast>();
	}

} // namespace RBX
