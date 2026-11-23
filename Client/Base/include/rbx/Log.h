// Base/include/rbx/Log.h: October 2013 reconstruction (Project OGRE).
// Evidence: iOS 1029 symbols RBX::Log::provider, formatMem(unsigned int), formatTime(double),
// writeEntry(Severity, char const*) and timeStamp(std::ofstream&, bool); the PLAYER bodies at
// 0x009b0f50-0x009b1993 (Base/rbx/Log.cpp) and the vftable below (one slot: the destructor).
// Layout as Log.cpp's constructor builds it: vfptr, name, worstSeverity, logFile, stream.
// Dropped from 2016 for want of any 2013 trace or caller: aggregateWorstSeverity and the private
// currentStream()/friend Entry helper.
#ifndef RBX_LOG_H
#define RBX_LOG_H

#include <list>
#include <string>

#include "rbx/Debug.h"

#include <sstream>
#include <fstream>

namespace RBX {

class Log;

// Supplies the Log that Log::current() returns; a threaded app can hand each thread its own.
class RBXInterface ILogProvider
{
public:
	virtual Log* provideLog() = 0;
};

// A named text log written to a file, each entry time-stamped.
// VTABLE: PLAYER 0x00bb8800
// VTABLE: STUDIO 0x00c784d0
class Log
{
	const std::string name;
public:
	// "N bytes"/"KB"/"MB" style and seconds-to-text helpers.
	static std::string formatMem(unsigned int bytes);
	static std::string formatTime(double time);

	enum Severity { Information = 0, Warning = 1, Error = 2 };
	Severity worstSeverity; // worst entry written so far

	void writeEntry(Severity severity, const char* message);
	void writeEntry(Severity severity, const wchar_t* message);
	void timeStamp(bool includeDate);

	static void setLogProvider(ILogProvider* provider);

	Log(const char* logFile, const char* name);
	virtual ~Log(void);

	const std::string logFile;

	static inline Log* current() { return provider ? provider->provideLog() : NULL; }

	static void timeStamp(std::ofstream& stream, bool includeDate);

private:
	std::ofstream stream;
	static ILogProvider* provider;
};

}

#endif

// boost date_time bodies Log::timeStamp instantiates (Log.obj copies).
// TEMPLATE: PLAYER 0x009b1240
// TEMPLATE: STUDIO 0x00a68330
// ?localtime@c_time@date_time@boost@@SAPAUtm@@PB_JPAU4@@Z

// TEMPLATE: PLAYER 0x009b12b0
// TEMPLATE: STUDIO 0x00a683a0
// ?create_time@?$second_clock@Vptime@posix_time@boost@@@date_time@boost@@CA?AVptime@posix_time@3@PAUtm@@@Z
