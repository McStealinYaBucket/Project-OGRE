/* App/tool/ToolsArrow.cpp -- October 2013, partial (2013 path Tool\ToolsArrow.cpp). sAdvArrowTool (read by
   TToolVerb<AdvArrowTool>'s ctor) and AdvArrowToolBase's statics (Tool/ToolsArrow.h). */

#include "stdafx.h"

#include "Tool/ToolsArrow.h"

namespace RBX {

	extern const char* const sAdvArrowTool;	// also in V8DataModel/Commands.h, which this TU leaves out (its templates add COMDATs)

	// GLOBAL: PLAYER 0x00b8d590
	// GLOBAL: STUDIO 0x00beec18
	const char* const sAdvArrowTool = "AdvArrow";

	// GLOBAL: PLAYER 0x00d98b40
	// GLOBAL: STUDIO 0x00eb3b94
	bool AdvArrowToolBase::advManualJointMode = true;

	// GLOBAL: PLAYER 0x00d98b44
	// GLOBAL: STUDIO 0x00eb3b98
	DRAG::ManualJointType AdvArrowToolBase::advManualJointType = DRAG::STRONG_MANUAL_JOINT;

	// GLOBAL: PLAYER 0x00e14628
	// GLOBAL: STUDIO 0x00f694f0
	DRAG::DraggerGridMode AdvArrowToolBase::advGridMode = DRAG::ONE_STUD;

} // namespace RBX
