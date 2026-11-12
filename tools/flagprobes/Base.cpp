// probe: Base (PLAYER Base range 0x9b1000-0x9bd680). Minimal local declarations; bodies after the
// 2016 source, checked against the 2013 bytes.
#include <windows.h>
#include <stdarg.h>
#include <string>
namespace RBX {
class Time {
public:
  class Interval { double sec; public: Interval() : sec(0) {} double seconds() const { return sec; } };
  Time() : sec(0) {}
  enum SampleMethod { Fast, Benchmark, Precise, Multimedia };
  template<SampleMethod sampleMethod> static Time now();
  static Time nowFast();
  static double nowFastSec();
  double timestampSeconds() const { return sec; }
private:
  double sec;
};
Time Time::nowFast() { return now<Time::Fast>(); }   // inlined into nowFastSec (same TU, Time.cpp)
double Time::nowFastSec() { return Time::nowFast().timestampSeconds(); }   // 0x9bb4d0

class TaskScheduler { public: class Job { public:
  struct Stats { Time::Interval timespanSinceLastStep; };
  struct Error { double error; bool urgent; Error() : error(0.0), urgent(false) {} Error(double error) : error(error), urgent(false) {} };
  Error computeStandardError(const Stats& stats, double desiredHz);
}; };
TaskScheduler::Job::Error TaskScheduler::Job::computeStandardError(const Stats& stats, double desiredHz)   // 0x9b98b0
{
  return stats.timespanSinceLastStep.seconds() * desiredHz;
}

class Debugable { public: static volatile bool doCrashEnabled; static void doCrash(const char*); };
#pragma optimize("", off)
void Debugable::doCrash(const char* message)   // 0x9b1a00
{
  if (doCrashEnabled)
    DebugBreak();
}
#pragma optimize("", on)

std::string vformat(const char* fmt, va_list argPtr);
std::string format(const char* fmt, ...)   // 0x9b5ba0
{
  va_list argList;
  va_start(argList, fmt);
  std::string result = vformat(fmt, argList);
  va_end(argList);
  return result;
}

class ExclusiveArbiter { public: virtual std::string arbiterName(); };
std::string ExclusiveArbiter::arbiterName() { return "ExclusiveArbiter"; }   // 0x9b3950
}

// boost c_time::localtime (0x9b1240), instantiated in a Base TU: an EH frame with no cookie (/GS-).
#include <boost/date_time/c_time.hpp>
std::tm* (*g_probe_localtime)(const std::time_t*, std::tm*) = &boost::date_time::c_time::localtime;

// RBX::Log::formatMem(unsigned int) (0x9b0f50, Log.cpp; strings "%dB" .. "%.0fGB"): a 64-byte char buffer with no
// cookie (/GS-) and unsigned -> double through fild + 2^32 fix-up (x87 doubles: not /arch:SSE2).
#include <stdio.h>
namespace RBX {
class Log { public: static std::string formatMem(unsigned int bytes); };
std::string Log::formatMem(unsigned int bytes)
{
  char buffer[64];
  if (bytes < 1000)                 _snprintf(buffer, 64, "%dB", bytes);
  else if (bytes < 100000)          _snprintf(buffer, 64, "%.1fKB", bytes / 1000.0);
  else if (bytes < 1000000)         _snprintf(buffer, 64, "%.0fKB", bytes / 1000.0);
  else if (bytes < 100000000)       _snprintf(buffer, 64, "%.1fMB", bytes / 1000000.0);
  else if (bytes < 1000000000)      _snprintf(buffer, 64, "%.0fMB", bytes / 1000000.0);
  else if (bytes < 100000000000)    _snprintf(buffer, 64, "%.1fGB", bytes / 1000000000.0);
  else                              _snprintf(buffer, 64, "%.0fGB", bytes / 1000000000.0);
  return buffer;
}
}

// RBX::Log::formatTime(double) (0x9b10e0): double compares through fucomip/fcomip (/arch:SSE: P6 compares, not
// fnstsw; not SSE2 comisd).
namespace RBX {
class Log2 { public: static std::string formatTime(double time); };
std::string Log2::formatTime(double time)
{
  char buffer[64];
  if (time == 0.0)
    _snprintf(buffer, 64, "0s");
  if (time < 0.0)          _snprintf(buffer, 64, "%.3gs", time);
  else if (time >= 0.1)    _snprintf(buffer, 64, "%.3gs", time);
  else                     _snprintf(buffer, 64, "%.3gms", time * 1000.0);
  return buffer;
}
}
