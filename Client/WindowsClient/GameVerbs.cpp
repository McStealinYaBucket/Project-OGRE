/* WindowsClient/GameVerbs.cpp -- October 2013 client shell (PLAYER compiles it; path from the 2016
   tree, see GameVerbs.h). The shell is /GL, so the verbs' ctors take `this` on the stack; the
   vftable slots keep __thiscall. */

#include "GameVerbs.h"

#include "Document.h"
#include "View.h"
#include "../Win/VideoControl.h"
#include "../ClientBase/RenderSettingsItem.h"
#include "V8DataModel/GameSettings.h"
#include "V8DataModel/Game.h"
#include "V8DataModel/DataModel.h"
#include "Util/SoundService.h"
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

// STUB: PLAYER 0x004382f0
ScreenshotVerb::ScreenshotVerb(const Document& doc, ViewBase* view, Game* game)
	: Verb(game->getDataModel().get(), "Screenshot")
	, game(game)
	, view(view)
	, doc(doc)
{
	boost::shared_ptr<DataModel> dataModel = game->getDataModel();
	dataModel->screenshotReadySignal.connect(boost::bind(&ScreenshotVerb::screenshotFinished, this, _1));
}

// 2013 body: if game->getDataModel() is set, dataModel->submitTask(boost::bind(
// &DataModel::TakeScreenshotTask, weak_ptr<DataModel>(dataModel)), DataModelJob::Write). Written out
// it scores 95.1 (pairing only), but the bind / function1<void, DataModel*> chain then moves to the
// shell's copies (0x0043a480, 0x0043a570, which engine code calls too, as 2013 has it) and six engine
// entries lose their pair (0x008445f0, 0x008447e0, 0x00844de0, 0x00844f60, 0x0073da60, 0x0073db90):
// their name rows say weak_ptr<DataModel>, but their only caller, DebrisService::addItem 0x00845000,
// binds weak_ptr<Instance>. Land it with those rows renamed (or TEMPLATE markers for the
// weak_ptr<Instance> chain in DebrisService.cpp). Wave 14 measured it again: 90.6 (pairing only),
// the six rows lost, net -3 on the link.
// STUB: PLAYER 0x00438460
void ScreenshotVerb::doIt(IDataState* dataState)
{
	STUB(0x00438460);
}

// Not written: it clears the view's screenshot request, shows "Screenshot saved" and offers the
// upload (GameSettings' post-image setting).
// STUB: PLAYER 0x004385c0
void ScreenshotVerb::screenshotFinished(const std::string& filename)
{
	STUB(0x004385c0);
}

// The helper thread: runs each job doIt hands it until the verb is destroyed. 2013 body:
// CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); while (!stop) { jobWait.Wait(); if (stop) break;
// job(); jobDone.Set(); } CoUninitialize(); threadDone.Set(). Written out, LTCG inlines job() (2013
// calls the shell's function0<void>::operator() 0x00434c20), so it scores 54.8, and the new shell copy
// of operator() takes TypedStatsItem<int>/<bool>::update's (0x0048e7a0, 0x0048e7d0) pairing.
// STUB: PLAYER 0x00438fa0
void RecordToggleVerb::action()
{
	STUB(0x00438fa0);
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

// 89.0: only the startRecording call differs (2013 passes soundService in ecx and the
// VideoControl in edi; startRecording is not written, so ours is an external __thiscall).
// STUB: PLAYER 0x00439370
void RecordToggleVerb::startAction()
{
	Soundscape::SoundService* soundService = ServiceProvider::create<Soundscape::SoundService>(game->getDataModel().get());

	videoControl->startRecording(soundService);

	fileName = videoControl->getFileName();

	GameSettings::singleton().videoRecordingSignal(true);
}

// STUB: PLAYER 0x00439450
void RecordToggleVerb::stopAction()
{
	STUB(0x00439450);
}

// Against 2016, recording starts only under the Direct3D or automatic graphics mode; otherwise the
// verb posts the window a WM_COMMAND (id 40003) and returns without waiting.
// FUNCTION: PLAYER 0x004396c0
void RecordToggleVerb::doIt(IDataState* dataState)
{
	OutputDebugString("RecordToggleVerb::doIt");

	if (videoControl->isVideoRecording())
	{
		job = boost::bind(&RecordToggleVerb::stopAction, this);
	}
	else
	{
		CRenderSettings::GraphicsMode mode = CRenderSettingsItem::singleton().graphicsMode;
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
