/* WindowsClient/GameVerbs.h -- October 2013 client shell. The game window's verbs (RTTI
   RBX::LeaveGameVerb, RecordToggleVerb, ScreenshotVerb, ToggleFullscreenVerb; vftables PLAYER
   0x00be1bcc..0x00be1c2c, seven slots each: dump, dtor, isEnabled, isChecked, isSelected, getText,
   doIt). No 2013 __FILE__ string names this file (none of its code asserts), so the path is the
   2016 tree's (WindowsClient/GameVerbs.cpp).
   Only ToggleFullscreenVerb and RecordToggleVerb's state queries are written so far. */

#pragma once

#include "V8Tree/Verb.h"

#include "decomp.h"

namespace RBX {

	class View;
	class VideoControl;

	// VTABLE: PLAYER 0x00be1c2c
	// SIZE 0x14
	// Ctor PLAYER 0x00439750 (`this` on the stack, ret 0x10): Verb(container, "ToggleFullScreen"),
	// view +0x10, videoControl +0xc. Its scalar deleting destructor is LeaveGameVerb's (0x004382b0).
	class ToggleFullscreenVerb : public Verb
	{
		// Toggling is disabled while recording.
		VideoControl* videoControl; // 0x0c
		View& view;                 // 0x10

	public:
		ToggleFullscreenVerb(View& view, VerbContainer* container, VideoControl* videoControl);
		/*override*/ bool isChecked() const;      // vtable+0x0c
		/*override*/ bool isEnabled() const;      // vtable+0x08
		/*override*/ void doIt(IDataState* dataState); // vtable+0x18
	};

	// VTABLE: PLAYER 0x00be1bec
	// Members read by the queries below: videoControl +0x14, videoUploadingEnabled +0x34. The ctor,
	// dtor (0x00439230) and doIt (0x004396c0) are not written yet.
	class RecordToggleVerb : public Verb
	{
		undefined m_unk0x0c[0x14 - 0x0c]; // 0x0c
		VideoControl* videoControl;       // 0x14
		undefined m_unk0x18[0x34 - 0x18]; // 0x18
		bool videoUploadingEnabled;       // 0x34

		bool isUploadingVideo() const { return videoUploadingEnabled; }

	public:
		/*override*/ bool isEnabled() const;  // vtable+0x08
		/*override*/ bool isChecked() const;  // vtable+0x0c
		/*override*/ bool isSelected() const; // vtable+0x10
	};

} // namespace RBX
