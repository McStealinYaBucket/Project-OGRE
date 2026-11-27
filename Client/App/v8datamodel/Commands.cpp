/* App/V8DataModel/Commands.cpp -- October 2013, partial: STUB bodies for the verbs
   Game::setupDataModel builds (Commands.h), in address order, so their calls and vftable slots pair.
   2013 asserts in this file sit at lines 116..1135; only CameraVerb::doIt is laid out to its line yet. */

#include "stdafx.h"

#include "V8DataModel/Commands.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/Workspace.h"
#include "V8DataModel/Selection.h"

#include "decomp.h"

namespace RBX {

	// The folded `ret 4` (PLAYER 0x00575d00).
	void FirstPersonCommand::doIt(IDataState* dataState)
	{
	}

	// FUNCTION: PLAYER 0x00856090
	// FUNCTION: STUDIO 0x007806b0
	FirstPersonCommand::FirstPersonCommand(DataModel* dataModel)
		: Verb(dataModel, "FirstPersonIndicator")
		, dataModel(dataModel)
	{
	}

	// FUNCTION: PLAYER 0x00856130
	// FUNCTION: STUDIO 0x00780750
	ToggleViewMode::ToggleViewMode(DataModel* dataModel)
		: Verb(dataModel, "ToggleViewMode")
		, dataModel(dataModel)
	{
	}

	// FUNCTION: PLAYER 0x008561b0
	// FUNCTION: STUDIO 0x007807d0
	bool ToggleViewMode::isEnabled() const
	{
		return dataModel->getNumPlayers() > 0;
	}

	// FUNCTION: PLAYER 0x008561d0
	// FUNCTION: STUDIO 0x007807f0
	void ToggleViewMode::doIt(IDataState* dataState)
	{
		FASTLOG(FLog::Verbs, "Gui:ToggleViewMode");
		dataModel->switchViewMode();
	}

	// Both fold into the engine's `xor al,al; ret` (PLAYER 0x00927cc0).
	bool ToggleViewMode::isChecked() const
	{
		return false;
	}

	bool ToggleViewMode::isSelected() const
	{
		return false;
	}

	// FUNCTION: PLAYER 0x00856200
	// FUNCTION: STUDIO 0x00780820
	StatsCommand::StatsCommand(DataModel* dataModel)
		: Verb(dataModel, "Stats")
		, dataModel(dataModel)
	{
	}

	// FUNCTION: PLAYER 0x00856280
	// FUNCTION: STUDIO 0x007808a0
	RenderStatsCommand::RenderStatsCommand(DataModel* dataModel)
		: Verb(dataModel, "RenderStats")
		, dataModel(dataModel)
	{
	}

	// FUNCTION: PLAYER 0x00856300
	// FUNCTION: STUDIO 0x00780920
	SummaryStatsCommand::SummaryStatsCommand(DataModel* dataModel)
		: Verb(dataModel, "SummaryStats")
		, dataModel(dataModel)
	{
	}

	// FUNCTION: PLAYER 0x00856380
	// FUNCTION: STUDIO 0x007809a0
	CustomStatsCommand::CustomStatsCommand(DataModel* dataModel)
		: Verb(dataModel, "CustomStats")
		, dataModel(dataModel)
	{
	}

	// FUNCTION: PLAYER 0x00856400
	// FUNCTION: STUDIO 0x00780a30
	PhysicsStatsCommand::PhysicsStatsCommand(DataModel* dataModel)
		: Verb(dataModel, "PhysicsStats")
		, dataModel(dataModel)
	{
	}

	// FUNCTION: PLAYER 0x00856480
	// FUNCTION: STUDIO 0x00780ab0
	EngineStatsCommand::EngineStatsCommand(DataModel* dataModel)
		: Verb(dataModel, "EngineStats")
		, dataModel(dataModel)
	{
	}

	// FUNCTION: PLAYER 0x00856500
	// FUNCTION: STUDIO 0x00780b30
	JoinCommand::JoinCommand(DataModel* dataModel)
		: Verb(dataModel, "JoinCommand")
		, dataModel(dataModel)
	{
	}

	// STUB: PLAYER 0x00856580
	G3D::Matrix3 RotateSelectionVerb::getRotationAxis()
	{
		STUB(0x00856580);
		return G3D::Matrix3::identity();
	}







































































































































































































































































































































































































































































































































































































































































































































































































































































































































































