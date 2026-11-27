/* App/tool/NullTool.cpp -- October 2013, partial (2013 path tool\NullTool.cpp). Only the tool name global(s) so far:
   TToolVerb<T>'s ctor (CommonVerbs.obj) reads each one through Name::declare<sT>(). */

#include "stdafx.h"


namespace RBX {

	extern const char* const sNullTool;	// also in V8DataModel/Commands.h, which this TU leaves out (its templates add COMDATs)

	// GLOBAL: PLAYER 0x00b8e450
	// GLOBAL: STUDIO 0x00c639ac
	const char* const sNullTool = "NullTool";

} // namespace RBX
