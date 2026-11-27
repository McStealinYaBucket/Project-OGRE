/* WindowsClient/Document.h -- October 2013 client shell. The game document (no 2013 __FILE__ string
   names Document.cpp; path from the 2016 tree). Layout from ~Document (PLAYER 0x0041d020), which
   destroys +0x20, the four verbs, game and then deletes the View (~View 0x00465b90), and from the
   verb factory (0x0041dd30), which creates each verb into its slot:
     +0x0  scoped_ptr<View>   LeaveGameVerb::doIt (0x004382d0) reads its hWnd; Shutdown resets it.
                              2016's Document has no View member.
     +0x4  shared_ptr<Game>   the factory and Shutdown call Game::getDataModel (0x0041ce40) on it.
     +0xc..+0x18              ToggleFullscreenVerb (ctor 0x00439750), LeaveGameVerb (0x00438220),
                              RecordToggleVerb (0x00439000), ScreenshotVerb (0x004382f0).
     +0x1c FunctionMarshaller*  RecordToggleVerb::stopAction submits uploadVideo to it (0x004394c4);
                              Shutdown releases it (FunctionMarshaller::ReleaseWindow 0x00437180).
     +0x20 a shared_ptr nothing written reads yet.
   game and marshaller are 2016's names; the rest stay placeholders until the Document code is written. */

#pragma once

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "decomp.h"

namespace RBX {

	class View;
	class FunctionMarshaller;
	class Game;
	class LeaveGameVerb;
	class RecordToggleVerb;
	class ScreenshotVerb;
	class ToggleFullscreenVerb;

	class Document
	{
	public:
		~Document();     // PLAYER 0x0041d020 (LTCG: `this` pushed, ret 4)
		void Shutdown(); // PLAYER 0x0041d270 (Application::Shutdown calls it before deleting the document)

		boost::scoped_ptr<View> m_unk0x00;                    // 0x00
		boost::shared_ptr<Game> game;                         // 0x04
		boost::scoped_ptr<ToggleFullscreenVerb> m_unk0x0c;    // 0x0c
		boost::scoped_ptr<LeaveGameVerb> m_unk0x10;           // 0x10
		boost::scoped_ptr<RecordToggleVerb> m_unk0x14;        // 0x14
		boost::scoped_ptr<ScreenshotVerb> m_unk0x18;          // 0x18
		FunctionMarshaller* marshaller;                       // 0x1c
		boost::shared_ptr<void> m_unk0x20;                    // 0x20 type not proven (released, never read)
	};

} // namespace RBX
