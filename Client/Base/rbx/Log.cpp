/* Base/rbx/Log.cpp -- October 2013 (Log helpers).
   formatMem 0x14588A (batches 612-613): B/%.1fKB/%.0fKB/%.1fMB/
   %.0fMB/%.0fGB ladder over decimal 1000s (C1C900=1000.0,
   C2F1A0=1e6/C2F198=1e9 -- all three VERIFIED by batch621 rodata read).
   Declared in Base/include/rbx/Log.h (pre-existing stub header).
   formatTime queues (no EA banked). */

#include "rbx/Log.h"
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
