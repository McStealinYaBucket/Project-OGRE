/* App/tool/GameTool.cpp -- October 2013, partial (no 2013 __FILE__; 2016 path). Only the tool name global(s) so far:
   TToolVerb<T>'s ctor (CommonVerbs.obj) reads each one through Name::declare<sT>(). */

#include "stdafx.h"


namespace RBX {

	extern const char* const sGameTool;	// also in V8DataModel/Commands.h, which this TU leaves out (its templates add COMDATs)

	// GLOBAL: PLAYER 0x00bb014c
	// GLOBAL: STUDIO 0x00c7238c
	const char* const sGameTool = "GameTool";

} // namespace RBX
