/* V8DataModel/ClickDetector.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<ClickDetector, Instance>;
     sClickDetector = "ClickDetector" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ClickDetector.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sClickDetector;

	class ClickDetector
		: public DescribedCreatable<ClickDetector, Instance, sClickDetector>
	{
	public:
		ClickDetector() {}
	};

} // namespace RBX
