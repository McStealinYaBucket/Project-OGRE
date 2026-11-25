/* WindowsClient/Document.h -- October 2013 client shell. The game document (no 2013 __FILE__ string
   names Document.cpp; path from the 2016 tree). Only what the verbs read is declared:
     +0x0  View*            LeaveGameVerb::doIt (PLAYER 0x004382d0) reads its hWnd; the verb
                            factory (0x0041dd30) passes its gfx view (+0xa0) to RecordToggleVerb.
                            2016's Document has no View member.
     +0x4  Game's pointer   the factory calls Game::getDataModel (0x0041ce40) on it.
   Names stay placeholders until the Document code is written. */

#pragma once

#include "decomp.h"

namespace RBX {

	class View;

	class Document
	{
	public:
		View* m_unk0x00; // 0x00
	};

} // namespace RBX
