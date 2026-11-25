/* Network/CrashReporter.h -- October 2013.
   Layout from CrashReporter::CrashReporter (PLAYER 0x00520070, STUDIO 0x005d78c0) and ~CrashReporter
   (0x005200f0 / 0x005d7940): vfptr, threadResult 0x04, exceptionInfo 0x08, reportCrashEvent 0x0c
   (CEvent(false)), watcherThread 0x10 (scoped_ptr<boost::thread>), hangReportingEnabled 0x14,
   isAlive 0x15, deadlockCounter 0x18, destructing 0x1c, immediateUploadEnabled 0x1d, controls 0x20.
   Size 0x268 (RobloxCrashReporter adds nothing; operator new 0x268 in the client shell).
   vftable 0x00b11ae0: logEvent (the shared empty `ret 4` 0x00575d00, so inline and empty here),
   ProcessException. No virtual destructor.
   Against 2016: GenerateDmpFileName has no fullDmp parameter (ret 0xc), there is no
   ProcessExceptionHelper, DisableHangReporting or silentCrashReporting member.
   No 2013 assert names this file; the path is 2016's (Network/include/network/CrashReporter.h). */

#pragma once

#include <windows.h>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>
#include "rbx/CEvent.h"

// The parameters Start's dumps use; RobloxCrashReporter's ctor fills them.
struct CrashReportControls
{
	char appName[128];        // 0x00
	char appVersion[128];     // 0x80
	char pathToMinidump[260]; // 0x100 directory only
	char crashExtention[64];  // 0x204
	int minidumpType;         // 0x244 a MINIDUMP_TYPE
};

// PLAYER 0x0051f7b0 (STUDIO does not link it).
void fixExceptionsThroughKernel();

// VTABLE: PLAYER 0x00b11ae0
// VTABLE: STUDIO 0x00bcace0
// SIZE 0x268
class CrashReporter
{
private:
	LONG threadResult;                               // 0x04
	struct _EXCEPTION_POINTERS* exceptionInfo;       // 0x08
	RBX::CEvent reportCrashEvent;                    // 0x0c
	boost::scoped_ptr<boost::thread> watcherThread;  // 0x10
	bool hangReportingEnabled;                       // 0x14
	bool isAlive;                                    // 0x15
	LONG deadlockCounter;                            // 0x18
	bool destructing;                                // 0x1c
	bool immediateUploadEnabled;                     // 0x1d

	void LaunchUploadProcess();

protected:
	virtual void logEvent(const char* msg) {}       // vtable+0x00

public:
	static CrashReporter* singleton;
	CrashReportControls controls;                    // 0x20

	CrashReporter();
	~CrashReporter();

	void Start();
	void WatcherThreadFunc();
	virtual LONG ProcessException(struct _EXCEPTION_POINTERS* ExceptionInfo, bool noMsg); // vtable+0x04
	LONG ProcessExceptionInThead(struct _EXCEPTION_POINTERS* ExceptionInfo);
	void EnableImmediateUpload(bool enabled);
	// Called from the main thread now and then to show the app is responsive.
	void NotifyAlive();

	HRESULT GenerateDmpFileName(char* dumpFilepath, int cchdumpFilepath, bool fastLog = false);
};
