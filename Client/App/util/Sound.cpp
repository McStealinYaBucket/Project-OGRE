/* App/util/Sound.cpp — October 2013.
   Sound-era mega-TU: both SoundService and SoundChannel families compile
   from this TU (binary assert-file evidence pins them to
   /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/util/Sound.cpp):
     ~SoundService: stockSounds.size()==0 @260, loadedSounds.size()==0 @261,
                    loaded3DSounds.size()==0 @262, !system @263, !soundJob @264
     ~SoundChannel: fmod_channel==NULL @737, !sound @738
     playSound(Instance const*): !sound @1190
   Bodies decoded from batch782/batch790/batch792.  Fidelity ~90% (DELTA
   list in the header comments of the touched files). */
#include "stdafx.h"
#include "rbx/atomic.h"
#include "Util/Sound.h"
#include "Util/SoundService.h"
#include "Util/SoundChannel.h"
#include "Util/SoundWorld.h"
#include "Util/ContentId.h"
#include "Util/FastFlags.h"
#include "Util/standardout.h"
#include "Util/utilities.h"
#include "FastLog.h"
#include "V8Tree/Property.h"
#include "V8DataModel/StockSound.h"
#include "V8DataModel/PartInstance.h"
#include "V8DataModel/RunService.h"
#include "V8DataModel/ContentProvider.h"
#include "V8DataModel/CoreGuiService.h"
#include "V8DataModel/Workspace.h"
#include "V8DataModel/GameSettings.h"
#include "V8DataModel/DebugSettings.h"
#include "V8Tree/Service.h"
#include "V8Tree/Instance.h"
#include "G3D/Vector3.h"
#include "G3D/CoordinateFrame.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include <string>
#include <stdexcept>

// Global FMOD shim helpers (defined in util/FmodShim.cpp).
void checkResult(FMOD_RESULT result);
void checkResultNoThrow(FMOD_RESULT result);
bool convert(const G3D::Vector3& vector, FMOD_VECTOR& result);

/* SoundService's log group (channel read directly by closeFmod, and by
   openFmod when it lands).  The landed convention declares+defines the
   group in the .cpp that consumes it (cf. ScriptContext.cpp, ThreadRef.cpp).
   DELTA: the binary's REGISTERED group-name string was not resolved in
   dbq24 ("FMOD" kept for consistency with CloseFmod/OpenFmod channel id
   FLog::FMOD[0]); registration name only matters to runtime log filters. */
LOGGROUP(FMOD);
LOGVARIABLE(FMOD, 1);

namespace RBX {
namespace Soundscape {
const char* const sSoundChannel = "Sound";
} // namespace Soundscape
} // namespace RBX

namespace RBX {

namespace Soundscape {

bool SoundService::soundDisabled = false;

FMOD::Sound* Sound::get(const Instance* instance)
{
	if (cachedSound != NULL)
		return cachedSound;

	ContentProvider* cp = ServiceProvider::create<ContentProvider>(instance);
	shared_ptr<const std::string> content = cp ? cp->requestContentString(soundId, 1.0f) : shared_ptr<const std::string>();

	if (content.get() != NULL && !content->empty())
	{
		bool flag = (GameSettings::singleton().highQualitySound == false);  // +109 == 0
		// FMOD_CREATESOUNDEXINFO (binary: v11[0]=124, v11[1]=strlen)
		// Mode bits: 32 (default) or 64 (high quality); 2048 (2D) or 2064 (3D).
		int mode = flag ? 32 : 64;
		int mode2 = is3D ? 2064 : 2048;

		FMOD_RESULT r = system->createSound(content->c_str(), (FMOD_MODE)(mode | mode2), NULL, &cachedSound);
		checkResult(r);
	}
	return cachedSound;
}

void Sound::release()
{
	if (cachedSound != NULL)
	{
		checkResultNoThrow(cachedSound->release());
		cachedSound = NULL;
	}
}

shared_ptr<Sound> SoundService::loadSound(SoundId id, bool is3D)
{
	LoadedSounds& loaded = is3D ? loaded3DSounds : loadedSounds;

	LoadedSounds::iterator it = loaded.find(id);
	if (it == loaded.end())
	{
		Sound* sound = new Sound();
		sound->system = system.get();
		sound->useCounter = 0;
		sound->soundId = id;
		sound->is3D = is3D;

		shared_ptr<Sound> sp(sound);
		loaded[id] = sp;

		return sp;
	}
	else
	{
		return it->second;
	}
}

} // namespace Soundscape

} // namespace RBX

