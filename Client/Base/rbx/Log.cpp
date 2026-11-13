/* Base/rbx/Log.cpp -- October 2013 (Log helpers).
   formatMem 0x14588A (batches 612-613): B/%.1fKB/%.0fKB/%.1fMB/
   %.0fMB/%.0fGB ladder over decimal 1000s (C1C900=1000.0,
   C2F1A0=1e6/C2F198=1e9 -- all three VERIFIED by batch621 rodata read).
   Declared in Base/include/rbx/Log.h (pre-existing stub header).
   formatTime queues (no EA banked). */

#include "rbx/Log.h"
#include "FastLog.h"
#include "RbxFormat.h"
#include <time.h>
#include <stdio.h>

namespace RBX {

	// STUB: PLAYER 0x009b0f50
	// STUB: STUDIO 0x00a68040
	std::string Log::formatMem(unsigned int bytes)
	{
		if (bytes <= 999)
			return RBX::format("%dB", bytes);
		double v = (double)bytes;
		if (bytes <= 99999)
			return RBX::format("%.1fKB", v / 1000.0);
		if (bytes <= 999999)
			return RBX::format("%.0fKB", v / 1000.0);
		if (bytes <= 99999999)
			return RBX::format("%.1fMB", v / 1000000.0);
		if (bytes <= 999999999)
			return RBX::format("%.0fMB", v / 1000000.0);
		return RBX::format("%.0fGB", v / 1000000000.0);
	}

	// formatTime 0x1459D2 (batches 615/621: v<0 or v>=0.1 -> "%.3gs",
	// else v*1000 "%.3gms". Threshold is CA92E8=0.1 VERIFIED (batch621
	// rodata read; an earlier comment misguessed 1.0 -- s514 fix).
	// NOTE: the "0s" literal at the head is unreachable as-read (the
	// zero path falls through into the ms arm, yielding "0ms") --
	// possible binary dead code, preserved as behavior).
	// STUB: PLAYER 0x009b10e0
	// STUB: STUDIO 0x00a681d0
	std::string Log::formatTime(double time)
	{
		if (time < 0.0 || time >= 0.1)
			return RBX::format("%.3gs", time);
		return RBX::format("%.3gms", time * 1000.0);
	}

	// timeStamp(ofstream&, bool): binary body unrecovered from the dumped
	// material (call sites only -- batch738:1241, batch745:238,
	// batch769:1669, disasm batch758:907 show the (ofstream&, bool) shape
	// and the "timestamp-prefix then message" flow). 2013-conventional
	// port form: "[YYYY/MM/DD HH:MM:SS] " with the date part only when
	// includeDate. LOUD DELTA (exact fmt bytes unrecovered).
	void Log::timeStamp(std::ofstream& stream, bool includeDate)
	{
		time_t timer = time(NULL);
		struct tm tmData;
		if (localtime_s(&tmData, &timer) != 0)
			return;

		char stamp[32];
		size_t stampLen;
		if (includeDate)
			stampLen = (size_t)_snprintf(stamp, sizeof(stamp), "[%04d/%02d/%02d %02d:%02d:%02d] ",
				tmData.tm_year + 1900, tmData.tm_mon + 1, tmData.tm_mday,
				tmData.tm_hour, tmData.tm_min, tmData.tm_sec);
		else
			stampLen = (size_t)_snprintf(stamp, sizeof(stamp), "[%02d:%02d:%02d] ",
				tmData.tm_hour, tmData.tm_min, tmData.tm_sec);
		stream.write(stamp, stampLen);
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
