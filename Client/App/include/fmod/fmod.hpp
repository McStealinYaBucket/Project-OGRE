/* fmod/fmod.hpp — October 2013 FMOD Ex shim (C++ wrapper layer).
   The 2013 binary calls NON-VIRTUAL wrapper methods
   (__ZN4FMOD7Channel9setPausedEb, __ZN4FMOD6System9playSoundEi... etc.),
   so the wrappers here are plain classes, not virtual tables. Only the
   calls the decompiled Sound.cpp bodies make are declared; util/FmodShim.cpp
   supplies their bodies (stubbed to FMOD_OK).

   DELTA: reconstruction artifact for a closed library; the System/Sound
   handle pointers are carried but never dereferenced by the shim. */

#ifndef _FMOD_HPP
#define _FMOD_HPP

#include "fmod.h"

namespace FMOD
{

class Channel
{
public:
	explicit Channel(FMOD_CHANNEL* handle)							: m_handle(handle) {}

	FMOD_RESULT setPaused(bool paused);
	FMOD_RESULT getPaused(bool* paused);
	FMOD_RESULT setVolume(float volume);
	FMOD_RESULT setFrequency(float frequency);
	FMOD_RESULT setUserData(void* userdata);
	FMOD_RESULT setCallback(FMOD_CHANNEL_CALLBACK callback);
	FMOD_RESULT getMode(FMOD_MODE* mode);
	FMOD_RESULT setMode(FMOD_MODE mode);
	FMOD_RESULT setLoopCount(int loopcount);
	FMOD_RESULT set3DAttributes(const FMOD_VECTOR* position, const FMOD_VECTOR* velocity);
	FMOD_RESULT setMute(bool mute);
	FMOD_RESULT stop();
	FMOD_RESULT isPlaying(bool* isplaying);

private:
	FMOD_CHANNEL* m_handle;
};

class Sound
{
public:
	explicit Sound(FMOD_SOUND* handle)								: m_handle(handle) {}

	/* playSound passes (&defaultFrequency, 0, 0, 0): the leading float* is
	   the sound's default frequency; the remaining pointer args are passed
	   null at both landed call sites. The vendor type names for them are
	   not recoverable from the binary -- DELTA. */
	FMOD_RESULT getDefaults(float* frequency, float* reserved1, float* reserved2, int* reserved3);
	FMOD_RESULT release();

private:
	FMOD_SOUND* m_handle;
};

class System
{
public:
	explicit System(FMOD_SYSTEM* handle)							: m_handle(handle) {}

	FMOD_RESULT playSound(FMOD_CHANNEL_INDEX channelid, Sound* sound, bool paused, Channel** channel);
	FMOD_RESULT createSound(const char* name_or_data, FMOD_MODE mode, void* exinfo, Sound** sound);
	FMOD_RESULT set3DSettings(float dopplerscale, float distancefactor, float rolloffscale);
	FMOD_RESULT release();

	/* openFmod 0x2410F0 call set. */
	FMOD_RESULT getVersion(unsigned int* version);
	FMOD_RESULT getNumDrivers(int* numdrivers);
	FMOD_RESULT setDriver(int driver);
	FMOD_RESULT getDriver(int* driver);
	FMOD_RESULT getDriverInfo(int driver, char* name, int namelen, FMOD_GUID* guid);
	FMOD_RESULT getDriverCaps(int driver, FMOD_CAPS* caps, int* controlpaneloutputrate, int* controlpanelspeakermode, FMOD_SPEAKERMODE* speakermode);
	FMOD_RESULT setSpeakerMode(FMOD_SPEAKERMODE speakermode);
	FMOD_RESULT setDSPBufferSize(unsigned int bufferlength, int numbuffers);
	FMOD_RESULT init(int maxchannels, FMOD_INITFLAGS flags, void* extradriverdata);

private:
	FMOD_SYSTEM* m_handle;
};

}
#endif /* _FMOD_HPP */
