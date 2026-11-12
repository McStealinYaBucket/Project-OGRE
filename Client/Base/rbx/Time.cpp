/* Base/rbx/Time.cpp -- October 2013 RBX::Time (the one clock in the client).
   The HEADER (Client/Base/include/rbx/rbxtime.h) is the authentic first-party
   2013 class and is unchanged apart from the addition of the free getStart()
   declaration that the database proves (see the end of that header).

   This file was a link-debt subset that used QueryPerformanceCounter as a
   stand-in.  The exact bodies are now decoded, and they use POSIX
   gettimeofday, so the QPC substitute is gone.

   EVIDENCE (symbolled 0.126.0.39503 Studio, idalib; Mach-O so the clock is
   the POSIX one -- this is the same client that ships as a Windows binary,
   and its Time::now<2> body is the one that runs on both):

     0x15310E  RBX::getStart()                                        134 bytes
     0x153194  RBX::Time RBX::Time::now<(RBX::Time::SampleMethod)2>() 125 bytes
     0x153212  RBX::Time RBX::Time::now<(RBX::Time::SampleMethod)0>()   9 bytes
     0x153240  RBX::Time RBX::Time::now<(RBX::Time::SampleMethod)1>()   9 bytes
     0x15321C  RBX::Time::nowFast()                                     9 bytes
     0x153226  RBX::Time::nowFastSec()                                 25 bytes
     0x15324B  RBX::Time::Interval RBX::operator-(Time const&, Time const&)

   0x15310E, verbatim:
       if (!ready) { __cxa_guard_acquire(&ready);
                     gettimeofday(&tv, NULL);
                     start = tv.tv_usec + 1000000LL * tv.tv_sec;
                     __cxa_guard_release(&ready); }
       return &start;
   A __cxa_guard-protected function-local static holding the microsecond
   count, with the function returning its ADDRESS.  Both callers of
   getStart() drop the returned pointer and re-read the PIC-relative global
   instead -- which is why hexrays types the return `__int64 *` and why the
   call at 0x1531D1 is followed by a read of qword_FAD660 rather than a use
   of eax.

   0x153194, verbatim -- the ONLY clock in the client:
       timeval tv;  gettimeofday(&tv, NULL);
       long long us = tv.tv_usec + 1000000LL * tv.tv_sec;
       getStart();
       return (double)(us - *getStart()) * 1.0e-6;      // rodata 0xC30150
   The result is returned in the EDX:EAX pair, i.e. a Time is one bare
   double (the header's `private: double sec;`), and the scaling is a
   MULTIPLY by the rodata double 1e-6, not a divide.

   0x153212, 0x15321C and 0x153240 are all nine bytes:
       push ebp; mov ebp,esp; pop ebp; jmp now<2>
   So now<0>, now<1> and nowFast are the SAME clock as now<2> in this build
   -- there is no mm-timer fast path and no separate QPC source, whatever
   the header's commentary about preciseOverride suggests.  (Which enumerator
   is 2 is not recoverable from a mangled template argument and does not
   matter: every one of them is a tail jump to the same body.)

   0x153226 nowFastSec(): calls now<2> and reloads the returned 8-byte value
   through the x87 stack before returning it.  The header declares
   `static double nowFastSec();`, and a `double` return on x86-32 comes back
   in st0 -- so that reload IS the body, and it is the same number.

   DELTA: the microsecond source on Win32.  The binary imports _gettimeofday
   out of libSystem.  The Win32 CRT this tree builds with has no
   <sys/time.h> and no gettimeofday -- probe-verified against the VS12
   toolchain at D:\BuildAgent\VS12 -- so clockMicroseconds() below reads the
   same quantity out of GetSystemTimeAsFileTime, which is the kernel call
   gettimeofday is itself layered over: FILETIME counts 100ns ticks since
   1601-01-01, so /10 lands in microseconds.  Same unit, same magnitude, and
   the only thing any caller does with the result is subtract it from
   another read of the same clock.

   NOT PORTED (declared in the header, no body in the database, nothing in
   the tree calls them): Time::isSpeedCheater, Time::isDebugged.  They are
   left undefined on purpose -- inventing them would be fabrication.  Neither
   is virtual, so no vtable is emitted and their absence cannot produce a
   link error unless a caller appears.

   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/Base/rbx/Time.cpp */

