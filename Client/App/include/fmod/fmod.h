/* fmod/fmod.h — October 2013 FMOD Ex shim (vendor C layer).
   The 2013 client ships FMOD Ex 1.07.02 as a closed static library; no
   headers survive with the source tree. This file (plus fmod.hpp and
   util/FmodShim.cpp) stands in for it so the landed sound-era bodies
   compile and link against the exact call set the binary makes.

   Fidelity ~99%: the constants and prototypes below are the subset the
   decompiled Sound.cpp bodies actually reference -- (fmod.h constants
   read directly by the landed code: FMOD_CHANNEL_FREE = -1 in
   System::playSound (0x2445AC family); FMOD_ERR_INVALID_HANDLE = 0x24
   required by SoundChannel::isPlaying 0x243AFA / isPlaying 0x243AB8
   (`cmp eax, 24h`); FMOD_LOOP_OFF/NORMAL in updateLooped 0x244ADE bit
   math; the channel-callback type for setCallback). FMOD_VECTOR is the
   struct copied by convert(G3D::Vector3 const&, FMOD_VECTOR&). Anything
   the decompiles did not touch is deliberately absent so every shim
   symbol seen by the link is one the binary would also have.

   DELTA: this is a reconstruction artifact, not a decompute -- it carries
   declared-but-stubbed prototypes for a closed third-party library. */

#ifndef _FMOD_H
#define _FMOD_H

#ifdef __cplusplus
extern "C" {
#endif

/* October 2013 client pins its FMOD Ex at version > 0x43204 (openFmod
   0x2410F0 throws "Incorrect FMODEX DLL version" below that; the 2013
   FMODEX build is 4.4x -- DELTA: the exact library version constant is
   inside the closed lib, 0x00044204 chosen to pass the same gate). */
#define FMOD_VERSION 0x00044204

typedef enum FMOD_RESULT
{
	FMOD_OK = 0,
	FMOD_ERR_INVALID_HANDLE = 36		/* isPaused/isPlaying compare against 0x24 */
} FMOD_RESULT;

typedef enum FMOD_CHANNEL_INDEX
{
	FMOD_CHANNEL_FREE = -1,
	FMOD_CHANNEL_REUSE = -2
} FMOD_CHANNEL_INDEX;

typedef enum FMOD_MODE
{
	FMOD_LOOP_OFF = 1,
	FMOD_LOOP_NORMAL = 2,
	FMOD_LOOP_BIDI = 4,
	FMOD_LOOP_BITMASK = 7
} FMOD_MODE;

typedef enum FMOD_CHANNEL_CALLBACKTYPE
{
	FMOD_CHANNEL_CALLBACKTYPE_END		= 0,
	FMOD_CHANNEL_CALLBACKTYPE_VIRTUALVOICE	= 1,
	FMOD_CHANNEL_CALLBACKTYPE_SYNCPOINT	= 2
} FMOD_CHANNEL_CALLBACKTYPE;

typedef struct FMOD_VECTOR
{
	float x;
	float y;
	float z;
} FMOD_VECTOR;

typedef enum FMOD_CAPS
{
	FMOD_CAPS_HARDWARE = 1,
	FMOD_CAPS_SOFTWARE = 2		/* openFmod 0x24126E tests (caps & 2) */
} FMOD_CAPS;

typedef enum FMOD_SPEAKERMODE
{
	FMOD_SPEAKERMODE_RAW = 0,
	FMOD_SPEAKERMODE_MONO = 1,
	FMOD_SPEAKERMODE_STEREO = 2	/* openFmod 0x24130E setSpeakerMode arg */
} FMOD_SPEAKERMODE;

typedef enum FMOD_INITFLAGS
{
	FMOD_INIT_NORMAL = 0,
	FMOD_INIT_ENABLE_PROFILE = 32	/* openFmod init flags = profile*32 */
} FMOD_INITFLAGS;

typedef struct FMOD_GUID
{
	unsigned int Data1;
	unsigned short Data2;
	unsigned short Data3;
	char Data4[8];
} FMOD_GUID;

typedef struct FMOD_SYSTEM		FMOD_SYSTEM;
typedef struct FMOD_SOUND		FMOD_SOUND;
typedef struct FMOD_CHANNEL		FMOD_CHANNEL;
typedef struct FMOD_CHANNELGROUP	FMOD_CHANNELGROUP;

typedef FMOD_RESULT (*FMOD_CHANNEL_CALLBACK)(FMOD_CHANNEL* channel, FMOD_CHANNEL_CALLBACKTYPE type, void* commanddata1, void* commanddata2);

FMOD_RESULT FMOD_System_Create(FMOD_SYSTEM** system);

#ifdef __cplusplus
}
#endif

#endif /* _FMOD_H */
