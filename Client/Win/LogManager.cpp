/* Win/LogManager.cpp -- October 2013 client shell (PLAYER and STUDIO compile it). Evidence and the
   2013-vs-2016 differences are in LogManager.h. The ctor's assert must stay on line 354 (layout). */
#include "LogManager.h"

#include <list>

#include <boost/thread/once.hpp>
#include <boost/thread/tss.hpp>

#include "rbx/Debug.h"
#include "RbxAssert.h"
#include "Util/standardout.h"
#include "Util/Guid.h"
#include "VersionInfo.h"

#include <dbghelp.h>

#include "LogManagerExterns.h"

// GLOBAL: PLAYER 0x00e2510c
MainLogManager* LogManager::mainLogManager = NULL;
// GLOBAL: PLAYER 0x00e2525c
RBX::mutex MainLogManager::fastLogChannelsLock;

// The logs folder (<user AppData>\logs), printed to the console the first time it is asked for.
static const ATL::CPath& DoGetPath()
{
	static ATL::CPath path;
	static bool first = true;
	if (first)
	{
		path.m_strPath = CString(RBX::FileSystem::getUserDirectory(true, RBX::DirAppData, "logs").c_str());
		printf("%s\r\n", RBX::FileSystem::getUserDirectory(true, RBX::DirAppData, "logs").c_str());
		first = false;
	}
	return path;
}

static void InitPath()
{
	DoGetPath();
}

const ATL::CPath& LogManager::GetLogPath()
{
	static boost::once_flag flag = BOOST_ONCE_INIT;
	boost::call_once(&InitPath, flag);
	return DoGetPath();
}

// The exe's file version ("0, 126, 0, 39503"), for the crash files' names.
// STUB: PLAYER 0x0043eb80
std::string GetAppVersion()
{
	CVersionInfo vi;
	FASTLOG1(FLog::CrashReporterInit, "Getting app version, module handle: %p", _AtlBaseModule.m_hInst);
	vi.Load(_AtlBaseModule.m_hInst);
	return vi.GetFileVersionAsString();
}

// FastLog's external sink: one "Log Channel" file per channel, created on its first message.
// FUNCTION: PLAYER 0x0043ec40
void MainLogManager::fastLogMessage(FLog::Channel id, const char* message)
{
	RBX::mutex::scoped_lock lock(fastLogChannelsLock);

	if (mainLogManager)
	{
		if (id >= mainLogManager->fastLogChannels.size())
			mainLogManager->fastLogChannels.resize(id + 1, NULL);

		if (mainLogManager->fastLogChannels[id] == NULL)
			mainLogManager->fastLogChannels[id] = new RBX::Log(mainLogManager->getFastLogFileName(id).c_str(), "Log Channel");

		mainLogManager->fastLogChannels[id]->writeEntry(RBX::Log::Information, message);
	}
}

// Characters 3..7 of the session GUID ("RBX" + 32 hex digits).
// FUNCTION: PLAYER 0x0043ed90
std::string MainLogManager::getSessionId()
{
	std::string id = guid;
	id.erase(8);
	id.erase(0, 3);
	return id;
}

// STUB: PLAYER 0x0043ee20
std::string MainLogManager::getCrashEventName()
{
	FASTLOG(FLog::CrashReporterInit, "Getting crash event name");
	ATL::CPath path = GetLogPath();

	std::string fileName = "log_";
	fileName += getSessionId();
	fileName += " ";
	fileName += GetAppVersion();
	fileName += crashEventExtention;

	path.Append(fileName.c_str());

	return (LPCTSTR)path;
}

// STUB: PLAYER 0x0043efb0
std::string MainLogManager::getLogFileName()
{
	ATL::CPath path = GetLogPath();

	std::string fileName = "log_";
	fileName += getSessionId();
	fileName += ".txt";

	path.Append(fileName.c_str());

	return (LPCTSTR)path;
}

// STUB: PLAYER 0x0043f0f0
std::string MainLogManager::getFastLogFileName(FLog::Channel channelId)
{
	ATL::CPath path = GetLogPath();

	CString fileName;
	fileName.Format("log_%s_%d.txt", getSessionId().c_str(), channelId);

	path.Append(fileName);

	return (LPCTSTR)path;
}

