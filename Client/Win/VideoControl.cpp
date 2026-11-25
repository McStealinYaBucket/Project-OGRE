/* Win/VideoControl.cpp -- October 2013 client shell (PLAYER and STUDIO compile it; see
   VideoControl.h). The ctor carries the asserts on lines 36 and 39; startRecording (111, 127) is
   not written yet. The frame callbacks are in the 2013 .obj order (slot 2 first). */

#include "VideoControl.h"

#include <string.h>

#include "rbx/Debug.h"
#include "Util/FileSystem.h"
#include "V8DataModel/GameSettings.h"

namespace RBX {

// Against 2016, 2013 also copies the user video directory into the path buffer. The LTCG shell
// passes `this` on the stack and the verb in ecx; until RecordToggleVerb's ctor (its only caller)
// is written ours keeps __thiscall, so this stays a STUB.
// (Spacing: the asserts sit on 2013 lines 36 and 39.)












// FUNCTION: PLAYER 0x00464c30
// FUNCTION: STUDIO 0x0048c7a0
VideoControl::VideoControl(IVideoCapture* capture, ViewBase* rbxView, FrameRateManager* frameRateManager, Verb* verb)
{
	this->capture.reset(capture);
	RBXASSERT(verb);
	this->verb = verb;

	RBXASSERT(rbxView);
	this->rbxView = rbxView;
	this->frameRateManager = frameRateManager;

	path[0] = 0;
	strcpy(path, FileSystem::getUserDirectory(true, DirVideo, NULL).c_str());

	recorded = false;
	videoQuality = -1;
	setVideoQuality(GameSettings::singleton().getVideoQualitySetting());
}

// FUNCTION: PLAYER 0x00464dd0
// FUNCTION: STUDIO 0x0048c940
VideoControl::~VideoControl()
{
}

// FUNCTION: PLAYER 0x00464e50
// FUNCTION: STUDIO 0x0048c9c0
void VideoControl::VTable0x08(void* p_unk)
{
	if (capture->isRunning())
		capture->VTable0x18(p_unk, verb);
}

// FUNCTION: PLAYER 0x00464e90
// FUNCTION: STUDIO 0x0048ca00
void VideoControl::VTable0x04(void* p_unk)
{
	if (capture->isRunning())
		capture->VTable0x14(p_unk, verb);
}

// FUNCTION: PLAYER 0x00464ed0
// FUNCTION: STUDIO 0x0048ca40
void VideoControl::VTable0x0c()
{
	if (capture->isRunning())
		capture->VTable0x20();
}

// FUNCTION: PLAYER 0x00464f00
// FUNCTION: STUDIO 0x0048ca70
void VideoControl::VTable0x10()
{
	if (capture->isRunning())
		capture->VTable0x24();
}

} // namespace RBX
