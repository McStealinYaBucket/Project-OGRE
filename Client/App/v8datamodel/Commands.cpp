/* App/V8DataModel/Commands.cpp -- October 2013, partial: STUB bodies for the verbs
   Game::setupDataModel builds (Commands.h), in address order, so their calls and vftable slots pair.
   2013 asserts in this file sit at lines 116..1135; nothing here is laid out to them yet. */

#include "stdafx.h"

#include "V8DataModel/Commands.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/Workspace.h"
#include "V8DataModel/Selection.h"

#include "decomp.h"

namespace RBX {

	// STUB: PLAYER 0x00856130
	// STUB: STUDIO 0x00780750
	ToggleViewMode::ToggleViewMode(DataModel* dataModel)
		: Verb(dataModel, "ToggleViewMode")
		, dataModel(dataModel)
	{
		STUB(0x00856130);
	}

	// STUB: PLAYER 0x008561b0
	// STUB: STUDIO 0x007807d0
	bool ToggleViewMode::isEnabled() const
	{
		STUB(0x008561b0);
		return true;
	}

	// STUB: PLAYER 0x008561d0
	// STUB: STUDIO 0x007807f0
	void ToggleViewMode::doIt(IDataState* dataState)
	{
		STUB(0x008561d0);
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

	// STUB: PLAYER 0x008565a0
	// STUB: STUDIO 0x00780bd0
	void CameraVerb::doIt(IDataState* dataState)
	{
		STUB(0x008565a0);
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

	// STUB: PLAYER 0x00857810
	// STUB: STUDIO 0x00781f70
	CameraVerb::CameraVerb(std::string name, Workspace* workspace)
		: Verb(NULL, name)	// 2013 passes the Workspace's VerbContainer base (+0x150); our Workspace lacks it yet
		, workspace(workspace)
		, selection(workspace)
	{
		STUB(0x00857810);
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

} // namespace RBX
