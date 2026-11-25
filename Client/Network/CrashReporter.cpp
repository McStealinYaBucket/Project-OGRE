/* Network/CrashReporter.cpp -- October 2013.
   The whole file below the includes is unoptimized: every function keeps an ebp frame and spills
   this to [ebp-N] (ctor PLAYER 0x00520070), while the strsafe workers it calls (StringCchCopyA
   0x0051f9d0, StringCchCatA 0x0051fa40) are optimized, so they were defined before the switch.
   Small helpers (scoped_ptr::reset, strlen) are still expanded inline.
   2013 .obj order (PLAYER): MyDummySetUnhandledExceptionFilter 0x0051f700,
   PreventSetUnhandledExceptionFilter 0x0051f710, fixExceptionsThroughKernel 0x0051f7b0, then the
   members, ProcessExceptionStatic 0x005202d0, TopLevelExceptionFilter 0x00520370, Start 0x005205a0. */

#include <windows.h>
#include <DbgHelp.h>
#include <stdio.h>

#include <boost/bind.hpp>

#include "Network/CrashReporter.h"
#include "FileOperations.h"
#include "rbx/Log.h"
#include "rbx/Thread.hpp"
#include "FastLog.h"
#include "decomp.h"

#include <strsafe.h>

#pragma optimize("", off)

LOGGROUP(Crash)
LOGGROUP(HangDetection)

// Stand-ins: 2013 registers both from App's FastLogSettings.cpp block (initializers PLAYER
// 0x00a65ca0 / 0x00a65cc0); move them there when that file defines its flags.
namespace FFlag
{
	// GLOBAL: PLAYER 0x00de3f6c
	// GLOBAL: STUDIO 0x00f84e45
	bool ImmediateCrashUploadEnabled = false;
	int ImmediateCrashUploadEnabledIniter = FLog::RegisterFlag("ImmediateCrashUploadEnabled", &ImmediateCrashUploadEnabled, NULL, FASTVARTYPE_STATIC);
	// GLOBAL: PLAYER 0x00de3f6d
	// GLOBAL: STUDIO 0x00f84e46
	bool ImmediateHangUploadEnabled = false;
	int ImmediateHangUploadEnabledIniter = FLog::RegisterFlag("ImmediateHangUploadEnabled", &ImmediateHangUploadEnabled, NULL, FASTVARTYPE_STATIC);
}

// GLOBAL: PLAYER 0x00dd6638
// GLOBAL: STUDIO 0x00f5b2d8
CrashReporter* CrashReporter::singleton = NULL;

// Stands in for kernel32's SetUnhandledExceptionFilter once that is patched to jump here, so
// nothing loaded later can replace our filter.
// FUNCTION: PLAYER 0x0051f700
// FUNCTION: STUDIO 0x005d7050
static LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER filter)
{
	return NULL;
}

// Overwrites the start of SetUnhandledExceptionFilter with a 5-byte relative jmp to the dummy.
// FUNCTION: PLAYER 0x0051f710
// FUNCTION: STUDIO 0x005d7060
BOOL PreventSetUnhandledExceptionFilter()
{
	// Local names matter here: the unoptimized frame layout follows them.
	HMODULE hKernel32 = LoadLibraryA("kernel32.dll");
	if (hKernel32 == NULL)
		return FALSE;
	void* pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
	if (pOrgEntry == NULL)
		return FALSE;

	unsigned char newJump[100];
	DWORD dwOrgEntryAddr = (DWORD)pOrgEntry;
	dwOrgEntryAddr += 5;
	void* pNewFunc = &MyDummySetUnhandledExceptionFilter;
	DWORD dwNewEntryAddr = (DWORD)pNewFunc;
	DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;
	newJump[0] = 0xE9;
	memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));
	SIZE_T bytesWritten;
	BOOL bRet = WriteProcessMemory(GetCurrentProcess(), pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten);
	return bRet;
}

typedef BOOL (WINAPI *SetProcessUserModeExceptionPolicyFn)(DWORD flags);
typedef BOOL (WINAPI *GetProcessUserModeExceptionPolicyFn)(LPDWORD flags);

