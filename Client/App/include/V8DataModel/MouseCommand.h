/* V8DataModel/MouseCommand.h — October 2013 STUB (mouse-command shell).
   Evidence: Workspace::start 0x515636 / stop 0x515896 assert
     "!getCurrentMouseCommand()->captured()" (Workspace.cpp:792/:810);
     Workspace::setDefaultMouseCommand 0x516402 resets the slot.
     2016 shape carried: captured flag ("captured()" exclusive-handling state),
     Workspace friendship, non-Instance class (INamed/IAdornable/enable_shared
     in 2016 — absent here, TODO).
   TODO: full decompile (mouse event routing, hover/selection machinery).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/MouseCommand.h */

#pragma once

namespace RBX {

	class Workspace;

	class MouseCommand
	{
		friend class Workspace;
		Workspace* workspace;
		bool capturedMouse;
	public:
		MouseCommand()
			: workspace(NULL)
			, capturedMouse(false)
		{}
		bool captured() const { return capturedMouse; }
	};

} // namespace RBX
