/* WindowsClient/GameVerbs.h -- October 2013 client shell. The game window's verbs (RTTI
   RBX::LeaveGameVerb, RecordToggleVerb, ScreenshotVerb, ToggleFullscreenVerb; vftables PLAYER
   0x00be1bcc..0x00be1c2c, seven slots each: dump, dtor, isEnabled, isChecked, isSelected, getText,
   doIt). No 2013 __FILE__ string names this file (none of its code asserts), so the path is the
   2016 tree's (WindowsClient/GameVerbs.cpp).
   Written so far: LeaveGameVerb, ToggleFullscreenVerb, RecordToggleVerb's dtor and state queries. */

#pragma once

#include <string>

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>

#include "V8Tree/Verb.h"
#include "rbx/CEvent.h"

#include "decomp.h"

namespace RBX {

	class Document;
	class Game;
	class View;
	class VideoControl;

	// VTABLE: PLAYER 0x00be1bcc
	// SIZE 0x10
	// Ctor PLAYER 0x00438220 (`this` on the stack, ret 0xc). Against 2016 it holds the Document,
	// not the View, and doIt only closes the window (2013's MainLogManager has no game state).
	// No user-declared dtor: its scalar deleting destructor is the shared 0x004382b0.
	class LeaveGameVerb : public Verb
	{
		Document& doc; // 0x0c

	public:
		LeaveGameVerb(Document& doc, VerbContainer* container);
		/*override*/ void doIt(IDataState* dataState); // vtable+0x18
	};

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
	// SIZE 0x70
	// Layout from the ctor (PLAYER 0x00439000, not written: it needs Game::getDataModel, the
	// DSVideoCaptureEngine and VideoControl ctors and two ViewBase virtuals) and the dtor. Against
	// 2016: no View* member (the ctor's second argument is the gfx view, passed on to VideoControl),
	// and the dtor does not abort a running capture. `job` sits at +0x40: boost::function's buffer is
	// 8-aligned here (the dtor destroys the functor at +0x48).
	class RecordToggleVerb : public Verb
	{
		const Document& doc;                          // 0x0c
		Game* game;                                   // 0x10
		boost::scoped_ptr<VideoControl> videoControl; // 0x14
		std::string fileName;                         // 0x18
		bool videoUploadingEnabled;                   // 0x34
		bool stop;                                    // 0x35
		boost::scoped_ptr<boost::thread> helper;      // 0x38
		boost::function<void()> job;                  // 0x40
		CEvent jobWait;                               // 0x60
		CEvent jobDone;                               // 0x64
		CEvent threadDone;                            // 0x68

		bool isUploadingVideo() const { return videoUploadingEnabled; }

	public:
		~RecordToggleVerb();
		/*override*/ bool isEnabled() const;  // vtable+0x08
		/*override*/ bool isChecked() const;  // vtable+0x0c
		/*override*/ bool isSelected() const; // vtable+0x10

		// SYNTHETIC: PLAYER 0x00439210
		// RBX::RecordToggleVerb::`scalar deleting destructor'
	};

} // namespace RBX
