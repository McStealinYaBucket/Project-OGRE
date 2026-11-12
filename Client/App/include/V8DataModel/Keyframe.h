/* V8DataModel/Keyframe.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<Keyframe, Instance> 0x41C58C;
     sKeyframe = "Keyframe".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/Keyframe.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sKeyframe;

	class Keyframe
		: public DescribedCreatable<Keyframe, Instance, sKeyframe>
	{
	public:
		Keyframe() {}
	};

} // namespace RBX
