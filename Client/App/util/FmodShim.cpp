/* App/util/FmodShim.cpp — October 2013 FMOD stub layer.
   The 2013 client links the closed FMOD Ex static library; its bodies are
   not in the decompiled PE set and the vendor library itself is not
   reproducible from the executable. util/Sound.cpp (the landed sound-era
   TU) calls the exact binary call set declared in include/fmod/fmod.hpp;
   this file implements those symbols as FMOD_OK stubs so the parsetest
   link proves every sound-era call resolved.

   Also carried here (global namespace, per the binary's mangles):
     checkResult(FMOD_RESULT)                 __Z11checkResult11FMOD_RESULT
     checkResultNoThrow(FMOD_RESULT)          __Z18checkResultNoThrow11FMOD_RESULT
     convert(G3D::Vector3 const&, FMOD_VECTOR&)   __Z7convertRKN3G3D7Vector3ER11FMOD_VECTOR
   Their bodies are undecoded -> stubs. DELTA: every body in this file is a
   stand-in, not a decompile; the vector convert stub reproduces the
   always-nonzero branch the callers see (update3D 0x244A85/0x244AB6 test
   al).  Channel/System wrappers allocate their wrapper handles (playSound
   stores a fresh Channel so a landed playing path never dereferences NULL
   through the shim). */

#include "stdafx.h"

#include "fmod/fmod.hpp"
#include "fmod/fmod.h"

#include "G3D/Vector3.h"

#include <string.h>

/* C-layer factory from fmod.h (extern "C"): the binary calls this once and
   punns the FMOD_SYSTEM* as FMOD::System.  Shim shape: allocate a real
   wrapper and hand back the punned pointer. */
FMOD_RESULT FMOD_System_Create(FMOD_SYSTEM** system)
{
	if (system)
		*system = reinterpret_cast<FMOD_SYSTEM*>(new FMOD::System((FMOD_SYSTEM*)0));
	return FMOD_OK;
}

namespace FMOD
{

FMOD_RESULT Channel::setPaused(bool /*paused*/)
{
	return FMOD_OK;
}

FMOD_RESULT Channel::getPaused(bool* paused)
{
	if (paused)
		*paused = false;
	return FMOD_OK;
}

FMOD_RESULT Channel::setVolume(float /*volume*/)
{
	return FMOD_OK;
}

FMOD_RESULT Channel::setFrequency(float /*frequency*/)
{
	return FMOD_OK;
}

FMOD_RESULT Channel::setUserData(void* /*userdata*/)
{
	return FMOD_OK;
}

FMOD_RESULT Channel::setCallback(FMOD_CHANNEL_CALLBACK /*callback*/)
{
	return FMOD_OK;
}

FMOD_RESULT Channel::getMode(FMOD_MODE* mode)
{
	if (mode)
		*mode = (FMOD_MODE)0;
	return FMOD_OK;
}

FMOD_RESULT Channel::setMode(FMOD_MODE /*mode*/)
{
	return FMOD_OK;
}

FMOD_RESULT Channel::setLoopCount(int /*loopcount*/)
{
	return FMOD_OK;
}

FMOD_RESULT Channel::set3DAttributes(const FMOD_VECTOR* /*position*/, const FMOD_VECTOR* /*velocity*/)
{
	return FMOD_OK;
}

FMOD_RESULT Channel::setMute(bool /*mute*/)
{
	return FMOD_OK;
}

FMOD_RESULT Channel::stop()
{
	return FMOD_OK;
}

FMOD_RESULT Channel::isPlaying(bool* isplaying)
{
	if (isplaying)
		*isplaying = false;
	return FMOD_OK;
}

FMOD_RESULT Sound::getDefaults(float* frequency, float* /*reserved1*/, float* /*reserved2*/, int* /*reserved3*/)
{
	if (frequency)
		*frequency = 0.0f;
	return FMOD_OK;
}

FMOD_RESULT Sound::release()
{
	return FMOD_OK;
}

FMOD_RESULT System::createSound(const char* /*name_or_data*/, FMOD_MODE /*mode*/, void* /*exinfo*/, Sound** sound)
{
	if (sound)
		*sound = new Sound(NULL);
	return FMOD_OK;
}

FMOD_RESULT System::playSound(FMOD_CHANNEL_INDEX /*channelid*/, Sound* /*sound*/, bool /*paused*/, Channel** channel)
{
	if (channel)
		*channel = new Channel((FMOD_CHANNEL*)0);
	return FMOD_OK;
}

FMOD_RESULT System::set3DSettings(float /*dopplerscale*/, float /*distancefactor*/, float /*rolloffscale*/)
{
	return FMOD_OK;
}

FMOD_RESULT System::release()
{
	return FMOD_OK;
}

/* openFmod 0x2410F0 call set (all FMOD_OK stubs; canned out-params noted
   per method where the binary's own library would answer). */

FMOD_RESULT System::getVersion(unsigned int* version)
{
	if (version)
		*version = FMOD_VERSION;	/* must clear the >0x43204 gate */
	return FMOD_OK;
}

FMOD_RESULT System::getNumDrivers(int* numdrivers)
{
	if (numdrivers)
		*numdrivers = 1;		/* 0 would hit the no-driver throw arm */
	return FMOD_OK;
}

FMOD_RESULT System::setDriver(int /*driver*/)
{
	return FMOD_OK;
}

FMOD_RESULT System::getDriver(int* driver)
{
	if (driver)
		*driver = 0;
	return FMOD_OK;
}

FMOD_RESULT System::getDriverInfo(int /*driver*/, char* name, int namelen, FMOD_GUID* guid)
{
	if (name && namelen > 0)
		memset(name, 0, (size_t)namelen);
	if (guid)
		memset(guid, 0, sizeof(*guid));
	return FMOD_OK;
}

FMOD_RESULT System::getDriverCaps(int /*driver*/, FMOD_CAPS* caps, int* controlpaneloutputrate, int* controlpanelspeakermode, FMOD_SPEAKERMODE* speakermode)
{
	if (caps)
		*caps = (FMOD_CAPS)0;	/* accelerated hardware answer; the
					   (caps & 2) acceleration-off arm stays
					   not-taken -- DELTA: canned stub answer */
	if (controlpaneloutputrate)
		*controlpaneloutputrate = 0;
	if (controlpanelspeakermode)
		*controlpanelspeakermode = 0;
	if (speakermode)
		*speakermode = FMOD_SPEAKERMODE_STEREO;
	return FMOD_OK;
}

FMOD_RESULT System::setSpeakerMode(FMOD_SPEAKERMODE /*speakermode*/)
{
	return FMOD_OK;
}

FMOD_RESULT System::setDSPBufferSize(unsigned int /*bufferlength*/, int /*numbuffers*/)
{
	return FMOD_OK;
}

FMOD_RESULT System::init(int /*maxchannels*/, FMOD_INITFLAGS /*flags*/, void* /*extradriverdata*/)
{
	return FMOD_OK;
}

} // namespace FMOD

/* Global-namespace helpers (mangle evidence in the PE import/strings;
   bodies undecoded, shim stubs -- DELTA). */

void checkResult(FMOD_RESULT)
{
}

void checkResultNoThrow(FMOD_RESULT)
{
}

bool convert(const G3D::Vector3& vector, FMOD_VECTOR& result)
{
	result.x = vector.x;
	result.y = vector.y;
	result.z = vector.z;
	return true;
}
