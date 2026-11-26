/* App/util/UserInputBase.cpp -- October 2013 (2013 __FILE__: Client\App\Util\UserInputBase.cpp).
   The code runs PLAYER 0x0072aaf0..0x0072aff2 (STUDIO 0x0082a3e0..). renderGameCursor and
   getGameCursor need Adorn's texture interface (Rendering) and are STUB bodies for now. */

#include "stdafx.h"

#include "Util/UserInputBase.h"
#include "decomp.h"

namespace RBX {

	// STUB: PLAYER 0x0072aaf0
	// STUB: STUDIO 0x0082a3e0
	void UserInputBase::renderGameCursor(Adorn* adorn)
	{
		STUB(0x0072aaf0);
	}

	// Asserts fallbackCursor at line 52 after loading Textures/ArrowCursor.png.
	// STUB: PLAYER 0x0072ad90
	// STUB: STUDIO 0x0082a680
	TextureProxyBaseRef UserInputBase::getGameCursor(Adorn* adorn)
	{
		STUB(0x0072ad90);
		return currentCursor;
	}

	// FUNCTION: PLAYER 0x0072af30
	// FUNCTION: STUDIO 0x0082a820
	bool UserInputBase::setCursorId(Adorn* adorn, const TextureId& id)
	{
		if (currentCursorId != id)
		{
			currentCursorId = id;
			currentCursor.reset();
			cursorIdChangedSignal();
			return true;
		}
		return false;
	}

	// FUNCTION: PLAYER 0x0072af80
	// FUNCTION: STUDIO 0x0082a870
	UserInputBase::UserInputBase()
	{
	}

} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::UserInputBase, 0x38);
