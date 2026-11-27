/* V8DataModel/Commands.h -- October 2013, partial: the camera verbs Game::setupDataModel builds
   (PLAYER 0x00655630) and ToggleViewMode. 2013 __FILE__: Client\App\V8DataModel\Commands.cpp
   (asserts at lines 116..1135); no string names the header, its path follows the .cpp.

   RTTI: each Camera*Command derives CameraVerb : Verb : Debugable. Layout from the CameraVerb ctor
   (PLAYER 0x00857810, ret 0x20: std::string by value, then Workspace*): Verb (0xc), workspace at
   0xc, ServiceClient<Selection> at 0x10 (context, shared_ptr), size 0x1c (setupDataModel news 0x1c).
   The six Camera*Command ctors are inline (setupDataModel calls CameraVerb's ctor, then stores the
   derived vptr), so Game.obj emits their vftables; their deleting destructors fold with CameraVerb's
   (PLAYER 0x00653a40, STUDIO 0x007cc4e0, also Game.obj). CameraVerb's isEnabled is inline
   `return true` (vftable slot 2 = the folded `mov al,1; ret`); PanLeft/PanRight do not override it.
   The verb classes CommonVerbs holds follow at the end (RunStateVerb, EditSelectionVerb and their subclasses). */

#pragma once

#include "V8Tree/Verb.h"
#include "V8Tree/Service.h"
#include "G3D/Matrix3.h"
#include "decomp.h"

#include <string>

namespace RBX {

	class DataModel;
	class Selection;
	class Workspace;

	// VTABLE: PLAYER 0x00b7c16c
	// VTABLE: STUDIO 0x00bfa9fc
	// SIZE 0x1c
	class CameraVerb : public Verb
	{
	protected:
		Workspace* workspace;				// 0x0c
		ServiceClient<Selection> selection;	// 0x10

	public:
		CameraVerb(std::string name, Workspace* workspace);
		/*override*/ virtual bool isEnabled() const { return true; }	// vtable+0x08
		/*override*/ virtual void doIt(IDataState* dataState);			// vtable+0x18
	};

	// VTABLE: PLAYER 0x00b30f84
	// VTABLE: STUDIO 0x00c0a878
	class CameraPanLeftCommand : public CameraVerb
	{
	public:
		CameraPanLeftCommand(Workspace* workspace) : CameraVerb("CameraPanLeft", workspace) {}
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};

	// VTABLE: PLAYER 0x00b30fa4
	// VTABLE: STUDIO 0x00c0a898
	class CameraPanRightCommand : public CameraVerb
	{
	public:
		CameraPanRightCommand(Workspace* workspace) : CameraVerb("CameraPanRight", workspace) {}
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};

	// VTABLE: PLAYER 0x00b31004
	// VTABLE: STUDIO 0x00c0a8f8
	class CameraTiltUpCommand : public CameraVerb
	{
	public:
		CameraTiltUpCommand(Workspace* workspace) : CameraVerb("CameraTiltUp", workspace) {}
		/*override*/ virtual bool isEnabled() const;				// vtable+0x08
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};

	// VTABLE: PLAYER 0x00b30f64
	// VTABLE: STUDIO 0x00c0a858
	class CameraTiltDownCommand : public CameraVerb
	{
	public:
		CameraTiltDownCommand(Workspace* workspace) : CameraVerb("CameraTiltDown", workspace) {}
		/*override*/ virtual bool isEnabled() const;				// vtable+0x08
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};

	// VTABLE: PLAYER 0x00b30fc4
	// VTABLE: STUDIO 0x00c0a8b8
	class CameraZoomInCommand : public CameraVerb
	{
	public:
		CameraZoomInCommand(Workspace* workspace) : CameraVerb("CameraZoomIn", workspace) {}
		/*override*/ virtual bool isEnabled() const;				// vtable+0x08
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};

	// VTABLE: PLAYER 0x00b30fe4
	// VTABLE: STUDIO 0x00c0a8d8
	class CameraZoomOutCommand : public CameraVerb
	{
	public:
		CameraZoomOutCommand(Workspace* workspace) : CameraVerb("CameraZoomOut", workspace) {}
		/*override*/ virtual bool isEnabled() const;				// vtable+0x08
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};