using namespace RBX;

namespace RBX {

template<>
std::string StringConverter<Soundscape::SoundId>::convertToString(const Soundscape::SoundId& value)
{
	return value.toString();
}

template<>
bool StringConverter<Soundscape::SoundId>::convertToValue(const std::string& text, Soundscape::SoundId& value)
{
	value = Soundscape::SoundId(ContentId(text));
	return true;
}

} // namespace RBX

REFLECTION_BEGIN();

namespace RBX {
namespace Reflection {

template<>
EnumDesc<SoundType>::EnumDesc()
	: EnumDescriptor("SoundType", typeid(SoundType))
{
	addPair(NO_SOUND, "NoSound");
	addPair(BOING_SOUND, "Boing");
	addPair(BOMB_SOUND, "Bomb");
	addPair(BREAK_SOUND, "Break");
	addPair(CLICK_SOUND, "Click");
	addPair(CLOCK_SOUND, "Clock");
	addPair(SLINGSHOT_SOUND, "Slingshot");
	addPair(PAGE_SOUND, "Page");
	addPair(PING_SOUND, "Ping");
	addPair(SNAP_SOUND, "Snap");
	addPair(SPLAT_SOUND, "Splat");
	addPair(STEP_SOUND, "Step");
	addPair(STEP_ON_SOUND, "StepOn");
	addPair(SWOOSH_SOUND, "Swoosh");
	addPair(VICTORY_SOUND, "Victory");
}

template<>
int TypedPropertyDescriptor<Soundscape::SoundId>::getDataSize(const DescribedBase* instance) const
{
	return sizeof(Soundscape::SoundId) + getValue(instance).toString().size();
}

template<>
bool TypedPropertyDescriptor<Soundscape::SoundId>::hasStringValue() const
{
	return true;
}

template<>
std::string TypedPropertyDescriptor<Soundscape::SoundId>::getStringValue(const DescribedBase* instance) const
{
	return getValue(instance).toString();
}

template<>
bool TypedPropertyDescriptor<Soundscape::SoundId>::setStringValue(DescribedBase* instance, const std::string& text) const
{
	setValue(instance, Soundscape::SoundId(ContentId(text)));
	return true;
}

template<>
void TypedPropertyDescriptor<Soundscape::SoundId>::readValue(DescribedBase* instance, const XmlElement* element, IReferenceBinder& binder) const
{
	if (!element->isXsiNil())
	{
		std::string value;
		if (element->getValue(value))
			setValue(instance, Soundscape::SoundId(ContentId(value)));
	}
}

template<>
void TypedPropertyDescriptor<Soundscape::SoundId>::writeValue(const DescribedBase* instance, XmlElement* element) const
{
	element->setValue(getValue(instance).toString());
}

} // namespace Reflection
} // namespace RBX

RBX_REGISTER_ENUM(RBX::SoundType);

namespace RBX {
namespace Soundscape {

template<class Class, typename V>
class MethodProp : public Reflection::TypedPropertyDescriptor<V>
{
	template<class C>
	class GetSet : public Reflection::TypedPropertyDescriptor<V>::GetSet
	{
		V (C::*getter)() const;
		void (C::*setter)(V);
	public:
		GetSet(V (C::*g)() const, void (C::*s)(V)) : getter(g), setter(s) {}
		virtual bool isReadOnly() const { return false; }
		virtual bool isWriteOnly() const { return false; }
		virtual V getValue(const Reflection::DescribedBase* object) const
		{
			const C* c = static_cast<const C*>(object);
			return (c->*getter)();
		}
		virtual void setValue(Reflection::DescribedBase* object, const V& value) const
		{
			C* c = static_cast<C*>(object);
			(c->*setter)(value);
		}
	};
public:
	MethodProp(const char* name, const char* category, V (Class::*getter)() const, void (Class::*setter)(V),
		typename Reflection::PropertyDescriptor::Attributes flags = Reflection::PropertyDescriptor::STANDARD,
		Security::Permissions security = Security::None)
		: Reflection::TypedPropertyDescriptor<V>(Class::classDescriptor(), name, category,
			std::auto_ptr<typename Reflection::TypedPropertyDescriptor<V>::GetSet>(new GetSet<Class>(getter, setter)),
			flags, security)
	{
	}
};

static MethodProp<SoundChannel, SoundId> sound_desc_SoundId("SoundId", category_Data, &SoundChannel::getSoundId, &SoundChannel::setSoundId);
static MethodProp<SoundChannel, float> sound_desc_Volume("Volume", category_Data, &SoundChannel::getVolume, &SoundChannel::setVolume);
static MethodProp<SoundChannel, float> sound_desc_Pitch("Pitch", category_Data, &SoundChannel::getPitch, &SoundChannel::setPitch);
static MethodProp<SoundChannel, int> sound_desc_PlayCount("PlayCount", category_Data, &SoundChannel::getPlayCount, &SoundChannel::setPlayCount, Reflection::PropertyDescriptor::REPLICATE_ONLY);
static MethodProp<SoundChannel, bool> sound_desc_Looped("Looped", category_Behavior, &SoundChannel::getLooped, &SoundChannel::setLooped);

} // namespace Soundscape
} // namespace RBX

