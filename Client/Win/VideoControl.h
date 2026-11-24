/* Win/VideoControl.h -- October 2013 client shell (Client\Win\VideoControl.cpp, asserts `verb` line 36,
   `rbxView` 39, `soundservice` 111, `captureStarted` 127).
   Only what the verbs read is declared so far: the capture object at +0x10c, whose isRunning is
   vftable slot 3 (RecordToggleVerb::isChecked, PLAYER 0x00439360, jumps through [vtbl+0xc]).
   2013's VideoControl is far larger than 2016's (0x10c bytes before the capture pointer); the rest of
   its layout is not proven yet. */

#pragma once

#include <string>

#include <boost/scoped_ptr.hpp>

#include "decomp.h"

namespace RBX {

	class Verb;

	// VTABLE: PLAYER 0x00be12b4
	// 2013 has 10 slots (dtor 0x00430a20, then nine pure); the first four follow the 2016 order,
	// which isRunning's slot confirms. The other six are not named yet.
	class IVideoCapture
	{
	public:
		virtual ~IVideoCapture() {}             // vtable+0x00
		virtual bool start(int cx, int cy, void* soundState) = 0; // vtable+0x04
		virtual bool stop() = 0;                 // vtable+0x08
		virtual bool isRunning() = 0;            // vtable+0x0c
	};

	class VideoControl
	{
		undefined m_unk0x00[0x10c];                 // 0x000
		boost::scoped_ptr<IVideoCapture> capture;  // 0x10c
	public:
		bool isVideoRecording() { return capture->isRunning(); }
	};

} // namespace RBX
