/* Base/rbx/Log.cpp -- October 2013 Log (PLAYER 0x009b0f50-0x009b1993): formatMem, formatTime,
   timeStamp, the ctor/dtor and both writeEntry overloads, written from the PLAYER bytes (2016
   Base/util/Log.cpp for intent). setLogProvider and Log::provider are not placed yet. */

#include "rbx/Log.h"
#include "FastLog.h"
#include "RbxFormat.h"
#include "rbx/rbxTime.h"
#include <stdio.h>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

namespace RBX {

	// FUNCTION: PLAYER 0x009b0f50
	// FUNCTION: STUDIO 0x00a68040
	std::string Log::formatMem(unsigned int bytes)
	{
		char buffer[64];
		if (bytes<1000)
			_snprintf(buffer, sizeof(buffer), "%dB", bytes);
		else if (bytes<100000)
			_snprintf(buffer, sizeof(buffer), "%.1fKB", ((double)bytes)/1000.0);
		else if (bytes<1000000)
			_snprintf(buffer, sizeof(buffer), "%.0fKB", ((double)bytes)/1000.0);
		else if (bytes<100000000)
			_snprintf(buffer, sizeof(buffer), "%.1fMB", ((double)bytes)/1000000.0);
		else if (bytes<1000000000)
			_snprintf(buffer, sizeof(buffer), "%.0fMB", ((double)bytes)/1000000.0);
		else if (bytes<100000000000)	// 64-bit compare in 2013 (cmp ecx,0x17 at 0x009b1053)
			_snprintf(buffer, sizeof(buffer), "%.1fGB", ((double)bytes)/1000000000.0);
		else
			_snprintf(buffer, sizeof(buffer), "%.0fGB", ((double)bytes)/1000000000.0);
		return buffer;
	}

	// FUNCTION: PLAYER 0x009b10e0
	// FUNCTION: STUDIO 0x00a681d0
	std::string Log::formatTime(double time)
	{
		char buffer[64];
		if (time==0.0)
			_snprintf(buffer, sizeof(buffer), "0s");
		if (time<0.0)
			_snprintf(buffer, sizeof(buffer), "%.3gs", time);
		else if (time>=0.1)
			_snprintf(buffer, sizeof(buffer), "%.3gs", time);
		else
			_snprintf(buffer, sizeof(buffer), "%.3gms", time*1000.0);
		return buffer;
	}

	// PLAYER bytes: second_clock::local_time (0x009b1240), three from_day_number calls for
	// day/month/year (0x009b13c0), the time of day (0x009b1190), Time::nowFastSec 0x009bb4d0.
	// FUNCTION: PLAYER 0x009b15e0
	// FUNCTION: STUDIO 0x00a686d0
	void Log::timeStamp(std::ofstream& stream, bool includeDate)
	{
		boost::posix_time::ptime stime(boost::posix_time::second_clock::local_time());
		char s[256];

		if (includeDate)
		{
			boost::gregorian::date date(stime.date());
			_snprintf(s, sizeof(s), "%02u.%02u.%u ", date.day().as_number(), date.month().as_number(), (unsigned short)date.year());
			stream << s;
		}
		boost::posix_time::time_duration dur(stime.time_of_day());
		_snprintf(s, sizeof(s), "%02u:%02u:%02u.%03u (%03.07f)", dur.hours(), dur.minutes(), dur.seconds(), (unsigned short)dur.total_milliseconds(), Time::nowFastSec());
		stream << s;
		stream.flush();
	}

	// FUNCTION: PLAYER 0x009b1740
	// FUNCTION: STUDIO 0x00a68830
	Log::Log(const char* logFile, const char* name)
		:stream(logFile)
		,logFile(logFile)
		,worstSeverity(Information)
		,name(name)
	{
		Log::timeStamp(stream, true);
		stream << "Log \"" << name << "\"\n";
		stream.flush();
	}

	// FUNCTION: PLAYER 0x009b1800
	// FUNCTION: STUDIO 0x00a688f0
	Log::~Log(void)
	{
		Log::timeStamp(stream, true);
		stream << "End Log\n";
	}

	// FUNCTION: PLAYER 0x009b1890
	// FUNCTION: STUDIO 0x00a68980
	void Log::writeEntry(Severity severity, const char* message)
	{
		static const char* error =       " Error:   ";
		static const char* warning =     " Warning: ";
		static const char* information = "          ";
		Log::timeStamp(stream, false);
		switch (severity)
		{
			case Log::Error:
				stream << error;
				break;
			case Log::Warning:
				stream << warning;
				break;
			case Log::Information:
				stream << information;
				break;
		}
		stream << message;
		stream << '\n';
		stream.flush();
	}