	// The asserts sit on their 2013 lines (PLAYER 1073, STUDIO 1066).
	// FUNCTION: PLAYER 0x008565a0
	// FUNCTION: STUDIO 0x00780bd0
	void CameraVerb::doIt(IDataState* dataState)
	{
#ifdef OGRE_TARGET_STUDIO
		RBXASSERT(dataState!=NULL);
#endif




#ifndef OGRE_TARGET_STUDIO
		RBXASSERT(dataState!=NULL);
#endif
		dataState->setDirty(true);
	}

	// STUB: PLAYER 0x00856600
	// STUB: STUDIO 0x00780c30
	void CameraPanLeftCommand::doIt(IDataState* dataState)
	{
		STUB(0x00856600);
	}

	// STUB: PLAYER 0x00856650
	// STUB: STUDIO 0x00780c80
	void CameraPanRightCommand::doIt(IDataState* dataState)
	{
		STUB(0x00856650);
	}

	// STUB: PLAYER 0x008566a0
	// STUB: STUDIO 0x00780cd0
	bool CameraTiltUpCommand::isEnabled() const
	{
		STUB(0x008566a0);
		return true;
	}

	// STUB: PLAYER 0x008566c0
	// STUB: STUDIO 0x00780cf0
	void CameraTiltUpCommand::doIt(IDataState* dataState)
	{
		STUB(0x008566c0);
	}

	// STUB: PLAYER 0x00856710
	// STUB: STUDIO 0x00780d40
	bool CameraTiltDownCommand::isEnabled() const
	{
		STUB(0x00856710);
		return true;
	}

	// STUB: PLAYER 0x00856730
	// STUB: STUDIO 0x00780d60
	void CameraTiltDownCommand::doIt(IDataState* dataState)
	{
		STUB(0x00856730);
	}

	// STUB: PLAYER 0x00856780
	// STUB: STUDIO 0x00780db0
	bool CameraZoomInCommand::isEnabled() const
	{
		STUB(0x00856780);
		return true;
	}

	// STUB: PLAYER 0x008567a0
	// STUB: STUDIO 0x00780dd0
	void CameraZoomInCommand::doIt(IDataState* dataState)
	{
		STUB(0x008567a0);
	}

	// STUB: PLAYER 0x00856800
	// STUB: STUDIO 0x00780e30
	bool CameraZoomOutCommand::isEnabled() const
	{
		STUB(0x00856800);
		return true;
	}

	// STUB: PLAYER 0x00856820
	// STUB: STUDIO 0x00780e50
	void CameraZoomOutCommand::doIt(IDataState* dataState)
	{
		STUB(0x00856820);
	}

	// FUNCTION: PLAYER 0x00856880
	// FUNCTION: STUDIO 0x00780eb0
	TurnOnManualJointCreation::TurnOnManualJointCreation(DataModel* dataModel)
		: Verb(dataModel, "TurnOnManualJointCreation")
		, dataModel(dataModel)
	{
	}

	// STUB: PLAYER 0x00856900
	void TurnOnManualJointCreation::doIt(IDataState* dataState)
	{
		STUB(0x00856900);
	}

	// FUNCTION: PLAYER 0x00856950
	// FUNCTION: STUDIO 0x00780f90
	SetGridToOne::SetGridToOne(DataModel* dataModel)
		: Verb(dataModel, "SetGridToOne")
		, dataModel(dataModel)
	{
	}

	// STUB: PLAYER 0x008569e0
	void SetGridToOne::doIt(IDataState* dataState)
	{
		STUB(0x008569e0);
	}

	// FUNCTION: PLAYER 0x008569f0
	// FUNCTION: STUDIO 0x00781030
	SetGridToOneFifth::SetGridToOneFifth(DataModel* dataModel)
		: Verb(dataModel, "SetGridToOneFifth")
		, dataModel(dataModel)
	{
	}

	// STUB: PLAYER 0x00856a80
	void SetGridToOneFifth::doIt(IDataState* dataState)
	{
		STUB(0x00856a80);
	}

	// FUNCTION: PLAYER 0x00856a90
	// FUNCTION: STUDIO 0x007810d0
	SetGridToOff::SetGridToOff(DataModel* dataModel)
		: Verb(dataModel, "SetGridToOff")
		, dataModel(dataModel)
	{
	}

	// STUB: PLAYER 0x00856b20
	void SetGridToOff::doIt(IDataState* dataState)
	{
		STUB(0x00856b20);
	}

