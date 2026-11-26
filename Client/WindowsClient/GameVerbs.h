/* WindowsClient/GameVerbs.h -- October 2013 client shell. The game window's verbs (RTTI
   RBX::LeaveGameVerb, RecordToggleVerb, ScreenshotVerb, ToggleFullscreenVerb; vftables PLAYER
   0x00be1bcc..0x00be1c2c, seven slots each: dump, dtor, isEnabled, isChecked, isSelected, getText,
   doIt). No 2013 __FILE__ string names this file (none of its code asserts), so the path is the
   2016 tree's (WindowsClient/GameVerbs.cpp).
   Written: LeaveGameVerb, ToggleFullscreenVerb, ScreenshotVerb's ctor, RecordToggleVerb's dtor, startAction, state
   queries and doIt (ScreenshotVerb::doIt and RecordToggleVerb::action: see GameVerbs.cpp). */

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
	class ViewBase;
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

	// VTABLE: PLAYER 0x00be1c0c
	// Ctor PLAYER 0x004382f0 (LTCG: `game` in ecx; `this`, doc and view on the stack, ret 0xc).
	// Members in 2016's order (screenshotFinished calls view's slot 0x70). Its scalar deleting
	// destructor is LeaveGameVerb's (0x004382b0).
	class ScreenshotVerb : public Verb
	{
		Game* game;          // 0x0c
		ViewBase* view;      // 0x10
		const Document& doc; // 0x14

		// Connected to DataModel::screenshotReadySignal by the ctor.
		void screenshotFinished(const std::string& filename); // PLAYER 0x004385c0

	public:
		ScreenshotVerb(const Document& doc, ViewBase* view, Game* game);
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

		// The helper thread's loop (the ctor starts it).
		void action();      // PLAYER 0x00438fa0

		// Run on the helper thread; doIt binds one of them into `job`.
		void startAction(); // PLAYER 0x00439370
		void stopAction();  // PLAYER 0x00439450

	public:
		~RecordToggleVerb();
		/*override*/ bool isEnabled() const;  // vtable+0x08
		/*override*/ bool isChecked() const;  // vtable+0x0c
		/*override*/ bool isSelected() const; // vtable+0x10
		/*override*/ void doIt(IDataState* dataState); // vtable+0x18

		// SYNTHETIC: PLAYER 0x00439210
		// RBX::RecordToggleVerb::`scalar deleting destructor'
	};

	// doIt's `job = bind(...)`, the shell's copy (it calls function0<void>::swap 0x0043b140, which
	// engine code calls too, so swap keeps __thiscall).
	// clang-format off
	// TEMPLATE: PLAYER 0x0043af20
	// ??$?4V?$bind_t@XV?$mf0@XVRecordToggleVerb@RBX@@@_mfi@boost@@V?$list1@V?$value@PAVRecordToggleVerb@RBX@@@_bi@boost@@@_bi@3@@_bi@boost@@@?$function@$$A6AXXZ@boost@@QAEAAV01@V?$bind_t@XV?$mf0@XVRecordToggleVerb@RBX@@@_mfi@boost@@V?$list1@V?$value@PAVRecordToggleVerb@RBX@@@_bi@boost@@@_bi@3@@_bi@1@@Z
	// clang-format on

} // namespace RBX
