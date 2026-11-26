/* Win/LogManager.h -- October 2013 client shell (PLAYER and STUDIO compile Client\Win\LogManager.cpp).
   The per-thread RBX::Log files (log_<session>.txt), the FastLog channel files, the crash-dump
   reporter and the assert/purecall hooks.

   Layouts from the PLAYER bytes (the shell is /GL, so some ctors and helpers take arguments in
   registers):
     LogManager       0x28  ThreadLogManager::getCurrent 0x00440c60 (operator new 0x28): vfptr,
                            log +0x04, threadID +0x08 (GetCurrentThreadId), name +0x0c
     MainLogManager   0x64  ctor 0x0043fa50: ILogProvider vfptr +0x00, LogManager +0x04,
                            crashReporter +0x2c, fastLogChannels +0x30, crashExtention +0x40
                            (".Client.dmp"), crashEventExtention +0x44 (".Client.crashevent"),
                            guid +0x48. 2013 has no gameState (2016's setLeaveGame).
   CrashReporter is Client/Network's (Network/CrashReporter.h: ctor 0x00520070, dtor 0x005200f0,
   ProcessException 0x0051fcb0, GenerateDmpFileName 0x0051fad0 with three parameters).

   LogManager.cpp:
   2013 path from the assert in the MainLogManager constructor (`mainLogManager == NULL`, line 354);
   the line is reproduced by layout, never by #line. The shell is /GL: the linker generates these
   bodies through LTCG (custom register conventions, no .obj order).

   PLAYER evidence, by function:
     DoGetPath 0x0043ea30, InitPath 0x0043eb70 (a tail jump), MainLogManager::fastLogMessage
     0x0043ec40, getSessionId 0x0043ed90, getLogFileName 0x0043efb0, getFastLogFileName 0x0043f0f0,
     MakeLogFileName 0x0043f230, ThreadLogManager::getLogFileName 0x0043f380, LogManager::getLog
     0x0043f470, MainLogManager::provideLog 0x0043f540, RobloxCrashReporter::logEvent 0x0043f710,
     purecallHandler 0x0043f9b0, MainLogManager ctor 0x0043fa50, CullLogs 0x004403a0, dtors
     0x00440600 / 0x00440780, the assert hooks 0x00440830-0x004409c7, ReportEvent 0x00440a70,
     ThreadLogManager::getCurrent 0x00440c60.

   Against the 2016 file, 2013:
     - creates G3D's common log next to the main one (<log>_G3D.txt) and routes G3D's assertion and
       failure hooks here; ~MainLogManager deletes that log;
     - has no logsEnabled switch in getLog, no gameState, no hang reporting;
     - prints the logs folder once, the first time DoGetPath runs;
     - culls both log folders to 128 files;
     - ReportEvent has no EVENTLOG_WARNING_TYPE case (the jump table at 0x00440bb8 maps 2 to the
       default), so the debug-assert hook, which reports a warning, prints nothing. */


#pragma once

#include <windows.h>
#include <atlstr.h>
#include <atlpath.h>

#include <string>
#include <vector>

#include <boost/scoped_ptr.hpp>

#include "rbx/Log.h"
#include "rbx/threadsafe.h"
#include "FastLog.h"
#include "decomp.h"

// Client/Network's crash reporter (logEvent, ProcessException; controls at +0x20; size 0x268).
#include "Network/CrashReporter.h"

// VTABLE: PLAYER 0x00be1f3c
// VTABLE: STUDIO 0x00d0cc04
// SIZE 0x28
class LogManager
{
	RBX::Log* log;          // 0x04
protected:
	const DWORD threadID;   // 0x08
	std::string name;       // 0x0c
	static class MainLogManager* mainLogManager;
public:
	RBX::Log* getLog();

	static void ReportEvent(WORD type, LPCSTR message);

	static const ATL::CPath& GetLogPath();

	static MainLogManager* getMainLogManager() { return mainLogManager; } // inlined into WndProc (PLAYER 0x00442744)

	virtual ~LogManager();
	virtual std::string getLogFileName() = 0;

protected:
	LogManager(const char* name) : log(NULL), threadID(GetCurrentThreadId()), name(name) {}
};

// VTABLE: PLAYER 0x00be1f5c
// VTABLE: STUDIO 0x00d0cc24
// SIZE 0x268
class RobloxCrashReporter : public CrashReporter
{
public:
	static bool silent;
	RobloxCrashReporter(const char* outputPath, const char* appName, const char* crashExtention);
	LONG ProcessException(struct _EXCEPTION_POINTERS* info, bool noMsg);
protected:
	/*override*/ void logEvent(const char* msg);
};

// VTABLE: PLAYER 0x00be1f48 RBX::ILogProvider
// VTABLE: PLAYER 0x00be1f50 LogManager
// VTABLE: STUDIO 0x00d0cc10 RBX::ILogProvider
// VTABLE: STUDIO 0x00d0cc18 LogManager
// SIZE 0x64
class MainLogManager
	: public RBX::ILogProvider
	, public LogManager
{
	boost::scoped_ptr<RobloxCrashReporter> crashReporter; // 0x2c
	std::vector<RBX::Log*> fastLogChannels;               // 0x30
	static RBX::mutex fastLogChannelsLock;
	const char* crashExtention;                           // 0x40
	const char* crashEventExtention;                      // 0x44
	std::string guid;                                     // 0x48
public:
	MainLogManager(LPCTSTR productName, const char* crashExtention, const char* crashEventExtention);
	~MainLogManager();

	RBX::Log* provideLog();
	virtual std::string getLogFileName();
	std::string getFastLogFileName(FLog::Channel channelId);
	std::string MakeLogFileName(const char* postfix);

	static void CullLogs(const char* folder, int filesRemaining);

	std::string getSessionId();
	std::string getCrashEventName();

	void WriteCrashDump();
	bool CreateFakeCrashDump();
	// Notifies the crash reporter that the main thread is still responsive (inlined into WndProc).
	void NotifyFGThreadAlive()
	{
		if (crashReporter)
			crashReporter->NotifyAlive();
	}

	static void fastLogMessage(FLog::Channel id, const char* message);

private:
	static bool handleDebugAssert(const char* expression, const char* filename, int lineNumber);
	static bool handleFailure(const char* expression, const char* filename, int lineNumber);
	static bool handleG3DDebugAssert(const char* expression, const std::string& message, const char* filename, int lineNumber, bool useGuiPrompt);
	static bool handleG3DFailure(const char* expression, const std::string& message, const char* filename, int lineNumber, bool useGuiPrompt);
};

// VTABLE: PLAYER 0x00be1f68
// VTABLE: STUDIO 0x00d0cc30
class ThreadLogManager
	: public LogManager
{
	ThreadLogManager();
public:
	static ThreadLogManager* getCurrent();
	virtual ~ThreadLogManager();
protected:
	virtual std::string getLogFileName();
};