	// FUNCTION: PLAYER 0x00856b30
	// FUNCTION: STUDIO 0x00781170
	SetManualJointToWeak::SetManualJointToWeak(DataModel* dataModel)
		: Verb(dataModel, "SetManualJointToWeak")
		, dataModel(dataModel)
	{
	}

	// STUB: PLAYER 0x00856bb0
	void SetManualJointToWeak::doIt(IDataState* dataState)
	{
		STUB(0x00856bb0);
	}

	// FUNCTION: PLAYER 0x00856bc0
	// FUNCTION: STUDIO 0x00781200
	SetManualJointToStrong::SetManualJointToStrong(DataModel* dataModel)
		: Verb(dataModel, "SetManualJointToStrong")
		, dataModel(dataModel)
	{
	}

	// STUB: PLAYER 0x00856c60
	void SetManualJointToStrong::doIt(IDataState* dataState)
	{
		STUB(0x00856c60);
	}

	// FUNCTION: PLAYER 0x00856c70
	// FUNCTION: STUDIO 0x007812a0
	SetManualJointToInfinite::SetManualJointToInfinite(DataModel* dataModel)
		: Verb(dataModel, "SetManualJointToInfinite")
		, dataModel(dataModel)
	{
	}

	// STUB: PLAYER 0x00856cf0
	void SetManualJointToInfinite::doIt(IDataState* dataState)
	{
		STUB(0x00856cf0);
	}

	// STUB: PLAYER 0x00856e20
	void SelectAllCommand::doIt(IDataState* dataState)
	{
		STUB(0x00856e20);
	}

	// STUB: PLAYER 0x00856ee0
	void EngineStatsCommand::doIt(IDataState* dataState)
	{
		STUB(0x00856ee0);
	}

	// STUB: PLAYER 0x00856f20
	void UnlockAllVerb::doIt(IDataState* dataState)
	{
		STUB(0x00856f20);
	}

	// Folds with UnlockAllVerb::doIt (PLAYER 0x00856f20).
	void AllCanSelectCommand::doIt(IDataState* dataState)
	{
		STUB(0x00856f20);
	}

	// STUB: PLAYER 0x00857010
	G3D::Matrix3 TiltSelectionVerb::getRotationAxis()
	{
		STUB(0x00857010);
		return G3D::Matrix3::identity();
	}

	// FUNCTION: PLAYER 0x00857430
	// FUNCTION: STUDIO 0x00781a70
	RunStateVerb::~RunStateVerb()
	{
	}

	// FUNCTION: PLAYER 0x008574b0
	// FUNCTION: STUDIO 0x00781af0
	EditSelectionVerb::~EditSelectionVerb()
	{
	}

	// FUNCTION: PLAYER 0x00857780
	// FUNCTION: STUDIO 0x00781ee0
	RunStateVerb::RunStateVerb(std::string name, DataModel* dataModel)
		: Verb(dataModel, name)
		, dataModel(dataModel)
		, runService(dataModel)
	{
	}

	// STUB: PLAYER 0x00857810
	// STUB: STUDIO 0x00781f70
	CameraVerb::CameraVerb(std::string name, Workspace* workspace)
		: Verb(NULL, name)	// 2013 passes the Workspace's VerbContainer base (+0x150); our Workspace lacks it yet
		, workspace(workspace)
		, selection(workspace)
	{
		STUB(0x00857810);
	}

	// STUB: PLAYER 0x00858b70
	// STUB: STUDIO 0x00783430
	NetworkStatsCommand::NetworkStatsCommand(DataModel* dataModel)
		: Verb(NULL, "NetworkStatsCommand")
		, dataModel(dataModel)
	{
		STUB(0x00858b70);
	}

	// FUNCTION: PLAYER 0x00858c10
	// STUB: STUDIO 0x007834c0
	EditSelectionVerb::EditSelectionVerb(std::string name, DataModel* dataModel)
		: Verb(dataModel, name)
		, workspace(shared_from(dataModel->getWorkspace()))
		, selection(dataModel)
		, dataModel(dataModel)
	{
	}

	// FUNCTION: PLAYER 0x00858cb0
	// FUNCTION: STUDIO 0x00783560
	GroupSelectionVerb::GroupSelectionVerb(DataModel* dataModel)
		: EditSelectionVerb("Group", dataModel)
	{
	}

