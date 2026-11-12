/* App/util/RbxStringTable.cpp — October 2013 (string table, minimal).
   Only id 4 evidenced (see header). TODO: full 2013 table. */

#include "stdafx.h"

#include "Util/RbxStringTable.h"

const char* getStringById(int id)
{
	switch (id)
	{
	case LuaStringStringId: return "lua";
	case CommandOutStringId: return "> %s";
	case StudioASHXFmt: return "fmt";
	case StudioASHX: return "ashx";
	case ExecScriptNewThread: return "Execute script in new thread, name: %s, identity: %u";
	case FullScriptCode: return "Full script code:\n %s";
	case EnableToCreateSBThread: return "Unable to create trusted sandbox thread";
	case EnableToCreateNewThread: return "Unable to create new thread";
	case ScriptStr: return "Script";
	case Rocky: return "rocky";
	case HasGamePassLuaWarning: return "Game passes can only be queried by a Script running on a ROBLOX game server";
	case NoTeleportInStudio: return "Teleporting while using ROBLOX Studio is not permitted";
	case LoadingScreenScriptPath: return "";
	case RunningScript: return "Running script %s";
	default: return "";
	}
}