#include "rbx/rbxtime.h"

#include <windows.h>

namespace RBX {

	// Declared in the header; absent from the 2013 database, so nothing in
	// the tree can be calling it.  It is defined here only because the
	// 2016 header declares it and a definition costs nothing.
	Time::SampleMethod Time::preciseOverride = Time::Precise;

	// The microsecond clock.  This IS the `tv.tv_usec + 1000000LL * tv.tv_sec`
	// fold that both 0x15310E and 0x153194 do inline.
	//
	// DELTA: see the file header.  The Win32 CRT has neither <sys/time.h> nor
	// gettimeofday, so the same microsecond count is read from
	// GetSystemTimeAsFileTime, the kernel call gettimeofday is layered over.
	static long long clockMicroseconds()
	{
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);

		ULARGE_INTEGER ticks;
		ticks.LowPart  = ft.dwLowDateTime;
		ticks.HighPart = ft.dwHighDateTime;
		return (long long)(ticks.QuadPart / 10ULL);
	}

	// mac: 0x15310E (134 bytes).  Returns the ADDRESS of the lazily captured start
	// timestamp, exactly as the binary does.
	long long* getStart()
	{
		static long long start = clockMicroseconds();
		return &start;
	}

	// The header's by-value spelling of the same thing.  Delegates so there is
	// exactly one start timestamp in the client.
	long long Time::getStart()
	{
		return *::RBX::getStart();
	}

	// Declared in the header, no body in the database, no caller in the tree.
	// Kept at the QPC tick rate because that is what a "tick count" means;
	// nothing reaches it, and the one real clock never uses it.
	long long Time::getTickCount()
	{
		LARGE_INTEGER ticks;
		QueryPerformanceCounter(&ticks);
		return ticks.QuadPart;
	}

	// mac: 0x153194 (125 bytes).  The SampleMethod enumerators are
	// { Fast, Benchmark, Precise, Multimedia } (rbxtime.h:91), so 2 is
	// Precise -- and Precise is the ONE body the database has.
	template<>
	Time Time::now<Time::Precise>()
	{
		long long microseconds = clockMicroseconds();
		// The header's Time(double) ctor is PROTECTED (rbxtime.h:159),
		// which is reachable here only because this is a static member of
		// Time -- the same reason the 2013 source can build a Time from a
		// raw double at all.
		return Time((double)(microseconds - *::RBX::getStart()) * 1.0e-6);
	}

	// mac: 0x153212 (9 bytes): `jmp now<2>`.  There is no separate fast clock.
	template<>
	Time Time::now<Time::Fast>()
	{
		return now<Time::Precise>();
	}

	// mac: 0x153240 (9 bytes): `jmp now<2>`.
	template<>
	Time Time::now<Time::Benchmark>()
	{
		return now<Time::Precise>();
	}

	// mac: 0x15321C (9 bytes): `jmp now<2>`.
	Time Time::nowFast()
	{
		return now<Time::Precise>();
	}

	// mac: 0x153226 (25 bytes): call now<2>, then reload the 8-byte result through
	// the x87 stack.  A `double` return comes back in st0, so the reload is
	// the conversion -- the same number nowFast() gives.
	// STUB: PLAYER 0x009bb4d0
	// STUB: STUDIO 0x00a72300
	double Time::nowFastSec()
	{
		return now<Time::Precise>().sec;
	}

	// mac: 0x15324B: the free `operator-(Time, Time)` the header declares as a
	// friend (rbxtime.h:148) -- "subtract two timestamps to get the time
	// Interval between" them.
	Time::Interval operator-(const Time& t1, const Time& t0)
	{
		return Time::Interval(t1.sec - t0.sec);
	}

} // namespace RBX
