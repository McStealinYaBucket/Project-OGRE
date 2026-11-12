/* V8DataModel/PlayerGui.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<PlayerGui, BasePlayerGui> 0x463A00;
     sPlayerGui = "PlayerGui".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/PlayerGui.h */

#pragma once

#include "V8DataModel/BasePlayerGui.h"

namespace RBX {

	extern const char* const sPlayerGui;

	class PlayerGui
		: public DescribedCreatable<PlayerGui, BasePlayerGui, sPlayerGui>
	{
	public:
		PlayerGui() {}
	};

} // namespace RBX
