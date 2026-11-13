/* include/gui/GuiEvent.h -- October 2013 (rewritten 2026-10-18 from the
   2016-carry: the binary GuiResponse is exactly 16 bytes -- the
   processShown_OutOfTitle copy 0x5CE98A is 2 qword moves, the family reads
   response@+0 / finished@+4 directly; the 4th field = a raw Instance* at
   +12, NOT boost::weak_ptr.  Default-ctor inlines at the family call sites
   as a 4-dword zeroize; SUNK==1 verified (LODWORD==1 tests, batch781/783).
   wasSunkAndFinished 0x5CF2AE asserts at include/Gui/GuiEvent.h:66 --
   RBXASSERT placed at line 66 below.
   Fidelity ~99%; DELTAS: GuiEvent field names ours (12-byte shape fixed,
   names unobservable); the static factory set = 2015-carry (inlined).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/include/Gui/GuiEvent.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	// No ctor symbol anywhere (8GuiEventC = 0 hits) and no dtor; only
	// const& receivers in the family.  Value shape (12 bytes):
	//   +0  int   eventType (consumer semantics in the family: 3=release?,
	//       4=press, 5=move; 7 short-circuits a zero response in
	//       GuiItem::process 0x5CD47E)
	//   +4  short mouseX, mouseY (mouse pointer; the rect tests read the
	//       low/high halves of the +4 dword)
	//   +8  short canvasX, canvasY (builds Canvas {x,y} for getMyRect)
	// DELTA: field names are ours (not observable in the binary).
	class GuiEvent
	{
	public:
		int eventType;
		short mouseX;
		short mouseY;
		short canvasX;
		short canvasY;
	};

	class GuiResponse
	{
	private:
		enum ResponseType {NOT_SUNK, SUNK};
		enum FinishedType {NOT_FINISHED, FINISHED};
		enum MouseOverType {NOT_MOUSE_OVER, MOUSE_OVER};

		ResponseType	response;		// +0 (1 = SUNK, binary-verified)
		FinishedType	finished;		// +4 (1 = FINISHED)
		MouseOverType	mouseWasOver;	// +8 (1 = MOUSE_OVER)
		Instance*		target;			// +12 (raw; 16-byte object total)

		GuiResponse(ResponseType response, FinishedType finished, MouseOverType mouseWasOver, Instance* newTarget)
			: response(response), finished(finished), mouseWasOver(mouseWasOver), target(newTarget) {}

		GuiResponse(ResponseType response)
			: response(response), finished(NOT_FINISHED), mouseWasOver(NOT_MOUSE_OVER), target(NULL) {}

	public:
		GuiResponse()
			: response(NOT_SUNK), finished(NOT_FINISHED), mouseWasOver(NOT_MOUSE_OVER), target(NULL) {}

		bool getMouseWasOver() const	{return mouseWasOver == MOUSE_OVER;}
		void setMouseWasOver()			{mouseWasOver = MOUSE_OVER;}
		bool wasSunk()		{return (response == SUNK);}
		// FUNCTION: PLAYER 0x00768100
		// FUNCTION: STUDIO 0x007b3ba0
		bool wasSunkAndFinished() {
			RBXASSERT(!((finished == FINISHED) && !wasSunk()));		// binary: include/Gui/GuiEvent.h:66
			return (finished == FINISHED);
		}

		static GuiResponse notSunk()							{return GuiResponse(NOT_SUNK);}
		static GuiResponse notSunkMouseWasOver()				{return GuiResponse(NOT_SUNK, NOT_FINISHED, MOUSE_OVER, NULL);}
		static GuiResponse sunk()								{return GuiResponse(SUNK);}
		static GuiResponse sunkAndFinished()					{return GuiResponse(SUNK, FINISHED, NOT_MOUSE_OVER, NULL);}
		static GuiResponse sunkWithTarget(Instance* newTarget)	{return GuiResponse(SUNK, NOT_FINISHED, NOT_MOUSE_OVER, newTarget);}
	};

	// 2013 secondary base of GuiItem (+0x60) and GuiBase (+0x60): RTTI .?AVGuiTarget@RBX@@, own vftable
	// PLAYER 0x00b47d5c (stored first by the GuiItem ctor 0x0076a4f0 and the GuiBase ctor 0x007bc4b0).
	// Its one slot is process; the default body (PLAYER 0x007bc510, folded) is in Gui/GUI.cpp.
	// The Mac vtable lists GuiItem::process in the primary table only because Itanium repeats
	// secondary-base overriders.
	class GuiTarget
	{
	public:
		virtual GuiResponse process(const GuiEvent& event);	// vtable+0x00
	};
}	// namespace

// GuiTarget lifetime log (GuiLayerCollector, BasePlayerGui, Workspace, GuiItem ctors and dtors). Its
// LOGVARIABLE belongs in FastLogSettings.cpp (2016), at PLAYER 0x00de3f47.
LOGGROUP(GuiTargetLifetime)
