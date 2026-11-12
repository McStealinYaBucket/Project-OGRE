/* V8DataModel/GuiMain.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<GuiMain, ScreenGui> 0x473460;
     sGuiMain = "GuiMain".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiMain.h */

#pragma once

#include "V8DataModel/ScreenGui.h"

namespace RBX {

	extern const char* const sGuiMain;

	class GuiMain
		: public DescribedCreatable<GuiMain, ScreenGui, sGuiMain>
	{
	public:
		GuiMain() {}
	};

} // namespace RBX
