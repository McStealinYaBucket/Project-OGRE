// Log/include/FastLog.h: October 2013 reconstruction (Project OGRE).
// FLog: ring-buffer logging channels and the fast variables (FFlag, FInt, FString, FLog groups),
// all living in Log.dll.
// Evidence: the 2013 Log.dll export table (bin/player): FastLog with 1, 3 and 5 pointer arguments
// (0x10002770, 0x10002790, 0x100027c0), FastLogF(4 floats) 0x10002940, FastLogS(char const*)
// 0x10002b30 and FastLogS(std::string const&) 0x10002b80, Init, SetExternalLogFunc,
// GetFastLogCounter, Register{LogGroup,Flag,Int,String}(name, var, bool**, FastVarType),
// ForEachVariable, SetValue, GetValue, SetValueFromServer, NowFast, WriteFastLogDump,
// GetNumSynchronizedVariable and Detail::SetBinaryLog. iOS 1029 has FFlag, DFFlag, SFFlag, FInt,
// DFInt, FLog and DFLog variables and FLog::ValueGetSet<bool>::setIsSync/getIsSyncAddr.
// Differences from that 2013 surface, kept on purpose because Log/FastLog.cpp (not rewritten here)
// defines them and the build must stay as it is:
//   - GetValue takes a third parameter (2013 exports the two-parameter form);
//   - ResetSynchronizedVariablesState is declared (2013 does not export it);
//   - FASTFLAGVARIABLE and the FINT/DFFLAG/... variable macros also define a PlaceFilter_<name>
//     FString, which 2013 does not have (PLAYER holds no "PlaceFilter_" string); and
//   - LOGAPI is dllexport for importers too, as in the tree this was imported from.
// Dropped (2016 additions with no 2013 export or caller): FastLogFormatted and
// FastLogFormattedNoAdorn (FastLog.cpp keeps their bodies under #if 0) with their macros, the
// ABTest variable macros and ABTestVariation, FASTLOGNOFILTER/2 and FILECHANNEL.
#pragma once

#include <stddef.h>
#include <string>

#if defined(_WIN32)
#define LOGAPI __declspec(dllexport)
#elif defined(LOG_DLL)
#define LOGAPI __attribute__((visibility("default")))
#else
#define LOGAPI
#endif

// Logging. Only string literals as messages (the pointer is stored, not the text); numbers and
// pointers go in the arguments, floats through the F forms, and at most one string through FASTLOGS.
// Every form tests the channel first, so a disabled group costs one byte compare.
// RBX_FASTLOG_IF(group, call) runs FLog::call only when the group is on; RBX_FASTLOG_P passes an
// argument as the untyped pointer FastLog stores.
#define RBX_FASTLOG_IF(group, call) do { if (group) FLog::call; } while (0)
#define RBX_FASTLOG_P(arg) reinterpret_cast<const void*>(arg)

#define FASTLOG(group, message) RBX_FASTLOG_IF(group, FastLog(group, message "", 0))
#define FASTLOG1(group, message, arg1) RBX_FASTLOG_IF(group, FastLog(group, message "", RBX_FASTLOG_P(arg1)))
#define FASTLOG2(group, message, arg1, arg2) \
	RBX_FASTLOG_IF(group, FastLog(group, message "", RBX_FASTLOG_P(arg1), RBX_FASTLOG_P(arg2), 0))
#define FASTLOG3(group, message, arg1, arg2, arg3) \
	RBX_FASTLOG_IF(group, FastLog(group, message "", RBX_FASTLOG_P(arg1), RBX_FASTLOG_P(arg2), RBX_FASTLOG_P(arg3)))
#define FASTLOG4(group, message, arg1, arg2, arg3, arg4) \
	RBX_FASTLOG_IF(group, FastLog(group, message "", RBX_FASTLOG_P(arg1), RBX_FASTLOG_P(arg2), RBX_FASTLOG_P(arg3), RBX_FASTLOG_P(arg4), 0))
#define FASTLOG5(group, message, arg1, arg2, arg3, arg4, arg5) \
	RBX_FASTLOG_IF(group, FastLog(group, message "", RBX_FASTLOG_P(arg1), RBX_FASTLOG_P(arg2), RBX_FASTLOG_P(arg3), RBX_FASTLOG_P(arg4), RBX_FASTLOG_P(arg5)))
#define FASTLOGS(group, message, sarg) RBX_FASTLOG_IF(group, FastLogS(group, message "", sarg))
#define FASTLOG1F(group, message, arg1) RBX_FASTLOG_IF(group, FastLogF(group, message "", arg1))
#define FASTLOG2F(group, message, arg1, arg2) RBX_FASTLOG_IF(group, FastLogF(group, message "", arg1, arg2))
#define FASTLOG3F(group, message, arg1, arg2, arg3) RBX_FASTLOG_IF(group, FastLogF(group, message "", arg1, arg2, arg3))
#define FASTLOG4F(group, message, arg1, arg2, arg3, arg4) RBX_FASTLOG_IF(group, FastLogF(group, message "", arg1, arg2, arg3, arg4))