REFLECTION_END();

namespace RBX {

namespace Network {

const Instance* Players::findConstLocalPlayer(const Instance* context)
{
	(void)context;
	return NULL;	// DELTA: stub
}

} // namespace Network

namespace Soundscape {

static FMOD_RESULT channelCallback(FMOD_CHANNEL* channel, FMOD_CHANNEL_CALLBACKTYPE type, void* commanddata1, void* commanddata2)
{
	(void)channel;
	(void)type;
	(void)commanddata1;
	(void)commanddata2;
	return FMOD_OK;
}

static float soundPreloadPriority = 1.0f;	// DELTA: binary global off_EAE4D0

/* ---------------------------------------------------------------- SoundService */

SoundService::SoundService()
	: flagDelta0x5C(true),
	  system(),
	  dopplerscale(1.0f),
	  distancefactor(10.0f),
	  rolloffscale(1.0f),
	  ambientReverb(NoReverb),
	  collisionSoundManager(),
	  deltaField0x9C(0),
	  deltaField0xA0(0),
	  initialized(false),
	  muted(false),
	  soundJob()
{
	setName("Soundscape");
}

SoundService::~SoundService()
{
	// ~SoundService asserts (binary lines 260-264) follow:
	RBXASSERT(stockSounds.size() == 0);
	RBXASSERT(loadedSounds.size() == 0);
	RBXASSERT(loaded3DSounds.size() == 0);
	RBXASSERT(!system);
	RBXASSERT(!soundJob);
}

/* mac: 0x241A22 closeFmod, binary-ordered:
     1. FASTLOG(FLog::FMOD, "Closing Fmod.")            -- guarded on channel
     2. stockSounds rb_tree erase + header reset        -> clear()
     3. Instance::removeAllChildren(this)
     4. for_each(loadedSounds, sub_241BD8)              (Sound::release on the
        pair's px, unconditional) + erase + header reset
     5. for_each(loaded3DSounds, sub_241BD8)            + erase + header reset
     6. +0x98 scoped destroy (if px non-zero: delete, px=0)
        -> scoped_ptr::reset()
     7. if (system) { System::release + checkResultNoThrow; px/pn = 0 }
        -> shared_ptr::reset()
     8. initialized = false                             (byte +0xD4)
     9. FASTLOG(FLog::FMOD, "Fmod Closed.")
   Return value NOT observed by the sole caller (onServiceProvider);
   the true here is a shim-safety standing (the binary's decompiled-blob
   return is register garbage from the FastLog tail) -- DELTA note. */
// STUB: PLAYER 0x006f1330
// STUB: STUDIO 0x00806480
bool SoundService::closeFmod()
{
	FASTLOG(FLog::FMOD, "Closing Fmod.");

	stockSounds.clear();
	removeAllChildren();

	for (LoadedSounds::iterator it = loadedSounds.begin(); it != loadedSounds.end(); ++it)
		it->second->release();
	for (LoadedSounds::iterator it = loaded3DSounds.begin(); it != loaded3DSounds.end(); ++it)
		it->second->release();

	loadedSounds.clear();
	loaded3DSounds.clear();

	collisionSoundManager.reset();

	if (system.get() != NULL)
	{
		FMOD_RESULT result = system->release();
		checkResultNoThrow(result);
		system.reset();
	}

	initialized = false;

	FASTLOG(FLog::FMOD, "Fmod Closed.");
	return true;
}

/* mac: 0x2410F0 openFmod, binary-ordered (raw-asm verified arms):
     0. gates -- soundDisabled static + GameSettings-item byte +0x6C
        (landed GameSettings::singleton().soundEnabled -- same byte the
        preloadSound 0x2449EC gate already lands by name, LAYOUT DELTA)
     1. FASTLOG(FLog::FMOD, "Opening Fmod.") -- cached-channel probe; the
        landed FASTLOG re-reads the channel (same static, compile artifact)
     2. FMOD_System_Create(&this+0x60) + checkResult
        -> landed punning: raw OUT into FMOD_SYSTEM*, shared_ptr reset
     3. System::getVersion + checkResult; version <= 0x43204:
        FastLog(channel=1=FLog::Always, "Incorrect FMODEX DLL version")
        + throw runtime_error (same string)
     4. getNumDrivers + checkResultNoThrow; 0 -> FastLog(FLog::Always,
        "No active sound driver detected") + StandardOut::print(level 2 =
        MESSAGE_WARNING, msg) + throw runtime_error(msg)
     5. setDriver(0) + getDriver + checkResultNoThrow; driver >= 0 ->
        getDriverInfo(driver, name, 128, &guid) -- DELTA: the binary passes
        an unlanded global struct's +0x244 name buffer; port uses a
        file-static 128B buffer
     6. getDriverCaps(driver, &caps, 0, 0, &speakerMode) + checkResult,
        setSpeakerMode(speakerMode) + checkResult; caps & 2 ->
        StandardOut::print(MESSAGE_WARNING, "The acceleration slider in the
        Sound Control Panel set to 'off'") + setDSPBufferSize(0x400, 10)
     7. profileFlag = DebugSettings-item byte +0x6A (landed
        DebugSettings::singleton().isFmodProfilingEnabled, LAYOUT DELTA);
        setSpeakerMode(FMOD_SPEAKERMODE_STEREO=2) + init(100, 32*profile, NULL)
     8. update3DSettings(); if (ambientReverb != 0) updateAmbientReverb()
     9. initialized = true (+0xD4); FASTLOG(FLog::FMOD,
        "Fmod initialization complete!")
   Binary returns void (caller 0x242BF6 does a plain call). */
// STUB: PLAYER 0x006f0f90
// STUB: STUDIO 0x008060e0
void SoundService::openFmod()
{
	if (soundDisabled)
		return;
	if (!GameSettings::singleton().getSoundEnabled())
		return;

	FASTLOG(FLog::FMOD, "Opening Fmod.");

	FMOD_SYSTEM* rawSystem = NULL;
	checkResult(FMOD_System_Create(&rawSystem));
	system = shared_ptr<FMOD::System>((FMOD::System*)rawSystem);

	unsigned int version = 0;
	checkResult(system->getVersion(&version));
	if (version <= 0x43204)
	{
		FASTLOG(FLog::Always, "Incorrect FMODEX DLL version");
		throw std::runtime_error("Incorrect FMODEX DLL version");
	}

	int numDrivers = 0;
	checkResultNoThrow(system->getNumDrivers(&numDrivers));
	if (numDrivers == 0)
	{
		FASTLOG(FLog::Always, "No active sound driver detected");
		StandardOut::singleton()->print(MESSAGE_WARNING, "No active sound driver detected");
		throw std::runtime_error("No active sound driver detected");
	}

	checkResultNoThrow(system->setDriver(0));

	int driver = -1;
	checkResultNoThrow(system->getDriver(&driver));

	FMOD_GUID guid;
	static char driverName[128];	/* DELTA buffer -- see header comment */
	if (driver >= 0)
		checkResultNoThrow(system->getDriverInfo(driver, driverName, 128, &guid));

	FMOD_CAPS caps;
	FMOD_SPEAKERMODE speakerMode = FMOD_SPEAKERMODE_RAW;
	checkResult(system->getDriverCaps(driver, &caps, 0, 0, &speakerMode));
	checkResult(system->setSpeakerMode(speakerMode));
	if (caps & 2)
	{
		StandardOut::singleton()->print(MESSAGE_WARNING,
			"The acceleration slider in the Sound Control Panel set to 'off'");
		checkResult(system->setDSPBufferSize(0x400, 10));
	}

	bool isProfile = DebugSettings::singleton().getIsFmodProfilingEnabled();
	checkResult(system->setSpeakerMode(FMOD_SPEAKERMODE_STEREO));
	checkResult(system->init(100, (FMOD_INITFLAGS)(32 * (int)isProfile), NULL));

	update3DSettings();
	if (ambientReverb != 0)
		updateAmbientReverb();
	initialized = true;

	FASTLOG(FLog::FMOD, "Fmod initialization complete!");
}

void SoundService::playSound(SoundType type)
{
	// Binary (0x242A5E): walks the stockSounds rb_tree to find the stock
	// sound with the largest key <= type, then calls SoundChannel::play().
	std::map<SoundType, shared_ptr<SoundChannel>>::iterator it = stockSounds.upper_bound(type);
	if (it != stockSounds.begin())
	{
		--it;
		if (it->second.get() != NULL)
			it->second->play();
	}
}

FMOD::System* SoundService::update3DSettings()
{
	FMOD::System* result = system.get();
	if (result != NULL)
	{
		FMOD_RESULT v2 = result->set3DSettings(dopplerscale, distancefactor, rolloffscale);
		checkResultNoThrow(v2);
	}
	return result;
}

void SoundService::updateAmbientReverb()
{
	if (system.get() != NULL)
	{
		// DELTA: boost::call_once + reverb props table undecoded.
		// Stubbed.
	}
}

void SoundService::setAmbientReverb(ReverbType value)
{
	if (value != ambientReverb)
	{
		ambientReverb = value;
		// DELTA: raisePropertyChanged(prop_AmbientReverb) undecoded.
		updateAmbientReverb();
	}
}

void SoundService::loadStockSounds()
{
	stockSounds.clear();
	removeAllChildren();

	loadStockSound(BOING_SOUND, "Sounds/bass.wav");
	loadStockSound(BOMB_SOUND, "Sounds/collide.wav");
	loadStockSound(BREAK_SOUND, "Sounds/glassbreak.wav");
	loadStockSound(CLICK_SOUND, "Sounds/switch.wav");
	loadStockSound(CLOCK_SOUND, "Sounds/clickfast.wav");
	loadStockSound(SLINGSHOT_SOUND, "Sounds/Rubber band sling shot.wav");
	loadStockSound(PAGE_SOUND, "Sounds/pageturn.wav");
	loadStockSound(PING_SOUND, "Sounds/electronicpingshort.wav");
	loadStockSound(SNAP_SOUND, "Sounds/snap.wav");
	loadStockSound(SPLAT_SOUND, "Sounds/splat.wav");
	loadStockSound(STEP_SOUND, "Sounds/SWITCH3.wav");
	loadStockSound(STEP_ON_SOUND, "Sounds/flashbulb.wav");
	loadStockSound(SWOOSH_SOUND, "Sounds/swoosh.wav");
	loadStockSound(VICTORY_SOUND, "Sounds/victory.wav");
}

void SoundService::loadStockSound(SoundType type, const std::string& url)
{
	shared_ptr<StockSound> stockSound = Creatable<Instance>::create<StockSound>();
	stockSound->setName(Reflection::EnumDesc<SoundType>::singleton().convertToString(type));
	stockSound->setSoundId(SoundId(ContentId::fromAssets(url.c_str())));
	stockSound->setParent(this);
	stockSounds[type] = stockSound;
}

/* ---------------------------------------------------------------- SoundChannel */

SoundChannel::SoundChannel()
	: sound(),
	  fmod_channel(NULL),
	  soundId(),
	  volume(0.5f),
	  pitch(1.0f),
	  defaultFrequency(),	// NOT initialized (playSound fills it)
	  playOnRemove(false),
	  flags(0),
	  playCount(-1),
	  reqPlayCount(-1),
	  part(NULL),
	  connection()
{
	flags &= 0xF8;
	setName("Sound");

}

SoundChannel::~SoundChannel()
{
























































































































































































































































































































































































































	// ~SoundChannel asserts (binary lines 737-738) follow:
	RBXASSERT(fmod_channel == NULL);
	RBXASSERT(!sound);

	connection.disconnect();

}

SoundId SoundChannel::getSoundId() const
{
	return soundId;
}

void SoundChannel::setSoundId(SoundId value)
{
	if (soundId != value)
	{
		soundId = value;
		preloadSound();
		releaseChannel();
		raisePropertyChanged(sound_desc_SoundId);
	}
}

float SoundChannel::getVolume() const
{
	return volume;
}

void SoundChannel::setVolume(float value)
{
	float v = 0.0f;
	if (value > 0.0f)
		v = (value < 1.0f) ? value : 1.0f;
	if (volume != v)
	{
		volume = v;
		if (fmod_channel != NULL)
			checkResultNoThrow(fmod_channel->setVolume(v));
		raisePropertyChanged(sound_desc_Volume);
	}
}

float SoundChannel::getPitch() const
{
	return pitch;
}

void SoundChannel::setPitch(float value)
{
	if (pitch != value)
	{
		pitch = value;
		if (fmod_channel != NULL)
			checkResultNoThrow(fmod_channel->setFrequency(pitch * defaultFrequency));
		raisePropertyChanged(sound_desc_Pitch);
	}
}

int SoundChannel::getPlayCount() const
{
	return playCount;
}

void SoundChannel::setPlayCount(int value)
{
	if (value != 0)
	{
		if (value == -1)
		{
			stop();
		}
		else if (reqPlayCount < value)
		{
			if (!FFlag::DE4528Fixed || ServiceProvider::find<Soundscape::SoundService>(this) != NULL)
			{
				playSound(this);
				playCount = value;
			}
			reqPlayCount = value;
			raisePropertyChanged(sound_desc_PlayCount);
		}
	}
	else
	{
		pause();
	}
}

bool SoundChannel::getLooped() const
{
	return (flags & 2) != 0;
}

// STUB: PLAYER 0x006f0870
// STUB: STUDIO 0x00805a30
void SoundChannel::setLooped(bool value)
{
	if (((flags & 2) != 0) != value)
	{
		flags = (flags & 0xFD) | (value ? 2 : 0);
		updateLooped();
		raisePropertyChanged(sound_desc_Looped);
	}
}

bool SoundChannel::isPlaying() const
{
	if (fmod_channel != NULL)
	{
		bool playing = false;
		FMOD_RESULT r = fmod_channel->isPlaying(&playing);
		if (r != 36)	// FMOD_ERR_INVALID_HANDLE
			return playing;
		return false;
	}
	return playCount > 0;
}

bool SoundChannel::isPaused() const
{
	if (fmod_channel != NULL)
	{
		bool paused = false;
		FMOD_RESULT r = fmod_channel->getPaused(&paused);
		if (r == 36)	// FMOD_ERR_INVALID_HANDLE
			return true;
		return paused;
	}
	return playCount == 0;
}

void SoundChannel::play()
{
	playSound(this);
	int v = reqPlayCount + 1;
	if (v <= 0)
		v = 1;
	reqPlayCount = v;
	playCount = v;
	raisePropertyChanged(sound_desc_PlayCount);

	SoundService* service = ServiceProvider::create<SoundService>(this);
	if (service != NULL && service->muted && fmod_channel != NULL)
		fmod_channel->setMute(true);
}

void SoundChannel::pause()
{
	if (fmod_channel != NULL)
		checkResultNoThrow(fmod_channel->setPaused(true));
	updateListenState();
	if (reqPlayCount != 0)
	{
		reqPlayCount = 0;
		playCount = 0;
		raisePropertyChanged(sound_desc_PlayCount);
	}
}

void SoundChannel::stop()
{
	if (fmod_channel != NULL)
	{
		checkResultNoThrow(fmod_channel->setUserData(NULL));
		checkResultNoThrow(fmod_channel->stop());
		fmod_channel = NULL;
	}
	updateListenState();
	if (reqPlayCount != -1)
	{
		reqPlayCount = -1;
		playCount = -1;
		raisePropertyChanged(sound_desc_PlayCount);
	}
}

// FUNCTION: PLAYER 0x006ceaa0 FOLDED
// FUNCTION: STUDIO 0x00812790 FOLDED
bool SoundChannel::askSetParent(const Instance* instance) const
{
	(void)instance;
	return true;
}

void SoundChannel::releaseChannel()
{
	if (fmod_channel != NULL)
	{
		checkResultNoThrow(fmod_channel->setUserData(NULL));
		fmod_channel = NULL;
	}
	if (sound.get() != NULL)
	{
		--sound.get()->useCounter;
		sound.reset();
	}
}

// STUB: PLAYER 0x006f30e0
// STUB: STUDIO 0x008080e0
void SoundChannel::updateListenState()
{
	if (part == NULL || !(flags & 1) || fmod_channel == NULL || SoundService::soundDisabled)
	{
		connection.disconnect();
	}
	else
	{
		RunService* runService = ServiceProvider::create<RunService>(this);
		if (runService != NULL)
		{
			rbx::signals::connection newConnection = runService->heartbeatSignal.connect(boost::bind(&SoundChannel::onHeartbeat, this, _1));
			if (connection != newConnection)
			{
				connection.disconnect();
				connection = newConnection;
			}
		}
	}
}

// STUB: PLAYER 0x006f07e0
// STUB: STUDIO 0x008059a0
void SoundChannel::updateLooped()
{
	if (fmod_channel != NULL)
	{
		FMOD_MODE mode = (FMOD_MODE)0;
		checkResultNoThrow(fmod_channel->getMode(&mode));
		bool loopedFlag = getLooped();
		mode = (FMOD_MODE)(((int)mode | (loopedFlag + 1)) & (loopedFlag - 3));
		checkResultNoThrow(fmod_channel->setMode(mode));
		checkResultNoThrow(fmod_channel->setLoopCount(getLooped() ? -1 : 0));
	}
}

void SoundChannel::update3D(FMOD::Channel* channel)
{
	if ((flags & 1) && part != NULL)
	{
		G3D::CoordinateFrame cf = part->getCoordinateFrame();
		FMOD_VECTOR pos;
		FMOD_VECTOR vel;
		if (convert(cf.translation, pos))	// DELTA: convert signature takes (Vector3, FMOD_VECTOR&) -> bool
		{
			if (convert(part->getLinearVelocity(), vel))
			{
				checkResultNoThrow(channel->set3DAttributes(&pos, &vel));
			}
		}
	}
}

void SoundChannel::onHeartbeat(const Heartbeat& heartbeat)
{
	(void)heartbeat;
	if (!isPaused())
		update3D(fmod_channel);
}

void SoundChannel::onAncestorChanged(const AncestorChanged& event)
{
	connection.disconnect();
	Instance::onAncestorChanged(event);

	if (event.child == this)
	{
		if (event.newParent != NULL && event.newParent->getDescriptor().isA(PartInstance::classDescriptor()))
			part = static_cast<PartInstance*>(const_cast<Instance*>(event.newParent));
		else
			part = NULL;
		updateListenState();
	}

	if (playOnRemove && !getLooped())
	{
		Workspace* oldWorkspace = ServiceProvider::find<Workspace>(event.oldParent);
		if (oldWorkspace != NULL)
		{
			const Instance* walker = event.oldParent;
			bool reachedOld = false;
			if (walker == oldWorkspace)
				reachedOld = true;
			else
			{
				while (walker != NULL)
				{
					walker = walker->getParent();
					if (walker == oldWorkspace)
					{
						reachedOld = true;
						break;
					}
				}
			}
			if (reachedOld)
			{
				Workspace* newWorkspace = ServiceProvider::find<Workspace>(event.newParent);
				bool playIt = false;
				if (newWorkspace == NULL)
					playIt = true;
				else
				{
					const Instance* walker2 = event.newParent;
					if (walker2 != newWorkspace)
					{
						bool reachedNew = false;
						while (walker2 != NULL)
						{
							walker2 = walker2->getParent();
							if (walker2 == newWorkspace)
							{
								reachedNew = true;
								break;
							}
						}
						if (!reachedNew)
							playIt = true;
					}
				}
				if (playIt)
				{
					playSound(oldWorkspace);
					releaseChannel();
				}
			}
		}
	}
}

// STUB: PLAYER 0x006f3c10
// STUB: STUDIO 0x00808c10
void SoundChannel::playSound(const Instance* instance)
{



















































































































	// playSound(Instance const*) asserts !sound (binary line 1190):
	if (flags & 4)
		return;

	SoundService* service = ServiceProvider::create<SoundService>(instance);
	if (service != NULL && service->muted)
		return;

	if (!isHeardLocally(instance))
		return;

	if (flags & 2)
	{
		if (isPlaying())
		{
			checkResult(fmod_channel->setPaused(false));
			return;
		}
	}

	if (service == NULL)
		return;

	if (service->initialized == 0)
	{
		flags |= 4;
		return;
	}

	releaseChannel();
	RBXASSERT(!sound);

	flags = (flags & 0xFE) | (part != NULL ? 1 : 0);

	try
	{
		sound = service->loadSound(soundId, (flags & 1) != 0);
		if (sound.get() != NULL)
			++sound.get()->useCounter;
		FMOD::Sound* fmodSound = (sound.get() != NULL) ? sound.get()->get(instance) : NULL;
		if (fmodSound != NULL)
		{
			checkResult(service->system->playSound(FMOD_CHANNEL_FREE, fmodSound, true, &fmod_channel));
			updateLooped();
			update3D(fmod_channel);
			checkResult(fmod_channel->setVolume(volume));
			checkResult(fmodSound->getDefaults(&defaultFrequency, NULL, NULL, NULL));
			if (pitch != 1.0f)
				checkResult(fmod_channel->setFrequency(pitch * defaultFrequency));
			checkResult(fmod_channel->setUserData((void*)this));
			checkResult(fmod_channel->setCallback(channelCallback));
			checkResult(fmod_channel->setPaused(false));
			updateListenState();
		}
	}
	catch (std::exception& e)
	{
		StandardOut::singleton()->print(MESSAGE_ERROR, e);
	}
}

// STUB: PLAYER 0x006f4220
// STUB: STUDIO 0x00809270
void SoundChannel::onServiceProvider(ServiceProvider* oldProvider, ServiceProvider* newProvider)
{
	if (oldProvider != NULL)
	{
		if (flags & 2)
			stop();
		releaseChannel();
	}
	if (newProvider != NULL)
	{
		SoundService* service = ServiceProvider::create<SoundService>(newProvider);
		if (service != NULL && service->initialized != 0)
		{
			flags &= ~4;
			preloadSound();
			updateListenState();
			if (playCount < reqPlayCount)
				playSound(this);
		}
	}
}

void SoundChannel::preloadSound()
{
	if (!SoundService::soundDisabled
		&& !(flags & 4)
		&& GameSettings::singleton().soundEnabled)
	{
		ContentProvider* contentProvider = ServiceProvider::create<ContentProvider>(this);
		if (contentProvider != NULL)
		{
			contentProvider->requestContentString(soundId, soundPreloadPriority);
		}
	}
}

bool SoundChannel::isHeardLocally(const Instance* instance)
{
	if (!FFlag::AreSoundsFiltered)
		return true;

	const Instance* walker = instance;
	const Instance* localPlayer = Network::Players::findConstLocalPlayer(instance);
	if (this == instance)
		walker = walker->getParent();

	if (walker == NULL)
		return false;

	while (walker != NULL)
	{
		const Reflection::ClassDescriptor& desc = walker->getDescriptor();
		if (desc.isA(SoundService::classDescriptor()))
			return true;
		if (desc.isA(Workspace::classDescriptor()))
			return true;
		if (desc.isA(CoreGuiService::classDescriptor()) || walker == localPlayer)
			return true;
		walker = walker->getParent();
	}
	return false;
}

} // namespace Soundscape

} // namespace RBX

/* ---------------------------------------------------------------- registrars */

namespace RBX {
namespace Reflection {
template<>
ClassRegistrar<Soundscape::SoundChannel> ClassRegistrar<Soundscape::SoundChannel>::registrar(0);
template<>
ClassRegistrar<Soundscape::SoundService> ClassRegistrar<Soundscape::SoundService>::registrar(0);
} // namespace Reflection
} // namespace RBX
namespace RBX {
namespace Reflection {

template<>
const Type& Type::getSingleton<Soundscape::SoundId>()
{
	static TType<Soundscape::SoundId> type("SoundId");
	return type;
}

template<>
Soundscape::SoundId& Variant::convert<Soundscape::SoundId>()
{
	Soundscape::SoundId* id = tryCast<Soundscape::SoundId>();
	if (id)
		return *id;
	if (_type->isType<std::string>())
	{
		Soundscape::SoundId v;
		if (StringConverter<Soundscape::SoundId>::convertToValue(cast<std::string>(), v))
		{
			value = v;
			_type = &Type::getSingleton<Soundscape::SoundId>();
			return cast<Soundscape::SoundId>();
		}
	}
	throw RBX::runtime_error("Unable to cast to SoundId");
}

} // namespace Reflection
} // namespace RBX