// STUB: PLAYER 0x0043f230
std::string MainLogManager::MakeLogFileName(const char* postfix)
{
	ATL::CPath path = GetLogPath();

	std::string fileName = "log_";
	fileName += getSessionId();
	fileName += postfix;
	fileName += ".txt";

	path.Append(fileName.c_str());

	return (LPCTSTR)path;
}

// The main log's name with "_<thread name>_<thread id>" before its ".txt".
// STUB: PLAYER 0x0043f380
std::string ThreadLogManager::getLogFileName()
{
	std::string fileName = mainLogManager->getLogFileName();
	CString suffix;
	suffix.Format("_%s_%d", name.c_str(), threadID);
	fileName.insert(fileName.size() - 4, suffix);
	return fileName;
}

// FUNCTION: PLAYER 0x0043f470
RBX::Log* LogManager::getLog()
{
	if (log == NULL)
		log = new RBX::Log(getLogFileName().c_str(), name.c_str());
	return log;
}

// RBX::Log::current(): the main thread's log, or the calling thread's own.
// FUNCTION: PLAYER 0x0043f540
RBX::Log* MainLogManager::provideLog()
{
	if (GetCurrentThreadId() == threadID)
		return getLog();

	return ThreadLogManager::getCurrent()->getLog();
}

// The minidump settings: data segments, plus indirectly referenced memory on Vista and later.
// STUB: PLAYER 0x0043f570
RobloxCrashReporter::RobloxCrashReporter(const char* outputPath, const char* appName, const char* crashExtention)
{
	controls.minidumpType = MiniDumpWithDataSegs;

	if (IsVistaPlus())
		controls.minidumpType |= MiniDumpWithIndirectlyReferencedMemory;

	strcpy(controls.pathToMinidump, outputPath);

	strcpy(controls.appName, appName);

	strncpy(controls.appVersion, GetAppVersion().c_str(), 128);
	strncpy(controls.crashExtention, crashExtention, sizeof(controls.crashExtention));
}

bool RobloxCrashReporter::silent;

// STUB: PLAYER 0x0043f660
LONG RobloxCrashReporter::ProcessException(struct _EXCEPTION_POINTERS* info, bool noMsg)
{
	LogManager::ReportEvent(EVENTLOG_INFORMATION_TYPE, "StartProcessException...");

	LONG result = CrashReporter::ProcessException(info, noMsg);
	static bool showedMessage = silent;
	if (!showedMessage && !noMsg)
	{
		showedMessage = true;
		::MessageBox(NULL, "An unexpected error occurred and ROBLOX needs to quit.  We're sorry!", "ROBLOX Crash", MB_OK);
	}

	LogManager::ReportEvent(EVENTLOG_INFORMATION_TYPE, "DoneProcessException");

	LogManager::ReportEvent(EVENTLOG_INFORMATION_TYPE, "Uploading .crashevent...");
	DumpErrorUploader::UploadCrashEventFile(info);
	LogManager::ReportEvent(EVENTLOG_INFORMATION_TYPE, "Done uploading .crashevent...");

	return result;
}

// FUNCTION: PLAYER 0x0043f710
void RobloxCrashReporter::logEvent(const char* msg)
{
	LogManager::ReportEvent(EVENTLOG_INFORMATION_TYPE, msg);
}

// STUB: PLAYER 0x0043f730
void MainLogManager::WriteCrashDump()
{
	std::string appName = "log_";
	appName += getSessionId();
	crashReporter.reset(new RobloxCrashReporter(GetLogPath(), appName.c_str(), crashExtention));
	crashReporter->Start();
	CString eventMessage;
	eventMessage.Format("CrashReporter Start");
	RBX::Log::current()->writeEntry(RBX::Log::Information, eventMessage);
}

