/* Win/VideoControl.h -- October 2013 client shell (Client\Win\VideoControl.cpp, asserts `verb` line 36,
   `rbxView` 39, `soundservice` 111, `captureStarted` 127; PLAYER and STUDIO).
   2013's VideoControl is an Ogre::IFrameDataCallback (RTTI), unlike 2016's, keeps a 0x100-byte path
   buffer, and its IVideoCapture has 10 slots. Written so far: the ctor (PLAYER 0x00464c30), the
   destructor and the four frame callbacks; startRecording (0x00464f30) needs SoundService's
   2013 callbacks and two ViewBase virtuals first. */

#pragma once

#include <string>

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

#include "decomp.h"

namespace FMOD
{
	class DSP;
}
struct FMOD_DSP_DESCRIPTION;

namespace Ogre
{
	// RbxOgre's frame-data callback (RTTI .?AVIFrameDataCallback@Ogre@@). Declared here until the
	// RbxOgre header that defines it exists (Client/Rendering). Its out-of-line dtor is PLAYER
	// 0x00464b20 (STUDIO 0x0048d680).
	// VTABLE: PLAYER 0x00be517c
	// VTABLE: STUDIO 0x00d19d9c
	class IFrameDataCallback
	{
	public:
		virtual ~IFrameDataCallback() {}           // vtable+0x00
		virtual void VTable0x04(void* p_unk) = 0;  // vtable+0x04
		virtual void VTable0x08(void* p_unk) = 0;  // vtable+0x08
		virtual void VTable0x0c() = 0;             // vtable+0x0c
		virtual void VTable0x10() = 0;             // vtable+0x10

		// SYNTHETIC: PLAYER 0x00464b30
		// SYNTHETIC: STUDIO 0x0048d660
		// Ogre::IFrameDataCallback::`scalar deleting destructor'
	};
}

namespace RBX {

	class Verb;
	class ViewBase;
	// GfxBase's FrameRateManager (Rendering): GfxBase/FrameRateManager.h is still empty, so only the call
	// the shell makes is declared here. Delete this when that header declares the class.
	class FrameRateManager
	{
	public:
		void ResumeAutoAdjustment(); // PLAYER 0x009c7ac0 (GfxBase, __thiscall; stores [this+0xc] = 1)
	};
	namespace Soundscape
	{
		class SoundService;
	}

	// The sound callbacks the recording loop uses (startRecording binds three SoundService
	// members). Its dtor is out of line: PLAYER 0x00464b60, STUDIO 0x0048c6e0.
	// SIZE 0x60
	struct SoundState
	{
		boost::function<FMOD::DSP*(FMOD_DSP_DESCRIPTION&)> createDSPFunction; // 0x00
		boost::function<int()> getSampleRateFunction;                         // 0x20
		boost::function<bool()> enabledFunction;                              // 0x40
	};

	// VTABLE: PLAYER 0x00be12b4
	// VTABLE: STUDIO 0x00d0c3a4
	// 10 slots in 2013 (dtor 0x00430a20, then nine pure). The first five follow the 2016 order
	// (isRunning: RecordToggleVerb::isChecked; setVideoQuality: VideoControl's ctor); the frame
	// callbacks forward to 0x14/0x18 (with the verb) and 0x20/0x24. 0x1c is not seen called here.
	class IVideoCapture
	{
	public:
		virtual ~IVideoCapture() {}                                     // vtable+0x00
		virtual bool start(int cx, int cy, SoundState* soundState) = 0; // vtable+0x04
		virtual bool stop() = 0;                                        // vtable+0x08
		virtual bool isRunning() = 0;                                   // vtable+0x0c
		virtual void setVideoQuality(int videoQuality) = 0;             // vtable+0x10
		virtual void VTable0x14(void* p_unk, Verb* verb) = 0;           // vtable+0x14
		virtual void VTable0x18(void* p_unk, Verb* verb) = 0;           // vtable+0x18
		virtual std::string& getFileName() = 0;                         // vtable+0x1c (RecordToggleVerb::startAction)
		virtual void VTable0x20() = 0;                                  // vtable+0x20
		virtual void VTable0x24() = 0;                                  // vtable+0x24
	};

	// VTABLE: PLAYER 0x00be5164
	// VTABLE: STUDIO 0x00d19d84
	// SIZE 0x120
	// Members from the ctor (PLAYER 0x00464c30): recorded +0x4 (false), videoQuality +0x8 (-1), the
	// path buffer +0xc (strcpy of FileSystem::getUserDirectory(true, DirVideo, NULL)), capture
	// +0x10c, soundState +0x110, rbxView +0x114, frameRateManager +0x118, verb +0x11c.
	class VideoControl : public Ogre::IFrameDataCallback
	{
		bool recorded;                             // 0x004
		int videoQuality;                          // 0x008
		char path[0x100];                          // 0x00c
		boost::scoped_ptr<IVideoCapture> capture;  // 0x10c
		boost::scoped_ptr<SoundState> soundState;  // 0x110
		ViewBase* rbxView;                         // 0x114
		FrameRateManager* frameRateManager;        // 0x118
		Verb* verb;                                // 0x11c

	public:
		VideoControl(IVideoCapture* capture, ViewBase* rbxView, FrameRateManager* frameRateManager, Verb* verb);
		~VideoControl();                                // vtable+0x00
		/*override*/ void VTable0x04(void* p_unk);      // vtable+0x04
		/*override*/ void VTable0x08(void* p_unk);      // vtable+0x08
		/*override*/ void VTable0x0c();                 // vtable+0x0c
		/*override*/ void VTable0x10();                 // vtable+0x10

		bool isVideoRecording() { return capture->isRunning(); }
		std::string& getFileName() { return capture->getFileName(); }
		// Inlined into RecordToggleVerb::stopAction (PLAYER 0x00439450); 2013 does not clear the view's
		// frame-data callback here (2016 does).
		void stopRecording()
		{
			recorded = true;
			capture->stop();
			frameRateManager->ResumeAutoAdjustment();
		}

		// PLAYER 0x00464f30, not written (asserts 111/127; needs SoundService's 2013 callbacks and
		// two ViewBase virtuals). LTCG passes soundService in ecx and `this` in edi.
		void startRecording(Soundscape::SoundService* soundService);
		// 2013 only forwards (the ctor calls the capture slot directly, no store to videoQuality).
		void setVideoQuality(int quality) { capture->setVideoQuality(quality); }

		// SYNTHETIC: PLAYER 0x00464db0
		// SYNTHETIC: STUDIO 0x0048c920
		// RBX::VideoControl::`scalar deleting destructor'
	};

} // namespace RBX
