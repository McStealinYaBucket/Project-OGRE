/* V8DataModel/Commands.h -- October 2013, partial: the camera verbs Game::setupDataModel builds
   (PLAYER 0x00655630) and ToggleViewMode. 2013 __FILE__: Client\App\V8DataModel\Commands.cpp
   (asserts at lines 116..1135); no string names the header, its path follows the .cpp.

   RTTI: each Camera*Command derives CameraVerb : Verb : Debugable. Layout from the CameraVerb ctor
   (PLAYER 0x00857810, ret 0x20: std::string by value, then Workspace*): Verb (0xc), workspace at
   0xc, ServiceClient<Selection> at 0x10 (context, shared_ptr), size 0x1c (setupDataModel news 0x1c).
   The six Camera*Command ctors are inline (setupDataModel calls CameraVerb's ctor, then stores the
   derived vptr), so Game.obj emits their vftables; their deleting destructors fold with CameraVerb's
   (PLAYER 0x00653a40, STUDIO 0x007cc4e0, also Game.obj). CameraVerb's isEnabled is inline
   `return true` (vftable slot 2 = the folded `mov al,1; ret`); PanLeft/PanRight do not override it. */

#pragma once

#include "V8Tree/Verb.h"
#include "V8Tree/Service.h"

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

} // namespace RBX
