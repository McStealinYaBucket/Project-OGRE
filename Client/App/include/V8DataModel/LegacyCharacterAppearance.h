/* V8DataModel/LegacyCharacterAppearance.h -- October 2013 STUB (minimal shell).
   Evidence: INFERRED parent (no own descriptor; parent of BodyColors); C++-only TODO-verify;
     sLegacyCharacterAppearance = "LegacyCharacterAppearance".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/LegacyCharacterAppearance.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sLegacyCharacterAppearance;

	class LegacyCharacterAppearance
		: public DescribedNonCreatable<LegacyCharacterAppearance, Instance, sLegacyCharacterAppearance>
	{
	public:
		LegacyCharacterAppearance() {}
	};

} // namespace RBX
