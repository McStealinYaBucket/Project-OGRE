/* V8DataModel/FormFactorPart.h — October 2013 (form-factor intermediate shell).
   Evidence: NonFactoryProduct<PartInstance, sFormFactorPart> (0x2817D4 C2
     + descriptor EAs); FactoryProduct<BasicPartInstance, FormFactorPart,
     sBasicPart> derives through it; BasicPartInstance setters
     (setLegacyPartTypeUi/Xml 0x27FBFC/0x27FB4E) live below it.
     sFormFactorPart has no metadata entry (C++-only intermediate).
   TODO: full decompile (formFactor storage + legacy-type mapping).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/FormFactorPart.h */

#pragma once

#include "V8DataModel/PartInstance.h"

namespace RBX {

	extern const char* const sFormFactorPart;

	class FormFactorPart
		: public DescribedNonCreatable<FormFactorPart, PartInstance, sFormFactorPart, Reflection::ClassDescriptor::PERSISTENT>
	{
	public:
		FormFactorPart() {}
	};

} // namespace RBX
