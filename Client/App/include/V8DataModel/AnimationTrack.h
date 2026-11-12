/* V8DataModel/AnimationTrack.h -- October 2013 STUB (minimal shell).
   Evidence: NonFactoryProduct<AnimationTrack, Instance> 0x264EE8;
     sAnimationTrack = "AnimationTrack".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/AnimationTrack.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sAnimationTrack;

	class AnimationTrack
		: public DescribedNonCreatable<AnimationTrack, Instance, sAnimationTrack>
	{
	public:
		AnimationTrack() {}
	};

} // namespace RBX
