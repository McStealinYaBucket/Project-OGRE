/* App/v8datamodel/ToolsPart.cpp -- October 2013, partial (no 2013 __FILE__; 2016 path). Only the tool name global(s) so far:
   TToolVerb<T>'s ctor (CommonVerbs.obj) reads each one through Name::declare<sT>(). */

#include "stdafx.h"


namespace RBX {

	extern const char* const sFillTool;	// also in V8DataModel/Commands.h, which this TU leaves out (its templates add COMDATs)
	extern const char* const sMaterialTool;
	extern const char* const sDropperTool;

	// GLOBAL: PLAYER 0x00bafc88
	// GLOBAL: STUDIO 0x00c1c2bc
	const char* const sFillTool = "Fill";

	// GLOBAL: PLAYER 0x00bafc8c
	// GLOBAL: STUDIO 0x00c1c2c0
	const char* const sMaterialTool = "Material";

	// GLOBAL: PLAYER 0x00bafc90
	// GLOBAL: STUDIO 0x00c1c2c4
	const char* const sDropperTool = "Dropper";

} // namespace RBX