// Lets exceptions in callbacks through the kernel (Windows 7 / 2008 R2 policy API) reach our filter.
// FUNCTION: PLAYER 0x0051f7b0
void fixExceptionsThroughKernel()
{
	HMODULE kernel = LoadLibraryA("kernel32.dll");
	if (kernel)
	{
		SetProcessUserModeExceptionPolicyFn setPolicy = (SetProcessUserModeExceptionPolicyFn)GetProcAddress(kernel, "SetProcessUserModeExceptionPolicy");
		GetProcessUserModeExceptionPolicyFn getPolicy = (GetProcessUserModeExceptionPolicyFn)GetProcAddress(kernel, "GetProcessUserModeExceptionPolicy");
		if (setPolicy && getPolicy)
		{
			FASTLOG(FLog::Crash, "Found Get/SetProcessUserModeExceptionPolicy");
			DWORD flags;
			if (getPolicy(&flags))
			{
				FASTLOG1(FLog::Crash, "UserMode Exception Flags: %u. Setting CALLBACK_FILER off", flags);
				setPolicy(flags & ~1);
			}
		}
	}
}

// FUNCTION: PLAYER 0x0051f870
// FUNCTION: STUDIO 0x005d7100
void CrashReporter::EnableImmediateUpload(bool enabled)
{
	immediateUploadEnabled = enabled;
}

