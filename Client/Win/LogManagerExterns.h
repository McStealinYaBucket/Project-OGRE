/* Win/LogManagerExterns.h -- declarations LogManager.cpp needs from code this tree has not written
   or does not declare yet. The G3D block goes away when G3D 8.00's Log.h/debugAssert.h hooks are
   built (Client/Rendering, the other contributor's). The other includes sit here, not in
   LogManager.cpp, so that file's assert stays on line 354. */

#pragma once

#include <stdio.h>
#include <string>

#include "FastLog.h"

// FileSystemDir and the 2013 getUserDirectory overload (PLAYER 0x006ea4c0).
#include "Util/FileSystem.h"

LOGGROUP(CrashReporterInit)

#include "VistaTools.h"
#include "DumpErrorUploader.h"

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
