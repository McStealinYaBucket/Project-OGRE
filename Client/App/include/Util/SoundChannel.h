/* Util/SoundChannel.h — October 2013.
   Soundscape::SoundChannel decode (batch792):
     C2 0x243C32     — sound px/pn zeroed (+0x5C/+0x60), fmod_channel 0
                       (+0x64), soundId empty-rep + getNullName (+0x68/
                       +0x6C), volume 0.5f (+0x70), pitch 1.0f (+0x74),
                       defaultFrequency NOT initialized (+0x78 — written by
                       playSound's getDefaults), +0x7C byte 0, flags &= 0xF8
                       (+0x7D, bits 0-2 cleared), playCount/reqPlayCount -1
                       (+0x80/+0x84), part/connection 0 (+0x88/+0x8C),
                       setName(vt slot 7, "Sound"). C2 head increments
                       Diagnostics::Countable<SoundChannel>::count.
     D2 0x243EF5     — asserts fmod_channel==NULL (Sound.cpp:737) and !sound
                       (:738), connection.disconnect, then member
                       destruction and the Countable decrement.
     playSound 0x2445AC — asserts !sound at Sound.cpp:1190.
   Bodies: getSoundId 0x2438B2, getVolume 0x2438DE, setVolume 0x2438EA,
   getPitch 0x24395C, setPitch 0x243968, setPlayCount 0x2439CA,
   getLooped 0x243A50, setLooped 0x243A62, isPlaying 0x243AB8,
   isPaused 0x243AFA, releaseChannel 0x24433C, askSetParent 0x24439A,
   updateListenState 0x2443A4, onAncestorChanged/onHeartbeat
   (batch792 line 2943/2915), playSound(Instance const*) 0x2445AC,
   onServiceProvider 0x244976, preloadSound 0x2449EC, update3D 0x244A62,
   updateLooped 0x244ADE, isHeardLocally 0x244B9A; play/pause/stop/setSoundId
   from batch790.
   Descriptions: 5 props at 0x2C stride — FAF2AC SoundId, FAF2D8 Volume,
   FAF304 Pitch, FAF330 PlayCount, FAF35C Looped (2013 names; the 2016
   MinDistance/MaxDistance/TimeLength/TimePosition props postdate this
   build — DELTA note). */

#pragma once

#include "V8Tree/Instance.h"
#include "Util/ContentId.h"
#include "Util/RunTransition.h"		/* Heartbeat */
#include "rbx/signal.h"
#include "fmod/fmod.hpp"

namespace G3D
{
	class Matrix3;
	class CoordinateFrame;
}

namespace RBX
{
	class PartInstance;

	namespace Soundscape
	{
		class Sound;		/* minimal shell: see Util/Sound.h */

		/* Soundscape::SoundId — exactly ContentId-shaped 8B in the binary
		   ({string, Name*}): SoundChannel::getSoundId 0x2438B2 copies the
		   string half then the Name* half; the loadedSounds/loaded3DSounds
		   map keys use std::less<SoundId>. SoundId's own C1 0x2429F4 is
		   undecoded — DELTA (landed as a trivial ContentId carrier). */
		class SoundId : public ContentId
		{
		public:
			SoundId() {}
			explicit SoundId(const ContentId& content)			: ContentId(content) {}
		};

		extern const char* const sSoundChannel;

		class SoundChannel
			: public DescribedCreatable<SoundChannel, Instance, sSoundChannel>
			, public Diagnostics::Countable<SoundChannel>	// ctor 0x006f2ffc / dtor 0x006ededd count after Instance
		{
		public:
			SoundChannel();
			~SoundChannel();

			SoundId getSoundId() const;						// 0x2438B2
			void setSoundId(SoundId value);					// batch790 0x242AAC region
			float getVolume() const;						// 0x2438DE
			void setVolume(float value);					// 0x2438EA
			float getPitch() const;							// 0x24395C
			void setPitch(float value);						// 0x243968
			int getPlayCount() const;						/* 2016-carry getter for the FAF330 desc — the 2013 getter body was not individually walked; DELTA */
			void setPlayCount(int value);					// 0x2439CA
			bool getLooped() const;							// 0x243A50
			void setLooped(bool value);						// 0x243A62
			bool isPlaying() const;							// 0x243AB8
			bool isPaused() const;							// 0x243AFA

			void play();									// batch790
			void pause();									// batch790
			void stop();									// batch790

			/*override*/ bool askSetParent(const Instance* instance) const;	// 0x24439A (returns true)

			void preloadSound();							// 0x2449EC
			void releaseChannel();							// 0x24433C
			void updateListenState();						// 0x2443A4
			void updateLooped();							// 0x244ADE
			void update3D(FMOD::Channel* channel);			// 0x244A62

		private:
			/*override*/ virtual void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider);	// 0x244976
			/*override*/ virtual void onAncestorChanged(const AncestorChanged& event);	// batch792 line 2943

			void playSound(const Instance* instance);		// 0x2445AC
			bool isHeardLocally(const Instance* instance);	// 0x244B9A
			void onHeartbeat(const Heartbeat& heartbeat);	// batch792 line 2915

			boost::shared_ptr<Sound> sound;					// px@0x5C pn@0x60 (the D2 assert `!sound` names it)
			FMOD::Channel* fmod_channel;					// +0x64
			SoundId soundId;								// {string@0x68, Name*@0x6C}
			float volume;									// +0x70: ctor 0.5f
			float pitch;									// +0x74: ctor 1.0f
			float defaultFrequency;							// +0x78: NOT ctor-initialized (playSound fills from getDefaults)
			bool playOnRemove;								// +0x7C: ctor 0; read as the on-ancestor-removed replay gate
			unsigned char flags;							// +0x7D: bit0 = is3D (set from part!=NULL in playSound),
															//        bit1 = looped (get/setLooped, updateLooped),
															//        bit2 = invalidChannel (set when the service is not ready; cleared in onServiceProvider)
			int playCount;									// +0x80: ctor -1
			int reqPlayCount;								// +0x84: ctor -1
			PartInstance* part;								// +0x88
			rbx::signals::connection connection;			// +0x8C: heartbeat connection (weak intrusive ptr in the binary)
		};

	} // namespace Soundscape
} // namespace RBX
