/* V8DataModel/CommonVerbs.h -- October 2013. The verbs a DataModel's Game holds (Game::setupDataModel,
   PLAYER 0x00655630, news 0x724 bytes and calls CommonVerbs(DataModel*) 0x00864450). Members, offsets
   and types from that ctor (RTTI of each vptr it stores or each ctor it calls) and from the destructor
   (PLAYER 0x006531e0, STUDIO 0x007cbbb0, emitted by Game.obj), which destroys them from +0x704 down.
   STUDIO has the same layout. Against 2016: Group/Ungroup verbs, no Color/Material verbs, SetGridToOne /
   OneFifth / Off and no SetGridSizeTo*, TToolVerb<NullTool>, member names are the 2016 ones where a
   2016 member of that class exists and the class name otherwise. Member classes: Commands.h. */

#pragma once

#include "V8DataModel/Commands.h"

namespace RBX {

	class DataModel;

	// SIZE 0x724
	class CommonVerbs
	{
	public:
		CommonVerbs(DataModel* dataModel);
		~CommonVerbs();

		PlayDeleteSelectionVerb      playDeleteSelectionVerb;     // 0x000
		DeleteSelectionVerb          deleteSelectionVerb;         // 0x028
		SelectAllCommand             selectAllCommand;            // 0x050
		GroupSelectionVerb           groupSelectionVerb;          // 0x06c
		UngroupSelectionVerb         ungroupSelectionVerb;        // 0x090
		SelectChildrenVerb           selectChildrenVerb;          // 0x0b4
		SnapSelectionVerb            snapSelectionVerb;           // 0x0d8
		UnlockAllVerb                unlockAllVerb;               // 0x0fc
		AnchorVerb                   anchorVerb;                  // 0x118
		TranslucentVerb              translucentVerb;             // 0x140
		CanCollideVerb               canCollideVerb;              // 0x168
		CanNotSelectCommand          canNotSelectCommand;         // 0x190
		AllCanSelectCommand          allCanSelectCommand;         // 0x1b4
		MoveUpPlateVerb              moveUpPlateVerb;             // 0x1d0
		MoveUpBrickVerb              moveUpBrickVerb;             // 0x1f8
		MoveDownSelectionVerb        moveDownSelectionVerb;       // 0x220
		RotateSelectionVerb          rotateSelectionVerb;         // 0x244
		TiltSelectionVerb            tiltSelectionVerb;           // 0x268
		RunCommand                   runCommand;                  // 0x28c
		StopCommand                  stopCommand;                 // 0x2a8
		ResetCommand                 resetCommand;                // 0x2c4
		FirstPersonCommand           firstPersonCommand;          // 0x2e0
		StatsCommand                 statsCommand;                // 0x2f0
		RenderStatsCommand           renderStatsCommand;          // 0x300
		EngineStatsCommand           engineStatsCommand;          // 0x310
		NetworkStatsCommand          networkStatsCommand;         // 0x320
		PhysicsStatsCommand          physicsStatsCommand;         // 0x334
		SummaryStatsCommand          summaryStatsCommand;         // 0x344
		CustomStatsCommand           customStatsCommand;          // 0x354
		JoinCommand                  joinCommand;                 // 0x364
		TurnOnManualJointCreation    turnOnManualJointCreation;   // 0x374
		SetGridToOne                 setGridToOne;                // 0x384
		SetGridToOneFifth            setGridToOneFifth;           // 0x394
		SetGridToOff                 setGridToOff;                // 0x3a4
		SetManualJointToWeak         setManualJointToWeak;        // 0x3b4
		SetManualJointToStrong       setManualJointToStrong;      // 0x3c4
		SetManualJointToInfinite     setManualJointToInfinite;    // 0x3d4
		TToolVerb<AxisRotateTool>    axisRotateToolVerb;          // 0x3e4
		TToolVerb<AdvMoveTool>       advMoveToolVerb;             // 0x404
		TToolVerb<AdvRotateTool>     advRotateToolVerb;           // 0x424
		TToolVerb<AdvArrowTool>      advArrowToolVerb;            // 0x444
		TToolVerb<MoveResizeJoinTool> moveResizeJoinToolVerb;      // 0x464
		TToolVerb<FlatTool>          flatToolVerb;                // 0x484
		TToolVerb<GlueTool>          glueToolVerb;                // 0x4a4
		TToolVerb<WeldTool>          weldToolVerb;                // 0x4c4
		TToolVerb<StudsTool>         studsToolVerb;               // 0x4e4
		TToolVerb<InletTool>         inletToolVerb;               // 0x504
		TToolVerb<UniversalTool>     universalToolVerb;           // 0x524
		TToolVerb<HingeTool>         hingeToolVerb;               // 0x544
		TToolVerb<RightMotorTool>    rightMotorToolVerb;          // 0x564
		TToolVerb<LeftMotorTool>     leftMotorToolVerb;           // 0x584
		TToolVerb<OscillateMotorTool> oscillateMotorToolVerb;      // 0x5a4
		TToolVerb<SmoothNoOutlinesTool> smoothNoOutlinesToolVerb;    // 0x5c4
		TToolVerb<AnchorTool>        anchorToolVerb;              // 0x5e4
		TToolVerb<LockTool>          lockToolVerb;                // 0x604
		TToolVerb<FillTool>          fillToolVerb;                // 0x624
		TToolVerb<MaterialTool>      materialToolVerb;            // 0x644
		TToolVerb<DropperTool>       dropperToolVerb;             // 0x664
		TToolVerb<NullTool>          nullToolVerb;                // 0x684
		TToolVerb<GameTool>          gameToolVerb;                // 0x6a4
		TToolVerb<GrabTool>          grabToolVerb;                // 0x6c4
		TToolVerb<CloneTool>         cloneToolVerb;               // 0x6e4
		TToolVerb<HammerTool>        hammerToolVerb;              // 0x704
	};

} // namespace RBX