	// Out-of-line ctors (PLAYER 0x0085b2a0 / 0x0085b310), vftables emitted in Commands.obj.
	// VTABLE: PLAYER 0x00b7c670
	// VTABLE: STUDIO 0x00bfb03c
	class CameraCenterCommand : public CameraVerb
	{
	public:
		CameraCenterCommand(Workspace* workspace);
		/*override*/ virtual bool isEnabled() const;				// vtable+0x08
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};

	// VTABLE: PLAYER 0x00b7c6a0
	// VTABLE: STUDIO 0x00bfb05c
	class CameraZoomExtentsCommand : public CameraVerb
	{
	public:
		CameraZoomExtentsCommand(Workspace* workspace);
		/*override*/ virtual bool isEnabled() const;				// vtable+0x08
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};

	// new 0x10 in setupDataModel; ctor PLAYER 0x00856130. isChecked/isSelected are the engine's folded
	// `xor al,al; ret` (0x00927cc0), unlike the camera verbs' inherited shell copies.
	// VTABLE: PLAYER 0x00b7bbcc
	// VTABLE: STUDIO 0x00bfa454
	// SIZE 0x10
	class ToggleViewMode : public Verb
	{
		DataModel* dataModel;	// 0x0c

	public:
		ToggleViewMode(DataModel* dataModel);
		/*override*/ virtual bool isEnabled() const;				// vtable+0x08
		/*override*/ virtual bool isChecked() const;				// vtable+0x0c
		/*override*/ virtual bool isSelected() const;			// vtable+0x10
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};


	// ---- The verbs CommonVerbs holds (CommonVerbs.h). Bases and sizes from RTTI and the 2013 ctors; every
	// member ctor and doIt is a STUB in Commands.cpp at its PLAYER address. Only the overrides a 2013 vftable
	// shows for doIt (+0x18) are declared yet; the other slots (isEnabled, isChecked, getText, ...) are not.
	class RunService;

	// ~RunStateVerb is out of line (PLAYER 0x00857430, STUDIO 0x00781a70); every RunStateVerb-derived
	// member's implicit destructor calls it, so ~CommonVerbs (Game.obj) does too.
	// VTABLE: PLAYER 0x00b7c12c
	// VTABLE: STUDIO 0x00bfa9bc
	// SIZE 0x1c
	class RunStateVerb : public Verb
	{
	protected:
		DataModel* dataModel;					// 0x0c
		ServiceClient<RunService> runService;	// 0x10 (2016 name and type; the ctor stores the DataModel as context)
		RunStateVerb(std::string name, DataModel* dataModel);
	public:
		virtual ~RunStateVerb();	// vtable+0x04
	};

	// ~EditSelectionVerb: PLAYER 0x008574b0, STUDIO 0x00781af0 (releases +0x18, then +0x0c).
	// VTABLE: PLAYER 0x00b7c14c
	// VTABLE: STUDIO 0x00bfa9dc
	// SIZE 0x24
	class EditSelectionVerb : public Verb
	{
	protected:
		boost::shared_ptr<Workspace> workspace;		// 0x0c (2016 name; built by 0x0091d170 from DataModel+0xb88)
		ServiceClient<Selection> selection;		// 0x14
		DataModel* dataModel;					// 0x20
		EditSelectionVerb(std::string name, DataModel* dataModel);
	public:
		virtual ~EditSelectionVerb();	// vtable+0x04
	};

	// SIZE 0x28
	class DeleteBase : public EditSelectionVerb
	{
		bool rewardHopper;	// 0x24 (2016 name)
	protected:
		DeleteBase(DataModel* dataModel, std::string name, bool rewardHopper);
	public:
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};

	// SIZE 0x28
	class BoolPropertyVerb : public EditSelectionVerb
	{
		undefined4 m_unk0x24;	// 0x24
	protected:
		BoolPropertyVerb(const std::string& name, DataModel* dataModel, const char* propertyName);	// the callers destroy the name temporary
	public:
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};

