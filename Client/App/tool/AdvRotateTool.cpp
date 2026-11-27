/* App/tool/AdvRotateTool.cpp -- October 2013, partial (2013 path tool\AdvRotateTool.cpp). Only the tool name global(s) so far:
   TToolVerb<T>'s ctor (CommonVerbs.obj) reads each one through Name::declare<sT>(). */

#include "stdafx.h"


namespace RBX {

	extern const char* const sAdvRotateTool;	// also in V8DataModel/Commands.h, which this TU leaves out (its templates add COMDATs)

	// GLOBAL: PLAYER 0x00bb0668
	// GLOBAL: STUDIO 0x00c72898
	const char* const sAdvRotateTool = "AdvRotate";

} // namespace RBX