	// FUNCTION: PLAYER 0x00858ce0
	// FUNCTION: STUDIO 0x00783590
	SnapSelectionVerb::SnapSelectionVerb(DataModel* dataModel)
		: EditSelectionVerb("SnapSelection", dataModel)
	{
	}

	// FUNCTION: PLAYER 0x00858d10
	// FUNCTION: STUDIO 0x00783740
	DeleteBase::DeleteBase(DataModel* dataModel, std::string name, bool rewardHopper)
		: EditSelectionVerb(name, dataModel)
		, rewardHopper(rewardHopper)
	{
	}

	// FUNCTION: PLAYER 0x00858d90
	// FUNCTION: STUDIO 0x007837c0
	MoveDownSelectionVerb::MoveDownSelectionVerb(DataModel* dataModel)
		: EditSelectionVerb("SelectionDown", dataModel)
	{
	}

	// FUNCTION: PLAYER 0x008590c0
	// FUNCTION: STUDIO 0x00783980
	RotateAxisCommand::RotateAxisCommand(std::string name, DataModel* dataModel)
		: EditSelectionVerb(name, dataModel)
	{
	}

	// STUB: PLAYER 0x00859130
	BoolPropertyVerb::BoolPropertyVerb(const std::string& name, DataModel* dataModel, const char* propertyName)
		: EditSelectionVerb(name, dataModel)
	{
		STUB(0x00859130);
	}

	// STUB: PLAYER 0x00859280
	void JoinCommand::doIt(IDataState* dataState)
	{
		STUB(0x00859280);
	}

	// FUNCTION: PLAYER 0x008593a0
	// FUNCTION: STUDIO 0x00783dd0
	RotateSelectionVerb::RotateSelectionVerb(DataModel* dataModel)
		: RotateAxisCommand("SelectionRotate", dataModel)
	{
	}

	// STUB: PLAYER 0x008593d0
	void RotateAxisCommand::doIt(IDataState* dataState)
	{
		STUB(0x008593d0);
	}

	// FUNCTION: PLAYER 0x00859480
	// FUNCTION: STUDIO 0x00783eb0
	TiltSelectionVerb::TiltSelectionVerb(DataModel* dataModel)
		: RotateAxisCommand("SelectionTilt", dataModel)
	{
	}

	// STUB: PLAYER 0x008594b0
	void MoveUpSelectionVerb::doIt(IDataState* dataState)
	{
		STUB(0x008594b0);
	}

	// STUB: PLAYER 0x008595e0
	void MoveDownSelectionVerb::doIt(IDataState* dataState)
	{
		STUB(0x008595e0);
	}

	// STUB: PLAYER 0x00859730
	// STUB: STUDIO 0x00784160
	UngroupSelectionVerb::UngroupSelectionVerb(DataModel* dataModel)
		: EditSelectionVerb("UngroupSelectionVerb", dataModel)
	{
		STUB(0x00859730);
	}

	// STUB: PLAYER 0x008597a0
	void UngroupSelectionVerb::doIt(IDataState* dataState)
	{
		STUB(0x008597a0);
	}

	// STUB: PLAYER 0x00859aa0
	// STUB: STUDIO 0x007844d0
	SelectChildrenVerb::SelectChildrenVerb(DataModel* dataModel)
		: EditSelectionVerb("SelectChildrenVerb", dataModel)
	{
		STUB(0x00859aa0);
	}

	// STUB: PLAYER 0x00859b10
	void SelectChildrenVerb::doIt(IDataState* dataState)
	{
		STUB(0x00859b10);
	}

	// STUB: PLAYER 0x00859fd0
	void StatsCommand::doIt(IDataState* dataState)
	{
		STUB(0x00859fd0);
	}

	// STUB: PLAYER 0x0085a1e0
	void RenderStatsCommand::doIt(IDataState* dataState)
	{
		STUB(0x0085a1e0);
	}

	// STUB: PLAYER 0x0085a490
	void SummaryStatsCommand::doIt(IDataState* dataState)
	{
		STUB(0x0085a490);
	}

	// STUB: PLAYER 0x0085a6c0
	void CustomStatsCommand::doIt(IDataState* dataState)
	{
		STUB(0x0085a6c0);
	}

	// STUB: PLAYER 0x0085a8f0
	void NetworkStatsCommand::doIt(IDataState* dataState)
	{
		STUB(0x0085a8f0);
	}

