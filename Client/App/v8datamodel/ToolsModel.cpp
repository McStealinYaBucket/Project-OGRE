/* App/v8datamodel/ToolsModel.cpp -- October 2013, partial (no 2013 __FILE__; 2016 path). Only the tool name global(s) so far:
   TToolVerb<T>'s ctor (CommonVerbs.obj) reads each one through Name::declare<sT>(). */

#include "stdafx.h"


namespace RBX {

	extern const char* const sAnchorTool;	// also in V8DataModel/Commands.h, which this TU leaves out (its templates add COMDATs)
	extern const char* const sLockTool;

	// GLOBAL: PLAYER 0x00bafd68
	// GLOBAL: STUDIO 0x00c71fac
	const char* const sAnchorTool = "Anchor";

	// GLOBAL: PLAYER 0x00bafd6c
	// GLOBAL: STUDIO 0x00c71fb0
	const char* const sLockTool = "Lock";

} // namespace RBX
