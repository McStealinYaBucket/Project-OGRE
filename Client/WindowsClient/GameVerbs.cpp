/* WindowsClient/GameVerbs.cpp -- October 2013 client shell (PLAYER compiles it; path from the 2016
   tree, see GameVerbs.h). The shell is /GL, so the verbs' ctors take `this` on the stack; the
   vftable slots keep __thiscall. */

#include "GameVerbs.h"

#include "Document.h"
#include "View.h"
#include "../Win/VideoControl.h"
#include "../ClientBase/RenderSettingsItem.h"
#include "V8DataModel/GameSettings.h"
#include "FastLog.h"

namespace RBX {

// FUNCTION: PLAYER 0x00438220
LeaveGameVerb::LeaveGameVerb(Document& doc, VerbContainer* container)
	: Verb(container, "Exit")
	, doc(doc)
{
}

// FUNCTION: PLAYER 0x004382d0
void LeaveGameVerb::doIt(IDataState* dataState)
{
	doc.m_unk0x00->CloseWindow();
}

// Stops the helper thread (2016 also aborts a running capture first; 2013 does not).
// FUNCTION: PLAYER 0x00439230
RecordToggleVerb::~RecordToggleVerb()
{
	stop = true;
	jobWait.Set();
	threadDone.Wait();
}

// FUNCTION: PLAYER 0x00439330
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

// STUB: PLAYER 0x00439370
void RecordToggleVerb::startAction()
{
	STUB(0x00439370);
}

// STUB: PLAYER 0x00439450
void RecordToggleVerb::stopAction()
{
	STUB(0x00439450);
}

// Against 2016, recording starts only under the Direct3D or automatic graphics mode; otherwise the
// verb posts the window a WM_COMMAND (id 40003) and returns without waiting.
// STUB: PLAYER 0x004396c0
void RecordToggleVerb::doIt(IDataState* dataState)
{
	OutputDebugString("RecordToggleVerb::doIt");

	if (videoControl->isVideoRecording())
	{
		job = boost::bind(&RecordToggleVerb::stopAction, this);
	}
	else
	{
		CRenderSettings::GraphicsMode mode = CRenderSettingsItem::singleton().getGraphicsMode();
		if (mode != CRenderSettings::AutoGraphicsMode && mode != CRenderSettings::Direct3D9)
		{
			::PostMessage(doc.m_unk0x00->GetHWnd(), WM_COMMAND, 40003, 0);
			return;
		}
		job = boost::bind(&RecordToggleVerb::startAction, this);
	}
	jobWait.Set();
	jobDone.Wait();
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
