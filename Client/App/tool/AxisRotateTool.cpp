/* App/tool/AxisRotateTool.cpp -- October 2013, partial (no 2013 __FILE__; 2016 path). Only the tool name global(s) so far:
   TToolVerb<T>'s ctor (CommonVerbs.obj) reads each one through Name::declare<sT>(). */

#include "stdafx.h"


namespace RBX {

	extern const char* const sAxisRotateTool;	// also in V8DataModel/Commands.h, which this TU leaves out (its templates add COMDATs)

	// GLOBAL: PLAYER 0x00bb0220
	// GLOBAL: STUDIO 0x00c72460
	const char* const sAxisRotateTool = "AxisRotate";

} // namespace RBX
