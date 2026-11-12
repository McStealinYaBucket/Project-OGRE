/* V8DataModel/ShirtGraphic.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<ShirtGraphic, LegacyCharacterAppearance> 0x2A93B8;
     sShirtGraphic = "ShirtGraphic".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ShirtGraphic.h */

#pragma once

#include "V8DataModel/LegacyCharacterAppearance.h"

namespace RBX {

	extern const char* const sShirtGraphic;

	class ShirtGraphic
		: public DescribedCreatable<ShirtGraphic, LegacyCharacterAppearance, sShirtGraphic>
	{
	public:
		ShirtGraphic() {}
	};

} // namespace RBX
