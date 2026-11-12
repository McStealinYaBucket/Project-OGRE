/* Util/RbxStringTable.h — October 2013 (obfuscated string table, minimal).
   Evidence: DataModel::loadCoreScripts 0x2D5790 calls getStringById(4) for the
     thread name passed to ScriptContext::executeInNewThread.
   2016 equivalent: Client/App/include/util/RbxStringTable.h (noinline lookup
     defeats string-table scans; same idiom here — the core-script URL itself
     is built char-by-char in 0x2D5790 for the same reason).
   Table contents below id 4 = "ashx" (evidenced); the rest carried from 2016
   with TODO (2013 ids unverified).
   Original file: (2013 path TODO; 2016 equivalent above). */

#pragma once

#ifdef _WIN32
__declspec(noinline) const char* getStringById(int id);
#elif defined(__APPLE__) || defined(__ANDROID__)
__attribute__((noinline)) const char* getStringById(int id);
#else
#error Unsupported Platform.
#endif

// 2013 ids TODO-verify (carried from 2016); only StudioASHX(4) is evidenced.
enum StringIDs {
	ArgStringID = 0,
	LuaStringStringId = 1,   // "lua"
	CommandOutStringId = 2,  // "> %s"
	StudioASHXFmt = 3,       // fmt
	StudioASHX = 4,          // "ashx" (0x2D5790)
	RunningScript = 5,
	ExecScriptNewThread = 6,
	FullScriptCode = 7,
	EnableToCreateSBThread = 8,
	EnableToCreateNewThread = 9,
	ScriptStr = 10,          // "Script"
	Rocky = 11,              // "rocky"
	HasGamePassLuaWarning = 12,
	NoTeleportInStudio = 13,
	LoadingScreenScriptPath = 14,
};
