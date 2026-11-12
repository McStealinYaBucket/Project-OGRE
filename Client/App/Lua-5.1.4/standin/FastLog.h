/* Stand-in for the 2013 RBX Log library header (Log/include/FastLog.h): only the fast-flag
** declarations the Lua sources use. 2013 FASTFLAGVARIABLE has no PlaceFilter_ string (none in
** PLAYER) and registers through FLog::RegisterFlag(name, &var, NULL, FASTVARTYPE_STATIC)
** (PLAYER initializer 0x00a82e60 for LuaEnableBytecode). */
#pragma once
#include <stddef.h>

namespace FLog
{
	enum FastVarType
	{
		FASTVARTYPE_STATIC = 0,
		FASTVARTYPE_DYNAMIC = 1,
		FASTVARTYPE_SYNC = 2
	};
	int RegisterFlag(const char* flagName, bool* flagVar, bool** isSync = NULL, FastVarType fastVarType = FASTVARTYPE_STATIC);
}

#define FASTFLAG(var) namespace FFlag { extern bool var; }
#define FASTFLAGVARIABLE(var, defaultvalue) \
	namespace FFlag { bool var = defaultvalue; int var##Initer = FLog::RegisterFlag(#var, &var, NULL, FLog::FASTVARTYPE_STATIC); }
#define DYNAMIC_FASTFLAG(var) namespace DFFlag { extern bool var; }
#define DYNAMIC_FASTFLAGVARIABLE(var, defaultvalue) \
	namespace DFFlag { bool var = defaultvalue; int var##Initer = FLog::RegisterFlag(#var, &var, NULL, FLog::FASTVARTYPE_DYNAMIC); }
