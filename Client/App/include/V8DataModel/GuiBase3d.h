/* V8DataModel/GuiBase3d.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<GuiBase3d, GuiBase> 0x395566; C++-only (no metadata);
     sGuiBase3d = "GuiBase3d".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiBase3d.h */

#pragma once

#include "V8DataModel/GuiBase.h"

namespace RBX {

	extern const char* const sGuiBase3d;

	class GuiBase3d
		: public DescribedNonCreatable<GuiBase3d, GuiBase, sGuiBase3d>
	{
	public:
		GuiBase3d() {}
	};

} // namespace RBX
