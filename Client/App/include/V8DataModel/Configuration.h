/* V8DataModel/Configuration.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Configuration, Instance>;
     sConfiguration = "Configuration" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Configuration.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sConfiguration;

	class Configuration
		: public DescribedCreatable<Configuration, Instance, sConfiguration>
	{
	public:
		Configuration() {}
	};

} // namespace RBX
