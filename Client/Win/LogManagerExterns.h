/* Win/LogManagerExterns.h -- declarations LogManager.cpp needs from code this tree has not written
   or does not declare yet. Each block goes away when its real header exists: App's
   Util/FileSystem.h (getUserDirectory's 2013 overload), Win/VistaTools.h, Win/DumpErrorUploader.h,
   and G3D 8.00's Log.h/debugAssert.h hooks (Client/Rendering, the other contributor's). */

#pragma once

#include <stdio.h>
#include <string>

#include "FastLog.h"

namespace RBX
{
	enum FileSystemDir
	{
		DirAppData = 0,
	};

	namespace FileSystem
	{
		// App's (PLAYER 0x006ea4c0, native). Not declared by the tree's Util/FileSystem.h yet.
		std::string getUserDirectory(bool create, FileSystemDir dir, const char* subDirectory);
	}
}

LOGGROUP(CrashReporterInit)

// Win/VistaTools.cpp (PLAYER 0x0046a2e0) and Win/DumpErrorUploader.cpp (0x00435f10), not written yet.
bool IsVistaPlus();
class DumpErrorUploader
{
public:
	static void UploadCrashEventFile(struct _EXCEPTION_POINTERS* excInfo);
};

namespace G3D
{
	// G3D 8.00's Log (PLAYER 0x009d5b70..): the g3d tree does not build Log.cpp yet.
	class Log
	{
		FILE* logFile;
		std::string filename;
		int stripFromStackBottom;
	public:
		Log(const std::string& filename = "log.txt", int stripFromStackBottom = 0);
		virtual ~Log();
		static Log* common();
		static std::string getCommonLogFilename();
	};

	typedef bool (*AssertionHook)(const char* _expression, const std::string& message, const char* filename, int lineNumber, bool useGuiPrompt);
	void setAssertionHook(AssertionHook hook);
	void setFailureHook(AssertionHook hook);
}
