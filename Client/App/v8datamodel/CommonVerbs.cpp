/* App/v8datamodel/CommonVerbs.cpp -- October 2013. No 2013 __FILE__ names it; the path follows the 2016
   tree. The ctor (PLAYER 0x00864450) only constructs the members (CommonVerbs.h). */

#include "stdafx.h"

#include "V8DataModel/CommonVerbs.h"
#include "V8DataModel/DataModel.h"

namespace RBX {

	// 2013 compiles these three in CommonVerbs.obj (calls to BoolPropertyVerb's ctor in Commands.obj).
	// FUNCTION: PLAYER 0x00860d10
	// FUNCTION: STUDIO 0x009d28c0
	TranslucentVerb::TranslucentVerb(DataModel* dataModel)
		: BoolPropertyVerb("TranslucentVerb", dataModel, "Transparent")
	{
	}

	// FUNCTION: PLAYER 0x00860d90
	// FUNCTION: STUDIO 0x009d2940
	CanCollideVerb::CanCollideVerb(DataModel* dataModel)
		: BoolPropertyVerb("CanCollideVerb", dataModel, "CanCollide")
	{
	}

	// FUNCTION: PLAYER 0x00861a40
	// FUNCTION: STUDIO 0x009d3580
	AnchorVerb::AnchorVerb(DataModel* dataModel)
		: BoolPropertyVerb("AnchorVerb", dataModel, "Anchored")
	{
	}

	// FUNCTION: PLAYER 0x00864450
	// FUNCTION: STUDIO 0x009d5e90
	CommonVerbs::CommonVerbs(DataModel* dataModel)
		: playDeleteSelectionVerb(dataModel)
		, deleteSelectionVerb(dataModel)
		, selectAllCommand(dataModel)
		, groupSelectionVerb(dataModel)
		, ungroupSelectionVerb(dataModel)
		, selectChildrenVerb(dataModel)
		, snapSelectionVerb(dataModel)
		, unlockAllVerb(dataModel)
		, anchorVerb(dataModel)
		, translucentVerb(dataModel)
		, canCollideVerb(dataModel)
		, canNotSelectCommand(dataModel)
		, allCanSelectCommand(dataModel)
		, moveUpPlateVerb(dataModel)
		, moveUpBrickVerb(dataModel)
		, moveDownSelectionVerb(dataModel)
		, rotateSelectionVerb(dataModel)
		, tiltSelectionVerb(dataModel)
		, runCommand(dataModel)
		, stopCommand(dataModel)
		, resetCommand(dataModel)
		, firstPersonCommand(dataModel)
		, statsCommand(dataModel)
		, renderStatsCommand(dataModel)
		, engineStatsCommand(dataModel)
		, networkStatsCommand(dataModel)
		, physicsStatsCommand(dataModel)
		, summaryStatsCommand(dataModel)
		, customStatsCommand(dataModel)
		, joinCommand(dataModel)
		, turnOnManualJointCreation(dataModel)
		, setGridToOne(dataModel)
		, setGridToOneFifth(dataModel)
		, setGridToOff(dataModel)
		, setManualJointToWeak(dataModel)
		, setManualJointToStrong(dataModel)
		, setManualJointToInfinite(dataModel)
		, axisRotateToolVerb(dataModel)
		, advMoveToolVerb(dataModel)
		, advRotateToolVerb(dataModel)
		, advArrowToolVerb(dataModel, false)
		, moveResizeJoinToolVerb(dataModel)
		, flatToolVerb(dataModel)
		, glueToolVerb(dataModel)
		, weldToolVerb(dataModel)
		, studsToolVerb(dataModel)
		, inletToolVerb(dataModel)
		, universalToolVerb(dataModel)
		, hingeToolVerb(dataModel)
		, rightMotorToolVerb(dataModel)
		, leftMotorToolVerb(dataModel)
		, oscillateMotorToolVerb(dataModel)
		, smoothNoOutlinesToolVerb(dataModel)
		, anchorToolVerb(dataModel)
		, lockToolVerb(dataModel)
		, fillToolVerb(dataModel, false)
		, materialToolVerb(dataModel, false)
		, dropperToolVerb(dataModel)
		, nullToolVerb(dataModel)
		, gameToolVerb(dataModel)
		, grabToolVerb(dataModel)
		, cloneToolVerb(dataModel)
		, hammerToolVerb(dataModel)
	{
	}

} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::CommonVerbs, 0x724);
DECOMP_SIZE_ASSERT(RBX::RunStateVerb, 0x1c);
DECOMP_SIZE_ASSERT(RBX::EditSelectionVerb, 0x24);
DECOMP_SIZE_ASSERT(RBX::DeleteBase, 0x28);
DECOMP_SIZE_ASSERT(RBX::NetworkStatsCommand, 0x14);
DECOMP_SIZE_ASSERT(RBX::TToolVerb<RBX::HammerTool>, 0x20);