// A FastLog dump and a 5-byte "Fake" .dmp, so the next start uploads the logs.
// STUB: PLAYER 0x0043f8c0
bool MainLogManager::CreateFakeCrashDump()
{
	if (!crashReporter)
		WriteCrashDump();

	char dumpFilepath[_MAX_PATH];
	if (FAILED(crashReporter->GenerateDmpFileName(dumpFilepath, _MAX_PATH, true)))
		return false;

	FLog::WriteFastLogDump(dumpFilepath, 2000);

	if (FAILED(crashReporter->GenerateDmpFileName(dumpFilepath, _MAX_PATH)))
		return false;

	HANDLE hFile = CreateFile(dumpFilepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	DWORD written;
	WriteFile(hFile, "Fake", 5, &written, NULL);

	CloseHandle(hFile);
	return true;
}

// The 2013 layout puts the ctor's assert on line 354; these blank lines stand in for 2013 text before
// purecallHandler that is not known yet (2016 has the console includes and the crash reporter switches here).






















































// _set_purecall_handler's target: report, then crash on purpose.
// STUB: PLAYER 0x0043f9b0
static void __cdecl purecallHandler()
{
	CString message;
	message.Format("Pure Call Error");
	LogManager::ReportEvent(EVENTLOG_ERROR_TYPE, message);
	RBXCRASH();
}

/* The constructor, PLAYER 0x0043fa50 (`this` in esi), in 2013 order:
     - LogManager("Roblox"), the two extensions, an empty guid, then generateRBXGUID(guid)
       (App, 0x006f93b0);
     - CullLogs on the logs folder and on its archive (128 files each: CullLogs takes the folder in
       ecx and has the count folded in);
     - G3D's common log: getLogFileName() (called directly) with ".txt" replaced by "_G3D.txt";
     - the assert below, line 354;
     - mainLogManager, RBX::Log's provider, RBX's and G3D's assertion and failure hooks, the
       purecall handler and FastLog's external sink. */

// STUB: PLAYER 0x0043fa50
MainLogManager::MainLogManager(LPCTSTR productName, const char* crashExtention, const char* crashEventExtention)
	: LogManager(productName)
	, crashExtention(crashExtention)
	, crashEventExtention(crashEventExtention)
{
	RBX::Guid::generateRBXGUID(guid);
	CullLogs("\\", 128);
	CullLogs("archive\\", 128);

	std::string g3dLogFile = getLogFileName();
	g3dLogFile.erase(g3dLogFile.size() - 4);
	g3dLogFile += "_G3D.txt";
	new G3D::Log(g3dLogFile);

	RBXASSERT(mainLogManager == NULL);
	mainLogManager = this;

	RBX::Log::setLogProvider(this);

	RBX::setAssertionHook(&MainLogManager::handleDebugAssert);
	RBX::setFailureHook(&MainLogManager::handleFailure);
	G3D::setAssertionHook(&MainLogManager::handleG3DDebugAssert);
	G3D::setFailureHook(&MainLogManager::handleG3DFailure);

	_set_purecall_handler(purecallHandler);

	FLog::SetExternalLogFunc(fastLogMessage);
}

// Oldest first, by creation time.
struct olderthan
{
	bool operator()(const WIN32_FIND_DATA& lhs, const WIN32_FIND_DATA& rhs) const
	{
		if (lhs.ftCreationTime.dwHighDateTime == rhs.ftCreationTime.dwHighDateTime)
			return lhs.ftCreationTime.dwLowDateTime < rhs.ftCreationTime.dwLowDateTime;
		return lhs.ftCreationTime.dwHighDateTime < rhs.ftCreationTime.dwHighDateTime;
	}
};

// Deletes the oldest files of logs\<folder> until filesRemaining are left.
// STUB: PLAYER 0x004403a0
void MainLogManager::CullLogs(const char* folder, int filesRemaining)
{
	ATL::CPath path = GetLogPath();
	path.Append(folder);
	path.AddBackslash();

	std::list<WIN32_FIND_DATA> files;
	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile(path + "*.*", &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				CString f = findData.cFileName;
				files.push_back(findData);
			}
		} while (FindNextFile(hFind, &findData));
	}

	files.sort(olderthan());

	std::list<WIN32_FIND_DATA>::iterator iter = files.begin();
	for (size_t count = filesRemaining; count < files.size(); ++iter, ++count)
		::DeleteFile(path + iter->cFileName);
}