// Restarts this exe with -d to upload the dump; never from RCCService.
// FUNCTION: PLAYER 0x0051f890
// FUNCTION: STUDIO 0x005d7120
void CrashReporter::LaunchUploadProcess()
{
	if (!immediateUploadEnabled)
		return;

	char filename[MAX_PATH];
	DWORD size = GetModuleFileNameA(NULL, filename, MAX_PATH);
	if (size && !strstr(filename, "RCCService"))
	{
		FASTLOG(FLog::Crash, "Launching process to upload dmp.");
		STARTUPINFOA si = {0};
		si.cb = sizeof(si);
		PROCESS_INFORMATION pi;
		char cmd[256];
		sprintf_s(cmd, 256, "\"%s\" -d", filename);
		CreateProcessA(NULL, cmd, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
	}
}

// FUNCTION: PLAYER 0x0051f990
void CrashReporter::NotifyAlive()
{
	FASTLOG(FLog::HangDetection, "Letting watcher thread know we're alive");
	hangReportingEnabled = true;
	isAlive = true;
}

// <pathToMinidump>\<appName appVersion, ':' -> '.'><crashExtention or ".txt">
// FUNCTION: PLAYER 0x0051fad0
// FUNCTION: STUDIO 0x005d7320
HRESULT CrashReporter::GenerateDmpFileName(char* dumpFilepath, int cchdumpFilepath, bool fastLog)
{
	char appDescriptor[MAX_PATH];
	char dumpFilename[MAX_PATH];
	HRESULT hr = S_OK;
	StringCchPrintfA(appDescriptor, MAX_PATH, "%s %s", controls.appName, controls.appVersion);

	if (FAILED(hr = StringCchCopyA(dumpFilepath, cchdumpFilepath, controls.pathToMinidump)))
		return hr;
	WriteFileWithDirectories(dumpFilepath, 0, 0);
	AddSlash(dumpFilepath);

	unsigned i, dumpFilenameLen;
	StringCchCopyA(dumpFilename, MAX_PATH, appDescriptor);
	dumpFilenameLen = (unsigned)strlen(appDescriptor);
	for (i = 0; i < dumpFilenameLen; i++)
		if (dumpFilename[i] == ':')
			dumpFilename[i] = '.';

	if (FAILED(hr = StringCchCatA(dumpFilepath, cchdumpFilepath, dumpFilename)))
		return hr;
	if (FAILED(hr = StringCchCatA(dumpFilepath, cchdumpFilepath, fastLog ? ".txt" : controls.crashExtention)))
		return hr;
	return S_OK;
}

// Writes the minidump; on failure the file gets the error text instead.
// FUNCTION: PLAYER 0x0051fcb0
// FUNCTION: STUDIO 0x005d7500
LONG CrashReporter::ProcessException(struct _EXCEPTION_POINTERS* ExceptionInfo, bool noMsg)
{
	char dumpFilepath[MAX_PATH];
	if (FAILED(GenerateDmpFileName(dumpFilepath, MAX_PATH)))
		return EXCEPTION_CONTINUE_SEARCH;

	HANDLE hFile = CreateFileA(dumpFilepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return EXCEPTION_CONTINUE_SEARCH;

	MINIDUMP_EXCEPTION_INFORMATION eInfo;
	eInfo.ThreadId = GetCurrentThreadId();
	eInfo.ExceptionPointers = ExceptionInfo;
	eInfo.ClientPointers = FALSE;

	LONG result = EXCEPTION_EXECUTE_HANDLER;
	if (!MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, (MINIDUMP_TYPE)controls.minidumpType,
			ExceptionInfo ? &eInfo : NULL, NULL, NULL))
	{
		HRESULT hr = GetLastError();
		DWORD bytesWritten = 0;
		char msg[1024];
		StringCchPrintfA(msg, 1024, "MiniDumpWriteDump() failed with hr = 0x%08x", hr);
		WriteFile(hFile, msg, strlen(msg) + 1, &bytesWritten, NULL);
		result = EXCEPTION_CONTINUE_SEARCH;
	}
	CloseHandle(hFile);
	return result;
}

// Waits for a crash (reportCrashEvent) or three minutes; a timeout without NotifyAlive is a hang,
// dumped once through a breakpoint exception. The hang branch calls a TaskScheduler member at
// PLAYER 0x009b6730 (walks the threads at +0x194) that Base's TaskScheduler.h does not declare.
// STUB: PLAYER 0x0051fe50
// STUB: STUDIO 0x005d76a0
void CrashReporter::WatcherThreadFunc()
{
	STUB(0x0051fe50);
}

// FUNCTION: PLAYER 0x00520070
// FUNCTION: STUDIO 0x005d78c0
CrashReporter::CrashReporter()
	: threadResult(0)
	, exceptionInfo(NULL)
	, reportCrashEvent(false)
	, hangReportingEnabled(false)
	, isAlive(true)
	, deadlockCounter(0)
	, destructing(false)
	, immediateUploadEnabled(true)
{
	singleton = this;
}

// FUNCTION: PLAYER 0x005200f0
// FUNCTION: STUDIO 0x005d7940
CrashReporter::~CrashReporter()
{
	destructing = true;
	if (watcherThread)
	{
		FASTLOG(FLog::Crash, "Closing crashreporter thread on destroy");
		reportCrashEvent.Set();
		watcherThread->join();
	}
}

// Hands the exception to the watcher thread, which writes the dump with a healthy stack.
// FUNCTION: PLAYER 0x005201e0
// FUNCTION: STUDIO 0x005d7a30
LONG CrashReporter::ProcessExceptionInThead(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	FASTLOG(FLog::Crash, "Processing exception in thread");
	if (watcherThread)
	{
		exceptionInfo = ExceptionInfo;
		reportCrashEvent.Set();
		watcherThread->join();
		watcherThread.reset();
		return threadResult;
	}
	else
		return ProcessException(ExceptionInfo, false);
}

// Without exception pointers, raise a breakpoint to get some. 97.9%: only the __except scope table
// operand (0x00c80b98) is unpaired.
// STUB: PLAYER 0x005202d0
// STUB: STUDIO 0x005d7b20
static LONG ProcessExceptionStatic(PEXCEPTION_POINTERS ExceptionInfo)
{
	if (ExceptionInfo == NULL)
	{
		LONG result;
		__try
		{
			RaiseException(EXCEPTION_BREAKPOINT, 0, 0, NULL);
		}
		__except (result = ProcessExceptionStatic(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
		{
		}
		return result;
	}
	else
		return CrashReporter::singleton->ProcessExceptionInThead(ExceptionInfo);
}

// FUNCTION: PLAYER 0x00520370
// FUNCTION: STUDIO 0x005d7bc0
static LONG WINAPI TopLevelExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	LONG result = ProcessExceptionStatic(ExceptionInfo);
	_exit(1);
}

// FUNCTION: PLAYER 0x005205a0
// FUNCTION: STUDIO 0x005d7da0
void CrashReporter::Start()
{
	SetUnhandledExceptionFilter(TopLevelExceptionFilter);
	PreventSetUnhandledExceptionFilter();
	watcherThread.reset(new boost::thread(boost::bind(&CrashReporter::WatcherThreadFunc, singleton)));
}
