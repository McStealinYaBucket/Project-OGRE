/* Util/Sound.h — October 2013 (Soundscape::Sound).
   Layout from SoundService::loadSound 0x244C78 + Sound::get 0x244C78-area:
     +0  FMOD::Sound* cachedSound  (created by get(); returned by get())
     +4  FMOD::System* system      (from SoundService::system)
     +8  int useCounter            (bumped by playSound, decremented by releaseChannel)
     +12 SoundId soundId           (ContentId-shaped; string+Name* halves)
     +20 bool is3D
   get() creates the FMOD sound via System::createSound and caches it at +0. */
#pragma once

#include "fmod/fmod.hpp"
#include "Util/SoundChannel.h"  // SoundId
#include <string>

namespace RBX
{
	class Instance;

	namespace Soundscape
	{
		class Sound
		{
		public:
			FMOD::Sound* get(const Instance* instance);
			void release();

			FMOD::Sound* cachedSound;
			FMOD::System* system;
			int useCounter;
			SoundId soundId;
			bool is3D;
		};

	} // namespace Soundscape
} // namespace RBX