// G3D's common log is deleted here (its file name is read first, as for the thread logs below).
// STUB: PLAYER 0x00440600
MainLogManager::~MainLogManager()
{
	if (G3D::Log::common())
	{
		std::string logFile = G3D::Log::getCommonLogFilename();
		delete G3D::Log::common();
	}

	RBX::mutex::scoped_lock lock(fastLogChannelsLock);

	FLog::SetExternalLogFunc(NULL);

	for (std::size_t i = 0; i < fastLogChannels.size(); i++)
		delete fastLogChannels[i];

	mainLogManager = NULL;
}

// FUNCTION: PLAYER 0x00440780
LogManager::~LogManager()
{
	if (log != NULL)
	{
		std::string logFile = log->logFile;
		delete log;
		log = NULL;
	}
}

// The G3D hooks drop the message and the prompt flag.
// FUNCTION: PLAYER 0x00440830
bool MainLogManager::handleG3DDebugAssert(const char* expression, const std::string& message, const char* filename, int lineNumber, bool useGuiPrompt)
{
	return handleDebugAssert(expression, filename, lineNumber);
}

// STUB: PLAYER 0x00440850
bool MainLogManager::handleDebugAssert(const char* expression, const char* filename, int lineNumber)
{
	CString message;
	message.Format("Assertion failed: %s\n%s(%d)", expression, filename, lineNumber);
	LogManager::ReportEvent(EVENTLOG_WARNING_TYPE, message);
	return false;
}

// FUNCTION: PLAYER 0x00440900
bool MainLogManager::handleG3DFailure(const char* expression, const std::string& message, const char* filename, int lineNumber, bool useGuiPrompt)
{
	return handleFailure(expression, filename, lineNumber);
}

// STUB: PLAYER 0x00440920
bool MainLogManager::handleFailure(const char* expression, const char* filename, int lineNumber)
{
	CString message;
	message.Format("G3D Error: %s\n%s(%d)", expression, filename, lineNumber);
	LogManager::ReportEvent(EVENTLOG_ERROR_TYPE, message);
	RBXCRASH();
	return false;
}

// Event-log types to StandardOut (`message` in esi in the PLAYER shell).
// FUNCTION: PLAYER 0x00440a70
void LogManager::ReportEvent(WORD type, LPCSTR message)
{
	switch (type)
	{
	case EVENTLOG_SUCCESS:
		RBX::StandardOut::singleton()->printf(RBX::MESSAGE_INFO, "%s", message);
		break;
	case EVENTLOG_ERROR_TYPE:
		RBX::StandardOut::singleton()->printf(RBX::MESSAGE_ERROR, "%s", message);
		break;
	case EVENTLOG_INFORMATION_TYPE:
		RBX::StandardOut::singleton()->printf(RBX::MESSAGE_INFO, "%s", message);
		break;
	case EVENTLOG_AUDIT_SUCCESS:
		RBX::StandardOut::singleton()->printf(RBX::MESSAGE_INFO, "%s", message);
		break;
	case EVENTLOG_AUDIT_FAILURE:
		RBX::StandardOut::singleton()->printf(RBX::MESSAGE_ERROR, "%s", message);
		break;
	}
}

ThreadLogManager::ThreadLogManager()
	: LogManager(RBX::get_thread_name())
{
}

// One ThreadLogManager per thread, in a thread_specific_ptr made on first use.
// GLOBAL: PLAYER 0x00e25098
static boost::thread_specific_ptr<ThreadLogManager>* threadLogManagers;

// FUNCTION: PLAYER 0x00440bd0
static void initThreadLogManagers()
{
	static boost::thread_specific_ptr<ThreadLogManager> managers;
	threadLogManagers = &managers;
}

// FUNCTION: PLAYER 0x00440c60
ThreadLogManager* ThreadLogManager::getCurrent()
{
	static boost::once_flag flag = BOOST_ONCE_INIT;
	boost::call_once(&initThreadLogManagers, flag);

	ThreadLogManager* manager = threadLogManagers->get();
	if (!manager)
	{
		manager = new ThreadLogManager();
		threadLogManagers->reset(manager);
	}
	return manager;
}

ThreadLogManager::~ThreadLogManager()
{
}
