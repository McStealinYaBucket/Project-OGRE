/* V8DataModel/Animation.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Animation, Instance> 0x262F3E;
     sAnimation = "Animation".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Animation.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sAnimation;

	class Animation
		: public DescribedCreatable<Animation, Instance, sAnimation>
	{
	public:
		Animation() {}
	};

} // namespace RBX
