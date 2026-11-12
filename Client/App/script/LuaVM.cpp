/* App/script/LuaVM.cpp -- October 2013 (Lua VM support TU).
   Currently anchors LuaSecureDouble::luaXorMask (declared in
   script/LuaVM.h, referenced by the Lua fork's double obfuscation)
   and RBX::pmcHash (declared in util/ProgramMemoryChecker.h,
   touched by the fork's lua_unlock hook).
   Both BSS-zero: identity XOR (plain doubles) and zero nonce. The
   randomized init + the 713-line checker live in the security era. */

#include "stdafx.h"

#include "script/LuaVM.h"
#include "Util/ProgramMemoryChecker.h"

int LuaSecureDouble::luaXorMask[4];

RBX::PmcHashContainer RBX::pmcHash;

// Fork hook (luaconf.h lua_unlock): fires when Lua executes from
// non-RBX text. 2016 reports via sendStatsToken + DataModel::addHackFlag
// (security era); no-op here -- test code is legitimate, flagging it
// would be wrong.
void lua_vmhooked_handler(lua_State* L)
{
	(void)L;
}
