/* V8DataModel/Sound.h -- October 2013 REDIRECT.
   The 2013 binary has no separate RBX::Sound (v8datamodel) class: the only
   "Sound" class is RBX::Soundscape::SoundChannel (registered under
   sSoundChannel = "Sound", landed in App/util/Sound.cpp).  RBX::Sound is kept
   as a typedef alias so legacy/test references (parsetest) resolve to the
   same type. */
#pragma once

#include "Util/SoundChannel.h"

namespace RBX {
	typedef Soundscape::SoundChannel Sound;
}