/* Util/SoundService.h — October 2013.
   Member map = binary's own (batch792: C2 init runs + D2 cross-destruction
   reads at 0x2416??-0x2419??; member names cross-validated against the
   March 2016 SoundService.h where the offsets/values agree):

     +0x5C  bool                    2013-only byte; the C2 writes 1 before the
                                    member zero-runs and nothing in the
                                    decoded family reads it — name DELTA.
     +0x60  shared_ptr<FMOD::System> system        (D2 assert `!system`)
     +0x68  std::map<SoundType, shared_ptr<SoundChannel>> stockSounds
                                            (24B std::map in the binary; D2
                                             assert .size()==0 at Sound.cpp:260)
     +0x80  float dopplerscale     ctor 1.0f
     +0x84  float distancefactor   ctor 10.0f  (names/values from 2018-carry
     +0x88  float rolloffscale     ctor 1.0f   (whose order 1/10/1 matches the binary))
     +0x8C  shared_ptr<Instance> statsItem        (name 2016-carry; D2 releases the pn at +0x90)
     +0x94  ReverbType ambientReverb              (name 2016-carry; ctor 0 — DELTA)
     +0x98  scoped_ptr<CollisionSoundManager> collisionSoundManager (D2 scoped-destroy)
     +0x9C  int            zero-init, no dtor action — name DELTA
     +0xA0  int            zero-init, no dtor action — name DELTA
     +0xA4  LoadedSounds loadedSounds    {map<SoundId, shared_ptr<Sound>>; D2 assert :261}
     +0xBC  LoadedSounds loaded3DSounds  (D2 assert :262)
     +0xD4  bool initialized     ctor 0 — the "ready" gate playSound
                                 (0x2445AC family) and onServiceProvider
                                 (0x2449AC) read; 2016-carry name — DELTA
     +0xD5  bool muted           (play() 0x242AAC + playSound consult it;
                                 2016-carry name — DELTA)
     +0xD8  shared_ptr<SoundJob> soundJob   (D2 assert `!soundJob` — the pn
                                             at +0xDC is released first)
     class size 0xE0 (holds through the loaded3DSounds count at +0xD0).
   D2 destruction order confirmable from the asm: soundJob pn ->
   loaded3DSounds erase -> loadedSounds erase -> the +0x98 scoped ->
   statsItem pn -> stockSounds erase -> base Instance.

   Descriptions: no property descriptors were decoded for SoundService --
   the 5-prop run at 0x2C stride (FAF2AC..) belongs to SoundChannel.

   Bases carried from March 2016 (DescribedCreatable + Service), matching
   the binary's vtable family (double swap off_ED3A18). */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "Util/SoundWorld.h"		/* SoundType */
#include "Util/SoundChannel.h"

namespace RBX
{
namespace Soundscape
{

	class CollisionSoundManager { };	// DELTA: body undecoded; empty so scoped_ptr can destruct
	class SoundJob;					// shared_ptr payload; class body undecoded

	enum ReverbType
	{
		NoReverb = 0,
		GenericReverb,
		PaddedCell,
		Room,
		Bathroom,
		LivingRoom,
		StoneRoom,
		Auditorium,
		ConcertHall,
		Cave,
		Arena,
		Hangar,
		CarpettedHallway,
		Hallway,
		StoneCorridor,
		Alley,
		Forest,
		City,
		Mountains,
		Quarry,
		Plain,
		ParkingLot,
		SewerPipe,
		UnderWater
	};

	extern const char* const sSoundService;

	class SoundService
		: public DescribedCreatable<SoundService, Instance, sSoundService, Reflection::ClassDescriptor::PERSISTENT_HIDDEN>
		, public Service
	{
	private:
		typedef DescribedCreatable<SoundService, Instance, sSoundService, Reflection::ClassDescriptor::PERSISTENT_HIDDEN> Super;

		friend class SoundChannel;

		bool flagDelta0x5C;					/* +0x5C: ctor-true, unread -- name DELTA */

		shared_ptr<FMOD::System> system;	/* +0x60 */

		/* +0x68: the binary's stockSounds is a std::map (Rb_tree exports in
		   the PE: _M_erase/_M_insert_unique over
		   map<SoundType, shared_ptr<SoundChannel>>); count at +0x7C. */
		std::map<RBX::SoundType, shared_ptr<SoundChannel> > stockSounds;
		float dopplerscale;					/* +0x80: ctor 1.0f */
		float distancefactor;				/* +0x84: ctor 10.0f */
		float rolloffscale;					/* +0x88: ctor 1.0f */

		shared_ptr<Instance> statsItem;		/* +0x8C (pn +0x90) */
		ReverbType ambientReverb;			/* +0x94 (ctor NoReverb) */

	public:
		/* static keeper of the no-sound-companion build: bodies that skip
		   all FMOD calls (service-host shells) set this true. preloadSound/
		   updateListenState consult it. */
		static bool soundDisabled;

		SoundService();
		~SoundService();

		/* mac: 0x244C78 -- body undecoded -> stub returns an empty shared_ptr (DELTA). */
		shared_ptr<Sound> loadSound(SoundId id, bool is3D);

		void playSound(SoundType type);								/* 0x242A5E */
		void loadStockSounds();										/* 0x241DC6 (14 Boing..Victory pairs) */
		void loadStockSound(SoundType type, const std::string& url);	/* 0x24273E */

		/* mac: 0x2415CA: apply dopplerscale/distancefactor/rolloffscale to FMOD. */
		FMOD::System* update3DSettings();

		/* mac: 0x24161A: apply ambientReverb to FMOD.  Stubbed (reverb props
		   table undecoded -- DELTA). */
		void updateAmbientReverb();

		/* mac: 0x242A1C: set ambientReverb + raisePropertyChanged + update. */
		void setAmbientReverb(ReverbType value);

		/* mac: 0x241A22: FASTLOG bookends; clear stockSounds; removeAllChildren;
		   for_each+erase loadedSounds/loaded3DSounds (Sound::release each
		   value); destroy +0x98 CollisionSoundManager; System::release+reset;
		   initialized(false).  Sole caller (onServiceProvider 0x242BF6)
		   discards the result. */
		bool closeFmod();

		/* mac: 0x2410F0: FMOD open sequence -- create, version gate (<= 0x43204
		   throws), driver probe (0 drivers throws), caps/speaker probing,
		   init(100, profile*32), initialized(true).  FASTLOG bookends on
		   FLog::FMOD + FLog::Always throw arms. */
		void openFmod();

	protected:
		/* vtable+0x20 (2013 vftable 0x00b451e4; iOS 10-29): opens FMOD when added to a
		   provider, closes it when removed (mac: 0x2449AC). Body not written yet. */
		/*override*/ virtual void onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider);

	private:
		typedef std::map<SoundId, shared_ptr<Sound> > LoadedSounds;

		boost::scoped_ptr<CollisionSoundManager> collisionSoundManager;	/* +0x98 */
		int deltaField0x9C;					/* +0x9C: zero-init, no dtor action -- name DELTA */
		int deltaField0xA0;					/* +0xA0: zero-init, no dtor action -- name DELTA */

		LoadedSounds loadedSounds;			/* +0xA4 (count +0xB8) */
		LoadedSounds loaded3DSounds;		/* +0xBC (count +0xD0) */

		bool initialized;					/* +0xD4: ctor 0; the playSound/onServiceProvider gate */
		bool muted;							/* +0xD5 */

		shared_ptr<SoundJob> soundJob;		/* +0xD8 (pn +0xDC) */
	};

} // namespace Soundscape
} // namespace RBX
