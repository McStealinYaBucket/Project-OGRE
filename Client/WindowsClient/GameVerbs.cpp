/* WindowsClient/GameVerbs.cpp -- October 2013 client shell (PLAYER compiles it; path from the 2016
   tree, see GameVerbs.h). The shell is /GL, so the verbs' ctors take `this` on the stack; the
   vftable slots keep __thiscall. */

#include "GameVerbs.h"

#include "View.h"
#include "../Win/VideoControl.h"
#include "V8DataModel/GameSettings.h"
#include "FastLog.h"

namespace RBX {

// STUB: PLAYER 0x00439330
bool RecordToggleVerb::isEnabled() const
{
	return GameSettings::singleton().videoCaptureEnabled && isUploadingVideo();
}

// isChecked and isSelected are one body in 2013 (both vftable slots point at it).
// FUNCTION: PLAYER 0x00439360
bool RecordToggleVerb::isChecked() const
{
	return videoControl->isVideoRecording();
}

bool RecordToggleVerb::isSelected() const
{
	return videoControl->isVideoRecording();
}

// FUNCTION: PLAYER 0x00439750
ToggleFullscreenVerb::ToggleFullscreenVerb(View& view, VerbContainer* container, VideoControl* videoControl)
	: Verb(container, "ToggleFullScreen")
	, videoControl(videoControl)
	, view(view)
{
}

// FUNCTION: PLAYER 0x004397e0
bool ToggleFullscreenVerb::isChecked() const
{
	return view.IsFullscreen();
}

// FUNCTION: PLAYER 0x004397f0
bool ToggleFullscreenVerb::isEnabled() const
{
	return !videoControl->isVideoRecording();
}

// STUB: PLAYER 0x00439810
void ToggleFullscreenVerb::doIt(IDataState* dataState)
{
	FASTLOG(FLog::Verbs, "Gui:ToggleFullscreen");

	view.SetFullscreen(!view.IsFullscreen());
}

} // namespace RBX
