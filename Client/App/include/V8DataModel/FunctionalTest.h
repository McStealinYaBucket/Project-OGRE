/* V8DataModel/FunctionalTest.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<FunctionalTest, Instance> 0x369D52 (Result enum TODO);
     sFunctionalTest = "FunctionalTest".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/FunctionalTest.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sFunctionalTest;

	class FunctionalTest
		: public DescribedCreatable<FunctionalTest, Instance, sFunctionalTest>
	{
	public:
		FunctionalTest() {}
	};

} // namespace RBX
