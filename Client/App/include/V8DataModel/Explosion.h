/* V8DataModel/Explosion.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Explosion, Instance>;
     sExplosion = "Explosion" (0.125 metadata class list).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Explosion.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sExplosion;

	class Explosion
		: public DescribedCreatable<Explosion, Instance, sExplosion>
	{
	public:
		Explosion() {}
	};

} // namespace RBX