	// STUB: PLAYER 0x0085ac10
	void PhysicsStatsCommand::doIt(IDataState* dataState)
	{
		STUB(0x0085ac10);
	}

	// STUB: PLAYER 0x0085afc0
	void GroupSelectionVerb::doIt(IDataState* dataState)
	{
		STUB(0x0085afc0);
	}

	// STUB: PLAYER 0x0085b2a0
	// STUB: STUDIO 0x00785d10
	CameraCenterCommand::CameraCenterCommand(Workspace* workspace)
		: CameraVerb("CameraCenter", workspace)
	{
		STUB(0x0085b2a0);
	}

	// STUB: PLAYER 0x0085b310
	// STUB: STUDIO 0x00785d80
	CameraZoomExtentsCommand::CameraZoomExtentsCommand(Workspace* workspace)
		: CameraVerb("CameraZoomExtents", workspace)
	{
		STUB(0x0085b310);
	}

	// STUB: PLAYER 0x0085b9a0
	void DeleteBase::doIt(IDataState* dataState)
	{
		STUB(0x0085b9a0);
	}

	// STUB: PLAYER 0x0085bc60
	void BoolPropertyVerb::doIt(IDataState* dataState)
	{
		STUB(0x0085bc60);
	}

	// STUB: PLAYER 0x0085bd10
	// STUB: STUDIO 0x00786600
	bool CameraCenterCommand::isEnabled() const
	{
		STUB(0x0085bd10);
		return true;
	}

	// STUB: PLAYER 0x0085be30
	// STUB: STUDIO 0x00786720
	void CameraCenterCommand::doIt(IDataState* dataState)
	{
		STUB(0x0085be30);
	}

	// STUB: PLAYER 0x0085c180
	void CanNotSelectCommand::doIt(IDataState* dataState)
	{
		STUB(0x0085c180);
	}

	// STUB: PLAYER 0x0085c270
	void RunCommand::doIt(IDataState* dataState)
	{
		STUB(0x0085c270);
	}

	// STUB: PLAYER 0x0085c320
	void StopCommand::doIt(IDataState* dataState)
	{
		STUB(0x0085c320);
	}

	// STUB: PLAYER 0x0085c3f0
	void ResetCommand::doIt(IDataState* dataState)
	{
		STUB(0x0085c3f0);
	}

	// STUB: PLAYER 0x0085c530
	void SnapSelectionVerb::doIt(IDataState* dataState)
	{
		STUB(0x0085c530);
	}

	// STUB: PLAYER 0x0085c6f0
	// STUB: STUDIO 0x00786fe0
	bool CameraZoomExtentsCommand::isEnabled() const
	{
		STUB(0x0085c6f0);
		return true;
	}

	// STUB: PLAYER 0x0085c830
	// STUB: STUDIO 0x00787120
	void CameraZoomExtentsCommand::doIt(IDataState* dataState)
	{
		STUB(0x0085c830);
	}

	// STUB: PLAYER 0x00860d10
	// STUB: STUDIO 0x009d28c0
	TranslucentVerb::TranslucentVerb(DataModel* dataModel)
		: BoolPropertyVerb("TranslucentVerb", dataModel, "Transparent")
	{
	}

	// STUB: PLAYER 0x00860d90
	// STUB: STUDIO 0x009d2940
	CanCollideVerb::CanCollideVerb(DataModel* dataModel)
		: BoolPropertyVerb("CanCollideVerb", dataModel, "CanCollide")
	{
	}

	// FUNCTION: PLAYER 0x00860e10
	// FUNCTION: STUDIO 0x009d29c0
	MoveUpPlateVerb::MoveUpPlateVerb(DataModel* dataModel)
		: MoveUpSelectionVerb(dataModel, "SelectionUpPlate", 0.4f)
	{
	}

	// FUNCTION: PLAYER 0x00860eb0
	// FUNCTION: STUDIO 0x009d2a60
	MoveUpBrickVerb::MoveUpBrickVerb(DataModel* dataModel)
		: MoveUpSelectionVerb(dataModel, "SelectionUpBrick", 1.2f)
	{
	}

	// STUB: PLAYER 0x00861a40
	// STUB: STUDIO 0x009d3580
	AnchorVerb::AnchorVerb(DataModel* dataModel)
		: BoolPropertyVerb("AnchorVerb", dataModel, "Anchored")
	{
	}

} // namespace RBX
