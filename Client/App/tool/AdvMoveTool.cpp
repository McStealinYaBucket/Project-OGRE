/* App/tool/AdvMoveTool.cpp -- October 2013, partial (2013 path tool\AdvMoveTool.cpp). Only the tool name global(s) so far:
   TToolVerb<T>'s ctor (CommonVerbs.obj) reads each one through Name::declare<sT>(). */

#include "stdafx.h"


namespace RBX {

	extern const char* const sAdvMoveTool;	// also in V8DataModel/Commands.h, which this TU leaves out (its templates add COMDATs)

	// GLOBAL: PLAYER 0x00bb0380
	// GLOBAL: STUDIO 0x00c725b0
	const char* const sAdvMoveTool = "AdvMove";

} // namespace RBX