	// FUNCTION: PLAYER 0x009b1920
	// FUNCTION: STUDIO 0x00a68a10
	void Log::writeEntry(Severity severity, const wchar_t* message)
	{
		size_t origsize = wcslen(message) + 1;
		const size_t newsize = origsize+100;
		size_t convertedChars = 0;
		char* nstring = new char[newsize];
		wcstombs_s(&convertedChars, nstring, origsize, message, _TRUNCATE);
		if ( convertedChars >= origsize-1 )
			nstring[origsize-1] = '\0';
		writeEntry(severity, nstring);
		delete [] nstring;
	}

} // namespace RBX

// Base's log channels, in the 2013 registration order (PLAYER initializers 0x00a8b4d0-0x00a8b630, STUDIO
// 0x00b3b1a0-0x00b3b300); 2016 adds TaskSchedulerSteps and KernelStats. LOGVARIABLE written out so each
// channel can carry its GLOBAL marker (the unresolved-symbol list had them all, FLog::Asserts included).
namespace FLog
{
	// GLOBAL: PLAYER 0x00db4084
	// GLOBAL: STUDIO 0x00f2f9d4
	Channel Crash = 1;
	int CrashIniter = RegisterLogGroup("Crash", &Crash, NULL, FASTVARTYPE_STATIC);
	// GLOBAL: PLAYER 0x00e1aa88
	// GLOBAL: STUDIO 0x00f9dc78
	Channel HangDetection = 0;
	int HangDetectionIniter = RegisterLogGroup("HangDetection", &HangDetection, NULL, FASTVARTYPE_STATIC);
	// GLOBAL: PLAYER 0x00e1aa89
	// GLOBAL: STUDIO 0x00f9dc79
	Channel ContentProviderCleanup = 0;
	int ContentProviderCleanupIniter = RegisterLogGroup("ContentProviderCleanup", &ContentProviderCleanup, NULL, FASTVARTYPE_STATIC);
	// GLOBAL: PLAYER 0x00e1aa8a
	// GLOBAL: STUDIO 0x00f9dc7a
	Channel ISteppedLifetime = 0;
	int ISteppedLifetimeIniter = RegisterLogGroup("ISteppedLifetime", &ISteppedLifetime, NULL, FASTVARTYPE_STATIC);
	// GLOBAL: PLAYER 0x00e1aa8b
	// GLOBAL: STUDIO 0x00f9dc7b
	Channel MutexLifetime = 0;
	int MutexLifetimeIniter = RegisterLogGroup("MutexLifetime", &MutexLifetime, NULL, FASTVARTYPE_STATIC);
	// GLOBAL: PLAYER 0x00e1aa8c
	// GLOBAL: STUDIO 0x00f9dc7c
	Channel TaskScheduler = 0;
	int TaskSchedulerIniter = RegisterLogGroup("TaskScheduler", &TaskScheduler, NULL, FASTVARTYPE_STATIC);
	// GLOBAL: PLAYER 0x00e1aa8d
	// GLOBAL: STUDIO 0x00f9dc7d
	Channel TaskSchedulerInit = 0;
	int TaskSchedulerInitIniter = RegisterLogGroup("TaskSchedulerInit", &TaskSchedulerInit, NULL, FASTVARTYPE_STATIC);
	// GLOBAL: PLAYER 0x00e1aa8e
	// GLOBAL: STUDIO 0x00f9dc7e
	Channel TaskSchedulerRun = 0;
	int TaskSchedulerRunIniter = RegisterLogGroup("TaskSchedulerRun", &TaskSchedulerRun, NULL, FASTVARTYPE_STATIC);
	// GLOBAL: PLAYER 0x00e1aa8f
	// GLOBAL: STUDIO 0x00f9dc7f
	Channel TaskSchedulerFindJob = 0;
	int TaskSchedulerFindJobIniter = RegisterLogGroup("TaskSchedulerFindJob", &TaskSchedulerFindJob, NULL, FASTVARTYPE_STATIC);
	// GLOBAL: PLAYER 0x00e1aa90
	// GLOBAL: STUDIO 0x00f9dc80
	Channel Asserts = 0;
	int AssertsIniter = RegisterLogGroup("Asserts", &Asserts, NULL, FASTVARTYPE_STATIC);
	// GLOBAL: PLAYER 0x00e1aa91
	// GLOBAL: STUDIO 0x00f9dc81
	Channel FWLifetime = 0;
	int FWLifetimeIniter = RegisterLogGroup("FWLifetime", &FWLifetime, NULL, FASTVARTYPE_STATIC);
	// GLOBAL: PLAYER 0x00e1aa92
	// GLOBAL: STUDIO 0x00f9dc82
	Channel FWUpdate = 0;
	int FWUpdateIniter = RegisterLogGroup("FWUpdate", &FWUpdate, NULL, FASTVARTYPE_STATIC);
}
