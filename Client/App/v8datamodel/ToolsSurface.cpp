/* App/v8datamodel/ToolsSurface.cpp -- October 2013, partial (2013 path V8DataModel\ToolsSurface.cpp). Only the tool name global(s) so far:
   TToolVerb<T>'s ctor (CommonVerbs.obj) reads each one through Name::declare<sT>(). */

#include "stdafx.h"


namespace RBX {

	extern const char* const sFlatTool;	// also in V8DataModel/Commands.h, which this TU leaves out (its templates add COMDATs)
	extern const char* const sGlueTool;
	extern const char* const sWeldTool;
	extern const char* const sStudsTool;
	extern const char* const sInletTool;
	extern const char* const sUniversalTool;
	extern const char* const sHingeTool;
	extern const char* const sRightMotorTool;
	extern const char* const sLeftMotorTool;
	extern const char* const sOscillateMotorTool;
	extern const char* const sSmoothNoOutlinesTool;

	// GLOBAL: PLAYER 0x00b8d388
	// GLOBAL: STUDIO 0x00c637a8
	const char* const sFlatTool = "Flat";

	// GLOBAL: PLAYER 0x00b8d38c
	// GLOBAL: STUDIO 0x00c637ac
	const char* const sGlueTool = "Glue";

	// GLOBAL: PLAYER 0x00b8d390
	// GLOBAL: STUDIO 0x00c637b0
	const char* const sWeldTool = "Weld";

	// GLOBAL: PLAYER 0x00b8d394
	// GLOBAL: STUDIO 0x00c637b4
	const char* const sStudsTool = "Studs";

	// GLOBAL: PLAYER 0x00b8d398
	// GLOBAL: STUDIO 0x00c637b8
	const char* const sInletTool = "Inlet";

	// GLOBAL: PLAYER 0x00b8d39c
	// GLOBAL: STUDIO 0x00c637bc
	const char* const sUniversalTool = "Universal";

	// GLOBAL: PLAYER 0x00b8d3a0
	// GLOBAL: STUDIO 0x00c637c0
	const char* const sHingeTool = "Hinge";

	// GLOBAL: PLAYER 0x00b8d3a4
	// GLOBAL: STUDIO 0x00c637c4
	const char* const sRightMotorTool = "RightMotor";

	// GLOBAL: PLAYER 0x00b8d3a8
	// GLOBAL: STUDIO 0x00c637c8
	const char* const sLeftMotorTool = "LeftMotor";

	// GLOBAL: PLAYER 0x00b8d3ac
	// GLOBAL: STUDIO 0x00c637cc
	const char* const sOscillateMotorTool = "OscillateMotor";

	// GLOBAL: PLAYER 0x00b8d3b0
	// GLOBAL: STUDIO 0x00c637d0
	const char* const sSmoothNoOutlinesTool = "SmoothNoOutlines";

} // namespace RBX
