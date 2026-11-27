/* V8DataModel/CommonVerbs.h -- October 2013, opaque stand-in (layout not written yet).
   Game::setupDataModel (PLAYER 0x00655630) news 0x724 bytes and calls CommonVerbs(DataModel*)
   (PLAYER 0x00864450): about 62 verb members, the first a Verb-derived object at +0 (vptr
   0x00b7d7c0), the next at +0x28. Its implicit destructor (PLAYER 0x006531e0, STUDIO 0x007cbbb0) is
   emitted by Game.obj and destroys them from +0x704 down. Until the members are written, the
   destructor is declared here and STUBbed in Game.cpp at that address. */

#pragma once

#include "decomp.h"

namespace RBX {

	class DataModel;

	// SIZE 0x724
	class CommonVerbs
	{
	public:
		CommonVerbs(DataModel* dataModel);
		~CommonVerbs();

	private:
		undefined m_unk0x00[0x724]; // 0x00
	};

} // namespace RBX
