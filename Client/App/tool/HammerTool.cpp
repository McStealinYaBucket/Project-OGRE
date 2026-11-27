/* App/tool/HammerTool.cpp -- October 2013, partial (no 2013 __FILE__; 2016 path). Only the tool name global(s) so far:
   TToolVerb<T>'s ctor (CommonVerbs.obj) reads each one through Name::declare<sT>(). */

#include "stdafx.h"


namespace RBX {

	extern const char* const sHammerTool;	// also in V8DataModel/Commands.h, which this TU leaves out (its templates add COMDATs)

	// GLOBAL: PLAYER 0x00bafe7c
	// GLOBAL: STUDIO 0x00c720bc
	const char* const sHammerTool = "Hammer";

} // namespace RBX
