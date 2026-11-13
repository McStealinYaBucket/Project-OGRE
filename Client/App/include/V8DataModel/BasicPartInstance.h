/* V8DataModel/BasicPartInstance.h — October 2013 (the concrete "Part").
   Evidence: FactoryProduct<BasicPartInstance, FormFactorPart, sBasicPart>
     (Creator EAs 0x2806F0+/0x487920+); 0.125 ReflectionMetadata.xml lists
     "Part" ("A plastic building block", browsable) alongside "BasePart"
     ("A structural class, not creatable" — sPart's class). sBasicPart =
     "Part": the factory creator registered under it is what Instance.new(
     "Part") and the file loaders resolve. "BasicPart"/"FormFactorPart" have
     no metadata entries (C++-only names; PE hits are RTTI/symbol noise).
     Legacy type setters: setLegacyPartTypeUi/Xml 0x27FBFC/0x27FB4E.
   TODO: full decompile (legacy part-type surface).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/BasicPartInstance.h */

#pragma once

#include "V8DataModel/FormFactorPart.h"

namespace RBX {

	extern const char* const sBasicPart;

	class BasicPartInstance
		: public DescribedCreatable<BasicPartInstance, FormFactorPart, sBasicPart>
	{
	public:
		enum LegacyPartType
		{
			BALL_LEGACY_PART = 0,
			BLOCK_LEGACY_PART = 1,
			CYLINDER_LEGACY_PART = 2,
		};

		BasicPartInstance() {}
	};

} // namespace RBX