// Entries kept per channel; a power of two, since the write counter is taken modulo it.
#define LOG_HISTORY 2048
#define LOGCHANNELS 5

// Declaring (X) and defining (X##VARIABLE) fast variables. Each definition registers itself with
// Log.dll from a dynamic initializer (the <name>Initer int), so the server can override it by name.
// D = dynamic (may change while running), SYNCHRONIZED = replicated by the server; reading one
// before it has been synchronized breaks into the debugger in debug and test builds.
// The two shapes every definition takes, in namespace ns: a plain registered variable, and a
// synchronized one that also gets an IsSync flag pointer and a checked getter.
#define RBX_FASTVAR(ns, Type, var, value, Register, kind) \
	namespace ns { Type var = value; int var##Initer = FLog::Register(#var, &var, NULL, kind); }
#define RBX_FASTVAR_SYNC(ns, Type, var, value, Register, CHECK) \
	namespace ns { Type var = value; bool* var##IsSync; \
	int var##Initer = FLog::Register(#var, &var, &var##IsSync, FASTVARTYPE_SYNC); \
	Type get##var() { CHECK(*var##IsSync); return var; } }
// assert's form for the synchronized log group getter
#define RBX_FASTVAR_ASSERT_SYNCED(isSync) assert(isSync == true)

#define LOGGROUP(group) namespace FLog { extern Channel group; }
#define LOGVARIABLE(group, defaulton) RBX_FASTVAR(FLog, Channel, group, defaulton, RegisterLogGroup, FASTVARTYPE_STATIC)
#define DYNAMIC_LOGGROUP(group) namespace DFLog { extern FLog::Channel group; }
#define DYNAMIC_LOGVARIABLE(group, defaulton) RBX_FASTVAR(DFLog, FLog::Channel, group, defaulton, RegisterLogGroup, FASTVARTYPE_DYNAMIC)
#define SYNCHRONIZED_LOGGROUP(group) namespace SFLog { extern FLog::Channel get##group(); }
#define SYNCHRONIZED_LOGVARIABLE(group, defaulton) \
	RBX_FASTVAR_SYNC(SFLog, FLog::Channel, group, defaulton, RegisterLogGroup, RBX_FASTVAR_ASSERT_SYNCED)

#define FASTSTRING(var) namespace FString { extern std::string var; }
#define FASTSTRINGVARIABLE(var, defaultvalue) RBX_FASTVAR(FString, std::string, var, defaultvalue, RegisterString, FASTVARTYPE_STATIC)

#define FASTFLAG(var) namespace FFlag { extern bool var; }
#define FASTFLAGVARIABLE(var, defaultvalue) \
	FASTSTRINGVARIABLE(PlaceFilter_##var, "") RBX_FASTVAR(FFlag, bool, var, defaultvalue, RegisterFlag, FASTVARTYPE_STATIC)
#define DYNAMIC_FASTFLAG(var) namespace DFFlag { extern bool var; }
#define DYNAMIC_FASTFLAGVARIABLE(var, defaultvalue) \
	FASTSTRINGVARIABLE(PlaceFilter_##var, "") RBX_FASTVAR(DFFlag, bool, var, defaultvalue, RegisterFlag, FASTVARTYPE_DYNAMIC)

#if defined(_WIN32)
#define RBX_DEBUG_BREAK() { __debugbreak(); }(void)0
#elif defined(__i386__)
#define RBX_DEBUG_BREAK() { __asm__ __volatile__("int $3"); }(void)0
#else
#define RBX_DEBUG_BREAK() { ::abort(); }(void)0
#endif

#if defined(_DEBUG) || defined(_NOOPT) || defined(RBX_TEST_BUILD)
#define VALIDATE_SYNCHRONIZED_FLAG(var) { if (!var) RBX_DEBUG_BREAK(); } ((void)0)
#else
#define VALIDATE_SYNCHRONIZED_FLAG(var) ((void)0)
#endif

#define SYNCHRONIZED_FASTFLAG(var) namespace SFFlag { extern bool get##var(); }
#define SYNCHRONIZED_FASTFLAGVARIABLE(var, defaultvalue) \
	FASTSTRINGVARIABLE(PlaceFilter_##var, "") RBX_FASTVAR_SYNC(SFFlag, bool, var, defaultvalue, RegisterFlag, VALIDATE_SYNCHRONIZED_FLAG)

#define FASTINT(var) namespace FInt { extern int var; }
#define FASTINTVARIABLE(var, defaultvalue) \
	FASTSTRINGVARIABLE(PlaceFilter_##var, "") RBX_FASTVAR(FInt, int, var, defaultvalue, RegisterInt, FASTVARTYPE_STATIC)
#define DYNAMIC_FASTINT(var) namespace DFInt { extern int var; }
#define DYNAMIC_FASTINTVARIABLE(var, defaultvalue) \
	FASTSTRINGVARIABLE(PlaceFilter_##var, "") RBX_FASTVAR(DFInt, int, var, defaultvalue, RegisterInt, FASTVARTYPE_DYNAMIC)
#define SYNCHRONIZED_FASTINT(var) namespace SFInt { extern int get##var(); }
#define SYNCHRONIZED_FASTINTVARIABLE(var, defaultvalue) \
	FASTSTRINGVARIABLE(PlaceFilter_##var, "") RBX_FASTVAR_SYNC(SFInt, int, var, defaultvalue, RegisterInt, VALIDATE_SYNCHRONIZED_FLAG)

#define DYNAMIC_FASTSTRING(var) namespace DFString { extern std::string var; }
#define DYNAMIC_FASTSTRINGVARIABLE(var, defaultvalue) RBX_FASTVAR(DFString, std::string, var, defaultvalue, RegisterString, FASTVARTYPE_DYNAMIC)
#define SYNCHRONIZED_FASTSTRING(var) namespace SFString { extern std::string get##var(); }
#define SYNCHRONIZED_FASTSTRINGVARIABLE(var, defaultvalue) \
	RBX_FASTVAR_SYNC(SFString, std::string, var, defaultvalue, RegisterString, VALIDATE_SYNCHRONIZED_FLAG)

// Kinds of fast variable, as bits. The AB values stay because FASTVARTYPE_ANY (31) is the default
// filter FastLog.cpp compiles against.
enum FastVarType
{
	FASTVARTYPE_STATIC = 0,
	FASTVARTYPE_DYNAMIC = 1,
	FASTVARTYPE_SYNC = 2,
	FASTVARTYPE_AB_NEWUSERS = 4,
	FASTVARTYPE_AB_NEWSTUDIOUSERS = 8,
	FASTVARTYPE_AB_ALLUSERS = 16,
	FASTVARTYPE_ANY = 1 + 2 + 4 + 8 + 16
};

namespace FLog {

// Channel numbers: 1 for lasting, important events, 2 for per-frame data, 3 to 5 for whatever is
// being debugged. 0 means off. In a debugger, channel N's newest entry is
// {,,Log}g_FastLog[N-1] + ({,,Log}g_LogCounters[N-1] % LOG_HISTORY).
typedef unsigned char Channel;

// Passing a FASTLOG macro too many arguments is an error, not a warning.
#pragma warning(error : 4002)

typedef double (*TimeFunc)();
typedef void (*ExternalLogFunc)(Channel channel, const char* message);
void LOGAPI Init(TimeFunc timeF);
void LOGAPI SetExternalLogFunc(ExternalLogFunc logF);

void LOGAPI FastLog(Channel channel, const char* message, const void* arg0);
void LOGAPI FastLog(Channel channel, const char* message, const void* arg0, const void* arg1, const void* arg2);
void LOGAPI FastLog(Channel channel, const char* message, const void* arg0, const void* arg1, const void* arg2, const void* arg3, const void* arg4);
void LOGAPI FastLogS(Channel channel, const char* message, const char* sarg);
void LOGAPI FastLogS(Channel channel, const char* message, const std::string& sarg);
void LOGAPI FastLogF(Channel channel, const char* message, float arg1, float arg2 = 0.0f, float arg3 = 0.0f, float arg4 = 0.0f);

int LOGAPI GetFastLogCounter(Channel channel);

// Each returns a dummy int so a namespace-scope definition can call it from its initializer.
int LOGAPI RegisterLogGroup(const char* groupName, Channel* groupVar, bool** isSync = NULL, FastVarType fastVarType = FASTVARTYPE_STATIC);
int LOGAPI RegisterFlag(const char* flagName, bool* flagVar, bool** isSync = NULL, FastVarType fastVarType = FASTVARTYPE_STATIC);
int LOGAPI RegisterInt(const char* flagName, int* flagVar, bool** isSync = NULL, FastVarType fastVarType = FASTVARTYPE_STATIC);
int LOGAPI RegisterString(const char* flagName, std::string* flagVar, bool** isSync = NULL, FastVarType fastVarType = FASTVARTYPE_STATIC);

typedef void (*VariableVisitor)(const std::string& name, const std::string& value, void* context);
void LOGAPI ForEachVariable(VariableVisitor visitor, void* context, FastVarType flagType);

bool LOGAPI SetValue(const std::string& name, const std::string& value, FastVarType fastVarType = FASTVARTYPE_ANY, bool loadedFromServer = false);
bool LOGAPI GetValue(const std::string& name, std::string& value, bool alsoCheckUnknownFlags = false);
bool LOGAPI SetValueFromServer(const std::string& name, const std::string& value);

double LOGAPI NowFast();
void LOGAPI WriteFastLogDump(const char* fileName, int numEntries);

unsigned short LOGAPI GetNumSynchronizedVariable();
void LOGAPI ResetSynchronizedVariablesState();

namespace Detail {
// Every entry is also passed, in binary form, to callback (see FastLogStream.h).
typedef void (*BinaryLogFunc)(void* context, const void* data, size_t size);
void LOGAPI SetBinaryLog(void* context, BinaryLogFunc callback);
}

}

LOGGROUP(Legacy)

// The fixed channels for messages that must always be logged.
namespace FLog {
const Channel Always = 1;
const Channel Error = 1;
const Channel Warning = 1;
}
