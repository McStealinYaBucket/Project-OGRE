// Base/include/SelectState.h: October 2013 reconstruction (Project OGRE).
// Selection highlight state passed by value to IAdornable::render3dSelect (GfxBase, slot 10) and
// its PartInstance/ModelInstance overrides.
#pragma once

namespace RBX {

enum SelectState
{
	SELECT_NORMAL,
	SELECT_LIMIT,
	SELECT_HOVER
};

}