	// The ctor is inline (MoveUpPlateVerb 0x00860e10 copies the name, then calls EditSelectionVerb's).
	// SIZE 0x28
	class MoveUpSelectionVerb : public EditSelectionVerb
	{
		float moveUpHeight;	// 0x24 (2016 name)
	protected:
		MoveUpSelectionVerb(DataModel* dataModel, const std::string& name, float moveUpHeight)
			: EditSelectionVerb(name, dataModel)
			, moveUpHeight(moveUpHeight)
		{
		}
	public:
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};

	// One more virtual than EditSelectionVerb (vtable+0x1c; iOS TiltSelectionVerb::getRotationAxis).
	// SIZE 0x24
	class RotateAxisCommand : public EditSelectionVerb
	{
	protected:
		RotateAxisCommand(std::string name, DataModel* dataModel);
		virtual G3D::Matrix3 getRotationAxis() = 0;	// vtable+0x1c
	public:
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class PlayDeleteSelectionVerb : public DeleteBase
	{
	public:
		PlayDeleteSelectionVerb(DataModel* dataModel) : DeleteBase(dataModel, "PlayDelete", true) {}
	};
	class DeleteSelectionVerb : public DeleteBase
	{
	public:
		DeleteSelectionVerb(DataModel* dataModel) : DeleteBase(dataModel, "Delete", false) {}
	};
	class SelectAllCommand : public RunStateVerb
	{
	public:
		SelectAllCommand(DataModel* dataModel) : RunStateVerb("SelectAll", dataModel) {}
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class GroupSelectionVerb : public EditSelectionVerb
	{
	public:
		GroupSelectionVerb(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class UngroupSelectionVerb : public EditSelectionVerb
	{
	public:
		UngroupSelectionVerb(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class SelectChildrenVerb : public EditSelectionVerb
	{
	public:
		SelectChildrenVerb(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class SnapSelectionVerb : public EditSelectionVerb
	{
	public:
		SnapSelectionVerb(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class UnlockAllVerb : public RunStateVerb
	{
	public:
		UnlockAllVerb(DataModel* dataModel) : RunStateVerb("UnlockAll", dataModel) {}
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class AnchorVerb : public BoolPropertyVerb
	{
	public:
		AnchorVerb(DataModel* dataModel);
	};
	class TranslucentVerb : public BoolPropertyVerb
	{
	public:
		TranslucentVerb(DataModel* dataModel);
	};
	class CanCollideVerb : public BoolPropertyVerb
	{
	public:
		CanCollideVerb(DataModel* dataModel);
	};
	class CanNotSelectCommand : public EditSelectionVerb
	{
	public:
		CanNotSelectCommand(DataModel* dataModel) : EditSelectionVerb("CanNotSelect", dataModel) {}
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class AllCanSelectCommand : public RunStateVerb
	{
	public:
		AllCanSelectCommand(DataModel* dataModel) : RunStateVerb("AllCanSelect", dataModel) {}
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class MoveUpPlateVerb : public MoveUpSelectionVerb
	{
	public:
		MoveUpPlateVerb(DataModel* dataModel);
	};
	class MoveUpBrickVerb : public MoveUpSelectionVerb
	{
	public:
		MoveUpBrickVerb(DataModel* dataModel);
	};
	class MoveDownSelectionVerb : public EditSelectionVerb
	{
	public:
		MoveDownSelectionVerb(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class RotateSelectionVerb : public RotateAxisCommand
	{
	public:
		RotateSelectionVerb(DataModel* dataModel);
		/*override*/ virtual G3D::Matrix3 getRotationAxis();	// vtable+0x1c
	};
	class TiltSelectionVerb : public RotateAxisCommand
	{
	public:
		TiltSelectionVerb(DataModel* dataModel);
		/*override*/ virtual G3D::Matrix3 getRotationAxis();	// vtable+0x1c
	};
	class RunCommand : public RunStateVerb
	{
	public:
		RunCommand(DataModel* dataModel) : RunStateVerb("Run", dataModel) {}
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class StopCommand : public RunStateVerb
	{
	public:
		StopCommand(DataModel* dataModel) : RunStateVerb("Stop", dataModel) {}
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class ResetCommand : public RunStateVerb
	{
	public:
		ResetCommand(DataModel* dataModel) : RunStateVerb("Reset", dataModel) {}
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class FirstPersonCommand : public Verb
	{
		DataModel* dataModel;	// 0x0c
	public:
		FirstPersonCommand(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class StatsCommand : public Verb
	{
		DataModel* dataModel;	// 0x0c
	public:
		StatsCommand(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class RenderStatsCommand : public Verb
	{
		DataModel* dataModel;	// 0x0c
	public:
		RenderStatsCommand(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class EngineStatsCommand : public Verb
	{
		DataModel* dataModel;	// 0x0c
	public:
		EngineStatsCommand(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class NetworkStatsCommand : public Verb
	{
		DataModel* dataModel;	// 0x0c
		bool m_unk0x10;			// 0x10
		bool m_unk0x11;			// 0x11
	public:
		NetworkStatsCommand(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class PhysicsStatsCommand : public Verb
	{
		DataModel* dataModel;	// 0x0c
	public:
		PhysicsStatsCommand(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class SummaryStatsCommand : public Verb
	{
		DataModel* dataModel;	// 0x0c
	public:
		SummaryStatsCommand(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class CustomStatsCommand : public Verb
	{
		DataModel* dataModel;	// 0x0c
	public:
		CustomStatsCommand(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class JoinCommand : public Verb
	{
		DataModel* dataModel;	// 0x0c
	public:
		JoinCommand(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class TurnOnManualJointCreation : public Verb
	{
		DataModel* dataModel;	// 0x0c
	public:
		TurnOnManualJointCreation(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class SetGridToOne : public Verb
	{
		DataModel* dataModel;	// 0x0c
	public:
		SetGridToOne(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class SetGridToOneFifth : public Verb
	{
		DataModel* dataModel;	// 0x0c
	public:
		SetGridToOneFifth(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class SetGridToOff : public Verb
	{
		DataModel* dataModel;	// 0x0c
	public:
		SetGridToOff(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class SetManualJointToWeak : public Verb
	{
		DataModel* dataModel;	// 0x0c
	public:
		SetManualJointToWeak(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class SetManualJointToStrong : public Verb
	{
		DataModel* dataModel;	// 0x0c
	public:
		SetManualJointToStrong(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	class SetManualJointToInfinite : public Verb
	{
		DataModel* dataModel;	// 0x0c
	public:
		SetManualJointToInfinite(DataModel* dataModel);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};
	// The CommonVerbs tool verbs: RTTI TToolVerb<Tool, RunStateVerb>. 2013 ctors are out-of-line COMDATs in
	// CommonVerbs.obj (PLAYER 0x00861c70..0x00864320); here they are STUB stand-ins without markers.
	class AxisRotateTool;
	class AdvMoveTool;
	class AdvRotateTool;
	class AdvArrowTool;
	class MoveResizeJoinTool;
	class FlatTool;
	class GlueTool;
	class WeldTool;
	class StudsTool;
	class InletTool;
	class UniversalTool;
	class HingeTool;
	class RightMotorTool;
	class LeftMotorTool;
	class OscillateMotorTool;
	class SmoothNoOutlinesTool;
	class AnchorTool;
	class LockTool;
	class FillTool;
	class MaterialTool;
	class DropperTool;
	class NullTool;
	class GameTool;
	class GrabTool;
	class CloneTool;
	class HammerTool;

	// SIZE 0x20
	template<class Tool, class Base = RunStateVerb>
	class TToolVerb : public Base
	{
		bool toggle;	// 0x1c (2016 name)
	public:
		TToolVerb(DataModel* dataModel, bool toggle = true);
		/*override*/ virtual void doIt(IDataState* dataState);	// vtable+0x18
	};

	// __declspec(noinline) is a stand-in until the real (larger) bodies are written.
	template<class Tool, class Base>
	__declspec(noinline) TToolVerb<Tool, Base>::TToolVerb(DataModel* dataModel, bool toggle)
		: Base("Tool", dataModel)
		, toggle(toggle)
	{
		static int unfold;	// stand-in: keeps the STUB instantiations apart under /OPT:ICF
		g_foldingDummyVariable = (int)&unfold;
	}

	template<class Tool, class Base>
	void TToolVerb<Tool, Base>::doIt(IDataState* dataState)
	{
		static int unfold;
		g_foldingDummyVariable = (int)&unfold;
	}

} // namespace RBX
