/* V8DataModel/BasePlayerGui.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<Instance, sBasePlayerGui> 0x463BB6; parent of PlayerGui (Factory 0x463A00) and CoreGuiService (NonFactory); C++-only (no metadata);
     sBasePlayerGui = "BasePlayerGui".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BasePlayerGui.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sBasePlayerGui;

	class BasePlayerGui
		: public DescribedNonCreatable<BasePlayerGui, Instance, sBasePlayerGui>
	{
	public:
		BasePlayerGui() {}
	};

} // namespace RBX
